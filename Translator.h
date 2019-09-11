#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include <cstdio>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cctype>

#include "Stack/Logger.h"
#include "Stack/CourseException.h"
#include "Stack/Guard.h"

#include "ProcessorEnums.h"

#include "TranslatorFiles/FileView.h"

namespace course {

using namespace course_stack;

class CTranslator
{
private:
    enum ETokenType
    {
        TOK_NONE = 0, TOK_NUM, TOK_IDX, TOK_REG, TOK_LBL
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
    void write_word_(UWord word);

    SToken                    parse_token_();
    ETokenType                parse_command_();
    void                      parse_label_();
    std::pair<SToken, SToken> parse_bracket_();

    void parse_call_args_(const char pattern_str[MAX_PATTERN_STR_LEN]);
    void parse_jump_args_(const char pattern_str[MAX_PATTERN_STR_LEN]);
    void parse_push_args_(const char pattern_str[MAX_PATTERN_STR_LEN]);
    void parse_pop_args_ (const char pattern_str[MAX_PATTERN_STR_LEN]);

#define DECLARE_JUMP_PARSE_ARGS_(name) \
    void parse_##name##_args_(const char pattern_str[MAX_PATTERN_STR_LEN]);

    DECLARE_JUMP_PARSE_ARGS_(jmp)
    DECLARE_JUMP_PARSE_ARGS_(jz )
    DECLARE_JUMP_PARSE_ARGS_(jnz)
    DECLARE_JUMP_PARSE_ARGS_(je )
    DECLARE_JUMP_PARSE_ARGS_(jne)
    DECLARE_JUMP_PARSE_ARGS_(jg )
    DECLARE_JUMP_PARSE_ARGS_(jge)
    DECLARE_JUMP_PARSE_ARGS_(jl )
    DECLARE_JUMP_PARSE_ARGS_(jle)

#undef DECLARE_JUMP_PARSE_ARGS_

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

    while (std::isspace(*cur_in_pos_)) cur_in_pos_++;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    while (*cur_in_pos_)
    {
        ETokenType type = parse_command_();

        if (type == ETokenType::TOK_LBL) continue;

        if (*cur_in_pos_ == '\n' || *cur_in_pos_ == '\r')
        {
            while (std::isspace(*cur_in_pos_))
                cur_in_pos_++;

            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
        }
        else if (*cur_in_pos_ == '\0')
            break;

        else CRS_PROCESS_ERROR("parse_input: "
                               "error: new line expected before \"%.16s\"",
                               cur_in_pos_)
    }

    label_container_.replace_bytes();

    write_word_(static_cast<uint32_t>(ECommand::CMD_NULL_TERMINATOR));

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

void CTranslator::write_word_(UWord word)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    memcpy(cur_out_pos_, &word, sizeof(UWord));//will be optimised for each level from -O1
    cur_out_pos_ += sizeof(UWord);

    if (cur_out_pos_ - output_file_view_.get_file_view_str() >=
        output_file_view_.get_file_view_size())
        CRS_PROCESS_ERROR("write_word_ : cur_out_pos is out of bounds: offset: %zu, size: %zu",
                          cur_out_pos_ - output_file_view_.get_file_view_str(),
                          output_file_view_.get_file_view_size())

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

CTranslator::SToken CTranslator::parse_token_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SToken result = {};

