#include <cstdlib>

#define CRS_GUARD_LEVEL 3

#include "Stack/Guard.h"
#include "Processor.h"
#include "Translator.h"
#include "TranslatorFiles/FileView.h"

using namespace course;

int main()
{
    CTranslator translator("asm/source.txt", "asm/executable.txt");

    translator.parse_input();

    for (const auto& instr : translator.get_instruction_vec())
        printf("%#x %#x %#x %#x \n", instr.command, instr.mode, instr.arg, instr.add);

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
