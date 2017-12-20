#include <cstdlib>

#define CRS_GUARD_LEVEL 1

#include "Stack/Guard.h"
#include "Processor.h"
#include "Translator.h"
#include "TranslatorFiles/FileView.h"

using namespace course;

int main()
{
    //for calling destructor, closing mapped files
    {
        CTranslator translator("asm/source.txt", "asm/executable.txt");
        translator.parse_input();
    }

    {
        CProcessor proc("asm/executable.txt");
        proc.execute();
    }

    return 0;
}
