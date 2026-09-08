#include "PEParser.h"

const std::vector<DLLtoImport> PEParser::getOriginalIAT() const
{
    return originalIAT;
}

const std::vector<SectionInfo> PEParser::parseSectionList(bool logSection) const {

    std::vector<SectionInfo> result;
#if 1
    WORD sectionCount = ntheader->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* secArray = IMAGE_FIRST_SECTION(ntheader);
    if(logSection )std::cout << "[*] Parsed sections:\n";
    for (int i = 0; i < sectionCount; i++) {
        if(logSection)std::cout << "Name: " << secArray[i].Name << " Characterisics: " << std::hex << secArray[i].Characteristics << '\n';
        SectionInfo sec;
        sec.raw = secArray[i];
        sec.characteristics = secArray[i].Characteristics;
        sec.virtualSize = secArray[i].Misc.VirtualSize;
        sec.sizeofRawData = secArray[i].SizeOfRawData;
        sec.virtualAddres = secArray[i].VirtualAddress;
        sec.pointerToRawData = secArray[i].PointerToRawData;
        memcpy(sec.name, secArray[i].Name, 8);
        sec.data.resize(sec.virtualSize);
        memcpy(sec.data.data(), (char*)originalBinFile.data() + secArray[i].PointerToRawData,min( sec.sizeofRawData,sec.virtualSize));
        result.push_back(sec);
    }
#endif
    return result;
}

DWORD PEParser::getVaFromExportTable(const char* funcName)
{
    return this->exportFunctions[funcName];
}

SectionInfo& PEParser::getSectionFromRVA(DWORD RVA) {

    for (auto& i : sectionList) {
        if (i.virtualAddres <= RVA && RVA <= i.virtualSize + i.virtualAddres) {
            return i;
        }
    }
    throw std::exception("Cant find section from RVA!");
}

