#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include <cstdio>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cctype>

#include "../Stack/Logger.h"
#include "../Stack/CourseException.h"
#include "../Stack/Guard.h"

#include "../CPU/ProcessorEnums.h"

#include "FileView.h"

namespace course {

using namespace course_stack;

class CTranslator final
{
private:
    enum ETokenType
    {
        TOK_NONE = 0, TOK_IMM, TOK_REG, TOK_FLT, TOK_LBL
    };

    struct SToken
    {
        SToken(): tok_type(ETokenType::TOK_NONE), tok_data(UWord{}) {}

        SToken(ETokenType tok_type_set, UWord tok_data_set):
            tok_type(tok_type_set), tok_data(tok_data_set) {}

        ETokenType tok_type;
        UWord      tok_data;
    };

    class CLabelContainer
    {
    public:
        struct SLabelUsePos
        {
            uint32_t cmd_idx;
            char*    arg_ptr;
        };

        static const size_t MAX_LABEL_LEN = 64;

    public:
        CLabelContainer();

        CLabelContainer             (const CLabelContainer&) = delete;
        CLabelContainer& operator = (const CLabelContainer&) = delete;

        //TODO: to implement move-semantics ("rule of 5" dummy realisation)
        CLabelContainer             (CLabelContainer&&) = delete;
        CLabelContainer& operator = (CLabelContainer&&) = delete;

        ~CLabelContainer();

        void     push_label_declare (const std::string& label_name, uint32_t label_position);
        uint32_t push_label_use_name(const std::string& label_name);
        void     push_label_use_pos (SLabelUsePos label_use_pos);

        void replace_bytes();

    private:
        std::vector<SLabelUsePos>                              replace_container_;
        std::vector<std::map<std::string, uint32_t>::iterator> label_use_container_;
        std::map<std::string, uint32_t>                        label_declare_container_;
    };

    static const size_t MAX_PATTERN_STR_LEN = 128;

    static const size_t CANARY_VALUE = "CTranslator"_crs_hash;

public:
    CTranslator(const char* input_file_name, const char* output_file_name);

    CTranslator             (const CTranslator&) = delete;
    CTranslator& operator = (const CTranslator&) = delete;

    //TODO: to implement move-semantics ("rule of 5" dummy realisation)
    CTranslator             (CTranslator&&) = delete;
    CTranslator& operator = (CTranslator&&) = delete;

    ~CTranslator();

private:
    [[nodiscard]] size_t calc_hash_value_() const;

public:
    void parse_input();

private:
    void shift_and_pass_spaces_(size_t shift = 1);

    void write_args_(char* cmd_pos, ECommand cmd, EArgument lhs_arg, EArgument rhs_arg);
    void write_word_(UWord word);

    bool parse_cmd_(ECommand* cmd_ptr);
    bool parse_lbl_();

    bool parse_all_arg_(EArgument* arg_ptr);
    bool parse_mov_arg_(EArgument* arg_ptr);
    bool parse_reg_arg_(EArgument* arg_ptr);
    bool parse_mem_arg_(EArgument* arg_ptr);
    bool parse_imm_arg_(EArgument* arg_ptr);
    bool parse_lbl_arg_(EArgument* arg_ptr);
    bool parse_flt_arg_(EArgument* arg_ptr);
    bool parse_nul_arg_(EArgument* arg_ptr);

public:
    [[nodiscard]] bool ok() const;

    void dump() const;

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    CFileView input_file_view_;
    CFileView output_file_view_;

    const char* cur_in_pos_;
    char*       cur_out_pos_;

    std::vector<const char*> command_pos_container_;

