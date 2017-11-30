#ifndef INPUT_HANDLER_INCLUDED
#define INPUT_HANDLER_INCLUDED

#include <cstdio>
#include <vector>
#include <cstring>
#include <cctype>

#include "Translator.h"

#include "TranslatorFiles/FileView.h"

namespace course {

using namespace course_stack;

class CInputHandler
{
public:
    static const size_t MAX_CMD_STR_LEN = 64;

    static const size_t CANARY_VALUE = "CInputHandler"_crs_hash;

public:
    explicit CInputHandler():
        instruction_vec_ ()
    {}

    CInputHandler             (const CInputHandler&) = delete;
    CInputHandler& operator = (const CInputHandler&) = delete;

    ~CInputHandler()
    {
        instruction_vec_.clear();
    }



private:
    std::vector<SInstruction> instruction_vec_;
};

}//namespace course

#endif // INPUT_HANDLER_INCLUDED

