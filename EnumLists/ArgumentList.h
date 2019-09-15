//Signature:
//HANDLE_ARGUMENT_(enum_name, code, name)

//Markdown:
//null     [0x00]       (NUL)
//reserved [0x01, 0x1F] ()
//unitary  [0x20, 0x3F] (IMM, REG, LBL, FLT)
//memory   [0x40, 0x7F] (MEM_IMM, MEM_REG, MEM_REG_IMM, MEM_REG_REG)
//reserved [0x80, 0xFE] ()
//error    [0xFF]       (ERROR_VALUE)

//HANDLE_ARGUMENT(ARG_NUL,         0x00, nul)

HANDLE_ARGUMENT_(ARG_IMM,         0x20, imm)
HANDLE_ARGUMENT_(ARG_REG,         0x21, reg)
HANDLE_ARGUMENT_(ARG_LBL,         0x22, lbl)
HANDLE_ARGUMENT_(ARG_FLT,         0x23, flt)

HANDLE_ARGUMENT_(ARG_MEM_IMM,     0x40, mem_imm)
HANDLE_ARGUMENT_(ARG_MEM_REG,     0x41, mem_reg)
HANDLE_ARGUMENT_(ARG_MEM_REG_IMM, 0x42, mem_reg_imm)
HANDLE_ARGUMENT_(ARG_MEM_REG_REG, 0x43, mem_reg_reg)

//Will be defined in the end of EArgument
//HANDLE_ARGUMENT_(ARG_ERROR_VALUE, 0xFF, error_value)