    CLabelContainer label_container_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

CTranslator::CLabelContainer::CLabelContainer():
        replace_container_      (),
        label_use_container_    (),
        label_declare_container_()
{}

CTranslator::CLabelContainer::~CLabelContainer()
{
    replace_container_.clear();

    label_use_container_    .clear();
    label_declare_container_.clear();
}

void CTranslator::CLabelContainer::push_label_declare(const std::string& label_name, uint32_t label_position)
{
    if (label_name.size() >= MAX_LABEL_LEN)
        CRS_PROCESS_ERROR("push_label_declare: "
                          "error: name length is %zu >= (%zu == MAX_LABEL_LEN)",
                          label_name.size(), MAX_LABEL_LEN)

    auto pos_iter = label_declare_container_.find(label_name);

    if (pos_iter == label_declare_container_.end())
    {
        label_declare_container_.insert({label_name, label_position});
    }
    else if (pos_iter->second == static_cast<uint32_t>(-1))
    {
        pos_iter->second = label_position;
    }
    else CRS_PROCESS_ERROR("push_label_declare: "
                           "error: label \"%.*s\" redeclaration",
                           static_cast<int>(MAX_LABEL_LEN), label_name.c_str())
}

uint32_t CTranslator::CLabelContainer::push_label_use_name(const std::string& label_name)
{
    if (label_name.size() >= MAX_LABEL_LEN)
        CRS_PROCESS_ERROR("push_label_use_name: "
                          "error: name length is %zu >= (%zu == MAX_LABEL_LEN)",
                          label_name.size(), MAX_LABEL_LEN)

    auto pos_iter = label_declare_container_.find(label_name);

    if (pos_iter == label_declare_container_.end())
    {
        auto insert_result = label_declare_container_.insert({label_name, static_cast<uint32_t>(-1)});//TODO:
        pos_iter = insert_result.first;

        if (!insert_result.second)
            CRS_PROCESS_ERROR("push_label_use_name: "
                              "error: reinsertion of label \"%.*s\"",
                              static_cast<int>(MAX_LABEL_LEN), label_name.c_str())
    }

    label_use_container_.push_back(pos_iter);

    return label_use_container_.size() - 1;
}

void CTranslator::CLabelContainer::push_label_use_pos(SLabelUsePos label_use_pos)
{
    replace_container_.push_back(label_use_pos);
}

void CTranslator::CLabelContainer::replace_bytes()
{
    for (const SLabelUsePos& label_use_pos : replace_container_)
    {
        uint32_t label_idx = 0;
        memcpy(&label_idx, label_use_pos.arg_ptr, sizeof(uint32_t));

        if (label_idx >= label_use_container_.size())
            CRS_PROCESS_ERROR("replace_bytes: "
                              "error: label index %d is out of range", label_idx)

        uint32_t label_pos = label_use_container_[label_idx]->second;

        if (label_pos != static_cast<uint32_t>(-1))
        {
            int32_t rel_offset = label_pos - label_use_pos.cmd_idx;//must be signed
            memcpy(label_use_pos.arg_ptr, &rel_offset, sizeof(rel_offset));
        }
        else CRS_PROCESS_ERROR("replace_bytes: "
                               "error: undeclared label \"%.*s\" usage",
                               static_cast<int>(MAX_LABEL_LEN), label_use_container_[label_idx]->first.c_str())
    }
}

CTranslator::CTranslator(const char* input_file_name, const char* output_file_name) :
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        input_file_view_ (ECMapMode::MAP_READONLY_FILE,  input_file_name),
        output_file_view_(ECMapMode::MAP_WRITEONLY_FILE, output_file_name, 4*input_file_view_.get_file_view_size()),

        cur_in_pos_ (nullptr),
        cur_out_pos_(nullptr),

        command_pos_container_(),
        label_container_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    cur_in_pos_  = input_file_view_ .get_file_view_str();
    cur_out_pos_ = output_file_view_.get_file_view_str();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
}

CTranslator::~CTranslator()
{
    CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

    CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
    CRS_IF_HASH_GUARD  (hash_value_ = 0;)

    cur_in_pos_ = nullptr;

    command_pos_container_.clear();
}

size_t CTranslator::calc_hash_value_() const
{
    size_t result = 0;
    CRS_IF_CANARY_GUARD(result ^= (beg_canary_ ^ end_canary_));

    result ^= input_file_view_ .get_file_view_size() ^
              output_file_view_.get_file_view_size();

    result ^= reinterpret_cast<uintptr_t>(cur_in_pos_) ^
              reinterpret_cast<uintptr_t>(cur_out_pos_);

    for (size_t i = 0; i < command_pos_container_.size(); i++)
        result ^= static_cast<size_t>(static_cast<uint8_t>(*command_pos_container_[i]) << (i%sizeof(size_t)));

    return result;
}

void CTranslator::parse_input()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    while (std::isspace(*cur_in_pos_))
        cur_in_pos_++;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    while (*cur_in_pos_)
    {
        ECommand cmd_type = ECommand::CMD_ERR_VALUE;
        if (parse_cmd_(&cmd_type))
            continue;
        else if (parse_lbl_())
            continue;
        else if (*cur_in_pos_ == '\n' || *cur_in_pos_ == '\r')
        {
            while (std::isspace(*cur_in_pos_))
                cur_in_pos_++;

            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
        }
        else if (*cur_in_pos_ == '\0')
            break;
        else
            CRS_PROCESS_ERROR("unexpected token instead of command or label: %c", *cur_in_pos_)
    }

