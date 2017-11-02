#ifndef INPUT_HANDLER_INCLUDED
#define INPUT_HANDLER_INCLUDED

#include <cstdio>

#include "Stack/CourseException.h"
#include "Stack/Guard.h"


namespace course {

using namespace course_stack;

class CInputHandler
{
public:
    struct SInstruction
    {
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

