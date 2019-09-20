#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

#include <vector>
#include <climits>
#include <cmath>

#include "../Stack/CourseException.h"
#include "../Stack/Stack.h"

#include "../Stack/Guard.h"
#include "ProcessorEnums.h"

#include "../Translator/FileView.h"

namespace course {

using namespace course_stack;
using course_stack::operator "" _crs_hash;

class CProcessor final
{
    static const size_t PROC_REG_COUNT = REGISTERS_NUM, PROC_RAM_SIZE = 0x1000;

    static const size_t CANARY_VALUE = "CProcessor"_crs_hash;

public:
    explicit CProcessor(const char* input_file_name);

    CProcessor             (const CProcessor&) = delete;
    CProcessor& operator = (const CProcessor&) = delete;

    //TODO: to implement move-semantics ("rule of 5" dummy realisation)
    CProcessor             (CProcessor&&) = delete;
    CProcessor& operator = (CProcessor&&) = delete;

    ~CProcessor();

public:
    void load_commands();
    void execute();

private:
    [[nodiscard]] size_t calc_hash_value_() const;

    UWord get_word_(const char* cur_ptr, uint32_t word_num) const;

    uint32_t get_arg_len_(EArgument arg_val) const;
    SCommand get_cmd_() const;

    bool  push_word_(UWord word);
    UWord  pop_word_();
    bool  move_word_(SArgument dest_arg, UWord word);
    UWord pull_word_(SArgument src_arg);

    bool      proc_cmd_(ECommand cmd_type, SArgument lhs_arg, SArgument rhs_arg);
    SArgument proc_arg_(EArgument arg_type, const char** cur_src_ptr) const;

    SArgument proc_arg_nul_(const char* beg_ptr) const;

    SArgument proc_arg_imm_(const char* beg_ptr) const;
    SArgument proc_arg_reg_(const char* beg_ptr) const;
    SArgument proc_arg_lbl_(const char* beg_ptr) const;
    SArgument proc_arg_flt_(const char* beg_ptr) const;

    SArgument proc_arg_mem_imm_(const char* beg_ptr) const;
    SArgument proc_arg_mem_reg_(const char* beg_ptr) const;

    SArgument proc_arg_mem_reg_imm_(const char* beg_ptr) const;
    SArgument proc_arg_mem_reg_reg_(const char* beg_ptr) const;

    void set_flag_state_(EFlagMask flag_mask, bool state);
    bool get_flag_state_(EFlagMask flag_mask);

    bool  proc_cmd_hlt_helper_ ();
    UWord proc_cmd_in_helper_  ();
    bool  proc_cmd_out_helper_ (UWord word);
    bool  proc_cmd_jump_helper_(SArgument arg);
    bool  proc_cmd_call_helper_(SArgument arg);
    bool  proc_cmd_loop_helper_(SArgument arg);
    bool  proc_cmd_ret_helper_ ();

    bool proc_interrupt_();

//implementation is brase-enclosed command list having access to lhs, rhs and result

#define HANDLE_COMMAND_(CMD_ENUM_NAME_, CMD_CODE_, CMD_NAME_, CMD_LHS_, CMD_RHS_) \
    bool proc_cmd_##CMD_NAME_##_(SArgument lhs, SArgument rhs);

    #include "../EnumLists/CommandList.h"

#undef HANDLE_COMMAND_

public:
    [[nodiscard]] bool ok() const noexcept;
    void dump() const noexcept;

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CStaticStack<UWord, 64>      proc_stack_;
    CStaticStack<uint32_t, 1024> proc_call_stack_;
    UWord                        proc_registers_[PROC_REG_COUNT];
    UWord                        proc_ram_      [PROC_RAM_SIZE];

    CFileView input_file_view_;

//TODO: fix all appearances of program_counter_ with REG_PC

//    uint32_t program_counter_;
    std::vector<const char*> instruction_pipe_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

CProcessor::CProcessor(const char* input_file_name) :
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    proc_stack_     (),
    proc_call_stack_(),
    proc_registers_ (),
    proc_ram_       (),

