#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

#include <climits>
#include <cmath>

#include "Processor.h"

namespace course {

class CVirtualMachine
{
public:
    static const size_t CANARY_VALUE = "CVirtualMachine"_crs_hash;

public:
    CVirtualMachine():
        input_file_view_(),
        instruction_vec_(),
        processor_      ()
    {

    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = 0;

        for (const auto& instruction : instruction_vec_)
        {
            result ^= (instruction.command ^
                       (instruction.mode << 1*sizeof(UWord)/4) ^
                       (instruction.mode << 2*sizeof(UWord)/4) ^
                       (instruction.mode << 3*sizeof(UWord)/4));
        }

        result ^= static_cast<uintptr_t>(input_file_view_);

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    void parse_instruction()
    {
        char command_str[16] = "";

        sscanf(instruction_str, "%15s", command_str);

        #define HANDLE_SIMPLE_CMD_(recognised_instruction, instruction) \
            case instruction: \
                CRS_STATIC_MSG(CRS_STRINGIZE(command) " command recognized"); \
                return { command, {}, {}, {} }; \
                break;

        switch ()

        if (!strcmp(command_str, "push"))
        {
            EPushMode push_mode = -1;
            UWord arg = {}, add = {};

            sscanf(instruction_str, "push %u", &push_mode);

            #define HANDLE_PUSH_MODE_(mode, args_format_literal) \
                case mode: \
                    sscanf(instruction_str, "push %u " args_format_literal, \
                           &push_mode, &arg, &add); \
                    \
                    CRS_STATIC_MSG("CMD_PUSH [%s " args_format_literal, "] command recognized"\
                                   CRS_STRINGIZE(mode), arg, add); \
                    \
                    return { CMD_PUSH, mode, arg, add };

            switch (push_mode)
            {
                HANDLE_PUSH_MODE_(EPushMode::PUSH_NUM,         "%f")
                HANDLE_PUSH_MODE_(EPushMode::PUSH_REG,         "%u")
                HANDLE_PUSH_MODE_(EPushMode::PUSH_RAM,         "%u")
                HANDLE_PUSH_MODE_(EPushMode::PUSH_RAM_REG,     "%u")
                HANDLE_PUSH_MODE_(EPushMode::PUSH_RAM_REG_NUM, "%u %u")
                HANDLE_PUSH_MODE_(EPushMode::PUSH_RAM_REG_REG, "%u %u")

                default:
                    CRS_PROCESS_ERROR("handle input: unrecognizable push mode: [mode: %#x]", mode)
                    return {};
            }

            #undef HANDLE_PUSH_MODE_
        }

        else if (!strcmp(command_str, "pop"))
        {
            EPopMode pop_mode = -1;
            UWord arg = {}, add = {};

            sscanf(instruction_str, "pop %u", &pop_mode);

            #define HANDLE_POP_MODE_(mode, args_format_literal) \
                case mode: \
                    sscanf(instruction_str, "pop %u " args_format_literal, \
                           &pop_mode, &arg, &add); \
                    \
                    CRS_STATIC_MSG("CMD_POP [%s " args_format_literal, "] command recognized"\
                                   CRS_STRINGIZE(mode), arg, add); \
                    \
                    return { CMD_POP, mode, arg, add };

            switch (pop_mode)
            {
                HANDLE_POP_MODE_(EPopMode::POP_REG,         "%u")
                HANDLE_POP_MODE_(EPopMode::POP_RAM,         "%u")
                HANDLE_POP_MODE_(EPopMode::POP_RAM_REG,     "%u")
                HANDLE_POP_MODE_(EPopMode::POP_RAM_REG_NUM, "%u %u")
                HANDLE_POP_MODE_(EPopMode::POP_RAM_REG_REG, "%u %u")

                default:
                    CRS_PROCESS_ERROR("handle input: unrecognizable pop mode: [mode: %#x]", mode)
                    return {};
            }

            #undef HANDLE_POP_MODE_
        }

        HANDLE_SIMPLE_CMD_(command_str, "dup",   CMD_DUP)

        HANDLE_SIMPLE_CMD_(command_str, "fadd",  CMD_FADD)
        HANDLE_SIMPLE_CMD_(command_str, "fsub",  CMD_FSUB)
        HANDLE_SIMPLE_CMD_(command_str, "fmul",  CMD_FMUL)
        HANDLE_SIMPLE_CMD_(command_str, "fdiv",  CMD_FDIV)

        HANDLE_SIMPLE_CMD_(command_str, "fsin",  CMD_FSIN)
        HANDLE_SIMPLE_CMD_(command_str, "fcos",  CMD_FCOS)
        HANDLE_SIMPLE_CMD_(command_str, "fsqrt", CMD_FSQRT)

        HANDLE_SIMPLE_CMD_(command_str, "hlt",  CMD_HLT)
        HANDLE_SIMPLE_CMD_(command_str, "in",   CMD_IN)
        HANDLE_SIMPLE_CMD_(command_str, "out",  CMD_OUT)
        HANDLE_SIMPLE_CMD_(command_str, "ok",   CMD_OK)
        HANDLE_SIMPLE_CMD_(command_str, "dump", CMD_DUMP)

        else
            CRS_PROCESS_ERROR("handle input error: unrecognizable command : %s", command_str)
            return;

        #undef HANDLE_SIMPLE_CMD_

        return { CMD_HLT, {}, {}, {} };
    }

public:
    void assert_ok() const
    {
        if (!ok())
            throw CCourseException("CInputHandler is not ok");
    }

    bool ok() const
    {
        return this && input_file_;
    }

    void dump() const
    {
        CRS_STATIC_DUMP("CInputHandler[%s, this : %p] \n"
                        "{ \n"
                        CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                        CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                        "    \n"
                        "    input_file : %p \n"
                        "    instruction_vec_ : \n"
                        "        size() : %d \n"
                        "    \n"
                        CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                        "} \n",

                        (ok() ? "OK" : "ERROR"), this,
                        CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                        CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                        input_file_,
                        instruction_vec_.size(),

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CFileView                 input_file_view_;
    std::vector<SInstruction> instruction_vec_;
    CProcessor                processor_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#endif // VIRTUAL_MACHINE_H_INCLUDED

