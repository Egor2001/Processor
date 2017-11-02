#ifndef PROCESSOR_ENUMS_H_INCLUDED
#define PROCESSOR_ENUMS_H_INCLUDED

namespace course {

enum ERegister
{
    REG_AX = 0, REG_BX = 1, REG_CX = 2, REG_DX = 3
};

enum ECommand
{
    CMD_PUSH     = 0x1,
    CMD_PUSH_REG = 0x2,
    CMD_POP      = 0x3,
    CMD_DUP      = 0x4,

    CMD_FADD  = 0x5,
    CMD_FSUB  = 0x6,
    CMD_FMUL  = 0x7,
    CMD_FDIV  = 0x8,

    CMD_FSIN  = 0x9,
    CMD_FCOS  = 0xA,
    CMD_FSQRT = 0xB,

    CMD_HLT = 0xC,

    //are used in input handler
    CMD_IN   = 0x10,
    CMD_OUT  = 0x11,
    CMD_OK   = 0x12,
    CMD_DUMP = 0x13
};

enum EPushMode
{
    PUSH_NUM         = 0x0,
    PUSH_REG         = 0x1,
    PUSH_PTR         = 0x2,
    PUSH_PTR_REG     = 0x3,
    PUSH_PTR_REG_NUM = 0x4,
    PUSH_PTR_REG_REG = 0x5
};

} //namespace course

#endif // PROCESSOR_ENUMS_H_INCLUDED

