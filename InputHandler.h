#ifndef INPUT_HANDLER_INCLUDED
#define INPUT_HANDLER_INCLUDED

#include <cstdio>

#include "Stack/CourseException.h"
#include "Stack/Guard.h"

#include "VirtualMachine.h"

namespace course {

using namespace course_stack;

class CInputHandler
{
public:
    struct SInstruction
    {
        ECommands command;
        CProcessor::word_t_ arg;
    };

    explicit CInputHandler(FILE* input_file_set):
        input_file_(input_file_set)
    {

    }

    ~CInputHandler()
    {

    }

    SInstruction handle_input()
    {
        char command_str[16] = "";

        fscanf(input_file "%15s", command_str);

        if (!strcmp(command_str, "push"))
        {
            char arg_str[32] = "";

            fscanf(input_file " %31s", arg_str);


        }
        else if (!strcmp(command_str, "pop"))
        {

        }
        else if (!strcmp(command_str, "dup"))
        {
            return { CMD_DUP, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fadd"))
        {
            return { CMD_FADD, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fsub"))
        {
            return { CMD_FSUB, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fmul"))
        {
            return { CMD_FMUL, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fdiv"))
        {
            return { CMD_FDIV, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fsin"))
        {
            return { CMD_FSIN, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fcos"))
        {
            return { CMD_FCOS, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "fsqrt"))
        {
            return { CMD_FSQRT, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "hlt"))
        {
            return { CMD_HLT, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "in"))
        {
            return { CMD_IN, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "out"))
        {
            return { CMD_OUT, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "ok"))
        {
            return { CMD_OK, static_cast<CProcessor::word_t_>(-1) };
        }
        else if (!strcmp(command_str, "dump"))
        {
            return { CMD_DUMP, static_cast<CProcessor::word_t_>(-1) };
        }
        else
        {
            char error_str[CCourseException::MAX_MSG_LEN] = "handle input error: "
                                                            "unrecognizable command : ";

            throw CCourseException(strncat(error_str, command_str, CCourseException::MAX_MSG_LEN));
        }

        return { CMD_HLT, static_cast<CProcessor::word_t_>(-1) };
    }

public:
    void assert_ok() const
    {
        if (!ok())
            throw CCourseException("CInputHandler is not ok");
    }

    bool ok() const
    {
        return this && input_file_;
    }

    void dump() const
    {

    }

private:
    FILE* input_file_;
};

}//namespace course

#endif // INPUT_HANDLER_INCLUDED

