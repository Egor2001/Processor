//TODO: regexp

HANDLE_COMMAND_(ECommand::CMD_HLT,  hlt, NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_PUSH, push, PARAM, "idx | reg | [idx] | [reg] | [reg + idx] | [reg + reg]")
HANDLE_COMMAND_(ECommand::CMD_POP,  pop,  PARAM, "reg | [idx] | [reg] | [reg + idx] | [reg + reg]")
HANDLE_COMMAND_(ECommand::CMD_DUP,  dup,  NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_CALL, call, PARAM,    "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_RET,  ret,  NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_JMP, jmp, PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JZ,  jz,  PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JNZ, jnz, PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JE,  je,  PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JNE, jne, PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JG,  jg,  PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JGE, jge, PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JL,  jl,  PARAM, "idx | reg | [idx] | [reg] | lbl")
HANDLE_COMMAND_(ECommand::CMD_JLE, jle, PARAM, "idx | reg | [idx] | [reg] | lbl")

HANDLE_COMMAND_(ECommand::CMD_FADD, fadd, NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_FSUB, fsub, NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_FMUL, fmul, NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_FDIV, fdiv, NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_FSIN,  fsin,  NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_FCOS,  fcos,  NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_FSQRT, fsqrt, NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_FTOI, ftoi, NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_ITOF, itof, NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_IN,   in,   NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_OUT,  out,  NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_OK,   ok,   NO_PARAM, "")
HANDLE_COMMAND_(ECommand::CMD_DUMP, dump, NO_PARAM, "")
