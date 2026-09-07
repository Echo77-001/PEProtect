#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    mov = 0,
    movsx,
    movsxd,
    movzx,
    movdqa,
    movdqu,
    lea,
    add,
    sub,
    sbb,
    inc,
    dec,
    imul,
    neg,
    and_,
    or_,
    xor_,
    not_,
    shl,
    shr,
    sar,
    rol,
    ror,
    cmp,
    test,
    push,
    pop,
    xchg,
    nop,
    jmp,
    call,
    ret,
    jz,
    jnz,
    js,
    jns,
    jl,
    jnl,
    jle,
    jb,
    jnb,
    jbe,
    jnbe,
    bt,
    cmpxchg,
    cmovz,
    cmovnz,
    setz,
    setnz,
    setnbe,
    pushfq,
    cpuid,
    xgetbv,
    int_,
    int3,
    outsb,
    setbe,

    COUNT
};

enum class OperandType : uint8_t {
    none,
    reg,
    number,
    externalCall
};

enum class Register64 : uint8_t {
    RAX = 0, RBX, RCX, RDX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    UNK = 0xFF
};

enum JumpType {
    internal_func,
    extenral_func_iat
};

const char* const OpCodeStrings[] = {

    "mov", "movsx", "movsxd", "movzx", "movdqa", "movdqu", "lea", "add", "sub", "sbb",
    "inc", "dec", "imul", "neg", "and", "or", "xor", "not", "shl", "shr",
    "sar", "rol", "ror", "cmp", "test", "push", "pop", "xchg", "nop",

    "jmp", "call", "ret", "jz", "jnz", "js", "jns", "jl", "jnl", "jle",
    "jb", "jnb", "jbe", "jnbe",

    "bt", "cmpxchg", "cmovz", "cmovnz", "setz", "setnz", "setnbe",
    "pushfq", "cpuid", "xgetbv", "int", "int3", "outsb", "setbe"
};

const char* const RegisterStrings[] = {
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"
};