    label_container_.replace_bytes();

    write_word_(static_cast<uint32_t>(ECommand::CMD_ERR_VALUE));

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::shift_and_pass_spaces_(size_t shift)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    cur_in_pos_ += shift;

    while (*cur_in_pos_ == ' ' || *cur_in_pos_ == '\t')
        cur_in_pos_++;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::write_args_(char* cmd_pos, ECommand cmd, EArgument lhs_arg, EArgument rhs_arg)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    CRS_STATIC_LOG("write args: cmd=%#X lhs_arg=%#X rhs_arg=%#X", cmd, lhs_arg, rhs_arg);

    if (cmd_pos - output_file_view_.get_file_view_str() >=
        output_file_view_.get_file_view_size())
        CRS_PROCESS_ERROR("write_args_ : cmd_pos is out of bounds: offset: %zu, size: %zu",
                          cmd_pos - output_file_view_.get_file_view_str(),
                          output_file_view_.get_file_view_size())

    UWord word = UWord(SCommand(cmd, lhs_arg, rhs_arg));

    #undef LHS_OFFSET
    #undef RHS_OFFSET

    CRS_CHECK_MEM_OPER(memcpy(cmd_pos, &word, sizeof(UWord)))//will be optimised for each level from -O1

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::write_word_(UWord word)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    CRS_STATIC_LOG("write word: %#X", word.as_imm);

    CRS_CHECK_MEM_OPER(memcpy(cur_out_pos_, &word, sizeof(UWord)))//will be optimised for each level from -O1
    cur_out_pos_ += sizeof(UWord);

    if (cur_out_pos_ - output_file_view_.get_file_view_str() >=
        output_file_view_.get_file_view_size())
        CRS_PROCESS_ERROR("write_word_ : cur_out_pos is out of bounds: offset: %zu, size: %zu",
                          cur_out_pos_ - output_file_view_.get_file_view_str(),
                          output_file_view_.get_file_view_size())

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

bool CTranslator::parse_cmd_(ECommand* cmd_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!cmd_ptr)
        CRS_PROCESS_ERROR("parse_cmd_ error: cmd_ptr is %p", cmd_ptr)

    bool result = false;

