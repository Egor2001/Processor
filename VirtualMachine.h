#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

#include <climits>
#include <cmath>

#include "Stack/CourseException.h"
#include "Stack/Stack.h"

#define CRS_GUARD_LEVEL 3

#include "Stack/Guard.h"
#include "ProcessorEnums.h"

namespace course {

using namespace course_stack;

class CProcessor
{
public:
    typedef union { uint32_t arg; float val; } word_t_;

    static const size_t PROC_REG_COUNT = 4, PROC_RAM_SIZE = 0x1000;

    static const size_t CANARY_VALUE = 0xFF00FF00;//TODO: "CProcessor"_crs_hash;

public:
    CProcessor():
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        proc_stack_(),
        proc_registers_(),
        proc_ram_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(word_t_));
        memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(float));

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    ~CProcessor()
    {
        CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

        CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
        CRS_IF_HASH_GUARD  (hash_value_ = 0;)

        proc_stack_.clear();
        memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(word_t_));
        memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(float));
    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = proc_stack_.get_hash_value();

        result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
                   proc_registers_[ERegister::REG_AX].arg << 0x8) ^
                   proc_registers_[ERegister::REG_BX].arg << 0x4) ^
                   proc_registers_[ERegister::REG_CX].arg >> 0x4) ^
                   proc_registers_[ERegister::REG_DX].arg >> 0x8));

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    void cmd_push(EPushMode mode, word_t_ arg, word_t_ add)
    {
        switch (mode)
        {
            case PUSH_NUM:
            case PUSH_REG:
            case PUSH_PTR:
            case PUSH_PTR_REG:
            case PUSH_PTR_REG_NUM:
            case PUSH_PTR_REG_REG:
        }
    }

    bool proc_command(ECommand command, word_t_ arg = std::numeric_limits<word_t_>::max())
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        switch (command)
        {
            case CMD_PUSH:
                CRS_STATIC_LOG("proc_command [PUSH %f]", *(float*)(&arg));
                proc_stack_.push(*(float*)(&arg));
                break;

            case CMD_PUSH_REG:
                CRS_STATIC_LOG("proc_command [PUSH_REG %u]", arg);
                proc_stack_.push(proc_registers_[arg]);
                break;

            case CMD_POP:
                CRS_STATIC_LOG("proc_command [POP %u]", arg);
                proc_registers_[arg] = proc_stack_.pop();
                break;

            case CMD_DUP:
                CRS_STATIC_MSG("proc_command [DUP]");
                proc_stack_.push(proc_stack_.top());
                break;

            case CMD_FADD:
                CRS_STATIC_MSG("proc_command [FADD]");
                proc_stack_.push(proc_stack_.pop() + proc_stack_.pop());
                break;

            case CMD_FSUB:
                CRS_STATIC_MSG("proc_command [FSUB]");
                proc_stack_.push(proc_stack_.pop() - proc_stack_.pop());
                break;

            case CMD_FMUL:
                CRS_STATIC_MSG("proc_command [FMUL]");
                proc_stack_.push(proc_stack_.pop() * proc_stack_.pop());
                break;

            case CMD_FDIV:
                CRS_STATIC_MSG("proc_command [FDIV]");
                proc_stack_.push(proc_stack_.pop() / proc_stack_.pop());
                break;

            case CMD_FSIN:
                CRS_STATIC_MSG("proc_command [FSIN]");
                proc_stack_.push(sinf (proc_stack_.pop()));
                break;

            case CMD_FCOS:
                CRS_STATIC_MSG("proc_command [FCOS]");
                proc_stack_.push(cosf (proc_stack_.pop()));
                break;

            case CMD_FSQRT:
                CRS_STATIC_MSG("proc_command [FSQRT]");
                proc_stack_.push(sqrtf(proc_stack_.pop()));
                break;

            case CMD_HLT:
                CRS_STATIC_MSG("proc_command [HLT]");
                return false;

            case CMD_IN:
            case CMD_OUT:
            case CMD_OK:
            case CMD_DUMP:
                CRS_STATIC_MSG("proc_command: [input handle command]");
                break;

            default:
                CRS_STATIC_MSG("proc_command: default case statement reached");

                return false;
        }

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return true;
    }

    float pop_out()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        float result = proc_stack_.pop();

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    float reg_out(ERegister reg_idx) const
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
        return this && proc_stack_.ok();
    }

    void dump() const
    {
        CRS_STATIC_DUMP("CVirtualMachine[%s, this : %p] \n"
                        "{ \n"
                        CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                        CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                        "    \n"
                        "    proc_stack_: \n"
                        "        size() : %d \n"
                        "    proc_registers_: \n"
                        "    { \n"
                        "        [AX: %f], \n"
                        "        [BX: %f], \n"
                        "        [CX: %f], \n"
                        "        [DX: %f], \n"
                        "    } \n"
                        "    \n"
                        CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                        "} \n",

                        (ok() ? "OK" : "ERROR"), this,
                        CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                        CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                        proc_stack_.size(),
                        proc_registers_[ERegister::REG_AX],
                        proc_registers_[ERegister::REG_BX],
                        proc_registers_[ERegister::REG_CX],
                        proc_registers_[ERegister::REG_DX]

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CStaticStack<float, 64> proc_stack_;
    word_t_                 proc_registers_[PROC_REG_COUNT];
    float                   proc_ram_      [PROC_RAM_SIZE]

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // VIRTUAL_MACHINE_H_INCLUDED