    input_file_view_(ECMapMode::MAP_READONLY_FILE, input_file_name),

//    program_counter_(0),
    instruction_pipe_()

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    CRS_CHECK_MEM_OPER(memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord)))
    CRS_CHECK_MEM_OPER(memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(UWord)))

    proc_registers_[ERegister::REG_PC].as_imm = 0;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
}

CProcessor::~CProcessor()
{
    CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

    CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
    CRS_IF_HASH_GUARD  (hash_value_ = 0;)

    proc_stack_     .clear();
    proc_call_stack_.clear();
    CRS_CHECK_MEM_OPER(memset(proc_registers_, 0x00, PROC_REG_COUNT*sizeof(UWord)))
    CRS_CHECK_MEM_OPER(memset(proc_ram_,       0x00, PROC_RAM_SIZE *sizeof(UWord)))

    //program_counter_ = 0;
    proc_registers_[ERegister::REG_PC].as_imm = 0;
    instruction_pipe_.clear();
}

size_t CProcessor::calc_hash_value_() const
{
    size_t result = proc_stack_     .get_hash_value() ^
                    proc_call_stack_.get_hash_value();
    //TODO: add registers via include
    result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
               (proc_registers_[ERegister::REG_AX].as_imm << 0x8) ^
               (proc_registers_[ERegister::REG_BX].as_imm << 0x4) ^
               (proc_registers_[ERegister::REG_CX].as_imm >> 0x4) ^
               (proc_registers_[ERegister::REG_DX].as_imm >> 0x8));

//    result ^= program_counter_;

    for (size_t i = 0; i < instruction_pipe_.size(); i++)
        result ^= (*instruction_pipe_[i] << (i%sizeof(size_t)));

    return result;
}

void CProcessor::load_commands()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    const char* end_pos = input_file_view_.get_file_view_str() +
                          input_file_view_.get_file_view_size();
    const char* cur_pos = input_file_view_.get_file_view_str();
    uint32_t    cur_cmd_len = 0;

    while (cur_pos + cur_cmd_len*sizeof(UWord) < end_pos)
    {
        cur_pos += cur_cmd_len*sizeof(UWord);

        instruction_pipe_.push_back(cur_pos);
        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        SCommand cur_cmd = get_cmd_();
        cur_cmd_len = get_arg_len_(EArgument(cur_cmd.lhs_type)) +
                      get_arg_len_(EArgument(cur_cmd.rhs_type)) + 1;

        if (cur_cmd.cmd_type == ECommand::CMD_ERR_VALUE || cur_cmd_len == 0)
            break;
    }

    instruction_pipe_.push_back(cur_pos);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)
}

uint32_t CProcessor::get_arg_len_(EArgument arg_val) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    uint32_t result = 0;

    #define HANDLE_ARGUMENT_(ARG_ENUM_NAME_, ARG_CODE_, ARG_NAME_, ARG_LEN_) \
        case EArgument:: ARG_ENUM_NAME_: \
            result = ARG_LEN_; \
        break;

    switch (arg_val)
    {
        #include "../EnumLists/ArgumentList.h"

        default:
            CRS_PROCESS_ERROR("get_arg_len_ error: unexpected arg_val: %#X", arg_val)
    }

    #undef HANDLE_ARGUMENT_

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SCommand CProcessor::get_cmd_() const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SCommand result = SCommand();

    UWord word = get_word_(instruction_pipe_.back(), 0);
    result = word.as_cmd;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

