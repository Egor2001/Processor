#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include <cstdio>
#include <vector>
#include <cstring>
#include <cctype>

#include "Stack/CourseException.h"
#include "Stack/Guard.h"

#include "ProcessorEnums.h"

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

public:
    CTranslator() = default;

    CTranslator             (const CTranslator&) = delete;
    CTranslator& operator = (const CTranslator&) = delete;

    ~CTranslator()
    {
        cur_pos_ = nullptr;
        instruction_vec_.clear();
    }

public:
    const std::vector<SInstruction>& instruction_vec() const { return instruction_vec_; }

    void parse_string(const char* str)
    {
        cur_pos_ = str;

        while (*cur_pos_)
            instruction_vec_.push_back(get_instruction());

        cur_pos_ = nullptr;
    }

    SInstruction get_instruction()
    {
        SInstruction result = {};

        #define HANDLE_SIMPLE_CMD_(command_literal, command) \
            else if (!strncmp(cur_pos_, command_literal, sizeof(command_literal)-1)) \
            { \
                cur_pos_ += sizeof(command_literal)-1; \
                pass_spaces_(); \
                \
                result = { command, {}, {}, {} }; \
            }

        if (!strncmp(cur_pos_, "push", sizeof("push")-1))
        {
            cur_pos_ += sizeof("push")-1;
            pass_spaces_();

            result = get_push_();
        }
        else if (!strncmp(cur_pos_, "pop", sizeof("pop")-1))
        {
            cur_pos_ += sizeof("pop")-1;
            pass_spaces_();

            result = get_pop_();
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

        else CRS_PROCESS_ERROR("handle input error: unrecognizable command: %8s", cur_pos_)

        #undef HANDLE_CMD_

        //if (*cur_pos_ != '\n' && *cur_pos_ != '\r' && *cur_pos_ != '\0')
        //    CRS_PROCESS_ERROR("get_instruction: could not reach end of argument list: '%c'", *cur_pos_)

        return result;
    }

private:
    void pass_spaces_()
    {
        while (std::isspace(*cur_pos_))
            cur_pos_++;
    }

    SToken get_num_()
    {
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

        return { ETokenType::TOK_NUM, result_word };
    }

    SToken get_idx_()
    {
        UWord result_word = {};

        sscanf(cur_pos_, "%d", &result_word);

        while (std::isdigit(*cur_pos_))
            cur_pos_++;

        return { ETokenType::TOK_IDX, result_word };
    }

    SToken get_reg_()
    {
        #define CHECK_REG_NAME(register, register_literal) \
            if (!strncmp(cur_pos_, register_literal, sizeof(register_literal)-1)) \
            { \
                cur_pos_ += sizeof(register_literal)-1; \
                pass_spaces_(); \
                \
                return { ETokenType::TOK_REG, static_cast<uint32_t>(register) }; \
            }

             CHECK_REG_NAME(ERegister::REG_AX, "ax")
        else CHECK_REG_NAME(ERegister::REG_BX, "bx")
        else CHECK_REG_NAME(ERegister::REG_CX, "cx")
        else CHECK_REG_NAME(ERegister::REG_DX, "dx")

        else CRS_PROCESS_ERROR("get_reg: unrecognizable register name %8s", cur_pos_)

        while (std::isalpha(*cur_pos_))
            cur_pos_++;

        #undef CHECK_REG_NAME
    }

    SToken get_reg_or_num_()
    {
        if      (std::isdigit(*cur_pos_) || *cur_pos_ == '.') return get_num_();
        else if (std::isalpha(*cur_pos_))                     return get_reg_();
        else CRS_PROCESS_ERROR("get_reg_or_num: invalid argument: %8s", cur_pos_)
    }

    SToken get_reg_or_idx_()
    {
        if      (std::isdigit(*cur_pos_)) return get_idx_();
        else if (std::isalpha(*cur_pos_)) return get_reg_();
        else CRS_PROCESS_ERROR("get_reg_or_idx: invalid argument: %8s", cur_pos_)
    }

    SCommandArgs get_bracket_()
    {
        SCommandArgs result = {};

        if (*cur_pos_ == '[') { cur_pos_++; pass_spaces_(); }
        else CRS_PROCESS_ERROR("get_bracket: '[' expected: '%8s'", cur_pos_)

        result.arg = get_reg_or_idx_();

        if (*cur_pos_ == '+') { cur_pos_++; pass_spaces_(); result.add = get_reg_or_idx_(); }

        if (*cur_pos_ == ']') { cur_pos_++; pass_spaces_(); }
        else CRS_PROCESS_ERROR("get_bracket: ']' expected: '%8s'", cur_pos_)

        return result;
    }

    SInstruction get_push_()
    {
        SCommandArgs args = {};
        SInstruction result = { ECommand::CMD_PUSH, {}, {}, {} };

        if (*cur_pos_ == '[')
        {
            args = get_bracket_();

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
            args.arg = get_reg_or_num_();

            if      (args.arg.tok_type == ETokenType::TOK_NUM) result.mode = EPushMode::PUSH_NUM;
            else if (args.arg.tok_type == ETokenType::TOK_REG) result.mode = EPushMode::PUSH_REG;

            else CRS_PROCESS_ERROR("get_push: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else CRS_PROCESS_ERROR("get_push: invalid empty argument list: %8s", cur_pos_)

        result.arg = args.arg.tok_data;
        result.add = args.add.tok_data;

        return result;
    }

    SInstruction get_pop_()
    {
        SCommandArgs args = {};
        SInstruction result = { ECommand::CMD_POP, {}, {}, {} };

        if (*cur_pos_ == '[')
        {
            args = get_bracket_();

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
            args.arg = get_reg_or_num_();

            if (args.arg.tok_type == ETokenType::TOK_REG) result.mode = EPopMode::POP_REG;

            else CRS_PROCESS_ERROR("get_pop: invalid argument list: "
                                   "arg tok_type: %d, add tok_type: %d",
                                   args.arg.tok_type, args.add.tok_type)
        }
        else CRS_PROCESS_ERROR("get_pop: invalid empty argument list: %8s", cur_pos_)

        result.arg = args.arg.tok_data;
        result.add = args.add.tok_data;

        return result;
    }

private:
    const char* cur_pos_;
    std::vector<SInstruction> instruction_vec_;
};

}//namespace course

#endif // TRANSLATOR_H_INCLUDED
