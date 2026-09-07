#include"Utils.h"
#include <Windows.h>

std::vector<uint8_t> read_bin_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        MessageBoxA(0, "cant open file!", 0, 0);
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);

    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }

    return {};
}
void print_hex_dump(const void* data, size_t size) {
    const unsigned char* p = (const unsigned char*)data;
    std::string output;
    output.reserve(size * 4);

    output += " Offset    00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  | ASCII          |\n";
    output += "------------------------------------------------------------------------------\n";

    char line[128];
    for (size_t i = 0; i < size; i += 16) {
        int pos = 0;
        pos += snprintf(line + pos, sizeof(line) - pos, " %08X  ", (unsigned int)i);

        for (int j = 0; j < 16; j++) {
            if (i + j < size) {
                pos += snprintf(line + pos, sizeof(line) - pos, "%02X ", p[i + j]);
            }
            else {
                pos += snprintf(line + pos, sizeof(line) - pos, "   ");
            }
            if (j == 7) pos += snprintf(line + pos, sizeof(line) - pos, " ");
        }

        pos += snprintf(line + pos, sizeof(line) - pos, " | ");

        for (int j = 0; j < 16; j++) {
            if (i + j < size) {
                unsigned char ch = p[i + j];
                line[pos++] = (ch >= 32 && ch <= 126) ? ch : '.';
            }
            else {
                line[pos++] = ' ';
            }
        }

        line[pos++] = ' ';
        line[pos++] = '|';
        line[pos++] = '\n';
        line[pos++] = '\0';

        output += line;

        if (output.size() > 8000) {
            printf("%s", output.c_str());
            output.clear();
        }
    }

    if (!output.empty()) {
        printf("%s", output.c_str());
    }
}

// LE
void append_u32_le(std::vector<uint8_t>& vec, uint32_t val) {
    vec.push_back(static_cast<uint8_t>(val & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
}

void append_u64_le(std::vector<uint8_t>& vec, uint64_t val) {
    vec.push_back(static_cast<uint8_t>(val & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 32) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 40) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 48) & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 56) & 0xFF));
}

void append_u16_le(std::vector<uint8_t>& vec, uint16_t val) {
    vec.push_back(static_cast<uint8_t>(val & 0xFF));
    vec.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
}

DWORD AlignUp(uint32_t size, uint32_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

DWORD RvaToOffset(const IMAGE_NT_HEADERS* const ntHeaders, DWORD rva) {
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        if (rva >= sectionHeader[i].VirtualAddress &&
            rva < sectionHeader[i].VirtualAddress + sectionHeader[i].Misc.VirtualSize)
        {
            return rva - sectionHeader[i].VirtualAddress + sectionHeader[i].PointerToRawData;
        }
    }
    return 0;
}

DWORD OffsetToRva(const IMAGE_NT_HEADERS* const ntHeaders, DWORD fileOffset) {
    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        if (fileOffset >= sectionHeader[i].PointerToRawData &&
            fileOffset < (sectionHeader[i].PointerToRawData + sectionHeader[i].SizeOfRawData)) {

            return fileOffset - sectionHeader[i].PointerToRawData + sectionHeader[i].VirtualAddress;
        }
    }
    return 0;
}

void append_imm_le(std::vector<uint8_t>& vec, uint64_t val, size_t size) {
    vec.reserve(vec.size() + size);

    for (size_t i = 0; i < size; ++i) {
        vec.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
    }
}

void append_imm_le_p(std::vector<uint8_t>& vec, const void* val, size_t size) {
    if (!val || size == 0) return;

    size_t old_size = vec.size();
    vec.resize(old_size + size);

    std::memcpy(vec.data() + old_size, val, size);
}