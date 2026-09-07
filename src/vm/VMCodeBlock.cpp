#include"VMCodeBlock.h"

int CodeBlock::calcInBytesInstructions() {
    int result = 0;

    for (auto& i : instructions) result += i->calcInstructionSizeInBytes();
    return result;
}

