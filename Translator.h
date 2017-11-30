#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include <cstdio>
#include <vector>
#include <cstring>
#include <cctype>

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
        TOK_NONE = 0, TOK_NUM, TOK_IDX, TOK_REG,
    };

    struct SToken
    {
        SToken(): tok_type(ETokenType::TOK_NONE), tok_data(UWord{}) {}

        SToken(ETokenType tok_type_set, UWord tok_data_set):
            tok_type(tok_type_set), tok_data(tok_data_set) {}

        ETokenType tok_type;
        UWord      tok_data;
    };

    struct SCommandArgs
    {
        SCommandArgs() = default;
        SCommandArgs(SToken arg_set, SToken add_set):
            arg(arg_set), add(add_set) {}

        SToken arg, add;
    };

    static const size_t CANARY_VALUE = "CTranslator"_crs_hash;

public:
    explicit CTranslator(const char* input_file_name):
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        //input_file_view_ (ECMapMode::MAP_READONLY_FILE, input_file_name),
        //output_file_view_(ECMapMode::MAP_READONLY_FILE, "asm/executable"),

        cur_pos_        (nullptr),
        instruction_vec_()

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        //cur_pos_ = input_file_view_.get_file_view_str();
        cur_pos_ = nullptr;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    CTranslator             (const CTranslator&) = delete;
    CTranslator& operator = (const CTranslator&) = delete;

    ~CTranslator()
    {
        CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

        CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
        CRS_IF_HASH_GUARD  (hash_value_ = 0;)

        instruction_vec_.clear();
        cur_pos_ = nullptr;
    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = reinterpret_cast<uintptr_t>(cur_pos_) ^ (instruction_vec_.size() << 0x8);
        CRS_IF_CANARY_GUARD(result ^= (beg_canary_ ^ end_canary_));

        for (const auto& instruction : instruction_vec_)
            result ^= (static_cast<uint32_t>(instruction.command) << 0x10) ^
                      (static_cast<uint32_t>(instruction.mode)    << 0x10) ^
                      instruction.arg.idx ^ instruction.add.idx;

        return result;
    }
    )//CRS_IF_HASH_GUARD

public:
    const std::vector<SInstruction>& get_instruction_vec() const { return instruction_vec_; }

    void parse_string(const char* str)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        cur_pos_ = str;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        while (*cur_pos_)
        {
            instruction_vec_.push_back(parse_instruction());

            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

            if (*cur_pos_ == '\n' || *cur_pos_ == '\r')
            {
                while (std::isspace(*cur_pos_))
                    cur_pos_++;
            }
            else if (*cur_pos_ == '\0') break;
            else CRS_PROCESS_ERROR("get_instruction: '\\n' expected before \"%.16s\"", cur_pos_)

            CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)
        }

        cur_pos_ = nullptr;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    SInstruction parse_instruction()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SInstruction result = {};
        //sizeof(command_literal) because of +1 at ' ' and -1 at '\0'
        #define HANDLE_SIMPLE_CMD_(command_literal, command) \
            else if (!strncmp(cur_pos_, command_literal " ", sizeof(command_literal))) \
            { \
                shift_and_pass_spaces_(sizeof(command_literal)); \
                \
                result = { command, {}, {}, {} }; \
            }

        if (!strncmp(cur_pos_, "push"" ", sizeof("push")))
        {
            shift_and_pass_spaces_(sizeof("push"));//this size because of +1 at ' ' and -1 at '\0'
            result = parse_push_();
        }
        else if (!strncmp(cur_pos_, "pop"" ", sizeof("pop")))
        {
            shift_and_pass_spaces_(sizeof("pop"));//this size because of +1 at ' ' and -1 at '\0'
            result = parse_pop_();
        }

        HANDLE_SIMPLE_CMD_("dup",  CMD_DUP)

        HANDLE_SIMPLE_CMD_("fadd", CMD_FADD)
        HANDLE_SIMPLE_CMD_("fsub", CMD_FSUB)
        HANDLE_SIMPLE_CMD_("fmul", CMD_FMUL)
        HANDLE_SIMPLE_CMD_("fdiv", CMD_FDIV)

        HANDLE_SIMPLE_CMD_("fsin",  CMD_FSIN)
        HANDLE_SIMPLE_CMD_("fcos",  CMD_FCOS)
        HANDLE_SIMPLE_CMD_("fsqrt", CMD_FSQRT)

        HANDLE_SIMPLE_CMD_("hlt",  CMD_HLT)
        HANDLE_SIMPLE_CMD_("in",   CMD_IN)
        HANDLE_SIMPLE_CMD_("out",  CMD_OUT)
        HANDLE_SIMPLE_CMD_("ok",   CMD_OK)
        HANDLE_SIMPLE_CMD_("dump", CMD_DUMP)

        else CRS_PROCESS_ERROR("handle input error: unrecognizable command: \"%.16s\"", cur_pos_)

        #undef HANDLE_CMD_

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

