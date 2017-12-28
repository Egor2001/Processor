//TODO: regexp

HANDLE_COMMAND_(ECommand::CMD_HLT,  hlt, NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_PUSH, push, PARAM, "idx | reg | mem")
HANDLE_COMMAND_(ECommand::CMD_POP,  pop,  PARAM, "reg | mem")
HANDLE_COMMAND_(ECommand::CMD_DUP,  dup,  NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_CALL, call, PARAM,    "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_RET,  ret,  NO_PARAM, "")

HANDLE_COMMAND_(ECommand::CMD_JMP, jmp, PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JZ,  jz,  PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JNZ, jnz, PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JE,  je,  PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JNE, jne, PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JG,  jg,  PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JGE, jge, PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JL,  jl,  PARAM, "idx | reg | mem | lbl")
HANDLE_COMMAND_(ECommand::CMD_JLE, jle, PARAM, "idx | reg | mem | lbl")
/*
HANDLE_COMMAND_(ECommand::CMD_MOV, mov, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")

HANDLE_COMMAND_(ECommand::CMD_CMP, cmp, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")

HANDLE_COMMAND_(ECommand::CMD_INC, inc, PARAM, "reg | mem")
HANDLE_COMMAND_(ECommand::CMD_DEC, dec, PARAM, "reg | mem")

HANDLE_COMMAND_(ECommand::CMD_ADD, add, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")
HANDLE_COMMAND_(ECommand::CMD_SUB, sub, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")
HANDLE_COMMAND_(ECommand::CMD_MUL, mul, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")
HANDLE_COMMAND_(ECommand::CMD_DIV, div, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")

HANDLE_COMMAND_(ECommand::CMD_AND, and, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")
HANDLE_COMMAND_(ECommand::CMD_OR,  or,  PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")
HANDLE_COMMAND_(ECommand::CMD_XOR, xor, PARAM, "reg reg | reg mem | reg idx | mem reg | mem mem | mem idx")

HANDLE_COMMAND_(ECommand::CMD_FADD, fadd, PARAM, " | reg | mem")
HANDLE_COMMAND_(ECommand::CMD_FSUB, fsub, PARAM, " | reg | mem")
HANDLE_COMMAND_(ECommand::CMD_FMUL, fmul, PARAM, " | reg | mem")
HANDLE_COMMAND_(ECommand::CMD_FDIV, fdiv, PARAM, " | reg | mem")
*/
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
