#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

#include <vector>
#include <climits>
#include <cmath>

#include "Stack/CourseException.h"
#include "Stack/Stack.h"

#include "Stack/Guard.h"
#include "ProcessorEnums.h"

namespace course {

using namespace course_stack;
using course_stack::operator "" _crs_hash;

struct SInstruction
{
    SInstruction(): command(ECommand::CMD_HLT), mode(), arg(), add() {}
    SInstruction(ECommand command_set, uint32_t mode_set, UWord arg_set, UWord add_set):
        command(command_set), mode(mode_set), arg(arg_set), add(add_set) {}

    ECommand command;
    uint32_t mode;
    UWord    arg, add;
};

class CProcessor
{
    static const size_t PROC_REG_COUNT = 4, PROC_RAM_SIZE = 0x1000;

    static const size_t CANARY_VALUE = "CProcessor"_crs_hash;

public:
    CProcessor(size_t instruction_pipe_size_set, const UWord* instruction_pipe_set):
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        proc_stack_    (),
        proc_registers_(),
        proc_ram_      (),

        instruction_pipe_size_(0),
        instruction_pipe_     (nullptr),

        program_counter_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        CRS_CHECK_MEM_OPER(memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord)))
        CRS_CHECK_MEM_OPER(memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(UWord)))

        if (instruction_pipe_set)
        {
            instruction_pipe_size_ = instruction_pipe_size_set + 1;
            CRS_CHECK_MEM_OPER(instruction_pipe_ = calloc(instruction_pipe_size_, sizeof(UWord)))
            CRS_CHECK_MEM_OPER(memcpy(instruction_pipe_, instruction_pipe_set,
                                      instruction_pipe_size_set*sizeof(UWord)))

            instruction_pipe_[instruction_pipe_size_-1] = ECommand::CMD_HLT;
        }

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

        if (instruction_pipe_)
        {
            free(instruction_pipe_);

            instruction_pipe_      = nullptr;
            instruction_pipe_size_ = 0;
        }

        program_counter_ = 0;
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

        for (size_t i = 0; i < instruction_pipe_size_; i++)
            result ^= instruction_pipe_[i];

        result ^= instruction_pipe_size_ ^ program_counter_;

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    void execute()
    {
        while (static_cast<instruction_pipe_[program_counter_].idx> != ECommand::CMD_HLT)
        {

        }
    }

private:
    void jump_helper_(EJumpMode mode, UWord arg)
    {
        switch (mode)
        {
            case EJumpMode::JUMP_REL:     program_counter_ += arg.idx;                                break;
            case EJumpMode::JUMP_ABS:     program_counter_ =  arg.idx;                                break;
            case EJumpMode::JUMP_REG:     program_counter_ = proc_registers_[arg.idx].idx;            break;
            case EJumpMode::JUMP_RAM_REG: program_counter_ = proc_ram_[proc_registers_[arg.idx]].idx; break;

            default:
                CRS_PROCESS_ERROR("jump_helper_: unrecognizable jump mode: %#x", mode)
                return;
        }

        if (program_counter_ >= instruction_pipe_size_)
        {
            CRS_PROCESS_ERROR("jump_helper_: jumping out of pipe bounds to %d address in pipe",
                              program_counter_)
            return;
        }
    }