    #define HANDLE_COMMAND_(CMD_ENUM_NAME_, CMD_CODE_, CMD_NAME_, CMD_LHS_, CMD_RHS_) \
        else if (!strncmp(cur_in_pos_, CRS_STRINGIZE(CMD_NAME_), sizeof(CRS_STRINGIZE(CMD_NAME_))-1) && \
                 !std::isalnum(cur_in_pos_[sizeof(CRS_STRINGIZE(CMD_NAME_))-1])) \
        { \
            CRS_STATIC_MSG("parse_command: " CRS_STRINGIZE(CMD_NAME_) " command detected"); \
            \
            *cmd_ptr = ECommand:: CMD_ENUM_NAME_; \
            \
            command_pos_container_.push_back(cur_out_pos_); \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            char* cmd_word_pos = cur_out_pos_; \
            write_word_(UWord(SCommand(ECommand:: CMD_ENUM_NAME_, EArgument::ARG_ERR_VALUE, EArgument::ARG_ERR_VALUE))); \
            shift_and_pass_spaces_(sizeof(CRS_STRINGIZE(CMD_NAME_))-1); \
            \
            EArgument lhs_arg = EArgument::ARG_ERR_VALUE; \
            EArgument rhs_arg = EArgument::ARG_ERR_VALUE; \
            \
            if (!parse_##CMD_LHS_##_(&lhs_arg)) \
                CRS_PROCESS_ERROR("parse_command: " CRS_STRINGIZE(CMD_NAME_) \
                                  " left arg parse error on symbol %c", *cur_in_pos_) \
            if (!parse_##CMD_RHS_##_(&rhs_arg)) \
                CRS_PROCESS_ERROR("parse_command: " CRS_STRINGIZE(CMD_NAME_) \
                                  " right arg parse error on symbol %c", *cur_in_pos_) \
            \
            write_args_(cmd_word_pos, ECommand:: CMD_ENUM_NAME_, lhs_arg, rhs_arg); \
            \
            result = true; \
        }

    if (*cur_in_pos_ == '\0')
        CRS_STATIC_MSG("parse_command: end of file reached");

        #include "../EnumLists/CommandList.h"

    else
    {
        *cmd_ptr = ECommand::CMD_ERR_VALUE;

        result = false;
    }

    #undef HANDLE_COMMAND_

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_lbl_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    const char* temp_pos = cur_in_pos_;

    if (std::isalpha(*temp_pos))
    {
        while (std::isalnum(*temp_pos))
            temp_pos++;

        std::string label_name(cur_in_pos_, temp_pos - cur_in_pos_);

        while (*temp_pos == ' ' || *temp_pos == '\t')
            temp_pos++;

        if (*temp_pos == ':')
        {
            label_container_.push_label_declare(label_name, command_pos_container_.size());

            ++temp_pos;
            shift_and_pass_spaces_(temp_pos - cur_in_pos_);

            result = true;
        }
        else
            result = false;
    }
    else
        result = false;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_all_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_all_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    if (parse_imm_arg_(arg_ptr))
        result = true;
    else if (parse_mov_arg_(arg_ptr))
        result = true;
    else if (parse_lbl_arg_(arg_ptr))
        result = true;
    else
        result = false;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_mov_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_mov_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    if (parse_reg_arg_(arg_ptr))
        result = true;
    else if (parse_mem_arg_(arg_ptr))
        result = true;
    else
        result = false;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_reg_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_reg_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    #define HANDLE_REGISTER_(REG_ENUM_NAME_, REG_NAME_) \
        else if (!strncmp(cur_in_pos_, REG_NAME_, sizeof(REG_NAME_)-1)) \
        { \
            write_word_(UWord(ERegister:: REG_ENUM_NAME_)); \
            shift_and_pass_spaces_(sizeof(REG_NAME_)-1); \
            \
            *arg_ptr = EArgument::ARG_REG; \
            result = true; \
        }

    if (*cur_in_pos_ == '\0')
        CRS_STATIC_MSG("parse_reg_: end of file reached");

        #include "../EnumLists/RegisterList.h"

    else
        result = false;

    if (!result)
        *arg_ptr = EArgument::ARG_ERR_VALUE;

    #undef HANDLE_REGISTER_

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_mem_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_mem_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    if (*cur_in_pos_ == '[')
    {
        shift_and_pass_spaces_(1);

        EArgument lhs_arg_type = EArgument::ARG_ERR_VALUE;
        EArgument rhs_arg_type = EArgument::ARG_ERR_VALUE;

        if (parse_imm_arg_(&lhs_arg_type))
        {
            *arg_ptr = EArgument::ARG_MEM_IMM;
            result = true;
        }
        else if (parse_reg_arg_(&lhs_arg_type))
        {
            *arg_ptr = EArgument::ARG_MEM_REG;
            result = true;
        }
        else
            CRS_PROCESS_ERROR("parse_mem_arg_ error: unexpected symbol instead of imm or reg: %c", *cur_in_pos_)

        if (*cur_in_pos_ == '+' && *arg_ptr == ARG_MEM_REG)
        {
            shift_and_pass_spaces_(1);

            if (parse_imm_arg_(&rhs_arg_type))
            {
                *arg_ptr = EArgument::ARG_MEM_REG_IMM;
                result = true;
            }
            else if (parse_reg_arg_(&rhs_arg_type))
            {
                *arg_ptr = EArgument::ARG_MEM_REG_REG;
                result = true;
            }
            else
                CRS_PROCESS_ERROR("parse_mem_arg_ error: unexpected symbol instead of '+' or ']': %c", *cur_in_pos_)
        }

        if (*cur_in_pos_ == ']')
            shift_and_pass_spaces_(1);
        else
            CRS_PROCESS_ERROR("parse_mem_arg_ error: unexpected symbol instead of ']': %c", *cur_in_pos_)
    }
    else
        result = false;

    if (!result)
        *arg_ptr = EArgument::ARG_ERR_VALUE;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_imm_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_imm_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    char* end_ptr = nullptr;
    uint32_t imm_val = std::strtoul(cur_in_pos_, &end_ptr, 0);

    if (end_ptr == cur_in_pos_)
        result = false;
    else if (*end_ptr == '.' || *end_ptr == 'e' || *end_ptr == 'E')
        result = false;
    else
    {
        write_word_(UWord(imm_val));
        shift_and_pass_spaces_(end_ptr - cur_in_pos_);

        *arg_ptr = EArgument::ARG_IMM;
        result = true;
    }

    if (!result)
        *arg_ptr = EArgument::ARG_ERR_VALUE;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_lbl_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_lbl_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    if (*cur_in_pos_ == '\0')
        CRS_STATIC_MSG("parse_lbl_arg_: end of file reached");
    else if (std::isalpha(*cur_in_pos_))
    {
        const char* temp_pos = cur_in_pos_;

        while (isalnum(*temp_pos)) temp_pos++;

        std::string label_name(cur_in_pos_, temp_pos - cur_in_pos_);
        shift_and_pass_spaces_(temp_pos - cur_in_pos_);

        uint32_t lbl_idx = label_container_.push_label_use_name(label_name);

        CLabelContainer::SLabelUsePos use_pos = {};
        use_pos.cmd_idx = static_cast<uint32_t>(command_pos_container_.size()-1);
        use_pos.arg_ptr = cur_out_pos_;

        label_container_.push_label_use_pos(use_pos);
        write_word_(UWord(lbl_idx));

        *arg_ptr = EArgument::ARG_LBL;
        result = true;
    }
    else
        result = false;

    if (!result)
        *arg_ptr = EArgument::ARG_ERR_VALUE;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_flt_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_flt_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = false;

    char* end_ptr = nullptr;
    float flt_val = std::strtof(cur_in_pos_, &end_ptr);

    if (*cur_in_pos_ == '\0')
        CRS_STATIC_MSG("parse_flt_arg_: end of file reached");
    else if (end_ptr == cur_in_pos_)
        result = false;
    else
    {
        write_word_(UWord(flt_val));
        shift_and_pass_spaces_(end_ptr - cur_in_pos_);

        *arg_ptr = EArgument::ARG_FLT;
        result = true;
    }

    if (!result)
        *arg_ptr = EArgument::ARG_ERR_VALUE;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::parse_nul_arg_(EArgument* arg_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!arg_ptr)
        CRS_PROCESS_ERROR("parse_nul_arg_ error: arg_ptr is %p", arg_ptr)

    bool result = true;
    *arg_ptr = EArgument::ARG_NUL;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CTranslator::ok() const
{
    return (this && CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                        end_canary_ == CANARY_VALUE &&)

            input_file_view_ .get_file_view_size() &&
            output_file_view_.get_file_view_size() &&
            cur_in_pos_ && cur_out_pos_

            CRS_IF_HASH_GUARD(&& hash_value_ == calc_hash_value_()));
}

void CTranslator::dump() const
{
    CRS_STATIC_DUMP("CTranslator[%s, this : %p] \n"
                    "{ \n"
                    CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                    CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                    "    \n"
                    "    input_file_view_ :  \n"
                    "        size : %d \n"
                    "    output_file_view_ : \n"
                    "        size : %d \n"
                    "    cur_in_pos_  : %p \n"
                    "    cur_out_pos_ : %p \n"
                    "    \n"
                    CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                    "} \n",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                    CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                    input_file_view_ .get_file_view_size(),
                    output_file_view_.get_file_view_size(),

                    cur_in_pos_,
                    cur_out_pos_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
}

}//namespace course

#endif // TRANSLATOR_H_INCLUDED
