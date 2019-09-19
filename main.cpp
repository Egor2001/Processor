#include <cstdlib>

#define CRS_GUARD_LEVEL 3
//#define CRS_NO_LOGGING

#include "Stack/Guard.h"
#include "CPU/Processor.h"
#include "Translator/Translator.h"

using namespace course;

enum
{
    FIB_RECURSIVE,
    FIB_ITERATIVE
};

int main(int argc, char* argv[])
{
    const char* file_name = "../asm/test.txt";

    //for calling destructor, closing mapped files
    try
    {
        {
            CTranslator translator(file_name, "../asm/executable.txt");
            translator.parse_input();
        }

        {
            CProcessor proc("../asm/executable.txt");
            proc.execute();
        }
    }
    catch (const std::exception& exc)
    {
        printf("exception caught: %s", exc.what());
    }

    return 0;
}