    if (std::isdigit(*cur_in_pos_) || *cur_in_pos_ == '.' ||
        *cur_in_pos_ == '+' || *cur_in_pos_ == '-')
    {
        const char* temp_pos = cur_in_pos_ + 1;

        while (std::isdigit(*temp_pos)) temp_pos++;

        if (*temp_pos == '.')
        {
            temp_pos++;
            while (std::isdigit(*temp_pos)) temp_pos++;

            sscanf(cur_in_pos_, "%f", &result.tok_data.val);
            result.tok_type = ETokenType::TOK_NUM;

            shift_and_pass_spaces_(temp_pos - cur_in_pos_);
        }
        else
        {
            sscanf(cur_in_pos_, "%d", &result.tok_data.idx);
            result.tok_type = ETokenType::TOK_IDX;

            shift_and_pass_spaces_(temp_pos - cur_in_pos_);
        }
    }
    else if (std::isalpha(*cur_in_pos_))
    {
        #define HANDLE_REGISTER_(regcode, name) \
            else if (!strncmp(cur_in_pos_, name, sizeof(name)-1)) \
            { \
                shift_and_pass_spaces_(sizeof(name)-1); \
                \
                result.tok_type = ETokenType::TOK_REG; \
                result.tok_data = UWord(static_cast<uint32_t>(regcode)); \
            }

        if (*cur_in_pos_ == '\0')
            CRS_STATIC_MSG("parse_token: end of file reached");

            #include "RegistersList.h"

        else
        {
            const char* temp_pos = cur_in_pos_;

            while (isalnum(*temp_pos)) temp_pos++;

            std::string label_name(cur_in_pos_, temp_pos - cur_in_pos_);
            uint32_t label_index = label_container_.push_label_use_name(label_name);

            shift_and_pass_spaces_(temp_pos - cur_in_pos_);

            result.tok_type = ETokenType::TOK_LBL;
            result.tok_data = UWord(label_index);//must be registered and replaced before writing into file
        }

        #undef HANDLE_REGISTER_
    }
    else CRS_PROCESS_ERROR("parse_token_: unrecognizable token \"%.16s\"", cur_in_pos_)

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

std::pair<CTranslator::SToken, CTranslator::SToken> CTranslator::parse_bracket_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    std::pair<SToken, SToken> result = std::make_pair(SToken(), SToken());

    if (*cur_in_pos_ != '[')
        CRS_PROCESS_ERROR("handle_bracket_: error: '[' missed before: \"%.16s\"", cur_in_pos_)

    shift_and_pass_spaces_();

    result.first = parse_token_();

    if (*cur_in_pos_ == '+')
    {
        shift_and_pass_spaces_();
        result.second = parse_token_();
    }
    else if (*cur_in_pos_ != ']')
        CRS_PROCESS_ERROR("handle_bracket_: error: ']' missed before: \"%.16s\"", cur_in_pos_)

    shift_and_pass_spaces_();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

void CTranslator::parse_call_args_(const char* pattern_str)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SToken arg = {};
    ECallMode mode = {};

    if (*cur_in_pos_ == '[')
    {
        auto bracket_args = parse_bracket_();

        arg = bracket_args.first;

        switch (bracket_args.first.tok_type)
        {
            case ETokenType::TOK_IDX: mode = ECallMode::CALL_RAM;     break;
            case ETokenType::TOK_REG: mode = ECallMode::CALL_RAM_REG; break;

            default: CRS_PROCESS_ERROR("handle_call_args_: invalid ram request argument: "
                                       "tok_type: %#x", arg.tok_type)
        }

        if (bracket_args.second.tok_type != ETokenType::TOK_NONE)
            CRS_PROCESS_ERROR("handle_call_args_: invalid ram request argument: "
                              "tok_type: %#x", arg.tok_type)
    }
    else
    {
        arg = parse_token_();

        switch (arg.tok_type)
        {
            case ETokenType::TOK_IDX:
            case ETokenType::TOK_LBL: mode = ECallMode::CALL_REL; break;
            case ETokenType::TOK_REG: mode = ECallMode::CALL_REG; break;

            default:
                CRS_PROCESS_ERROR("handle_call_args_: invalid argument tok_type: %#x", arg.tok_type)
            break;
        }
    }

    write_word_(UWord(static_cast<uint32_t>(mode)));

    if (arg.tok_type == ETokenType::TOK_LBL)
    {
        label_container_.push_label_use_pos({static_cast<uint32_t>(command_pos_container_.size()-1), cur_out_pos_});
        write_word_(arg.tok_data);
    }
    else
    {
        write_word_(arg.tok_data);
    }

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::parse_jump_args_(const char* pattern_str)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SToken arg = {};
    EJumpMode mode = {};

