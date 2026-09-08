#pragma once

#include<vector>
#include<string>
#include<fstream>
#include<Windows.h>

std::vector<uint8_t> read_bin_file(const std::string& filename);

void print_hex_dump(const void* data, size_t size);

void append_u32_le(std::vector<uint8_t>& vec, uint32_t val);
void append_u16_le(std::vector<uint8_t>& vec, uint16_t val);
void append_u64_le(std::vector<uint8_t>& vec, uint64_t val);

DWORD AlignUp(uint32_t size, uint32_t alignment);

DWORD RvaToOffset(const IMAGE_NT_HEADERS* const ntHeaders, DWORD rva);

DWORD OffsetToRva(const IMAGE_NT_HEADERS* const ntHeaders, DWORD fileOffset);

void append_imm_le(std::vector<uint8_t>& vec, uint64_t val, size_t size);

void append_imm_le_p(std::vector<uint8_t>& vec, const void* val, size_t size);