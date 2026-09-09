//#define ZYDIS_STATIC_BUILD
#include<iostream>
#include<vector>
#include<fstream>
#include<string>
#include<sstream>

#include<Windows.h>
#include <winternl.h>

#include <cinttypes>
#include <list>
#include <functional>
#include <map>
#include<cmath>

#include"core/Utils.h"
#include"enums/Enums.h"
#include "core/PEParser.h"
#include"core/PEBuilder.h"
#include "vm/VMInstruction.h"
#include "vm/VMCodeBlock.h"

#include <zstd.h>

PEParser inputFile;
CodeBlock entryBlock;

bool noReloc = 0;
bool printVmData = 0;
bool showSections = 0;

// target rva, current rip, instruction size
// like 0xFF, 0x15 ? ? ? ? call [rip + ...]
DWORD calcRipOffset(DWORD targetRVA, DWORD currentRVA, DWORD instructionSize = 7) {
    DWORD result = 0;
    // rip + 7 + off  = target => off = target - 7 - rip
    return targetRVA - instructionSize - currentRVA;
}

std::vector<uint8_t> compress_data(const std::vector<uint8_t>& input_data, int compression_level = 3) {

    size_t const max_compressed_size = ZSTD_compressBound(input_data.size());
    std::vector<uint8_t> compressed_buffer(max_compressed_size);
    
    size_t const compressed_size = ZSTD_compress(
        compressed_buffer.data(), compressed_buffer.size(),
        input_data.data(), input_data.size(),
        compression_level
    );

    if (ZSTD_isError(compressed_size)) {
        throw std::runtime_error(std::string("ZSTD compression error: ") + ZSTD_getErrorName(compressed_size));
    }

    compressed_buffer.resize(compressed_size);
    return compressed_buffer;
}

