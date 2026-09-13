#include"VMCodeBlock.h"
#include<map>
#include <unordered_map>

Register64 ZydisRegisterToVM(const ZydisRegister& reg) {
    static const std::unordered_map<ZydisRegister, Register64> regMap = {
        {ZYDIS_REGISTER_RAX, Register64::RAX}, {ZYDIS_REGISTER_EAX, Register64::RAX},
        {ZYDIS_REGISTER_AX,  Register64::RAX}, {ZYDIS_REGISTER_AL,  Register64::RAX}, 
        {ZYDIS_REGISTER_AH, Register64::RAX},

        {ZYDIS_REGISTER_RBX, Register64::RBX}, {ZYDIS_REGISTER_EBX, Register64::RBX},
        {ZYDIS_REGISTER_BX,  Register64::RBX}, {ZYDIS_REGISTER_BL,  Register64::RBX}, 
        {ZYDIS_REGISTER_BH, Register64::RBX},

        {ZYDIS_REGISTER_RCX, Register64::RCX}, {ZYDIS_REGISTER_ECX, Register64::RCX},
        {ZYDIS_REGISTER_CX,  Register64::RCX}, {ZYDIS_REGISTER_CL,  Register64::RCX}, 
        {ZYDIS_REGISTER_CH, Register64::RCX},

        {ZYDIS_REGISTER_RDX, Register64::RDX}, {ZYDIS_REGISTER_EDX, Register64::RDX},
        {ZYDIS_REGISTER_DX,  Register64::RDX}, {ZYDIS_REGISTER_DL,  Register64::RDX}, 
        {ZYDIS_REGISTER_DH, Register64::RDX},

        {ZYDIS_REGISTER_RSI, Register64::RSI}, {ZYDIS_REGISTER_ESI, Register64::RSI},
        {ZYDIS_REGISTER_SI,  Register64::RSI}, {ZYDIS_REGISTER_SIL, Register64::RSI},

        {ZYDIS_REGISTER_RDI, Register64::RDI}, {ZYDIS_REGISTER_EDI, Register64::RDI},
        {ZYDIS_REGISTER_DI,  Register64::RDI}, {ZYDIS_REGISTER_DIL, Register64::RDI},

        {ZYDIS_REGISTER_RBP, Register64::RBP}, {ZYDIS_REGISTER_EBP, Register64::RBP},
        {ZYDIS_REGISTER_BP,  Register64::RBP}, {ZYDIS_REGISTER_BPL, Register64::RBP},

        {ZYDIS_REGISTER_RSP, Register64::RSP}, {ZYDIS_REGISTER_ESP, Register64::RSP},
        {ZYDIS_REGISTER_SP,  Register64::RSP}, {ZYDIS_REGISTER_SPL, Register64::RSP},

        {ZYDIS_REGISTER_R8,   Register64::R8},  {ZYDIS_REGISTER_R8D,  Register64::R8},  {ZYDIS_REGISTER_R8W,  Register64::R8},  {ZYDIS_REGISTER_R8B,  Register64::R8},
        {ZYDIS_REGISTER_R9,   Register64::R9},  {ZYDIS_REGISTER_R9D,  Register64::R9},  {ZYDIS_REGISTER_R9W,  Register64::R9},  {ZYDIS_REGISTER_R9B,  Register64::R9},
        {ZYDIS_REGISTER_R10,  Register64::R10}, {ZYDIS_REGISTER_R10D, Register64::R10}, {ZYDIS_REGISTER_R10W, Register64::R10}, {ZYDIS_REGISTER_R10B, Register64::R10},
        {ZYDIS_REGISTER_R11,  Register64::R11}, {ZYDIS_REGISTER_R11D, Register64::R11}, {ZYDIS_REGISTER_R11W, Register64::R11}, {ZYDIS_REGISTER_R11B, Register64::R11},
        {ZYDIS_REGISTER_R12,  Register64::R12}, {ZYDIS_REGISTER_R12D, Register64::R12}, {ZYDIS_REGISTER_R12W, Register64::R12}, {ZYDIS_REGISTER_R12B, Register64::R12},
        {ZYDIS_REGISTER_R13,  Register64::R13}, {ZYDIS_REGISTER_R13D, Register64::R13}, {ZYDIS_REGISTER_R13W, Register64::R13}, {ZYDIS_REGISTER_R13B, Register64::R13},
        {ZYDIS_REGISTER_R14,  Register64::R14}, {ZYDIS_REGISTER_R14D, Register64::R14}, {ZYDIS_REGISTER_R14W, Register64::R14}, {ZYDIS_REGISTER_R14B, Register64::R14},
        {ZYDIS_REGISTER_R15,  Register64::R15}, {ZYDIS_REGISTER_R15D, Register64::R15}, {ZYDIS_REGISTER_R15W, Register64::R15}, {ZYDIS_REGISTER_R15B, Register64::R15}
    };

    auto it = regMap.find(reg);
    return (it != regMap.end()) ? it->second : Register64::UNK;
}

