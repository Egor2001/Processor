#include <cstdlib>

#include "VirtualMachine.h"

using namespace course;

int main()
{
    CProcessor proc;

    float var = 1.0f;

    proc.proc_command(ECommand::CMD_PUSH, *(CProcessor::word_t_*)(&var));

    var = 0.5f;

    proc.proc_command(ECommand::CMD_PUSH, *(CProcessor::word_t_*)(&var));
    proc.proc_command(ECommand::CMD_FDIV);
    proc.proc_command(ECommand::CMD_POP, ERegister::REG_AX);

    return 0;
}
