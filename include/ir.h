#ifndef __IR_H__
#define __IR_H__

#include <stdlib.h>
#include "symtable.h"

#define REG_ZERO 0
#define REG_T 8
#define REG_A 4
#define REG_V 2
#define REG_RA 31
#define REG_STACK 23
#define REG_SP 29

#define RS      0x03e00000
#define RT      0x001f0000
#define RD      0x0000f800
#define IMM     0x0000ffff
#define TO_RS   21
#define TO_RT   16
#define TO_RD   11

#define OP_SPECIAL  0x00000000
#define OP_SPECIAL2 0x70000000
#define OP_LW       0x98000000
#define OP_ADDI     0x20000000
#define OP_ADDIU    0x24000000
#define OP_LUI      0x3c000000
#define OP_SLTI     0x28000000
#define OP_SLTIU    0x2c000000
#define OP_SW       0xac000000
#define OP_ADDU     0x00000021
#define OP_JALR     0x00000009
#define OP_SUBU     0x00000023
#define OP_MUL      0x70000002
#define OP_MFHI     0x00000010
#define OP_MFLO     0x00000012
#define OP_SLT      0x0000002a
#define OP_AND      0x00000024
#define OP_OR       0x00000025

enum ir_type
{
    IR_BREAK, //not in final ir
    IR_SINGLE,
    IR_DOUBLE,
    IR_CALL_MEMBER,
    IR_CALL,
    IR_RET,
    IR_SAVE_REGS,
    IR_RESTORE_REGS,
    IR_PRINT,
    IR_NEW,
    IR_B,
    IR_J
};

struct ir
{
    uint8_t generated;
    enum ir_type type;
    union
    {
        char *code;
        uint32_t *bcode;
    };
    struct symres *env;
    uint32_t number;
};

void gen_code(uint32_t i, struct ir ir[]);

#endif