public:
    void cmd_push()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        EPushMode push_mode = static_cast<EPushMode>(instruction_pipe_[program_counter_+1].idx);

        #define ARG_1_ instruction_pipe_[program_counter_+2]//because of possible overflow at end of pipe
        #define ARG_2_ instruction_pipe_[program_counter_+3]

        #define HANDLE_MODE_(args_num, mode, expression) \
            case mode: \
            { \
                CRS_STATIC_LOG("cmd_push [mode: " CRS_STRINGIZE(mode) "]"); \
                \
                expression; \
                program_counter_ += args_num+2; \
            } \
            break;

        switch (push_mode)
        {
            HANDLE_MODE_(1, PUSH_NUM,         proc_stack_.push(ARG_1_))
            HANDLE_MODE_(1, PUSH_REG,         proc_stack_.push(proc_registers_[ARG_1_.idx]))
            HANDLE_MODE_(1, PUSH_RAM,         proc_stack_.push(proc_ram_[ARG_1_.idx]))
            HANDLE_MODE_(1, PUSH_RAM_REG,     proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx]))
            HANDLE_MODE_(2, PUSH_RAM_REG_NUM, proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx +
                                                                         ARG_2_.idx]))
            HANDLE_MODE_(2, PUSH_RAM_REG_REG, proc_stack_.push(proc_ram_[proc_registers_[ARG_1_.idx].idx +
                                                                         proc_registers_[ARG_2_.idx].idx]))
            default:
                CRS_PROCESS_ERROR("cmd_push: unrecognizable mode: %#x", push_mode)
                return;
        }

        #undef HANDLE_MODE_

        #undef ARG_1_
        #undef ARG_2_

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_pop()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        EPopMode pop_mode = static_cast<EPopMode>(instruction_pipe_[program_counter_+1].idx);

        #define ARG_1_ instruction_pipe_[program_counter_+2]//because of possible overflow at end of pipe
        #define ARG_2_ instruction_pipe_[program_counter_+3]

        #define HANDLE_MODE_(args_num, mode, expression) \
            case mode: \
            { \
                CRS_STATIC_LOG("cmd_pop [mode: " CRS_STRINGIZE(mode) "); \
                \
                expression; \
                program_counter_ += args_num+2; \
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

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    #define DECLARE_JUMP_(name, cond) \
        void cmd_##name() \
        { \
            CRS_IF_GUARD(CRS_BEG_CHECK();) \
            \
            bool is_jump = cond; \
            \
            if (is_jump) jump_helper_(static_cast<EJumpMode>(instruction_pipe_[program_counter_+1]), \
                                                             instruction_pipe_[program_counter_+2]); \
            else program_counter_ += 3; \
            \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            CRS_IF_GUARD(CRS_END_CHECK();) \
        }

    DECLARE_JUMP_(jmp, true)
    DECLARE_JUMP_(jz,  proc_stack_.pop().idx == 0x0)
    DECLARE_JUMP_(jnz, proc_stack_.pop().idx != 0x0)
    DECLARE_JUMP_(je,  proc_stack_.pop() == proc_stack_.pop())
    DECLARE_JUMP_(jne, proc_stack_.pop() != proc_stack_.pop())
    DECLARE_JUMP_(jg,  proc_stack_.pop() >  proc_stack_.pop())
    DECLARE_JUMP_(jge, proc_stack_.pop() >= proc_stack_.pop())
    DECLARE_JUMP_(jl,  proc_stack_.pop() <  proc_stack_.pop())
    DECLARE_JUMP_(jle, proc_stack_.pop() <= proc_stack_.pop())

    #define DECLARE_SIMPLE_COMMAND_(name, expression) \
        void cmd_##name() \
        { \
            CRS_IF_GUARD(CRS_BEG_CHECK();) \
            \
            expression; \
            program_counter_++; \
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

    #undef DECLARE_SIMPLE_COMMAND_

    UWord pop_out()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        UWord result = proc_stack_.pop();

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    UWord reg_out(ERegister reg_idx) const
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (reg_idx >= PROC_REG_COUNT)
            throw CCourseException("register index is out of range");

        CRS_IF_GUARD(CRS_END_CHECK();)

        return proc_registers_[reg_idx];
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
                CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_() &&)
                proc_stack_.ok() && instruction_pipe_ && (program_counter_ < instruction_pipe_size_));
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
                        "        size : %d \n"
                        "    \n"
                        "    instruction_pipe_size_ : %d \n"
                        "    instruction_pipe_[%s] : %p \n"
                        "    \n"
                        "    program_counter_[%s] : [%d] = %#x \n"
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

                        PROC_RAM_SIZE,

                        instruction_pipe_size_,
                        instruction_pipe_, (instruction_pipe_ ? "OK" : "ERROR: NULL"),

                        program_counter_, (program_counter_ < instruction_pipe_size_ ? "OK" : "ERROR: OUT_OF_RANGE"),
                        (program_counter_ < instruction_pipe_size_ ? instruction_pipe_[program_counter_] : 0xFFFFFFFF)

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CStaticStack<UWord, 64> proc_stack_;
    UWord                   proc_registers_[PROC_REG_COUNT];
    UWord                   proc_ram_      [PROC_RAM_SIZE];

    size_t instruction_pipe_size_;
    UWord* instruction_pipe_;

    size_t program_counter_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // PROCESSOR_H_INCLUDED

