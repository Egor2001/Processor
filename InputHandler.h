#ifndef INPUT_HANDLER_INCLUDED
#define INPUT_HANDLER_INCLUDED

#include <cstdio>

#include "Stack/CourseException.h"
#include "Stack/Guard.h"

#include "ProcessorEnums.h"

namespace course {

using namespace course_stack;

class CInputHandler
{
public:
    struct SInstruction
    {
        ECommand command;
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

        #define HANDLE_SIMPLE_CMD_(recognised_str, command_str, command) \
            else if (!strcmp(recognised_str, command_str)) \
            { \
                CRS_STATIC_MSG(CRS_STRINGIZE(command) " command recognized"); \
                return { command, static_cast<CProcessor::word_t_>(-1) }; \
            }

        if (!strcmp(command_str, "push"))
        {
            char arg_str[32] = "";

            fscanf(input_file " %31s", arg_str);

            #define HANDLE_PUSH_MODE_(recognised_mode, mode) \
                else if (recognised_mode == mode) \
                { \
                    CRS_STATIC_MSG(CRS_STRINGIZE(mode) " push mode recognized"); \
                }

            #undef HANDLE_PUSH_MODE_
        }
        else if (!strcmp(command_str, "pop"))
        {

        }

        HANDLE_SIMPLE_CMD_(command_str, "dup",   CMD_DUP)

        HANDLE_SIMPLE_CMD_(command_str, "fadd",  CMD_FADD)
        HANDLE_SIMPLE_CMD_(command_str, "fsub",  CMD_FSUB)
        HANDLE_SIMPLE_CMD_(command_str, "fmul",  CMD_FMUL)
        HANDLE_SIMPLE_CMD_(command_str, "fdiv",  CMD_FDIV)

        HANDLE_SIMPLE_CMD_(command_str, "fsin",  CMD_FSIN)
        HANDLE_SIMPLE_CMD_(command_str, "fcos",  CMD_FCOS)
        HANDLE_SIMPLE_CMD_(command_str, "fsqrt", CMD_FSQRT)

        HANDLE_SIMPLE_CMD_(command_str, "hlt",  CMD_HLT)
        HANDLE_SIMPLE_CMD_(command_str, "in",   CMD_IN)
        HANDLE_SIMPLE_CMD_(command_str, "out",  CMD_OUT)
        HANDLE_SIMPLE_CMD_(command_str, "ok",   CMD_OK)
        HANDLE_SIMPLE_CMD_(command_str, "dump", CMD_DUMP)

        else
        {
            char error_str[CCourseException::MAX_MSG_LEN] = "handle input error: "
                                                            "unrecognizable command : ";

            throw CCourseException(strncat(error_str, command_str, CCourseException::MAX_MSG_LEN));
        }

        #undef HANDLE_SIMPLE_CMD_

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