std::vector<uint8_t> buildData() {
    std::vector<uint8_t> result;

    const int maxDataToUse = 4096;
    
    result.reserve(maxDataToUse);
    
    // some helper funcs
    std::vector<uint8_t> getPebShellCode{
        0x65, 0x48, 0x8B, 04, 0x25, 0x60, 00, 00, 00, //mov rax, gs: [60h]
        0xC3 // ret
    };

    result.insert(result.end(), getPebShellCode.begin(), getPebShellCode.end());

    int bytesUsed = 0;
    int bytesUsedOriginal = 0;
    std::vector<SectionInfo> sectionsToWrite;
    auto secList = inputFile.getSectionList();

    for (int i = 0; i < secList.size(); i++) {

        sectionsToWrite.push_back(secList[i]);

        bytesUsedOriginal += secList[i].sizeofRawData;
    }

    std::cout << "[*] Packing " << std::dec << sectionsToWrite.size() << " sections.\n";

    append_u32_le(result, inputFile.getSizeOfImage());
    append_u32_le(result, inputFile.getEntryPointRVA());
    
    append_u64_le(result, inputFile.getImageBase());
    
    result.push_back((uint8_t)sectionsToWrite.size());
    
    // section list
    for (int i = 0; i < sectionsToWrite.size(); i++) {
        /*
        section format:
        [u32 sizeofRawData, virtsize] (vm should alloc full page for each section)
        [u32 virt addr]
        [str original sec name]
        [u32 permision ]

        [n bytes of section] (sizeofRawData)
        */

        append_u32_le(result, sectionsToWrite[i].sizeofRawData);
        append_u32_le(result, sectionsToWrite[i].virtualSize);
        append_u32_le(result, sectionsToWrite[i].virtualAddres);

        result.insert(result.end(),&sectionsToWrite[i].name[0], &sectionsToWrite[i].name[8]);

        append_u32_le(result, sectionsToWrite[i].characteristics);

        result.insert(result.end(), sectionsToWrite[i].data.data(), sectionsToWrite[i].data.data() + sectionsToWrite[i].sizeofRawData);
    }

    auto origIAT = inputFile.getOriginalIAT();
    append_u32_le(result, origIAT.size());

    for (int i = 0; i < origIAT.size(); i++) {

        // len without \0

        result.insert(result.end(), origIAT[i].name.data(), origIAT[i].name.data() + origIAT[i].name.size());
        result.push_back(0);

        append_u32_le(result, origIAT[i].functionsToImport.size()); // func count to import

        for (int j = 0; j < origIAT[i].functionsToImport.size(); j++) {
            /*

            import func:
            [u32 id (RVA from original dll)]
            [str func name0]

            */

            append_u32_le(result, origIAT[i].functionsToImport[j].RealAddressRva);

            if (origIAT[i].functionsToImport[j].ordinal) {
                result.push_back(1);
                append_u16_le(result,origIAT[i].functionsToImport[j].ordValue);
                //result.insert(result.end(), origIAT[i].functionsToImport[j].name.data(), origIAT[i].functionsToImport[j].name.data() + origIAT[i].functionsToImport[j].name.size());
                //result.push_back(0);
            }
            else {
                result.push_back(0);
                
                result.insert(result.end(), origIAT[i].functionsToImport[j].name.data(), origIAT[i].functionsToImport[j].name.data() + origIAT[i].functionsToImport[j].name.size());
                result.push_back(0);
            }
        }

    }

    auto dataDirs = inputFile.getDataDirectory();
    
    for (int i = 0;i < 16;i++) {
        append_u32_le(result,dataDirs[i].rva);

        append_u32_le(result, dataDirs[i].raw.Size);
        append_u32_le(result, dataDirs[i].raw.VirtualAddress);
    }

    auto relocs = inputFile.getRelocs();
    if (!noReloc) {
        // append_u32_le(result, noReloc);
        append_u32_le(result, relocs.size());

        for (auto& i : relocs) {
            append_u32_le(result, i.type);
            append_u32_le(result, i.virtualAddress);
        }
    }
    else {
        std::cout << "[WARN] DELETING RELOCS!\n";
        append_u32_le(result, 0);
    }

    std::vector<uint8_t> magikSig = {
        0x0E, 0x0D, 0x11, 0x10, 0xDF, 0x0D, 0x04, 0xDF,
        0x0F, 0x1A, 0x1F, 0x14, 0x03, 0x12, 0x11, 0xDF,
        0x10, 0x0F, 0x11, 0x08, 0x17, 0x0D, 0x1F, 0x14,
        0xDF, 0x02, 0x0D, 0x11, 0xDF, 0xC0, 0xDF, 0x0A,
        0x18, 0xDF, 0x15, 0x0D, 0x11, 0xDF, 0x08, 0x17,
        0x0D, 0x1F, 0x1A, 0x0D, 0xDF, 0x02, 0x0D, 0x11,
        0xDF, 0x12, 0x11, 0xDF, 0x1D, 0x11, 0x0D, 0xDF,
        0x13, 0x11, 0x16, 0xDF, 0x1B, 0x0E, 0xC5, 0xDF,
        0x9A, 0x9C, 0x97, 0x90, 0xA0, 0xC8, 0xC8, 0xC8,
        0xC8, 0xC8, 0x13, 0x37, 0x42, 0x89, 0x54, 0x73
    };

    result.insert(result.end(), magikSig.begin(), magikSig.end());
    
    //print_hex_dump(result.data(), result.size());
    if (printVmData)print_hex_dump(result.data(), result.size());

    uint32_t origSz = result.size();
    std::vector<uint8_t> compressed = compress_data(result);
    uint32_t compSz = compressed.size();

    std::vector<uint8_t> final_buffer;
    append_u32_le(final_buffer, compSz);
    append_u32_le(final_buffer,origSz);
    final_buffer.insert(final_buffer.end(), compressed.begin(), compressed.end());

    if (final_buffer.size() < origSz) final_buffer.resize(origSz, 0x00);
    
    return final_buffer;
}

