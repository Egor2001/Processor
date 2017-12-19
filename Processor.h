#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

#include <vector>
#include <climits>
#include <cmath>

#include "Stack/CourseException.h"
#include "Stack/Stack.h"

#include "Stack/Guard.h"
#include "ProcessorEnums.h"

#include "TranslatorFiles/FileView.h"

namespace course {

using namespace course_stack;
using course_stack::operator "" _crs_hash;

class CProcessor
{
    static const size_t PROC_REG_COUNT = 4, PROC_RAM_SIZE = 0x1000;

    static const size_t CANARY_VALUE = "CProcessor"_crs_hash;

public:
    CProcessor(const char* input_file_name):
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        proc_stack_    (),
        proc_registers_(),
        proc_ram_      (),

        input_file_view_(ECMapMode::MAP_READONLY_FILE, input_file_name),

        program_counter_(0),
        instruction_pipe_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        CRS_CHECK_MEM_OPER(memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord)))
        CRS_CHECK_MEM_OPER(memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(UWord)))

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    ~CProcessor()
    {
        CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

        CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
        CRS_IF_HASH_GUARD  (hash_value_ = 0;)

        proc_stack_.clear();
        CRS_CHECK_MEM_OPER(memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord)))
        CRS_CHECK_MEM_OPER(memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(UWord)))

        program_counter_ = 0;
        instruction_pipe_.clear();
    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = proc_stack_.get_hash_value();

        result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
                   (proc_registers_[ERegister::REG_AX].idx << 0x8) ^
                   (proc_registers_[ERegister::REG_BX].idx << 0x4) ^
                   (proc_registers_[ERegister::REG_CX].idx >> 0x4) ^
                   (proc_registers_[ERegister::REG_DX].idx >> 0x8));

        result ^= program_counter_;

        for (size_t i = 0; i < instruction_pipe_.size(); i++)
            result ^= (*instruction_pipe_[i] << (i%sizeof(size_t)));

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    uint32_t get_command_len_(const char* token_pos) const
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        uint32_t result = 0;

        ECommand command = static_cast<ECommand>(get_word_(token_pos, 0).idx);

        switch (command)
        {
            case ECommand::CMD_PUSH:
            {
                switch (static_cast<EPushMode>(get_word_(token_pos, 1).idx))
                {
                    case EPushMode::PUSH_NUM:
                    case EPushMode::PUSH_REG:
                    case EPushMode::PUSH_RAM:
                    case EPushMode::PUSH_RAM_REG:
                        result = 3;
                        break;

                    case EPushMode::PUSH_RAM_REG_NUM:
                    case EPushMode::PUSH_RAM_REG_REG:
                        result = 4;
                        break;
                }
            }
            break;

            case ECommand::CMD_POP:
            {
                switch (static_cast<EPopMode>(get_word_(token_pos, 1).idx))
                {
                    case EPopMode::POP_REG:
                    case EPopMode::POP_RAM:
                    case EPopMode::POP_RAM_REG:
                        result = 3;
                        break;

                    case EPopMode::POP_RAM_REG_NUM:
                    case EPopMode::POP_RAM_REG_REG:
                        result = 4;
                        break;
                }
            }
            break;

            case ECommand::CMD_JMP:
            case ECommand::CMD_JZ:
            case ECommand::CMD_JNZ:
            case ECommand::CMD_JE:
            case ECommand::CMD_JNE:
            case ECommand::CMD_JG:
            case ECommand::CMD_JGE:
            case ECommand::CMD_JL:
            case ECommand::CMD_JLE:
                result = 3;
                break;

            default:
                result = 1;
                break;
        }

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    void load_commands()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        const char* end_pos = input_file_view_.get_file_view_str() +
                              input_file_view_.get_file_view_size();
        const char* cur_pos = input_file_view_.get_file_view_str();
        uint32_t    cur_cmd_len = 0;

        while (cur_pos + cur_cmd_len < end_pos)
        {
            cur_pos += cur_cmd_len*sizeof(UWord);

            instruction_pipe_.push_back(cur_pos);
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

            cur_cmd_len = get_command_len_(cur_pos);
        }

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void execute()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (instruction_pipe_.empty())
            load_commands();

        #define HANDLE_COMMAND_(opcode, name, parametered, pattern) \
            case opcode: \
            { \
                cmd_##name##_(); \
            } \
            break;

        while (program_counter_ < instruction_pipe_.size())
        {
            ECommand command = static_cast<ECommand>(get_word_(instruction_pipe_[program_counter_], 0).idx);

            switch (command)
            {
                #include "CommandList.h"

                default:
                    CRS_PROCESS_ERROR("provessor error: unrecognisable command: %#x", command)
            }
        }

        #undef HANDLE_COMMAND_

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