void CProcessor::execute()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (instruction_pipe_.empty())
        load_commands();

    proc_registers_[ERegister::REG_IP].as_imm = 0;
    while (proc_registers_[ERegister::REG_IP].as_imm < instruction_pipe_.size())
    {
        proc_registers_[ERegister::REG_PC].as_imm = proc_registers_[ERegister::REG_IP].as_imm;
        const char* cur_src = instruction_pipe_[proc_registers_[ERegister::REG_PC].as_imm];

        SCommand cmd = get_word_(cur_src, 0).as_cmd;
        cur_src += sizeof(UWord);

        SArgument lhs_arg = proc_arg_(EArgument(cmd.lhs_type), &cur_src);
        SArgument rhs_arg = proc_arg_(EArgument(cmd.rhs_type), &cur_src);

        //after proc_cmd_ REG_IP can change its value, but only relatively to REG_PC, so increment does not matter in that case
        proc_registers_[ERegister::REG_IP].as_imm = proc_registers_[ERegister::REG_PC].as_imm + 1;
        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        proc_cmd_(ECommand(cmd.cmd_type), lhs_arg, rhs_arg);
    }

    proc_registers_[ERegister::REG_PC].as_imm = instruction_pipe_.size();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)
}

UWord CProcessor::get_word_(const char* cur_ptr, uint32_t word_num) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    UWord result = {};
    memcpy(&result, cur_ptr + word_num*sizeof(UWord), sizeof(UWord));
    CRS_STATIC_LOG("get word: %#X", result.as_imm);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_(ECommand cmd_type, SArgument lhs_arg, SArgument rhs_arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    #define HANDLE_COMMAND_(CMD_ENUM_NAME_, CMD_CODE_, CMD_NAME_, CMD_LHS_, CMD_RHS_) \
        case ECommand:: CMD_ENUM_NAME_: \
        { \
            CRS_STATIC_MSG("proc_cmd_ detected command: ECommand::" CRS_STRINGIZE(CMD_ENUM_NAME_)); \
            \
            result = proc_cmd_##CMD_NAME_##_(lhs_arg, rhs_arg); \
        } \
        break;

    switch (cmd_type)
    {
        #include "../EnumLists/CommandList.h"

        case ECommand::CMD_ERR_VALUE:
            result = false;
        break;

        default:
            CRS_PROCESS_ERROR("proc_cmd_ error: unrecognisable command: %#x", cmd_type)
    }

    #undef HANDLE_COMMAND_

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_(EArgument arg_type, const char** cur_src_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!cur_src_ptr)
        CRS_PROCESS_ERROR("proc_arg error: cur_src_ptr = %p", cur_src_ptr)

    SArgument result = SArgument();

    #define HANDLE_ARGUMENT_(ARG_ENUM_NAME_, ARG_CODE_, ARG_NAME_, ARG_LEN_) \
        case EArgument:: ARG_ENUM_NAME_: \
        { \
            result = proc_arg_##ARG_NAME_##_(*cur_src_ptr); \
            *cur_src_ptr += ARG_LEN_*sizeof(UWord); \
        } \
        break;

    switch (arg_type)
    {
        #include "../EnumLists/ArgumentList.h"

        default:
            CRS_PROCESS_ERROR("proc_arg_ error: "
                              "unexected arg_type: %#X", static_cast<uint32_t>(arg_type))
    }

    #undef HANDLE_ARGUMENT_

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::push_word_(UWord word)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    proc_stack_.push(word);
    result = true;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

UWord CProcessor::pop_word_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    UWord result = proc_stack_.pop();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::move_word_(SArgument dest_arg, UWord word)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    switch(dest_arg.arg_type)
    {
        case EArgumentType::ARG_TYPE_REG:
        {
            ERegister reg = dest_arg.arg_data.as_reg;

            if (reg < PROC_REG_COUNT)
            {
                proc_registers_[reg] = word;
                result = true;
            }
            else
                CRS_PROCESS_ERROR("move_word_ to reg error: "
                                  "%#x >= PROC_REG_COUNT=%#x", reg, PROC_REG_COUNT)
        }
        break;

        case EArgumentType::ARG_TYPE_MEM:
        {
            uint32_t mem_addr = dest_arg.arg_data.as_mem.mem_addr;

            if (mem_addr < PROC_RAM_SIZE)
            {
                proc_ram_[mem_addr] = word;
                result = true;
            }
            else
                CRS_PROCESS_ERROR("move_word_ to mem error: "
                                  "%#x > PROC_RAM_SIZE=%#x", mem_addr, PROC_RAM_SIZE)
        }
        break;

        default:
            CRS_PROCESS_ERROR("move_word_ error: arg type: %#x is not writable", dest_arg.arg_type)
    }

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

UWord CProcessor::pull_word_(SArgument src_arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    UWord result = UWord();
    switch(src_arg.arg_type)
    {
        case EArgumentType::ARG_TYPE_IMM:
            result = UWord(static_cast<uint32_t>(src_arg.arg_data.as_imm));
        break;

        case EArgumentType::ARG_TYPE_FLT:
            result = UWord(src_arg.arg_data.as_flt);
        break;

        case EArgumentType::ARG_TYPE_REG:
        {
            ERegister reg = src_arg.arg_data.as_reg;

            if (reg < PROC_REG_COUNT)
                result = proc_registers_[reg];
            else
                CRS_PROCESS_ERROR("pull_word_ from reg error: "
                                  "%#x >= PROC_REG_COUNT=%#x", reg, PROC_REG_COUNT)
        }
        break;

        case EArgumentType::ARG_TYPE_MEM:
        {
            uint32_t mem_addr = src_arg.arg_data.as_mem.mem_addr;

            if (mem_addr < PROC_RAM_SIZE)
                result = proc_ram_[mem_addr];
            else
                CRS_PROCESS_ERROR("pull_word_ from mem error: "
                                  "%#x > PROC_RAM_SIZE=%#x", mem_addr, PROC_RAM_SIZE)
        }
        break;

        default:
            CRS_PROCESS_ERROR("move_word_ error: arg type: %#x is not readable", src_arg.arg_type)
    }

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_nul_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_nul_ error: beg_ptr is %p", beg_ptr)

    SArgument result = SArgument();
    result.arg_type = EArgumentType::ARG_TYPE_NUL;
    result.arg_data = UArgumentData();

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_imm_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_imm_ error: beg_ptr is %p", beg_ptr)

    UWord word_imm = get_word_(beg_ptr, 0);
    SArgument result = SArgument(static_cast<int32_t>(word_imm.as_imm));

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_reg_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_reg_ error: beg_ptr is %p", beg_ptr)

    UWord word_reg = get_word_(beg_ptr, 0);
    SArgument result = SArgument(ERegister(word_reg.as_reg));

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_lbl_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_lbl_ error: beg_ptr is %p", beg_ptr)

    UWord word_lbl = get_word_(beg_ptr, 0);
    SArgument result = SArgument(static_cast<int32_t>(word_lbl.as_imm));

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_flt_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_flt_ error: beg_ptr is %p", beg_ptr)

    UWord word_flt = get_word_(beg_ptr, 0);
    SArgument result = SArgument(word_flt.as_flt);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_mem_imm_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_mem_imm_ error: beg_ptr is %p", beg_ptr)

    UWord word_idx_imm = get_word_(beg_ptr, 0);
    SMemoryAddr mem_addr = { word_idx_imm.as_imm };
    SArgument result = SArgument(mem_addr);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_mem_reg_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_mem_reg_ error: beg_ptr is %p", beg_ptr)

    UWord word_idx_reg = get_word_(beg_ptr, 0);
    SMemoryAddr mem_addr = { proc_registers_[word_idx_reg.as_reg].as_imm };
    SArgument result = SArgument(mem_addr);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_mem_reg_imm_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_mem_reg_imm_ error: beg_ptr is %p", beg_ptr)

    UWord word_idx_reg = get_word_(beg_ptr, 0);
    UWord word_add_imm = get_word_(beg_ptr, 1);
    SMemoryAddr mem_addr = { proc_registers_[word_idx_reg.as_reg].as_imm +
                             word_add_imm.as_imm };
    SArgument result = SArgument(mem_addr);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

SArgument CProcessor::proc_arg_mem_reg_reg_(const char* beg_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!beg_ptr)
        CRS_PROCESS_ERROR("proc_arg_mem_reg_reg_ error: beg_ptr is %p", beg_ptr)

    UWord word_idx_reg = get_word_(beg_ptr, 0);
    UWord word_add_reg = get_word_(beg_ptr, 1);
    SMemoryAddr mem_addr = { proc_registers_[word_idx_reg.as_reg].as_imm +
                             proc_registers_[word_idx_reg.as_reg].as_imm };
    SArgument result = SArgument(mem_addr);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

void CProcessor::set_flag_state_(EFlagMask flag_mask, bool state)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    uint32_t bit = 0;

    //could use bit = !!state (it's not guaranteed by standart to return 1, but it will)
    //could use bit = (state? 1 : 0) (but it's difficult to read code with ternary operators)
    if (state)
        bit = flag_mask;

    proc_registers_[ERegister::REG_SR].as_imm &= ~flag_mask;
    proc_registers_[ERegister::REG_SR].as_imm &= bit;

    CRS_IF_GUARD(CRS_END_CHECK();)
}

bool CProcessor::get_flag_state_(EFlagMask flag_mask)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    //use !! just for extra guarantee in case of inaccurate return value usage as single-bit value
    //but it gives no strong guarantees yet anyway
    bool result = !!(proc_registers_[ERegister::REG_SR].as_imm & flag_mask);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_hlt_helper_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    proc_registers_[ERegister::REG_IP].as_imm = instruction_pipe_.size();
    result = true;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

UWord CProcessor::proc_cmd_in_helper_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    UWord result = UWord();
    printf("enter number: ");
    scanf("%u", &result.as_imm);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_out_helper_(UWord word)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    printf("%u", word.as_imm);
    result = true;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_jump_helper_(SArgument arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    int32_t next_cmd_addr = 0;
    switch (arg.arg_type)
    {
        case EArgumentType::ARG_TYPE_IMM:
        {
            next_cmd_addr = static_cast<int32_t>(proc_registers_[ERegister::REG_PC].as_imm);
            next_cmd_addr += arg.arg_data.as_imm;
            result = true;
        }
            break;

        case EArgumentType::ARG_TYPE_REG:
        case EArgumentType::ARG_TYPE_MEM:
        {
            next_cmd_addr = static_cast<int32_t>(pull_word_(arg).as_imm);
            result = true;
        }
        break;

        default:
            CRS_PROCESS_ERROR("proc_cmd_jump_helper error: impossible jump type: %#X", arg.arg_type)
    }

    if (next_cmd_addr < 0 || next_cmd_addr >= instruction_pipe_.size())
        CRS_PROCESS_ERROR("proc_cmd_jump_helper error: next_cmd_addr %#X is out of bounds"
                          "[0, %#X]", next_cmd_addr, instruction_pipe_.size())

    if (result)
        proc_registers_[ERegister::REG_IP].as_imm = static_cast<uint32_t>(next_cmd_addr);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_call_helper_(SArgument arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    proc_call_stack_.push(proc_registers_[ERegister::REG_PC].as_imm + 1);
    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    result = proc_cmd_jump_helper_(arg);

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_loop_helper_(SArgument arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    int32_t next_cmd_addr = 0;
    if (arg.arg_type == EArgumentType::ARG_TYPE_IMM)
    {
        next_cmd_addr = arg.arg_data.as_imm;
        result = true;
    }
    else
        CRS_PROCESS_ERROR("proc_cmd_loop_helper_ error:"
                          "impossible loop type %#X", arg.arg_type)

    if (next_cmd_addr < 0 || next_cmd_addr >= instruction_pipe_.size())
        CRS_PROCESS_ERROR("proc_cmd_loop_helper_ error: next_cmd_addr %#X is out of bounds"
                          "[0, %#X]", next_cmd_addr, instruction_pipe_.size())

    if (result)
        proc_registers_[ERegister::REG_IP].as_imm = static_cast<uint32_t>(next_cmd_addr);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CProcessor::proc_cmd_ret_helper_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    proc_registers_[ERegister::REG_IP].as_imm = proc_call_stack_.pop();
    result = true;

    if (proc_registers_[ERegister::REG_IP].as_imm >= instruction_pipe_.size())
        CRS_PROCESS_ERROR("proc_cmd_ret_helper_ error: REG_IP %#X is out of bounds"
                          "[0, %#X]", proc_registers_[ERegister::REG_IP].as_imm, instruction_pipe_.size())

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}
//TODO: fix this dummy!
bool CProcessor::proc_interrupt_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;
    proc_registers_[ERegister::REG_IP].as_imm = instruction_pipe_.size();
    result = true;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

#define DECLARE_PROC_CMD(name, implementation) \
    bool CProcessor::proc_cmd_##name##_(SArgument lhs, SArgument rhs) \
    { \
        CRS_IF_GUARD(CRS_BEG_CHECK();) \
        \
        bool result = false; \
        implementation; \
        \
        CRS_IF_GUARD(hash_value_ = calc_hash_value_();) \
        CRS_IF_GUARD(CRS_END_CHECK();) \
        \
        return result; \
    }

//halt
    DECLARE_PROC_CMD(hlt, { result = proc_cmd_hlt_helper_(); })

//interaction
    DECLARE_PROC_CMD(in,  { result = push_word_(proc_cmd_in_helper_()); } )
    DECLARE_PROC_CMD(out, { result = proc_cmd_out_helper_(pop_word_()); })
    DECLARE_PROC_CMD(ok,  { result = ok(); })
    DECLARE_PROC_CMD(dump,{ result = ok(); dump(); })

//stack
    DECLARE_PROC_CMD(push, { result = push_word_(pull_word_(lhs)); })
    DECLARE_PROC_CMD(pop,  { result = move_word_(lhs, pop_word_()); })
    DECLARE_PROC_CMD(dup,  { UWord top = pop_word_();
                            result = push_word_(top) && push_word_(top); })

//memory
    DECLARE_PROC_CMD(mov, { result = move_word_(lhs, pull_word_(rhs)); })

//control-flow
    DECLARE_PROC_CMD(call, { result = proc_cmd_call_helper_(lhs); })
    DECLARE_PROC_CMD(ret,  { result = proc_cmd_ret_helper_(); })
    DECLARE_PROC_CMD(loop, { result = proc_cmd_loop_helper_(lhs); })

    #define CALL_JUMP_HELPER_IF(cond) \
        if (cond) result = proc_cmd_jump_helper_(lhs); \
        else result = false;

    #define CHECK_PROC_FLAG(flag, val) \
        (get_flag_state_(EFlagMask:: flag) == !!val)

    DECLARE_PROC_CMD(jmp, { CALL_JUMP_HELPER_IF(true); })
    DECLARE_PROC_CMD(jz,  { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_ZF, 1)); })
    DECLARE_PROC_CMD(jnz, { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_ZF, 0)); })
    DECLARE_PROC_CMD(je,  { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_ZF, 0)); })
    DECLARE_PROC_CMD(jne, { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_ZF, 0)); })
    DECLARE_PROC_CMD(jg,  { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_CF, 0) && CHECK_PROC_FLAG(FLAG_ZF, 0)); })
    DECLARE_PROC_CMD(jge, { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_CF, 0) || CHECK_PROC_FLAG(FLAG_ZF, 1)); })
    DECLARE_PROC_CMD(jl,  { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_CF, 1) && CHECK_PROC_FLAG(FLAG_ZF, 0)); })
    DECLARE_PROC_CMD(jle, { CALL_JUMP_HELPER_IF(CHECK_PROC_FLAG(FLAG_CF, 1) || CHECK_PROC_FLAG(FLAG_ZF, 1)); })

    #undef CHECK_PROC_FLAG

    #undef CALL_JUMP_HELPER_IF

