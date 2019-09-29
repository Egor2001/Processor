#ifndef COURSE_EXCEPTION_INCLUDED
#define COURSE_EXCEPTION_INCLUDED

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace course_util {

class CCourseException : public std::exception
{
public:
    static const size_t MAX_MSG_LEN = 128;

public:
    CCourseException(const char* what_str_set):
        std::exception(), what_str_{}
    {
        const char prefix_str[] = "[course error]: ";

        strncat(what_str_, prefix_str, sizeof(prefix_str));
        strncat(what_str_, what_str_set, MAX_MSG_LEN - sizeof(prefix_str));
    }

    virtual ~CCourseException() override
    {
        memset(what_str_, 0x00, sizeof(what_str_));
    }

    virtual const char* what() const noexcept override
    {
        return what_str_;
    }

private:
    char what_str_[MAX_MSG_LEN];
};

} //namespace course_stack

#endif // COURSE_EXCEPTION_INCLUDED