private:
    UWord get_word_(const char* cur_ptr, uint32_t word_num) const
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        UWord result = {};
        memcpy(&result, cur_ptr + word_num*sizeof(UWord), sizeof(UWord));

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    void jump_helper_(EJumpMode mode, UWord arg)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        switch (mode)
        {
            case EJumpMode::JUMP_REL:
                program_counter_ += static_cast<int32_t>(arg.idx);
                break;

            case EJumpMode::JUMP_REG:
                program_counter_ = proc_registers_[arg.idx].idx;
                break;

            case EJumpMode::JUMP_RAM:
                program_counter_ = proc_ram_[arg.idx].idx;
                break;

            case EJumpMode::JUMP_RAM_REG:
                program_counter_ = proc_ram_[proc_registers_[arg.idx].idx].idx;
                break;

            default:
                CRS_PROCESS_ERROR("provessor error: unrecognizable jump mode: %#x", mode)
                return;
        }

        if (program_counter_ >= instruction_pipe_.size())
            CRS_PROCESS_ERROR("provessor error: "
                              "program counter is out of range after jump: \"%#x\"", program_counter_)

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_push_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        EPushMode push_mode = static_cast<EPushMode>
                              (get_word_(instruction_pipe_[program_counter_], 1).idx);
        //because of possible overflow at end of pipe
        #define ARG_1_ get_word_(instruction_pipe_[program_counter_], 2)
        #define ARG_2_ get_word_(instruction_pipe_[program_counter_], 3)

        #define HANDLE_MODE_(mode, expression) \
            case mode: \
            { \
                CRS_STATIC_MSG("cmd_push_ [mode: " CRS_STRINGIZE(mode) "]"); \
                \
                expression; \
            } \
            break;

        switch (push_mode)
        {
            HANDLE_MODE_(PUSH_NUM,         proc_stack_.push(ARG_1_))
            HANDLE_MODE_(PUSH_REG,         proc_stack_.push(proc_registers_[ARG_1_.idx]))
            HANDLE_MODE_(PUSH_RAM,         proc_stack_.push(proc_ram_[ARG_1_.idx]))
            HANDLE_MODE_(PUSH_RAM_REG,     proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx]))
            HANDLE_MODE_(PUSH_RAM_REG_NUM, proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx +
                                                                      ARG_2_.idx]))
            HANDLE_MODE_(PUSH_RAM_REG_REG, proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx +
                                                                      proc_registers_[ARG_2_.idx].idx]))
            default:
                CRS_PROCESS_ERROR("cmd_push: unrecognizable mode: %#x", push_mode)
                return;
        }

        #undef HANDLE_MODE_

        #undef ARG_1_
        #undef ARG_2_

        program_counter_++;/*TODO:*/

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_pop_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        EPopMode pop_mode = static_cast<EPopMode>
                            (get_word_(instruction_pipe_[program_counter_], 1).idx);
        //because of possible overflow at end of pipe
        #define ARG_1_ get_word_(instruction_pipe_[program_counter_], 2)
        #define ARG_2_ get_word_(instruction_pipe_[program_counter_], 3)

        #define HANDLE_MODE_(mode, expression) \
            case mode: \
            { \
                CRS_STATIC_MSG("cmd_pop_ [mode: " CRS_STRINGIZE(mode) "]"); \
                \
                expression; \
            } \
            break;

        switch (pop_mode)
        {
            HANDLE_MODE_(POP_REG,         proc_registers_[ARG_1_.idx]                = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM,         proc_ram_[ARG_1_.idx]                      = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG,     proc_ram_[proc_registers_[ARG_1_.idx].idx] = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG_NUM, proc_ram_[proc_registers_[ARG_1_.idx].idx + ARG_2_.idx]
                                                                                     = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG_REG, proc_ram_[proc_registers_[ARG_1_.idx].idx +
                                                    proc_registers_[ARG_2_.idx].idx] = proc_stack_.pop())
            default:
                CRS_PROCESS_ERROR("cmd_pop: unrecognizable mode: %#x", pop_mode)
                return;
        }

        #undef HANDLE_MODE_

        #undef ARG_1_
        #undef ARG_2_

        program_counter_++;/*TODO:*/

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    #define DECLARE_JUMP_(name, cond) \
        void cmd_##name##_() \
        { \
            CRS_IF_GUARD(CRS_BEG_CHECK();) \
            \
            bool is_jump = (cond); \
            \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            if (is_jump) jump_helper_(static_cast<EJumpMode>(get_word_(instruction_pipe_[program_counter_], 1).idx), \
                                                             get_word_(instruction_pipe_[program_counter_], 2).idx); \
            else program_counter_++;/*TODO:*/ \
            \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            CRS_IF_GUARD(CRS_END_CHECK();) \
        }

    DECLARE_JUMP_(jmp, true)
    DECLARE_JUMP_(jz,  proc_stack_.pop().idx == 0x0)
    DECLARE_JUMP_(jnz, proc_stack_.pop().idx != 0x0)
    DECLARE_JUMP_(je,  proc_stack_.pop().idx == proc_stack_.pop().idx)
    DECLARE_JUMP_(jne, proc_stack_.pop().idx != proc_stack_.pop().idx)
    DECLARE_JUMP_(jg,  proc_stack_.pop().idx >  proc_stack_.pop().idx)
    DECLARE_JUMP_(jge, proc_stack_.pop().idx >= proc_stack_.pop().idx)
    DECLARE_JUMP_(jl,  proc_stack_.pop().idx <  proc_stack_.pop().idx)
    DECLARE_JUMP_(jle, proc_stack_.pop().idx <= proc_stack_.pop().idx)

    #undef DECLARE_JUMP_

    #define DECLARE_SIMPLE_COMMAND_(name, expression) \
        void cmd_##name##_() \
        { \
            CRS_IF_GUARD(CRS_BEG_CHECK();) \
            \
            expression; \
            program_counter_++;/*TODO:*/ \
            \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            CRS_IF_GUARD(CRS_END_CHECK();) \
        }

    DECLARE_SIMPLE_COMMAND_(dup, proc_stack_.push(proc_stack_.top()))

    DECLARE_SIMPLE_COMMAND_(fadd, proc_stack_.push(proc_stack_.pop().val + proc_stack_.pop().val))
    DECLARE_SIMPLE_COMMAND_(fsub, proc_stack_.push(proc_stack_.pop().val - proc_stack_.pop().val))
    DECLARE_SIMPLE_COMMAND_(fmul, proc_stack_.push(proc_stack_.pop().val * proc_stack_.pop().val))
    DECLARE_SIMPLE_COMMAND_(fdiv, proc_stack_.push(proc_stack_.pop().val / proc_stack_.pop().val))

    DECLARE_SIMPLE_COMMAND_(fsin,  proc_stack_.push(sinf (proc_stack_.pop().val)))
    DECLARE_SIMPLE_COMMAND_(fcos,  proc_stack_.push(cosf (proc_stack_.pop().val)))
    DECLARE_SIMPLE_COMMAND_(fsqrt, proc_stack_.push(sqrtf(proc_stack_.pop().val)))
    DECLARE_SIMPLE_COMMAND_(hlt, program_counter_ = instruction_pipe_.size())

    #undef DECLARE_SIMPLE_COMMAND_

    void cmd_in_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        UWord word_to_push = {};

        printf("enter value: ");
        scanf ("%f", &word_to_push.val);

        proc_stack_.push(word_to_push);

        program_counter_++;/*TODO:*/

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_out_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        printf("stack top: %f \n", proc_stack_.pop().val);

        program_counter_++;/*TODO:*/

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_dump_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        dump();
        program_counter_++;/*TODO:*/

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_ok_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        printf("stack %s \n", (ok() ? "is ok" : "is not ok"));
        program_counter_++;/*TODO:*/

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