bool mapMnemonicToOpcode(ZydisMnemonic mnemonic, OpCode& out) {
    static const std::unordered_map<ZydisMnemonic, OpCode> opcodeMap = {
        {ZYDIS_MNEMONIC_MOV,    {OpCode::mov}},
        {ZYDIS_MNEMONIC_MOVSX,  {OpCode::movsx}},
        {ZYDIS_MNEMONIC_MOVSXD, {OpCode::movsxd}},
        {ZYDIS_MNEMONIC_MOVZX,  {OpCode::movzx}},
        {ZYDIS_MNEMONIC_MOVDQA, {OpCode::movdqa}},
        {ZYDIS_MNEMONIC_MOVDQU, {OpCode::movdqu}},
        {ZYDIS_MNEMONIC_LEA,    {OpCode::lea}},
        {ZYDIS_MNEMONIC_ADD,    {OpCode::add}},
        {ZYDIS_MNEMONIC_SUB,    {OpCode::sub}},
        {ZYDIS_MNEMONIC_SBB,    {OpCode::sbb}},
        {ZYDIS_MNEMONIC_INC,    {OpCode::inc}},
        {ZYDIS_MNEMONIC_DEC,    {OpCode::dec}},
        {ZYDIS_MNEMONIC_IMUL,   {OpCode::imul}},
        {ZYDIS_MNEMONIC_NEG,    {OpCode::neg}},
        {ZYDIS_MNEMONIC_AND,    {OpCode::and_}},
        {ZYDIS_MNEMONIC_OR,     {OpCode::or_}},
        {ZYDIS_MNEMONIC_XOR,    {OpCode::xor_}},
        {ZYDIS_MNEMONIC_NOT,    {OpCode::not_}},
        {ZYDIS_MNEMONIC_SHL,    {OpCode::shl}},
        {ZYDIS_MNEMONIC_SHR,    {OpCode::shr}},
        {ZYDIS_MNEMONIC_SAR,    {OpCode::sar}},
        {ZYDIS_MNEMONIC_ROL,    {OpCode::rol}},
        {ZYDIS_MNEMONIC_ROR,    {OpCode::ror}},
        {ZYDIS_MNEMONIC_CMP,    {OpCode::cmp}},
        {ZYDIS_MNEMONIC_TEST,   {OpCode::test}},
        {ZYDIS_MNEMONIC_PUSH,   {OpCode::push}},
        {ZYDIS_MNEMONIC_POP,    {OpCode::pop}},
        {ZYDIS_MNEMONIC_XCHG,   {OpCode::xchg}},
        {ZYDIS_MNEMONIC_NOP,    {OpCode::nop}},

        {ZYDIS_MNEMONIC_JMP,    {OpCode::jmp}},
        {ZYDIS_MNEMONIC_CALL,   {OpCode::call}},
        {ZYDIS_MNEMONIC_RET,    {OpCode::ret}},
        {ZYDIS_MNEMONIC_JZ,     {OpCode::jz}},
        {ZYDIS_MNEMONIC_JNZ,    {OpCode::jnz}},
        {ZYDIS_MNEMONIC_JS,     {OpCode::js}},
        {ZYDIS_MNEMONIC_JNS,    {OpCode::jns}},
        {ZYDIS_MNEMONIC_JL,     {OpCode::jl}},
        {ZYDIS_MNEMONIC_JNL,    {OpCode::jnl}},
        {ZYDIS_MNEMONIC_JLE,    {OpCode::jle}},
        {ZYDIS_MNEMONIC_JB,     {OpCode::jb}},
        {ZYDIS_MNEMONIC_JNB,    {OpCode::jnb}},
        {ZYDIS_MNEMONIC_JBE,    {OpCode::jbe}},
        {ZYDIS_MNEMONIC_JNBE,   {OpCode::jnbe}},

        {ZYDIS_MNEMONIC_BT,     {OpCode::bt}},
        {ZYDIS_MNEMONIC_CMPXCHG, {OpCode::cmpxchg}},
        {ZYDIS_MNEMONIC_CMOVZ,  {OpCode::cmovz}},
        {ZYDIS_MNEMONIC_CMOVNZ, {OpCode::cmovnz}},
        {ZYDIS_MNEMONIC_SETZ,   {OpCode::setz}},
        {ZYDIS_MNEMONIC_SETNZ,  {OpCode::setnz}},
        {ZYDIS_MNEMONIC_SETNBE, {OpCode::setnbe}},
        {ZYDIS_MNEMONIC_PUSHFQ, {OpCode::pushfq}},
        {ZYDIS_MNEMONIC_CPUID,  {OpCode::cpuid}},
        {ZYDIS_MNEMONIC_XGETBV, {OpCode::xgetbv}},
        {ZYDIS_MNEMONIC_INT,    {OpCode::int_}},
        {ZYDIS_MNEMONIC_INT3,   {OpCode::int3}},
        {ZYDIS_MNEMONIC_OUTSB,  {OpCode::outsb}},
        {ZYDIS_MNEMONIC_SETBE,  {OpCode::setbe}},
    };

    auto it = opcodeMap.find(mnemonic);
    if (it == opcodeMap.end()) {
        const char* name = ZydisMnemonicGetString(static_cast<ZydisMnemonic>(mnemonic));
        std::cerr << "UNKNOWN mnemonic: 0x" << std::hex << mnemonic;
        if (name) std::cerr << " (" << name << ")";
        std::cerr << std::dec << std::endl;
        return false;
    }

    out = it->second;
    return true;
}

