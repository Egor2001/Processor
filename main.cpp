#include <cstdlib>

#define CRS_GUARD_LEVEL 3

#include "Stack/Guard.h"

#include "Processor.h"

#include "Translator.h"

using namespace course;

int main()
{
    CTranslator translator;

    translator.parse_string("fadd push ax hlt");

    CProcessor proc;

    float var = 1.0f;

    proc.cmd_push(EPushMode::PUSH_NUM, var);

    var = 0.5f;

    proc.cmd_push(EPushMode::PUSH_NUM, var);
    proc.cmd_fdiv();
    proc.cmd_pop(EPopMode::POP_REG, uint32_t(ERegister::REG_AX));

    printf("%f ", proc.reg_out(ERegister::REG_AX));

    return 0;
}
