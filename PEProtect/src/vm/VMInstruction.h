#pragma once
#include <cstdint>
#include <vector>
#include "../enums/Enums.h"
#include "../core/Utils.h"
#include <iostream>

#pragma pack(push, 1)

struct alignas(1) VM_Memory {
    uint8_t  base;
    uint8_t  index;
    uint8_t  scale;
    uint32_t offset;
};

struct alignas(1) BasicInstruction {
    uint8_t opCode;
    uint8_t size;
    uint8_t type0;
    uint8_t type1;

    union {
        uint8_t   reg0;
        VM_Memory mem0;
    };

    union {
        uint8_t   reg1;
        VM_Memory mem1;
    };
};
#pragma pack(pop)

const char* GetSubRegister(int regIndex, int sizeInBytes);
