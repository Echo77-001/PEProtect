#pragma once
#include <cstdint>
#include <vector>
#include "../enums/Enums.h"
#include "../core/Utils.h"
#include <iostream>

#pragma pack(push, 1)
struct MovInstruction {
    uint64_t opCode : 6;
    uint64_t operandSize : 3; // 3 бита: Размер (1, 2, 4, 8, 16 байт) (log2(sizeOperandInBits/8))
    uint64_t operandType0 : 2; 
    uint64_t operandType1 : 2;
    uint64_t reserved : 3;

    uint64_t operand0 : 16;
};

struct alignas(1) CallInstruction {
    uint8_t opCode : 6;
    uint8_t operandType0 : 2;
    uint32_t operand0 : 32;
};

static_assert(sizeof(CallInstruction) == 5);

struct VMInstructionBase {
    // опкод весит 6 бит, если у инструкции нету параметров то размер окрулгяется до байта

    OpCode opCode;
    std::vector<uint8_t> params;

    virtual  std::vector<uint8_t> serializeInstruction() {

        return {};
    }

    virtual int calcInstructionSizeInBytes() const {
        return 0;
    }

};

struct VMInstructionMov : public VMInstructionBase {
   

    std::vector<uint8_t> serializeInstruction() override;

    int calcInstructionSizeInBytes() const override;

};

struct VMInstructionJmp : public VMInstructionBase {
   //VMOpCodeType type = unk;
   //int opCode = 0;

    std::vector<uint8_t> serializeInstruction() override;

    int calcInstructionSizeInBytes() const override;

};

struct VMInstructionCall : public VMInstructionBase {

   //VMOpCodeType type = unk;
   //int opCode = 0;

    std::vector<uint8_t> serializeInstruction() override;

    int calcInstructionSizeInBytes() const override;

};

const char* GetSubRegister(int regIndex, int sizeInBytes);

#pragma pack(pop)