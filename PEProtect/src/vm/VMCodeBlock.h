#pragma once
#include "VMInstruction.h"
#include <vector>

struct CodeBlock {
    std::vector<VMInstructionBase*> instructions;
    int id = 0;
    int calcInBytesInstructions();

};