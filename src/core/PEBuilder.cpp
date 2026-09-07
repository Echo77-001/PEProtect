#include "PEBuilder.h"

#include<iostream>

PEBuilder::PEBuilder()
{
    
}

void PEBuilder::createHeaders()
{
    if (dosHeader) {
        // already configured
        std::cout << "dos header already configured\n";
        return;
    }
    dosHeader = new IMAGE_DOS_HEADER;

    dosHeader->e_magic = 0x5A4D; // 'MZ'
    // dosHeader.e_cblp = ...; // Bytes on last page of file
    dosHeader->e_cp; // count pages in file
    dosHeader->e_crlc = 0; // Relocations
    dosHeader->e_cparhdr = 4; // Size of header in paragraphs
    dosHeader->e_minalloc = 0; // Minimum extra paragraphs needed
    dosHeader->e_maxalloc = 0xFFFF; // Maximum extra paragraphs needed
    dosHeader->e_ss = 0; // Initial (relative) SS value
    // dosHeader.e_sp = ...; // Initial SP value
    dosHeader->e_csum = 0; // Checksum
    dosHeader->e_ip = 0; // Initial IP value
    dosHeader->e_cs = 0; // Initial (relative) CS value
    // dosHeader.e_lfarlc = ...; // File address of relocation table
    dosHeader->e_ovno = 0; // Overlay number
    //dosHeader.e_oemid = ...; // OEM identifier (for e_oeminfo)
    // dosHeader.e_oeminfo = ...; // OEM information; e_oemid specific
    // 
    dosHeader->e_lfanew = sizeof(IMAGE_DOS_HEADER); // NT заголовок идет сразу после DOS заголовка пропуская DOS заглушку

    ntheader = new IMAGE_NT_HEADERS64;

    ntheader->Signature = 0x4550; // PE\0\0
    IMAGE_FILE_HEADER& fileHeader = ntheader->FileHeader;
    fileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
    // fileHeader.NumberOfSections = ...;
    fileHeader.TimeDateStamp = time(0);
    fileHeader.NumberOfSymbols = 0x0;
    // fileHeader.SizeOfOptionalHeader = ...; 
    fileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE;

    IMAGE_OPTIONAL_HEADER64& optHeader = ntheader->OptionalHeader;
    optHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    optHeader.MajorLinkerVersion = 0x13;
    optHeader.MinorLinkerVersion = 0x37;
    // optHeader.SizeOfCode = ...; //
    // optHeader.SizeOfInitializedData = ...;
    // optHeader.SizeOfUninitializedData = ...;
    // optHeader.AddressOfEntryPoint = ..;
    // optHeader.BaseOfCode = ...;
    optHeader.ImageBase = 0x140000000;
    optHeader.SectionAlignment = 0x1000; // default value is 4kb
    // optHeader.FileAlignment = ...;
    optHeader.MajorSubsystemVersion = 6;
    optHeader.MinorSubsystemVersion = 0;
    // optHeader.SizeOfImage = ...;
    // optHeader.SizeOfHeaders = ...;
    optHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI; // IMAGE_SUBSYSTEM_WINDOWS_GUI / IMAGE_SUBSYSTEM_WINDOWS_CUI
    optHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | // ASLR support
        IMAGE_DLLCHARACTERISTICS_NX_COMPAT;

    optHeader.SizeOfStackReserve = 0x100000;
    optHeader.SizeOfStackCommit = 0x1000;
    optHeader.SizeOfHeapReserve = 0x100000;
    optHeader.SizeOfHeapCommit = 0x1000;

    optHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

    //ntheader = new IMAGE_NT_HEADERS64;
    this->optHeader = &ntheader->OptionalHeader;
    //optHeader
}

void PEBuilder::destroyHeaders()
{
    if (dosHeader) {
        delete dosHeader;
        dosHeader = nullptr;
    }
    if (ntheader) {
        delete ntheader;
        ntheader = nullptr;
    }
}

void PEBuilder::write(const std::string& fileName)
{
    std::ofstream exe(fileName, std::ios::binary);
    
    exe.write((char*)dosHeader, sizeof(*dosHeader));
    exe.write((char*)ntheader, sizeof(*ntheader));

    IMAGE_SECTION_HEADER* sections = new IMAGE_SECTION_HEADER[sectionList.size()];

    for (int i = 0;i < sectionList.size();i++) {
        memcpy(&sections[i], &sectionList[i].raw, sizeof(IMAGE_SECTION_HEADER));
    }

    exe.write((char*)sections, sectionList.size() * sizeof(IMAGE_SECTION_HEADER));
    
    size_t current_offset = exe.tellp();

    int padCount = optHeader->SizeOfHeaders - current_offset;

    std::vector<char> header_pad(padCount, 0);

    exe.write(header_pad.data(), header_pad.size());
    // std::cout << "### WRITING SECTIONS ###\n";
    for (size_t i = 0; i < sectionList.size(); i++) {
        SectionInfo& currentSection = sectionList[i];

        if (!currentSection.pointerToRawData) continue;

        // std::cout << "writing " << std::hex << sectionList[i].name << ' ' << sectionList[i].virtualAddres << " rsz: " << sectionList[i].sizeofRawData << " data sz: " << sectionList[i].data.size() << "\n";
        exe.seekp(currentSection.pointerToRawData);

        if (currentSection.data.size() >= currentSection.sizeofRawData) {
            // size in ram > real data on disk
            exe.write((char*)currentSection.data.data(), currentSection.sizeofRawData);
        }
        else {
            exe.write((char*)currentSection.data.data(), currentSection.data.size());

            size_t padSize = currentSection.sizeofRawData - currentSection.data.size();
            std::vector<char> pad(padSize, 0);
            exe.write(pad.data(), pad.size());
        }
    }

    exe.close();
    
    delete[] sections;
    std::cout << "[*] Done!\n";
}