#pragma once

#include<Windows.h>
#include <vector>
#include <string>
#include<map>

#include"Utils.h"

struct SectionInfo {
    IMAGE_SECTION_HEADER raw;
    DWORD virtualSize = 0;
    DWORD sizeofRawData = 0;
    DWORD characteristics = 0;
    DWORD virtualAddres = 0;
    DWORD pointerToRawData = 0;
    char name[8] = { 0 };
    std::vector<uint8_t> data;

    SectionInfo() = default;
};

struct RelocationEntry {
    ULONG_PTR virtualAddress;
    WORD type;
};

struct ImportFunctionInfo {
    ImportFunctionInfo() = default;
    std::string name;
    DWORD RealAddressRva;
    bool ordinal;
    UINT16 ordValue;
    
    ImportFunctionInfo(const std::string& n, DWORD RealAddrRva,UINT16 ord,bool ordinal) {
        name = n;
        RealAddressRva = RealAddrRva;
        this->ordinal = ordinal;
        ordValue = ord;
    }
};

struct DLLtoImport {
    std::string name;
    std::vector< ImportFunctionInfo> functionsToImport;
};

struct DataDirectoryInfo {
    IMAGE_DATA_DIRECTORY raw;
    UINT32 rva;
};

struct CodeBlockToVirt {
    DWORD rva, size;
};



class BasicPE {
protected:

    std::vector<RelocationEntry> ParseRelocations();

    std::vector<uint8_t> originalBinFile;
    
    IMAGE_OPTIONAL_HEADER* optHeader = nullptr;
    IMAGE_NT_HEADERS64* ntheader = nullptr;
    IMAGE_DOS_HEADER* dosHeader = nullptr;
    
    DWORD entryPointRVA = 0;
    
    uintptr_t imgBase = 0x0, baseOfCode = 0;

    std::map<std::string, DWORD> exportFunctions;

    std::vector<SectionInfo> sectionList;

    std::vector<RelocationEntry> relocs;

    std::vector<CodeBlockToVirt> blocksToVirt;
    
    //DWORD SizeOfImage, SizeOfHeaders, SizeOfCode;
    
    DataDirectoryInfo DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];

public:

    std::vector<DWORD> PatternScan(const char* patter);
    
    std::vector<RelocationEntry> getRelocs();

    void setSectionsFromArray(const std::vector<SectionInfo>& ready);

    SectionInfo* const getSectionFromName(const char* name) const;

    DataDirectoryInfo* const getDataDirectory() const;

    SectionInfo* addSection(DWORD characteristics, DWORD virtSize, const char name[8], void* data);

    DWORD getSizeOfImage() const;

    DWORD getSizeOfHeaders() const;

    DWORD getSizeOfCode() const;

    DWORD getBaseOfCode() const;

    DWORD getEntryPointRVA() const;

    DWORD getFileAlignment() const;

    DWORD getSecAlignment() const;

    UINT64 getImageBase() const;

    IMAGE_DOS_HEADER* getDosHeader();
    IMAGE_NT_HEADERS64* getNtHeader() const;
    
    std::vector<uint8_t>& const getRawBytes() const;

    std::vector<SectionInfo>& const getSectionList() const;

    SectionInfo* const getMaxSectionVA() const;
    SectionInfo* const getMaxSectionPA() const;

};