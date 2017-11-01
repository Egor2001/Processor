#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

#include <climits>
#include <cmath>

#include "Stack/CourseException.h"
#include "Stack/Stack.h"

#define CRS_GUARD_LEVEL 3

#include "Stack/Guard.h"

namespace course {

using namespace course_stack;

class CProcessor
{
public:
    enum ERegisters
    {
        AX = 0, BX = 1, CX = 2, DX = 3
    };

    enum class ECommands
    {
        PUSH     = 0x1,
        PUSH_REG = 0x2,
        POP      = 0x3,
        DUP      = 0x4,

        FADD  = 0x5,
        FSUB  = 0x6,
        FMUL  = 0x7,
        FDIV  = 0x8,

        FSIN  = 0x9,
        FCOS  = 0xA,
        FSQRT = 0xB,

        HLT = 0xF4
    };

    typedef uint32_t type_t_;
    static const size_t REGISTERS_NUM = 4;

    static const size_t CANARY_VALUE = 0xFF00FF00;//TODO: "CProcessor"_crs_hash;

public:
    CProcessor():
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        proc_stack_(),
        proc_registers_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    ~CProcessor()
    {
        CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

        CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
        CRS_IF_HASH_GUARD  (hash_value_ = 0;)

        proc_stack_.clear();
        memset(proc_registers_, 0x00, REGISTERS_NUM*sizeof(type_t_));
    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = proc_stack_.get_hash_value();

        result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
                   (*(const type_t_*)(&proc_registers_[ERegisters::AX]) << 0x8) ^
                   (*(const type_t_*)(&proc_registers_[ERegisters::BX]) << 0x4) ^
                   (*(const type_t_*)(&proc_registers_[ERegisters::CX]) >> 0x4) ^
                   (*(const type_t_*)(&proc_registers_[ERegisters::DX]) >> 0x8));

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    bool proc_command(ECommands command, type_t_ arg = std::numeric_limits<type_t_>::max())
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        switch (command)
        {
            case ECommands::PUSH:
                CRS_STATIC_LOG("proc_command [PUSH %f]", *(float*)(&arg));
                proc_stack_.push(*(float*)(&arg));
                break;

            case ECommands::PUSH_REG:
                CRS_STATIC_LOG("proc_command [PUSH_REG %u]", arg);
                proc_stack_.push(proc_registers_[arg]);
                break;

            case ECommands::POP:
                CRS_STATIC_LOG("proc_command [POP %u]", arg);
                proc_registers_[arg] = proc_stack_.pop();
                break;

            case ECommands::DUP:
                CRS_STATIC_MSG("proc_command [DUP]");
                proc_stack_.push(proc_stack_.top());
                break;

            case ECommands::FADD:
                CRS_STATIC_MSG("proc_command [FADD]");
                proc_stack_.push(proc_stack_.pop() + proc_stack_.pop());
                break;

            case ECommands::FSUB:
                CRS_STATIC_MSG("proc_command [FSUB]");
                proc_stack_.push(proc_stack_.pop() - proc_stack_.pop());
                break;

            case ECommands::FMUL:
                CRS_STATIC_MSG("proc_command [FMUL]");
                proc_stack_.push(proc_stack_.pop() * proc_stack_.pop());
                break;

            case ECommands::FDIV:
                CRS_STATIC_MSG("proc_command [FDIV]");
                proc_stack_.push(proc_stack_.pop() / proc_stack_.pop());
                break;

            case ECommands::FSIN:
                CRS_STATIC_MSG("proc_command [FSIN]");
                proc_stack_.push(sinf (proc_stack_.pop()));
                break;

            case ECommands::FCOS:
                CRS_STATIC_MSG("proc_command [FCOS]");
                proc_stack_.push(cosf (proc_stack_.pop()));
                break;

            case ECommands::FSQRT:
                CRS_STATIC_MSG("proc_command [FSQRT]");
                proc_stack_.push(sqrtf(proc_stack_.pop()));
                break;

            case ECommands::HLT:
                CRS_STATIC_MSG("proc_command [HLT]");
                return false;

            default:
                CRS_STATIC_MSG("proc_command: default case statement reached");

                return false;
        }

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return true;
    }

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
                        proc_registers_[ERegisters::AX],
                        proc_registers_[ERegisters::BX],
                        proc_registers_[ERegisters::CX],
                        proc_registers_[ERegisters::DX]

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CStaticStack<float, 64> proc_stack_;
    float                   proc_registers_[REGISTERS_NUM];

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // VIRTUAL_MACHINE_H_INCLUDED
