#include "VMInstruction.h"

const char* GetSubRegister(int regIndex, int sizeInBytes) {
    if (regIndex < 0 || regIndex > 15) return nullptr;

    static const char* const RegisterMatrix[16][4] = {
        { "al",   "ax",   "eax",  "rax" },
        { "bl",   "bx",   "ebx",  "rbx" },
        { "cl",   "cx",   "ecx",  "rcx" },
        { "dl",   "dx",   "edx",  "rdx" },
        { "sil",  "si",   "esi",  "rsi" },
        { "dil",  "di",   "edi",  "rdi" },
        { "bpl",  "bp",   "ebp",  "rbp" },
        { "spl",  "sp",   "esp",  "rsp" },
        { "r8b",  "r8w",  "r8d",  "r8"  },
        { "r9b",  "r9w",  "r9d",  "r9"  },
        { "r10b", "r10w", "r10d", "r10" },
        { "r11b", "r11w", "r11d", "r11" },
        { "r12b", "r12w", "r12d", "r12" },
        { "r13b", "r13w", "r13d", "r13" },
        { "r14b", "r14w", "r14d", "r14" },
        { "r15b", "r15w", "r15d", "r15" }
    };

    int sizeIndex = -1;
    switch (sizeInBytes) {
    case 1: sizeIndex = 0; break;
    case 2: sizeIndex = 1; break;
    case 4: sizeIndex = 2; break;
    case 8: sizeIndex = 3; break;
    default: return nullptr;
    }

    return RegisterMatrix[regIndex][sizeIndex];
}

std::vector<uint8_t>  VMInstructionMov::serializeInstruction(){
    std::vector<uint8_t> result;

    MovInstruction tmp;
    tmp.opCode = (uint64_t)OpCode::mov;
    tmp.operandSize = params[2];
    tmp.operandType0 = params[0];
    tmp.operandType1 = params[1];
    tmp.operand0 = *(uint8_t*)&params[3];
    result.resize(sizeof(MovInstruction));

    std::memcpy(result.data(), &tmp, sizeof(MovInstruction));

    append_imm_le_p(result, (uint64_t*)&params[4], std::pow(2, tmp.operandSize));

    return result;
}

int VMInstructionMov::calcInstructionSizeInBytes() const {

    int result = 8 + (int)pow(2, *(uint8_t*)&params[2]);

    return result;
}

std::vector<uint8_t> VMInstructionJmp::serializeInstruction() {
    std::vector<uint8_t> result;

    CallInstruction tmp;
    tmp.opCode = (uint64_t)OpCode::jmp;
    tmp.operand0 = *(uint32_t*)&params[1];
    tmp.operandType0 = params[0];

    result.resize(sizeof(CallInstruction));
    memcpy(result.data(), &tmp, sizeof(CallInstruction));


    return result;
}

int VMInstructionJmp::calcInstructionSizeInBytes() const{

    return 5;
}

std::vector<uint8_t> VMInstructionCall::serializeInstruction() {
    std::vector<uint8_t> result;

    CallInstruction tmp;
    tmp.opCode = (uint64_t)OpCode::call;
    tmp.operand0 = *(uint64_t*)&params[1];
    tmp.operandType0 = params[0];

    result.resize(sizeof(CallInstruction));
    memcpy(result.data(), &tmp, sizeof(CallInstruction));


    return result;
}

int VMInstructionCall::calcInstructionSizeInBytes() const {

    return 5;
}