void PEParser::read(const std::string& path,bool logSection)
{
	originalBinFile = read_bin_file(path);

    dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(originalBinFile.data());

    if (!(originalBinFile.data()[0] == 0x4D && originalBinFile.data()[1] == 0x5A)) {
        std::cout << "[ERR] Invalid 'MZ' signature!\n";
        throw std::exception("Invalid 'MZ' signature!");
    }
    
    ntheader = (IMAGE_NT_HEADERS*)(originalBinFile.data() + dosHeader->e_lfanew);
    if (ntheader->Signature != 0x4550) { // PE\0\0 sig
        std::cout << "[ERR] Invalid 'PE\0\0' signature: " << std::hex << ntheader->Signature << '\n';
        throw std::exception("Invalid 'MZ' signature!");
    }
    optHeader = &ntheader->OptionalHeader;

    if (optHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        std::cout << "[ERR] x64 support only!\n";
        throw std::exception("x64 support only!");
    }
    
    for (int i = 0;i < 16;i++) {
        memcpy(&DataDirectory[i].raw, &optHeader->DataDirectory[i], sizeof(IMAGE_DATA_DIRECTORY));
    }
    
    // memcpy(DataDirectory, optHeader->DataDirectory, sizeof(IMAGE_DATA_DIRECTORY)*16);

    WORD subSys = optHeader->Subsystem;

    if (!
        (subSys == IMAGE_SUBSYSTEM_WINDOWS_GUI || 
            subSys == IMAGE_SUBSYSTEM_WINDOWS_CUI)
        ) {
        std::cout << "[ERR] IMAGE_SUBSYSTEM_WINDOWS_GUI and IMAGE_SUBSYSTEM_WINDOWS_CUI only!\n";
        throw std::exception("x64 support only!");
    }
    
    imgBase = optHeader->ImageBase;

    IMAGE_DATA_DIRECTORY* importDirectory = &optHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_DATA_DIRECTORY* exportDirectory = &optHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    DWORD importOffset = RvaToOffset(ntheader, importDirectory->VirtualAddress);

    DWORD exportOffset = RvaToOffset(ntheader,exportDirectory->VirtualAddress);

    DWORD codeBaseOff = RvaToOffset(ntheader, optHeader->BaseOfCode);

    entryPointRVA = optHeader->AddressOfEntryPoint;
    
    baseOfCode = optHeader->BaseOfCode;
    sectionList = parseSectionList(logSection);

    IMAGE_IMPORT_DESCRIPTOR* importDesc = (IMAGE_IMPORT_DESCRIPTOR*)(originalBinFile.data() + importOffset);
    IMAGE_EXPORT_DIRECTORY* tableExport = (IMAGE_EXPORT_DIRECTORY*)(exportOffset + originalBinFile.data());
    
    DWORD namesOffset = RvaToOffset(ntheader, tableExport->AddressOfNames);
    DWORD ordinalsOffset = RvaToOffset(ntheader, tableExport->AddressOfNameOrdinals);
    DWORD functionsOffset = RvaToOffset(ntheader, tableExport->AddressOfFunctions);

    const DWORD* nameRvaTable = reinterpret_cast<const DWORD*>(originalBinFile.data() + namesOffset);
    const WORD* ordinalTable = reinterpret_cast<const WORD*>(originalBinFile.data() + ordinalsOffset);
    const DWORD* functionTable = reinterpret_cast<const DWORD*>(originalBinFile.data() + functionsOffset);
    
    for (DWORD i = 0; i < tableExport->NumberOfNames; i++) {
        DWORD nameRva = nameRvaTable[i];
        if (nameRva == 0) continue;

        DWORD nameOffset = RvaToOffset(ntheader, nameRva);
        const char* funcName = (char*)(originalBinFile.data() + nameOffset);
        
        WORD ordinalIdx = ordinalTable[i];

        if (ordinalIdx >= tableExport->NumberOfFunctions) continue;

        DWORD funcRva = functionTable[ordinalIdx];

        //std::cout << i << " exported func is - " << funcName << '\n';
        exportFunctions[funcName] = funcRva;
    }

    this->relocs = this->ParseRelocations();
    while (importDesc->Name) {
        DWORD nameRVA = RvaToOffset(ntheader, importDesc->Name);

        if (nameRVA) {
            const char* nm = (const char*)originalBinFile.data() + nameRVA;

            originalIAT.push_back({});
            DLLtoImport& currentElement = originalIAT.back();
            currentElement.name = nm;

            DWORD thunkRVA = importDesc->OriginalFirstThunk ? importDesc->OriginalFirstThunk : importDesc->FirstThunk;
            DWORD thunkOffset = RvaToOffset(ntheader, thunkRVA);

            if (!thunkOffset) {
                importDesc++;
                continue;
            }

            IMAGE_THUNK_DATA64* thunkData = (IMAGE_THUNK_DATA64*)(originalBinFile.data() + thunkOffset);

            DWORD_PTR currentIatRva = importDesc->FirstThunk;

            while (thunkData->u1.AddressOfData) {
                ULONGLONG rawData = thunkData->u1.AddressOfData;

                if (rawData & IMAGE_ORDINAL_FLAG64) {

                    UINT16 ordinal = (rawData & 0xFFFF);

                    currentElement.functionsToImport.push_back(
                        ImportFunctionInfo("none", currentIatRva, ordinal, 1)
                    );

                }
                else {
                    DWORD nameOffset = RvaToOffset(ntheader, (DWORD)rawData);
                    IMAGE_IMPORT_BY_NAME* fnName = (IMAGE_IMPORT_BY_NAME*)((char*)originalBinFile.data() + nameOffset);

                    currentElement.functionsToImport.push_back(
                        ImportFunctionInfo(fnName->Name, currentIatRva, 0, false)
                    );
                }

                thunkData++;
                currentIatRva += sizeof(ULONGLONG);
            }
        }

        importDesc++;
    }
}


ImportFunctionInfo PEParser::getImportFuncFromRVA(DWORD RVA) {
    for (auto& import : originalIAT) {
        auto it = std::find_if(import.functionsToImport.begin(), import.functionsToImport.end(),
            [RVA](auto& func) { return func.RealAddressRva == RVA; });

        if (it != import.functionsToImport.end()) {
            return *it;
        }
    }
    return {};
}
