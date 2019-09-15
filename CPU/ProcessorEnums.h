#ifndef PROCESSOR_ENUMS_H_INCLUDED
#define PROCESSOR_ENUMS_H_INCLUDED

namespace course {

union UWord
{
    UWord(): idx(static_cast<uint32_t>(-1)) {}
    UWord(uint32_t idx_set): idx(idx_set) {}
    UWord(float    flt_set): flt(flt_set) {}

    uint32_t idx;
    float    flt;
};

//TODO: inherit from uint8_t
enum ERegister
{
#define HANDLE_REGISTER_(enum_name, name) \
    enum_name,

    #include "../EnumLists/RegisterList.h"
    REGISTERS_CNT

#undef HANDLE_REGISTER_
};

//[DIRTY HACK!] constant for compile - time implicit register count determination
const size_t REGISTERS_NUM = static_cast<size_t>(ERegister::REGISTERS_CNT);

//TODO: inherit from uint16_t
enum ECommand
{
#define HANDLE_COMMAND_(enum_name, code, name, parametered, lhs, rhs) \
    enum_name = code,

    #include "../EnumLists/CommandList.h"
    CMD_ERR_VALUE = 0xFFFF

#undef HANDLE_COMMAND_
};

//TODO: inherit from uint8_t
enum EArgument
{
#define HANDLE_ARGUMENT_(enum_name, code, name) \
    enum_name = code,

    #include "../EnumLists/ArgumentList.h"
    ARG_ERR_VALUE = 0xFF

#undef HANDLE_ARGUMENT_
};

enum EArgumentGroup
{
    ARG_GROUP_ALL,
    ARG_GROUP_MOV,
    ARG_GROUP_REG,
    ARG_GROUP_LBL
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