    if (*cur_in_pos_ == '[')
    {
        auto bracket_args = parse_bracket_();

        arg = bracket_args.first;

        switch (bracket_args.first.tok_type)
        {
            case ETokenType::TOK_IDX: mode = EJumpMode::JUMP_RAM;     break;
            case ETokenType::TOK_REG: mode = EJumpMode::JUMP_RAM_REG; break;

            default: CRS_PROCESS_ERROR("handle_jump_args_: invalid ram request argument: "
                                       "tok_type: %#x", arg.tok_type)
        }

        if (bracket_args.second.tok_type != ETokenType::TOK_NONE)
            CRS_PROCESS_ERROR("handle_jump_args_: invalid ram request argument: "
                              "tok_type: %#x", arg.tok_type)
    }
    else
    {
        arg = parse_token_();

        switch (arg.tok_type)
        {
            case ETokenType::TOK_IDX:
            case ETokenType::TOK_LBL: mode = EJumpMode::JUMP_REL; break;
            case ETokenType::TOK_REG: mode = EJumpMode::JUMP_REG; break;

            default:
                CRS_PROCESS_ERROR("handle_jump_args_: invalid argument tok_type: %#x", arg.tok_type)
            break;
        }
    }

    write_word_(UWord(static_cast<uint32_t>(mode)));

    if (arg.tok_type == ETokenType::TOK_LBL)
    {
        label_container_.push_label_use_pos({static_cast<uint32_t>(command_pos_container_.size()-1), cur_out_pos_});
        write_word_(arg.tok_data);
    }
    else
    {
        write_word_(arg.tok_data);
    }

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::parse_push_args_(const char* pattern_str)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SToken arg = {}, add = {};
    EPushMode mode = {};

    if (*cur_in_pos_ == '[')
    {
        auto bracket_args = parse_bracket_();

        arg = bracket_args.first;
        add = bracket_args.second;

        if      (arg.tok_type == ETokenType::TOK_IDX &&
                 add.tok_type == ETokenType::TOK_NONE) mode = EPushMode::PUSH_RAM;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_NONE) mode = EPushMode::PUSH_RAM_REG;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_IDX)  mode = EPushMode::PUSH_RAM_REG_NUM;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_REG)  mode = EPushMode::PUSH_RAM_REG_REG;

        else CRS_PROCESS_ERROR("parse_push_args_: error: invalid argument types: "
                               "arg tok_type: %#x, add tok_type: %#x",
                               arg.tok_type, add.tok_type)
    }
    else
    {
        arg = parse_token_();

        if      (arg.tok_type == ETokenType::TOK_NUM) mode = EPushMode::PUSH_NUM;
        else if (arg.tok_type == ETokenType::TOK_REG) mode = EPushMode::PUSH_REG;

        else CRS_PROCESS_ERROR("parse_push_args_: error: invalid argument types: "
                               "arg tok_type: %#x, add tok_type: %#x",
                               arg.tok_type, add.tok_type)
    }

    write_word_(UWord(static_cast<uint32_t>(mode)));

    if (arg.tok_type != ETokenType::TOK_NONE) write_word_(arg.tok_data);
    if (add.tok_type != ETokenType::TOK_NONE) write_word_(add.tok_data);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::parse_pop_args_(const char* pattern_str)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    SToken arg = {}, add = {};
    EPopMode mode = {};

    if (*cur_in_pos_ == '[')
    {
        auto bracket_args = parse_bracket_();

        arg = bracket_args.first;
        add = bracket_args.second;

        if      (arg.tok_type == ETokenType::TOK_IDX &&
                 add.tok_type == ETokenType::TOK_NONE) mode = EPopMode::POP_RAM;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_NONE) mode = EPopMode::POP_RAM_REG;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_IDX)  mode = EPopMode::POP_RAM_REG_NUM;
        else if (arg.tok_type == ETokenType::TOK_REG &&
                 add.tok_type == ETokenType::TOK_REG)  mode = EPopMode::POP_RAM_REG_REG;

        else CRS_PROCESS_ERROR("handle_pop_args_: error: invalid argument types: "
                               "arg tok_type: %#x, add tok_type: %#x",
                               arg.tok_type, add.tok_type)
    }
    else
    {
        arg = parse_token_();

        if (arg.tok_type == ETokenType::TOK_REG) mode = EPopMode::POP_REG;

        else CRS_PROCESS_ERROR("handle_pop_args_: error: invalid argument types: "
                               "arg tok_type: %#x, add tok_type: %#x",
                               arg.tok_type, add.tok_type)
    }

    write_word_(UWord(static_cast<uint32_t>(mode)));

    if (arg.tok_type != ETokenType::TOK_NONE) write_word_(arg.tok_data);
    if (add.tok_type != ETokenType::TOK_NONE) write_word_(add.tok_data);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