//integer
    DECLARE_PROC_CMD(add, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm + pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(sub, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm - pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(mul, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm * pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(div, { uint32_t div = pull_word_(rhs).as_imm; if (!div) proc_interrupt_();
                            else result = move_word_(lhs, UWord(pull_word_(lhs).as_imm / div)); })
    DECLARE_PROC_CMD(mod, { uint32_t mod = pull_word_(rhs).as_imm; if (!mod) proc_interrupt_();
                            else result = move_word_(lhs, UWord(pull_word_(lhs).as_imm / mod)); })

    DECLARE_PROC_CMD(inc, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm + 1)); })
    DECLARE_PROC_CMD(dec, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm - 1)); })

    DECLARE_PROC_CMD(and, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm & pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(or,  { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm | pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(xor, { result = move_word_(lhs, UWord(pull_word_(lhs).as_imm ^ pull_word_(rhs).as_imm)); })
    DECLARE_PROC_CMD(inv, { result = move_word_(lhs, UWord(~pull_word_(lhs).as_imm)); })

    DECLARE_PROC_CMD(cmp, { ; })

//TODO: carry out to FPU
//TODO: replace NO_PARAM with PARAM
//floating-point
    DECLARE_PROC_CMD(fadd, { result = move_word_(lhs, UWord(pull_word_(lhs).as_flt + pull_word_(rhs).as_flt)); })
    DECLARE_PROC_CMD(fsub, { result = move_word_(lhs, UWord(pull_word_(lhs).as_flt - pull_word_(rhs).as_flt)); })
    DECLARE_PROC_CMD(fmul, { result = move_word_(lhs, UWord(pull_word_(lhs).as_flt * pull_word_(rhs).as_flt)); })
    DECLARE_PROC_CMD(fdiv, { result = move_word_(lhs, UWord(pull_word_(lhs).as_flt / pull_word_(rhs).as_flt)); })

    DECLARE_PROC_CMD(ftoi, { result = move_word_(lhs, UWord(static_cast<uint32_t>(pull_word_(lhs).as_flt))); })
    DECLARE_PROC_CMD(itof, { result = move_word_(lhs, UWord(static_cast<float>   (pull_word_(lhs).as_imm))); })

    DECLARE_PROC_CMD(fsin,  { result = move_word_(lhs, UWord(sinf (pull_word_(lhs).as_flt))); })
    DECLARE_PROC_CMD(fcos,  { result = move_word_(lhs, UWord(cosf (pull_word_(lhs).as_flt))); })
    DECLARE_PROC_CMD(fsqrt, { result = move_word_(lhs, UWord(sqrtf(pull_word_(lhs).as_flt))); })

    DECLARE_PROC_CMD(fcmp, { ; })

//Will be defined in the end of EArgument
//error
    //DECLARE_PROC_CMD(error_value, { result = proc_cmd_hlt_helper_(); })

#undef DECLARE_PROC_CMD

bool CProcessor::ok() const noexcept
{
    return (this && CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                        end_canary_ == CANARY_VALUE &&)
            CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_() &&) proc_stack_.ok() &&
            (proc_registers_[ERegister::REG_PC].as_imm <= instruction_pipe_.size() ||
             instruction_pipe_.size() == 0));
}

void CProcessor::dump() const noexcept
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
                    //"    program_counter_[%s] : %d \n"
                    "    \n"
                    CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                    "} \n",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                            CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                    proc_stack_.size(),

                    //TODO: add registers via include
                    proc_registers_[ERegister::REG_AX].as_imm,
                    proc_registers_[ERegister::REG_BX].as_imm,
                    proc_registers_[ERegister::REG_CX].as_imm,
                    proc_registers_[ERegister::REG_DX].as_imm,

                    //PROC_RAM_SIZE,

                    instruction_pipe_.size()//,
                    //(program_counter_ < instruction_pipe_.size() ? "OK" : "OUT_OF_RANGE"),
                    //program_counter_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
}

}//namespace course

#undef CRS_GUARD_LEVEL

#endif // PROCESSOR_H_INCLUDED

