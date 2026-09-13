#pragma once
#include "VMInstruction.h"
#include"../core/PEParser.h"

#include <vector>
#include <Zydis.h>

struct CodeBlock {
    std::vector<uint8_t> instructions;
    DWORD id = 0; // original rva
    DWORD originalSize = 0;
    
    void parseOperands(const ZydisDisassembledInstruction& instruction, BasicInstruction& inst, std::vector<uint8_t>& additionalBuffer);

    void generateFromInstructions(PEParser& file, int sectionIndex, DWORD startOff, DWORD end);

};