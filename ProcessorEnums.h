#ifndef PROCESSOR_ENUMS_H_INCLUDED
#define PROCESSOR_ENUMS_H_INCLUDED

namespace course {

const size_t REGISTERS_NUM = 4;

union UWord
{
    UWord() { memset(this, 0xFF, sizeof(UWord)); }
    UWord(uint32_t idx_set): idx(idx_set) {}
    UWord(float    val_set): val(val_set) {}

    uint32_t idx;
    float    val;
};

enum ERegister
{
    REG_AX = 0, REG_BX = 1, REG_CX = 2, REG_DX = 3
};

enum ECommand
{
    CMD_HLT = 0x0, //halt operation (default = 0x0)
    CMD_PUSH, CMD_POP, CMD_DUP, //stack operations
    CMD_JMP, CMD_JZ, CMD_JNZ, CMD_JE, CMD_JNE, CMD_JG, CMD_JGE, CMD_JL, CMD_JLE, //jumps
    CMD_FADD, CMD_FSUB, CMD_FMUL, CMD_FDIV, CMD_FSIN, CMD_FCOS, CMD_FSQRT, //floating point arithm

    //are used in input handler
    CMD_IN, CMD_OUT, CMD_OK, CMD_DUMP
};

enum EPushMode
{
    PUSH_NUM,
    PUSH_REG,
    PUSH_RAM,
    PUSH_RAM_REG,
    PUSH_RAM_REG_NUM,
    PUSH_RAM_REG_REG
};

enum EPopMode
{
    POP_REG,
    POP_RAM,
    POP_RAM_REG,
    POP_RAM_REG_NUM,
    POP_RAM_REG_REG
};

enum EJumpMode
{
    JUMP_REL,
    JUMP_REG,
    JUMP_RAM,
    JUMP_RAM_REG
};

} //namespace course

#endif // PROCESSOR_ENUMS_H_INCLUDED