private:
    void shift_and_pass_spaces_(size_t shift = 1)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        cur_pos_ += shift;

        while (*cur_pos_ == ' ' || *cur_pos_ == '\t')
            cur_pos_++;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    SToken parse_num_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        UWord result_word = {};

        sscanf(cur_pos_, "%f", &result_word);

        while (std::isdigit(*cur_pos_))
            cur_pos_++;

        if (*cur_pos_ == '.')
        {
            cur_pos_++;
            while (std::isdigit(*cur_pos_))
                cur_pos_++;
        }

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return { ETokenType::TOK_NUM, result_word };
    }

    SToken parse_idx_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        UWord result_word = {};

        sscanf(cur_pos_, "%d", &result_word);

        while (std::isdigit(*cur_pos_))
            cur_pos_++;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return { ETokenType::TOK_IDX, result_word };
    }

    SToken parse_reg_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SToken result = {};

        #define CHECK_REG_NAME(register, register_literal) \
            if (!strncmp(cur_pos_, register_literal, sizeof(register_literal)-1)) \
            { \
                shift_and_pass_spaces_(sizeof(register_literal)-1); \
                \
                result = { ETokenType::TOK_REG, static_cast<uint32_t>(register) }; \
            }

             CHECK_REG_NAME(ERegister::REG_AX, "ax")
        else CHECK_REG_NAME(ERegister::REG_BX, "bx")
        else CHECK_REG_NAME(ERegister::REG_CX, "cx")
        else CHECK_REG_NAME(ERegister::REG_DX, "dx")

        else CRS_PROCESS_ERROR("get_reg: unrecognizable register name \"%.16s\"", cur_pos_)

        #undef CHECK_REG_NAME

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    parse_label_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        const size_t MAX_LABEL_NAME_LEN = 64;

        char label_name[MAX_LABEL_NAME_LEN] = "";
        size_t label_name_len = 0;

        while (std::isalnum(*cur_pos_))
            label_name_len++;

        if (std::isalpha(*cur_pos_))

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    SToken parse_reg_or_num_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SToken result = {};

        if      (std::isdigit(*cur_pos_) || *cur_pos_ == '.') result = parse_num_();
        else if (std::isalpha(*cur_pos_))                     result = parse_reg_();
        else CRS_PROCESS_ERROR("get_reg_or_num: invalid argument: \"%.16s\"", cur_pos_)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    SToken parse_reg_or_idx_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SToken result = {};

        if      (std::isdigit(*cur_pos_)) result = parse_idx_();
        else if (std::isalpha(*cur_pos_)) result = parse_reg_();
        else CRS_PROCESS_ERROR("get_reg_or_idx: invalid argument: \"%.16s\"", cur_pos_)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    SCommandArgs parse_bracket_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SCommandArgs result = {};

        if (*cur_pos_ == '[') shift_and_pass_spaces_();
        else CRS_PROCESS_ERROR("get_bracket: '[' expected: '\"%.16s\"'", cur_pos_)

        result.arg = parse_reg_or_idx_();

        if (*cur_pos_ == '+')
        {
            shift_and_pass_spaces_();
            result.add = parse_reg_or_idx_();
        }

        if (*cur_pos_ == ']') shift_and_pass_spaces_();
        else CRS_PROCESS_ERROR("get_bracket: ']' expected: '\"%.16s\"'", cur_pos_)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    SInstruction parse_push_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SCommandArgs args = {};
        SInstruction result = { ECommand::CMD_PUSH, {}, {}, {} };

        if (*cur_pos_ == '[')
        {
            args = parse_bracket_();

            if      (args.arg.tok_type == ETokenType::TOK_IDX &&
                     args.add.tok_type == ETokenType::TOK_NONE) result.mode = EPushMode::PUSH_RAM;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_NONE) result.mode = EPushMode::PUSH_RAM_REG;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_IDX)  result.mode = EPushMode::PUSH_RAM_REG_NUM;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_REG)  result.mode = EPushMode::PUSH_RAM_REG_REG;

            else CRS_PROCESS_ERROR("get_push: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else if (std::isdigit(*cur_pos_) || std::isalpha(*cur_pos_) || *cur_pos_ == '.')
        {
            args.arg = parse_reg_or_num_();

            if      (args.arg.tok_type == ETokenType::TOK_NUM) result.mode = EPushMode::PUSH_NUM;
            else if (args.arg.tok_type == ETokenType::TOK_REG) result.mode = EPushMode::PUSH_REG;

            else CRS_PROCESS_ERROR("get_push: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else CRS_PROCESS_ERROR("get_push: invalid empty argument list: \"%.16s\"", cur_pos_)

        result.arg = args.arg.tok_data;
        result.add = args.add.tok_data;

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

    SInstruction parse_pop_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        SCommandArgs args = {};
        SInstruction result = { ECommand::CMD_POP, {}, {}, {} };

        if (*cur_pos_ == '[')
        {
            args = parse_bracket_();

            if      (args.arg.tok_type == ETokenType::TOK_IDX &&
                     args.add.tok_type == ETokenType::TOK_NONE) result.mode = EPopMode::POP_RAM;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_NONE) result.mode = EPopMode::POP_RAM_REG;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_IDX)  result.mode = EPopMode::POP_RAM_REG_NUM;
            else if (args.arg.tok_type == ETokenType::TOK_REG &&
                     args.add.tok_type == ETokenType::TOK_REG)  result.mode = EPopMode::POP_RAM_REG_REG;

            else CRS_PROCESS_ERROR("get_pop: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else if (std::isdigit(*cur_pos_) || std::isalpha(*cur_pos_) || *cur_pos_ == '.')
        {
            args.arg = parse_reg_or_num_();

            if (args.arg.tok_type == ETokenType::TOK_REG) result.mode = EPopMode::POP_REG;

            else CRS_PROCESS_ERROR("get_pop: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else CRS_PROCESS_ERROR("get_pop: invalid empty argument list: \"%.16s\"", cur_pos_)

        result.arg = args.arg.tok_data;
        result.add = args.add.tok_data;

        CRS_IF_GUARD(CRS_END_CHECK();)

        return result;
    }

public:
    bool ok() const
    {
        return (this && CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                            end_canary_ == CANARY_VALUE &&)
                CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_()));
    }

    void dump() const
    {
        CRS_STATIC_DUMP("CTranslator[%s, this : %p] \n"
                        "{ \n"
                        CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                        CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                        "    \n"
                        //"    input_file_view_ :  \n"
                        //"        size : %d \n"
                        //"    output_file_view_ : \n"
                        //"        size : %d \n"
                        "    cur_pos_ : %p \n"
                        "    instruction_vec_ : \n"
                        "        size() : %d \n"
                        "    \n"
                        CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                        "} \n",

                        (ok() ? "OK" : "ERROR"), this,
                        CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                        CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                        //input_file_view_ .get_file_view_size(),
                        //output_file_view_.get_file_view_size(),

                        cur_pos_,
                        instruction_vec_.size()

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    //CFileView input_file_view_;
    //CFileView output_file_view_;

    const char* cur_pos_;
    std::vector<SInstruction> instruction_vec_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}//namespace course

#endif // TRANSLATOR_H_INCLUDED