void CodeBlock::parseOperands(const ZydisDisassembledInstruction& instruction,BasicInstruction& inst, std::vector<uint8_t>& additionalBuffer)
{
    auto& op0 = instruction.operands[0];
    auto& op1 = instruction.operands[1];

    inst.size = log2(op0.size / 8);

    switch (op0.type) {
    case ZYDIS_OPERAND_TYPE_MEMORY:
        inst.type0 = (uint8_t)OperandType::memory;
        inst.mem0.base = (uint8_t)ZydisRegisterToVM(op0.mem.base);
        inst.mem0.index = (uint8_t)ZydisRegisterToVM(op0.mem.index);
        inst.mem0.scale = op0.mem.scale;
        inst.mem0.offset = (uint8_t)op0.mem.disp.value;
        break;
    case ZYDIS_OPERAND_TYPE_REGISTER:
        inst.type0 = (uint8_t)OperandType::reg;
        inst.reg0 = (uint8_t)ZydisRegisterToVM(op0.reg.value);
        break;
    default:
        std::cout << "unk destination operand!\n";
        break;
    }

    switch (op1.type) {
    case ZYDIS_OPERAND_TYPE_IMMEDIATE:
        inst.type1 = (uint8_t)OperandType::imm;
        append_imm_le(additionalBuffer, op1.imm.value.u, (1 << inst.size));
        break;
    case ZYDIS_OPERAND_TYPE_REGISTER:
        inst.type1 = (uint8_t)OperandType::reg;
        inst.reg1 = (uint8_t)ZydisRegisterToVM(op1.reg.value);
        break;
    case ZYDIS_OPERAND_TYPE_MEMORY:
        inst.type1 = (uint8_t)OperandType::memory;
        inst.mem1.base = (uint8_t)ZydisRegisterToVM(op1.mem.base);
        inst.mem1.index = (uint8_t)ZydisRegisterToVM(op1.mem.index);
        inst.mem1.scale = op1.mem.scale;
        inst.mem1.offset = (uint8_t)op1.mem.disp.value;
        break;
    default:
        std::cout << "unk source operand!\n";
        break;
    }
}

