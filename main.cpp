#include <cstdlib>

#define CRS_GUARD_LEVEL 3
//#define CRS_NO_LOGGING

#include "Stack/Guard.h"
#include "Processor.h"
#include "Translator.h"
#include "TranslatorFiles/FileView.h"

using namespace course;

enum
{
    FIB_RECURSIVE,
    FIB_ITERATIVE
};

int main(int argc, char* argv[])
{
    const char* file_name = "../asm/test.txt";
    /*
    printf("choose mode: \n"
           "tap 0 for recursive \n"
           "tap 1 for iterative \n",
           FIB_RECURSIVE, FIB_ITERATIVE);

    int mode = -1;

    scanf("%d", &mode);

    switch (mode)
    {
        case FIB_RECURSIVE:
            file_name = "asm/fib_recursive.txt";
            break;

        case FIB_ITERATIVE:
            file_name = "asm/fib_iterative.txt";
            break;

        default:
            printf("error: invalid mode");
            return EXIT_FAILURE;
    }
    */

    //for calling destructor, closing mapped files
    {

        CTranslator translator(file_name, "../asm/executable.txt");
        translator.parse_input();
    }

    {
        CProcessor proc("../asm/executable.txt");
        proc.execute();
    }

    return 0;
}
