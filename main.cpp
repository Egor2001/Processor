#include <cstdlib>

#include "VirtualMachine.h"

using namespace course;

int main()
{
    CProcessor proc;

    float var = 1.0f;

    proc.proc_command(CProcessor::ECommands::PUSH, *(CProcessor::type_t_*)(&var));

    var = 0.5f;

    proc.proc_command(CProcessor::ECommands::PUSH, *(CProcessor::type_t_*)(&var));
    proc.proc_command(CProcessor::ECommands::FDIV);
    proc.proc_command(CProcessor::ECommands::POP, CProcessor::ERegisters::AX);

    return 0;
}