void CodeBlock::generateFromInstructions(PEParser& file, int sectionIndex, DWORD startOff, DWORD end)
{
    auto& sections = file.getSectionList();
    auto& section = sections[sectionIndex];
    char* rawInstructions = (char*)section.data.data() + startOff;

    int len = end - startOff;
    ZyanU64 runtime_address = 0x7ff6fc980000;

    ZyanUSize offset = 0;
    ZydisDisassembledInstruction instruction;
    std::vector<uint8_t> additionalBuffer;
    this->originalSize = 19 + 19 - 6;
    
    while (offset < len) {

        ZydisDisassembleIntel(
            /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
            /* runtime_address: */ runtime_address,
            /* buffer:          */ rawInstructions + offset,
            /* length:          */ len - offset,
            /* instruction:     */ &instruction
        );
        
        BasicInstruction inst = { 0 };
        inst.opCode = (uint8_t)OpCode::UNK;

        // supported: add/mov/xor/sub/and/or/shr/shl/movzx
        // todo: neg / not / sar / imul / idiv
        switch (instruction.info.mnemonic)
        {
        case ZYDIS_MNEMONIC_ADD: inst.opCode = (uint8_t)OpCode::add; break;
        case ZYDIS_MNEMONIC_MOV: inst.opCode = (uint8_t)OpCode::mov; break;
        case ZYDIS_MNEMONIC_XOR: inst.opCode = (uint8_t)OpCode::xor_; break;
        case ZYDIS_MNEMONIC_CMP: inst.opCode = (uint8_t)OpCode::cmp; break;
        case ZYDIS_MNEMONIC_SUB: inst.opCode = (uint8_t)OpCode::sub; break;
        case ZYDIS_MNEMONIC_AND: inst.opCode = (uint8_t)OpCode::and_; break;
        case ZYDIS_MNEMONIC_OR:  inst.opCode = (uint8_t)OpCode::or_;  break;
        case ZYDIS_MNEMONIC_SHR: inst.opCode = (uint8_t)OpCode::shr; break;
        case ZYDIS_MNEMONIC_SHL: inst.opCode = (uint8_t)OpCode::shl; break;
        case ZYDIS_MNEMONIC_MOVZX: inst.opCode = (uint8_t)OpCode::movzx; break;
        default:
            std::cout << "unknown instruction: " << instruction.text << '\n';
            break;
        }

        if (inst.opCode != (uint8_t)OpCode::UNK) {
            parseOperands(instruction, inst, additionalBuffer);
            this->instructions.insert(this->instructions.end(), (uint8_t*)&inst, (uint8_t*)(&inst) + sizeof(inst));
        }

        this->instructions.insert(this->instructions.end(), additionalBuffer.begin(), additionalBuffer.end());
        additionalBuffer.clear();

        offset += instruction.info.length;
        runtime_address += instruction.info.length;
    }
    this->originalSize += offset;
    // memset(rawInstructions - 19, 0x90, len + 19);
    
    for (int i = 0;i < len + 19;i++) {
        rawInstructions[i - 19] = rand() % 0xFF;
    }

}