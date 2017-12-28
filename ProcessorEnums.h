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
    CMD_CALL, CMD_RET,//procedure managment
    CMD_JMP, CMD_JZ, CMD_JNZ, CMD_JE, CMD_JNE, CMD_JG, CMD_JGE, CMD_JL, CMD_JLE, //jumps
  /*CMD_MOV, CMD_CMP, CMD_INC, CMD_DEC,
    CMD_ADD, CMD_SUB, CMD_MUL, CMD_DIV, CMD_AND, CMD_OR, CMD_XOR,
    CMD_FADD, CMD_FSUB, CMD_FMUL, CMD_FDIV, */
    CMD_FADD, CMD_FSUB, CMD_FMUL, CMD_FDIV, CMD_FSIN, CMD_FCOS, CMD_FSQRT, //floating point arithm

    CMD_FTOI, CMD_ITOF,//float <-> int conversion

    //are used in input handler
    CMD_IN, CMD_OUT, CMD_OK, CMD_DUMP,

    //not a command, has the same function with '\0'
    CMD_NULL_TERMINATOR = 0xFFFFFFFF
};

#define DECLARE_MODES_(prefix) \
    prefix##_REG, \
    prefix##_RAM, \
    prefix##_RAM_REG, \
    prefix##_RAM_REG_IDX, \
    prefix##_RAM_REG_REG

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

enum ECallMode
{
    CALL_REL,
    CALL_REG,
    CALL_RAM,
    CALL_RAM_REG
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

