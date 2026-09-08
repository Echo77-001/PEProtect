#include"Disassembler.h"
#include "../core/BasicPE.h"

void DisassembleCodeBlock(PEParser& targetFile,const CodeBlock& block) {
    std::cout << "--- DISASM START ---" << std::endl;
    std::cout << "inst count:" << std::dec << block.instructions.size() << '\n';
    std::cout << "block id: " << std::hex << block.id << '\n';

    for (const auto& inst : block.instructions) {
        std::stringstream ss;
        std::string mnemonic = OpCodeStrings[(int)inst->opCode];
        ss << mnemonic;

        switch (inst->opCode) {
        case OpCode::mov:

            if (inst->params[0] == (uint8_t)OperandType::reg) {
                //std::cout << "inst.params[2] = " << (int)inst.params[2] << '\n';
                //print_hex_dump(inst.params.data(), inst.params.size());
                ss << ' ' << GetSubRegister(inst->params[3], std::pow(2, inst->params[2]));// RegisterStrings[inst.params[3]];
            }
            else { // support will be in the futue
                std::cout << "unknown operand!\n";
            }

            if (inst->params[1] == (uint8_t)OperandType::number) {
                ss << ", ";
                void* ptr = (void*)&inst->params[4]; 

                switch ((int)std::pow(2, inst->params[2])) {
                case 1: ss << (int)*(int8_t*)ptr;   break;
                case 2: ss << *(int16_t*)ptr;       break;
                case 4: ss << *(int32_t*)ptr;       break;
                case 8: ss << *(int64_t*)ptr;       break;
                default: ss << "/* unknown size */"; break;
                }
            }
            else {
                std::cout << "unknown operand!\n";
            }

            break;
        case OpCode::jmp:
            inst->opCode;
            //logicalBlocks[];

            if (inst->params[0] == (uint8_t)JumpType::extenral_func_iat) {
                ImportFunctionInfo func = targetFile.getImportFuncFromRVA(*(int*)&inst->params[1]);
                ss << " - " << func.name;
            }
            else {
                // other cases...
            }

            break;
        case OpCode::call:
            ss << " - " << std::hex;
            if (inst->params.empty()) {
                ss << "null";
            }
            else {
                if (inst->params[0] == JumpType::internal_func) {
                    ss << *(int*)&inst->params[1];
                }
                //ss << (*(int*)&inst.params[0]);
            }

            break;
        default:
            mnemonic = "???";
            break;
        }

        /* if (inst.type == transition && inst.opCode == VMOpcode::Transition::call) {

             ss << " - " << std::hex;
             if (inst.params.empty()) {
                 ss << "null";
             }
             else {
                 ss << (*(int*)&inst.params[0]);
             }
         }*/

        std::cout << ss.str() << '\n';

    }

}