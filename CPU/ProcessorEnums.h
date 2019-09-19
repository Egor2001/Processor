#ifndef PROCESSOR_ENUMS_H_INCLUDED
#define PROCESSOR_ENUMS_H_INCLUDED

namespace course {

union UWord;
struct SCommand;
struct SArgument;

//TODO: inherit from uint8_t
enum ERegister
{
#define HANDLE_REGISTER_(enum_name, name) \
    enum_name,

    #include "../EnumLists/RegisterList.h"

    REG_IP, //instruction pointer
    REG_PC, //program counter
    REG_SR, //status register

    REGISTERS_CNT //hack to simply determine registers count

#undef HANDLE_REGISTER_
};

//[DIRTY HACK!] constant for compile - time implicit register count determination
const size_t REGISTERS_NUM = static_cast<size_t>(ERegister::REGISTERS_CNT);

//TODO: inherit from uint16_t
enum ECommand
{
#define HANDLE_COMMAND_(enum_name, code, name, lhs, rhs) \
    enum_name = code,

    #include "../EnumLists/CommandList.h"
    CMD_ERR_VALUE = 0xFFFF

#undef HANDLE_COMMAND_
};

//TODO: inherit from uint8_t
enum EArgument
{
#define HANDLE_ARGUMENT_(enum_name, code, name, len) \
    enum_name = code,

    #include "../EnumLists/ArgumentList.h"
    ARG_ERR_VALUE = 0xFF

#undef HANDLE_ARGUMENT_
};

enum EFlagMask
{
    FLAG_CF = 0x0001, //carry
    FLAG_PF = 0x0004, //parity
    FLAG_AF = 0x0010, //adjust
    FLAG_ZF = 0x0040, //zero
    FLAG_SF = 0x0080, //sign
    FLAG_TF = 0x0100, //trap
    FLAG_IF = 0x0200, //interrupt
    FLAG_DF = 0x0400, //direction
    FLAG_OF = 0x0800, //overflow
};

struct SMemoryAddr
{
    uint32_t mem_addr;
};

enum EArgumentType
{
    ARG_TYPE_NUL = 0x00,
    ARG_TYPE_IMM, ARG_TYPE_FLT,
    ARG_TYPE_REG, ARG_TYPE_MEM
};

union UArgumentData
{
    UArgumentData(): as_imm(static_cast<uint32_t>(-1)) {}

    UArgumentData(int32_t     imm_val): as_imm(imm_val) {}
    UArgumentData(ERegister   reg_val): as_reg(reg_val) {}
    UArgumentData(SMemoryAddr mem_val): as_mem(mem_val) {}
    UArgumentData(float       flt_val): as_flt(flt_val) {}

    int32_t     as_imm;
    ERegister   as_reg;
    SMemoryAddr as_mem;
    float       as_flt;
};

struct SArgument
{
    SArgument(): arg_type(EArgumentType::ARG_TYPE_NUL), arg_data() {}

    SArgument(int32_t     imm_val): arg_type(EArgumentType::ARG_TYPE_IMM), arg_data(imm_val) {}
    SArgument(ERegister   reg_val): arg_type(EArgumentType::ARG_TYPE_REG), arg_data(reg_val) {}
    SArgument(SMemoryAddr mem_val): arg_type(EArgumentType::ARG_TYPE_MEM), arg_data(mem_val) {}
    SArgument(float       flt_val): arg_type(EArgumentType::ARG_TYPE_FLT), arg_data(flt_val) {}

    EArgumentType arg_type;
    UArgumentData arg_data;
};

struct SCommand
{
    SCommand():
        cmd_type(ECommand::CMD_ERR_VALUE),
        lhs_type(EArgument::ARG_ERR_VALUE),
        rhs_type(EArgument::ARG_ERR_VALUE)
    {}

    SCommand(ECommand cmd, EArgument lhs, EArgument rhs):
        cmd_type(cmd),
        lhs_type(lhs),
        rhs_type(rhs)
    {}

    uint16_t cmd_type;
    uint8_t  lhs_type;
    uint8_t  rhs_type;
};

union UWord
{
    UWord(): as_imm(static_cast<uint32_t>(-1)) {}

    UWord(uint32_t  idx_set): as_imm(idx_set) {}
    UWord(ERegister reg_set): as_reg(reg_set) {}
    UWord(SCommand  cmd_set): as_cmd(cmd_set) {}
    UWord(float     flt_set): as_flt(flt_set) {}

    uint32_t  as_imm;
    ERegister as_reg;
    SCommand  as_cmd;
    float     as_flt;
};

} //namespace course

#endif // PROCESSOR_ENUMS_H_INCLUDED

