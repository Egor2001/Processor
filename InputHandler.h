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
    explicit CInputHandler(FILE* input_file_set = stdin):
        input_file_(input_file_set)
    {

    }

    ~CInputHandler()
    {

    }

public:
    void assert_ok() const
    {
        if (!ok())
            throw CCourseException("CInputHandler is not ok");
    }

    bool ok() const
    {
        return input_file_;
    }

    void dump() const
    {

    }

private:
    FILE* input_file_;
};

}//namespace course

#endif // INPUT_HANDLER_INCLUDED

