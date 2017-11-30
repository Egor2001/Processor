#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

#include <climits>
#include <cmath>

#include "Stack/CourseException.h"
#include "Stack/Stack.h"

#include "Stack/Guard.h"
#include "ProcessorEnums.h"

namespace course {

using namespace course_stack;
using course_stack::operator "" _crs_hash;

union UWord
{
    UWord() { memset(this, 0xFF, sizeof(UWord)); }
    UWord(uint32_t idx_set): idx(idx_set) {}
    UWord(float    val_set): val(val_set) {}

    uint32_t idx;
    float    val;
};

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
    CProcessor():
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        proc_stack_(),
        proc_registers_(),
        proc_ram_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord));
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
        memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord));
        memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(float));
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

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    void cmd_push(EPushMode mode, UWord arg, UWord add = {})
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        #define HANDLE_MODE_(mode, expression) \
            case mode: \
                CRS_STATIC_LOG("cmd_push [mode: " CRS_STRINGIZE(mode) ", arg: %#x, add: %#x]", \
                               arg.idx, add.idx); \
                (expression); \
                break;

        switch (mode)
        {
            HANDLE_MODE_(PUSH_NUM,         proc_stack_.push(arg.val))
            HANDLE_MODE_(PUSH_REG,         proc_stack_.push(proc_registers_[arg.idx].val))
            HANDLE_MODE_(PUSH_RAM,         proc_stack_.push(proc_ram_[arg.idx]))
            HANDLE_MODE_(PUSH_RAM_REG,     proc_stack_.push(proc_ram_[proc_registers_[arg.idx].idx]))
            HANDLE_MODE_(PUSH_RAM_REG_NUM, proc_stack_.push(proc_ram_[proc_registers_[arg.idx].idx +
                                                                      add.idx]))
            HANDLE_MODE_(PUSH_RAM_REG_REG, proc_stack_.push(proc_ram_[proc_registers_[arg.idx].idx +
                                                                      proc_registers_[add.idx].idx]))
            default:
                CRS_PROCESS_ERROR("cmd_push: unrecognizable mode: "
                                  "[mode: %#x, arg: %#x, add:%#x]",
                                  mode, arg.idx, add.idx)
                return;
        }

        #undef HANDLE_MODE_

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void cmd_pop(EPopMode mode, UWord arg, UWord add = {})
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        #define HANDLE_MODE_(mode, expression) \
            case mode: \
                CRS_STATIC_LOG("cmd_pop [mode: " CRS_STRINGIZE(mode) ", arg: %#x, add: %#x]", \
                               arg.idx, add.idx); \
                (expression); \
                break;

        switch (mode)
        {
            HANDLE_MODE_(POP_REG,         proc_registers_[arg.idx].val            = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM,         proc_ram_[arg.idx]                      = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG,     proc_ram_[proc_registers_[arg.idx].idx] = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG_NUM, proc_ram_[proc_registers_[arg.idx].idx + add.idx]
                                                                                  = proc_stack_.pop())
            HANDLE_MODE_(POP_RAM_REG_REG, proc_ram_[proc_registers_[arg.idx].idx +
                                                    proc_registers_[add.idx].idx] = proc_stack_.pop())
            default:
                CRS_PROCESS_ERROR("cmd_pop: unrecognizable mode: "
                                  "[mode: %#x, arg: %#x, add:%#x]",
                                  mode, arg.idx, add.idx)
                return;
        }

        #undef HANDLE_MODE_

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    #define DECLARE_SIMPLE_COMMAND_(fnc_name, expression) \
        void fnc_name() \
        { \
            CRS_IF_GUARD(CRS_BEG_CHECK();) \
            \
            expression; \
            \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            CRS_IF_GUARD(CRS_END_CHECK();) \
        }

    DECLARE_SIMPLE_COMMAND_(cmd_dup, proc_stack_.push(proc_stack_.top()))

    DECLARE_SIMPLE_COMMAND_(cmd_fadd, proc_stack_.push(proc_stack_.pop() + proc_stack_.pop()))
    DECLARE_SIMPLE_COMMAND_(cmd_fsub, proc_stack_.push(proc_stack_.pop() - proc_stack_.pop()))
    DECLARE_SIMPLE_COMMAND_(cmd_fmul, proc_stack_.push(proc_stack_.pop() * proc_stack_.pop()))
    DECLARE_SIMPLE_COMMAND_(cmd_fdiv, proc_stack_.push(proc_stack_.pop() / proc_stack_.pop()))

    DECLARE_SIMPLE_COMMAND_(cmd_fsin,  proc_stack_.push(sinf (proc_stack_.pop())))
    DECLARE_SIMPLE_COMMAND_(cmd_fcos,  proc_stack_.push(cosf (proc_stack_.pop())))
    DECLARE_SIMPLE_COMMAND_(cmd_fsqrt, proc_stack_.push(sqrtf(proc_stack_.pop())))

    #undef DECLARE_SIMPLE_COMMAND_

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

        return proc_registers_[reg_idx].val;
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
                proc_stack_.ok());
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
    UWord                   proc_registers_[PROC_REG_COUNT];
    float                   proc_ram_      [PROC_RAM_SIZE];

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // PROCESSOR_H_INCLUDED