int buildEXE() {
    
    PEParser vmEngineParser;
    PEBuilder builder;

    builder.createHeaders();
    
    vmEngineParser.read("PEProtect_core.dll");
    
    uintptr_t crt_init_rva = vmEngineParser.getEntryPointRVA(); // CRTStartup

    std::vector<uint8_t> entry_stub;

    entry_stub.insert(entry_stub.end(), {
        0xE8,0,0,0,0, // call rip+??? init CRT
        0x48, 0x8d, 0xd, 0 ,0, 0, 0, // lea rcx, [rip + ???] ; loading vm ctxt
        0xE9,0,0,0,0 // jmp runVM
    });

    // writing vmdata immediately after trampoline
    std::vector<uint8_t> vmData = buildData();

    DWORD vmDataOffset = entry_stub.size();
    entry_stub.insert(entry_stub.end(), vmData.begin(), vmData.end());
   
    auto vmSection = vmEngineParser.addSection(IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE,
        entry_stub.size(),
        ".PACK0  ",
        entry_stub.data());
    
    DWORD realMain = vmEngineParser.getVaFromExportTable("runVM");
    std::cout << "[*] runVM va: 0x" <<std::hex <<  realMain << '\n';
    std::cout << "[*] CRT va: 0x" << crt_init_rva << '\n';

    // fixing rva
    *(DWORD*)&vmSection->data[1] = calcRipOffset(crt_init_rva, vmSection->virtualAddres, 5); // CRT startup
    *(DWORD*)&vmSection->data[8] = calcRipOffset(vmSection->virtualAddres + vmDataOffset, vmSection->virtualAddres + 5, 7); // inputParams
    *(DWORD*)&vmSection->data[13] = calcRipOffset(realMain, vmSection->virtualAddres + 7 + 5, 5); // runVM
    
    auto entrySection = vmEngineParser.getSectionFromRVA(crt_init_rva);

    auto originalSections = vmEngineParser.getSectionList();
    
    builder.setSectionsFromArray(originalSections);
    
    IMAGE_NT_HEADERS64* const ntheader = builder.getNtHeader();

    IMAGE_FILE_HEADER *fileHeader = &ntheader->FileHeader;

    IMAGE_OPTIONAL_HEADER64* optHeader = &ntheader->OptionalHeader;

    fileHeader->SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    
    uint32_t fileAlign = vmEngineParser.getNtHeader()->OptionalHeader.FileAlignment;
    uint32_t secAlign = vmEngineParser.getNtHeader()->OptionalHeader.SectionAlignment;
    
    optHeader->FileAlignment = fileAlign;
    optHeader->SectionAlignment = secAlign;

    // memcpy(optHeader->DataDirectory, vmEngineParser.getDataDirectory(), IMAGE_NUMBEROF_DIRECTORY_ENTRIES * sizeof(IMAGE_DATA_DIRECTORY));
    for (int i = 0;i < 16;i++) {
        memcpy(&optHeader->DataDirectory[i], &vmEngineParser.getDataDirectory()[i].raw, sizeof(IMAGE_DATA_DIRECTORY));
    }
    
    memset(&optHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], 0, sizeof(IMAGE_DATA_DIRECTORY)); // erasing exports
    
    optHeader->SizeOfCode = vmEngineParser.getSizeOfCode();
    
    optHeader->BaseOfCode = vmEngineParser.getBaseOfCode();
    optHeader->AddressOfEntryPoint = vmSection->virtualAddres;
    
    optHeader->SizeOfHeaders = vmEngineParser.getSizeOfHeaders();

    optHeader->SizeOfImage = vmEngineParser.getSizeOfImage();
    
    fileHeader->NumberOfSections = originalSections.size();
    
    builder.write("out.exe");

    builder.destroyHeaders();
    return 0;
}

int main(int argc, char** argv) {

    if (argc <= 1) {
        std::cout << R"(
Usage: PEProtect input.exe [options]

Options:
  -printData    Print data dump for debugging
  -noreloc      Remove relocation table
  -showSection  Prints info about all sections

Example:
  PEProtect target.exe -printData
)";
        return 1;
    }

    noReloc = 0;

    for (int i = 0;i < argc;i++) {
        if (!strcmp(argv[i], "-noreloc")) {
            noReloc = 1;
        }
        if (!strcmp(argv[i], "-printData")) {
            printVmData = 1;
        }
        if (!strcmp(argv[i], "-showSection")) {
            showSections = 1;
        }
    }

    if (argc > 1)inputFile.read(argv[1],showSections);
    
    buildEXE();

	return 0;
}