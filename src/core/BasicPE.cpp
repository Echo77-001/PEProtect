#include "BasicPE.h"

SectionInfo* const BasicPE::getMaxSectionVA() const
{
    if (sectionList.empty()) return nullptr;

    auto it = std::max_element(sectionList.begin(), sectionList.end(),
        [](const SectionInfo& a, const SectionInfo& b) {
            return a.virtualAddres < b.virtualAddres;
        });

    return const_cast<SectionInfo*>(&*it);
}

SectionInfo* const BasicPE::getMaxSectionPA() const
{
    if (sectionList.empty()) return nullptr;

    auto it = std::max_element(sectionList.begin(), sectionList.end(),
        [](const SectionInfo& a, const SectionInfo& b) {
            return a.pointerToRawData < b.pointerToRawData;
        });

    return const_cast<SectionInfo*>(&*it);
}

std::vector<RelocationEntry> BasicPE::ParseRelocations() {
    std::vector<RelocationEntry> relocationList;

    IMAGE_DATA_DIRECTORY relocDir = ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (relocDir.Size == 0 || relocDir.VirtualAddress == 0) {
        return relocationList;
    }

    auto* currentBlock = (IMAGE_BASE_RELOCATION*)( this->originalBinFile.data() + RvaToOffset(ntheader,relocDir.VirtualAddress));

    uint8_t* endOfRelocDir = (uint8_t*)(currentBlock) + relocDir.Size;
    
    while ((uint8_t*)currentBlock < endOfRelocDir && currentBlock->SizeOfBlock > 0) {

        DWORD entriesCount = (currentBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

        WORD* relEntry = reinterpret_cast<WORD*>(reinterpret_cast<uint8_t*>(currentBlock) + sizeof(IMAGE_BASE_RELOCATION));

        for (DWORD i = 0; i < entriesCount; i++) {
            WORD offset = relEntry[i] & 0x0FFF;
            WORD type = relEntry[i] >> 12;

            if (type != IMAGE_REL_BASED_ABSOLUTE) {
                RelocationEntry entry;
                entry.virtualAddress = currentBlock->VirtualAddress + offset;
                entry.type = type;

                relocationList.push_back(entry);
            }
        }

        currentBlock = (IMAGE_BASE_RELOCATION*)(reinterpret_cast<uint8_t*>(currentBlock) + currentBlock->SizeOfBlock);
    }

    return relocationList;
}

std::vector<RelocationEntry> BasicPE::getRelocs()
{
    return this->relocs;
}

void BasicPE::setSectionsFromArray(const std::vector<SectionInfo>& ready)
{
    this->sectionList = ready;
}

SectionInfo* const BasicPE::getSectionFromName(const char* name) const
{

    auto it = std::find_if(sectionList.begin(), sectionList.end(),
        [name](const SectionInfo& section) {
            return std::strcmp(name, section.name) == 0;
        });

    return (it != sectionList.end()) ? const_cast<SectionInfo*>(&*it) : nullptr;
}

SectionInfo* BasicPE::addSection(DWORD characteristics, DWORD virtSize, const char name[8], void* data)
{
    IMAGE_SECTION_HEADER newHeader{ 0 };
    SectionInfo section;

    newHeader.Characteristics = characteristics;
    section.characteristics = characteristics;

    newHeader.Misc.VirtualSize = virtSize;
    section.virtualSize = virtSize;

    newHeader.SizeOfRawData = AlignUp(virtSize, ntheader->OptionalHeader.FileAlignment);
    section.sizeofRawData = newHeader.SizeOfRawData;

    memcpy(newHeader.Name, name, 8);
    memcpy(section.name, name, 8);

    auto maxSec = getMaxSectionVA();
    DWORD lastSecEnd = maxSec->virtualAddres + AlignUp(maxSec->virtualSize, ntheader->OptionalHeader.SectionAlignment);
    section.virtualAddres = AlignUp(lastSecEnd, ntheader->OptionalHeader.SectionAlignment);
    newHeader.VirtualAddress = section.virtualAddres;

    auto maxPaSec = getMaxSectionPA();
    newHeader.PointerToRawData = AlignUp(maxPaSec->raw.PointerToRawData + maxPaSec->raw.SizeOfRawData, ntheader->OptionalHeader.FileAlignment);
    section.pointerToRawData = newHeader.PointerToRawData;

    optHeader->SizeOfImage = AlignUp(newHeader.VirtualAddress + newHeader.Misc.VirtualSize, ntheader->OptionalHeader.SectionAlignment);

    DWORD ntHeadersOffset = dosHeader->e_lfanew;
    DWORD sizeOfNtHeaders = (ntheader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) ? sizeof(IMAGE_NT_HEADERS64) : sizeof(IMAGE_NT_HEADERS32);
    DWORD sizeOfAllSectionHeaders = (this->sectionList.size() + 1) * sizeof(IMAGE_SECTION_HEADER);
    DWORD totalHeadersSizeRequired = ntHeadersOffset + sizeOfNtHeaders + sizeOfAllSectionHeaders;

    if (totalHeadersSizeRequired > optHeader->SizeOfHeaders) {
        MessageBoxA(0, "Not enough space for new section header!", "Error", MB_ICONERROR);
        return nullptr;
    }

    section.raw = newHeader;
    section.data.resize(section.sizeofRawData, 0);
    if (data != nullptr && virtSize > 0) {
        memcpy(section.data.data(), data, virtSize);
    }

    IMAGE_SECTION_HEADER* secTable = IMAGE_FIRST_SECTION(ntheader);
    secTable[ntheader->FileHeader.NumberOfSections] = newHeader;
    ntheader->FileHeader.NumberOfSections++;

    sectionList.push_back(section);
    return &sectionList.back();
}


std::vector<SectionInfo>& const BasicPE::getSectionList() const
{
    return const_cast<std::vector<SectionInfo>&const>(sectionList);
}

IMAGE_DOS_HEADER* BasicPE::getDosHeader()
{
    return dosHeader;
}

IMAGE_NT_HEADERS64* BasicPE::getNtHeader() const
{
    return ntheader;
}


std::vector<uint8_t>& const BasicPE::getRawBytes() const {
    return const_cast<std::vector<uint8_t>&const>(this->originalBinFile);
}

DWORD BasicPE::getEntryPointRVA() const
{
    return this->entryPointRVA;
}

DWORD BasicPE::getFileAlignment() const
{
    return optHeader->FileAlignment;
}

DWORD BasicPE::getSecAlignment() const
{
    return optHeader->SectionAlignment;
}

UINT64 BasicPE::getImageBase() const
{
    return optHeader->ImageBase;
}

DataDirectoryInfo* const BasicPE::getDataDirectory() const
{
    return const_cast<DataDirectoryInfo* const>(DataDirectory);
}

DWORD BasicPE::getSizeOfImage() const {
    return optHeader->SizeOfImage;
}

DWORD BasicPE::getSizeOfHeaders() const {
    return optHeader->SizeOfHeaders;
}

DWORD BasicPE::getSizeOfCode() const {
    return optHeader->SizeOfCode;
}

DWORD BasicPE::getBaseOfCode() const
{
    return this->baseOfCode;
}