public:
    void assert_ok() const
    {
        if (!ok())
            throw CCourseException("CProcessor is not ok");
    }

    bool ok() const
    {
        return (this && CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                            end_canary_ == CANARY_VALUE &&)
                CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_() &&) proc_stack_.ok() &&
                (program_counter_ <= instruction_pipe_.size() || instruction_pipe_.size() == 0));
    }

    void dump() const
    {
        CRS_STATIC_DUMP("CProcessor[%s, this : %p] \n"
                        "{ \n"
                        CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                        CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                        "    \n"
                        "    proc_stack_: \n"
                        "        size() : %d \n"
                        "    proc_registers_: \n"
                        "    { \n"
                        "        [AX: %#x], \n"
                        "        [BX: %#x], \n"
                        "        [CX: %#x], \n"
                        "        [DX: %#x], \n"
                        "    } \n"
                        "    proc_ram_ : \n"
                        //"        size : %d \n"
                        "    \n"
                        "    instruction_pipe_ \n"
                        "        size() : %d \n"
                        "    \n"
                        "    program_counter_[%s] : %d \n"
                        "    \n"
                        CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                        "} \n",

                        (ok() ? "OK" : "ERROR"), this,
                        CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                        CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                        proc_stack_.size(),

                        proc_registers_[ERegister::REG_AX].idx,
                        proc_registers_[ERegister::REG_BX].idx,
                        proc_registers_[ERegister::REG_CX].idx,
                        proc_registers_[ERegister::REG_DX].idx,

                        //PROC_RAM_SIZE,

                        instruction_pipe_.size(),
                        (program_counter_ < instruction_pipe_.size() ? "OK" : "OUT_OF_RANGE"),
                        program_counter_

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CStaticStack<UWord, 64> proc_stack_;
    UWord                   proc_registers_[PROC_REG_COUNT];
    UWord                   proc_ram_      [PROC_RAM_SIZE];

    CFileView input_file_view_;

    uint32_t program_counter_;
    std::vector<const char*> instruction_pipe_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // PROCESSOR_H_INCLUDED

