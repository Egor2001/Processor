#include <cstdlib>

#include "VirtualMachine.h"

using namespace course;

int main()
{
    CProcessor proc;

    float var = 1.0f;

    proc.proc_command(ECommands::CMD_PUSH, *(CProcessor::word_t_*)(&var));

    var = 0.5f;

    proc.proc_command(ECommands::CMD_PUSH, *(CProcessor::word_t_*)(&var));
    proc.proc_command(ECommands::CMD_FDIV);
    proc.proc_command(ECommands::CMD_POP, ERegisters::REG_AX);

    return 0;
}
