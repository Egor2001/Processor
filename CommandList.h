//TODO: regexp

HANDLE_COMMAND_(CMD_HLT,  hlt, NO_PARAM, "")

HANDLE_COMMAND_(CMD_PUSH, push, PARAM, "idx | reg | [idx] | [reg] | [reg + idx] | [reg + reg]")
HANDLE_COMMAND_(CMD_POP,  pop,  PARAM, "reg | [idx] | [reg] | [reg + idx] | [reg + reg]")
HANDLE_COMMAND_(CMD_DUP,  dup,  NO_PARAM, "")

HANDLE_COMMAND_(CMD_JMP, jmp, PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JZ,  jz,  PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JNZ, jnz, PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JE,  je,  PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JNE, jne, PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JG,  jg,  PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JGE, jge, PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JL,  jl,  PARAM, "idx | reg | lbl")
HANDLE_COMMAND_(CMD_JLE, jle, PARAM, "idx | reg | lbl")

HANDLE_COMMAND_(CMD_FADD, fadd, NO_PARAM, "")
HANDLE_COMMAND_(CMD_FSUB, fsub, NO_PARAM, "")
HANDLE_COMMAND_(CMD_FMUL, fmul, NO_PARAM, "")
HANDLE_COMMAND_(CMD_FDIV, fdiv, NO_PARAM, "")

HANDLE_COMMAND_(CMD_FSIN,  fsin,  NO_PARAM, "")
HANDLE_COMMAND_(CMD_FCOS,  fcos,  NO_PARAM, "")
HANDLE_COMMAND_(CMD_FSQRT, fsqrt, NO_PARAM, "")

HANDLE_COMMAND_(CMD_IN,   in,   NO_PARAM, "")
HANDLE_COMMAND_(CMD_OUT,  out,  NO_PARAM, "")
HANDLE_COMMAND_(CMD_OK,   ok,   NO_PARAM, "")
HANDLE_COMMAND_(CMD_DUMP, dump, NO_PARAM, "")