void CTranslator::parse_label_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    const char* temp_pos = cur_in_pos_;

    while (std::isalnum(*temp_pos)) temp_pos++;
    while (std::isspace(*temp_pos)) temp_pos++;

    std::string label_name(cur_in_pos_, temp_pos - cur_in_pos_);
    label_container_.push_label_declare(label_name, command_pos_container_.size());

    if (*temp_pos == ':') temp_pos++;
    else CRS_PROCESS_ERROR("parse_label_: error: ':' missed after \"%.16s\"", label_name.c_str())

    shift_and_pass_spaces_(temp_pos - cur_in_pos_);

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

CTranslator::ETokenType CTranslator::parse_command_()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    ETokenType result = ETokenType::TOK_NONE;

    #define NO_PARAM_PARSE_ARGS_(name, pattern)
    #define PARAM_PARSE_ARGS_(name, pattern) parse_##name##_args_(pattern);

    #define HANDLE_COMMAND_(opcode, name, parametered, pattern) \
        else if (!strncmp(cur_in_pos_, CRS_STRINGIZE(name), sizeof(CRS_STRINGIZE(name))-1) && \
                 !std::isalnum(cur_in_pos_[sizeof(CRS_STRINGIZE(name))-1])) \
        { \
            CRS_STATIC_MSG("parse_command: " CRS_STRINGIZE(name) " command detected"); \
            \
            command_pos_container_.push_back(cur_out_pos_); \
            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();) \
            \
            write_word_(UWord(static_cast<uint32_t>(opcode))); \
            shift_and_pass_spaces_(sizeof(CRS_STRINGIZE(name))-1); \
            \
            parametered##_PARSE_ARGS_(name, pattern) \
        }

    if (*cur_in_pos_ == '\0')
        CRS_STATIC_MSG("parse_command: end of file reached");

        #include "CommandList.h"

    else if (isalpha(*cur_in_pos_))
    {
        parse_label_();

        result = ETokenType::TOK_LBL;
    }

    else CRS_PROCESS_ERROR("parse_command: unrecognizable command: \"%.16s\"", cur_in_pos_)

    #undef HANDLE_COMMAND_

    #undef NO_PARAM_PARSE_ARGS_
    #undef PARAM_PARSE_ARGS_

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

#define DECLARE_JUMP_PARSE_ARGS_(name) \
    void CTranslator::parse_##name##_args_(const char pattern_str[MAX_PATTERN_STR_LEN]) \
    { \
        CRS_IF_GUARD(CRS_BEG_CHECK();) \
        \
        parse_jump_args_(pattern_str); \
        \
        CRS_IF_GUARD(CRS_END_CHECK();) \
    }

DECLARE_JUMP_PARSE_ARGS_(jmp)
DECLARE_JUMP_PARSE_ARGS_(jz )
DECLARE_JUMP_PARSE_ARGS_(jnz)
DECLARE_JUMP_PARSE_ARGS_(je )
DECLARE_JUMP_PARSE_ARGS_(jne)
DECLARE_JUMP_PARSE_ARGS_(jg )
DECLARE_JUMP_PARSE_ARGS_(jge)
DECLARE_JUMP_PARSE_ARGS_(jl )
DECLARE_JUMP_PARSE_ARGS_(jle)

#undef DECLARE_JUMP_PARSE_ARGS_

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
