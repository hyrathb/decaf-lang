#include <stdlib.h>
#include "symtable.h"
#include "ir.h"

static uint32_t tcode[50];

static uint32_t s = 16;
static uint32_t t = 8;

struct op zero = {0, 0};

extern struct stringlist *stringlist;
struct symres root;

void reset_ir_gen()
{
}

void reset_s()
{
    s = REG_S;
    t = REG_T:
}

uint32_t get_reg(const char *id, struct symres *env)
{
    return s++;
}

uint32_t get_tmp_reg()
{
    return t++;
}

uint32_t in_reg(const char *id, struct symres *env)
{
    return -1;
}

uint32_t gen_readstring(struct ir *ir)
{
    return REG_A+0;
}

uint32_t gen_imm(const char *imm, struct ir *ir)
{
    uint32_t i = atoi(imm);
    uint32_t treg = get_tmp_reg();
    uint32_t op = OP_LUI | treg << TO_RT | i&IMM;
    tcode[ir->number++] = op;
    DBGPRINT("lui %u, %u\n", treg, i);
    return treg;
    
}

uint32_t gen_readint(struct ir *ir)
{
    return REG_A+0;
}

void gen_lw(uint32_t base, uint32_t rt, uint32_t offset, struct ir *ir)
{
    uint32_t op = OP_LW | base << TO_RS | rt << TO_RT | offset&IMM;
    tcode[ir->number++] = op;
    DBGPRINT("lw %d(%u), %u\n", offset, base, rt);
}

uint32_t gen_not(uint32_t ops, struct ir *ir)
{
    uint32_t treg = get_tmp_reg();
    uint32_t op = OP_SLTIU | ops << TO_RS | treg << TO_RT | 1;
    tcode[ir->number++] = op;
    DBGPRINT("sltiu %u, %u, 1\n", ops, treg);
    return treg;
}

uint32_t gen_realloc_addr(const char *name, struct ir *ir)
{
    uint32_t treg = get_tmp_reg();
    uint32_t op = OP_LUI | treg << TO_RT | 0;
    tcode[ir->number++] = op;
    DBGPRINT("lui %u, %u\n", treg, 0);
    return treg;
}

void gen_sw(uint32_t base, uint32_t offset, uint32_t rt, struct ir *ir)
{
    uint32_t op = OP_SW | base << TO_RS | rt << TO_RT | offset&IMM;
    tcode[ir->number++] = op;
    DBGPRINT("sw %u(%u), %u", offset, base, rt);
}

void gen_addu(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_ADDU | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("addu %u, %u, %u\n", rs, rt, rd);
}

void gen_subu(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_SUBU | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("subu %u, %u, %u\n", rs, rt, rd);
}

void gen_mul(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_MUL | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("mul %u, %u, %u\n", rs, rt, rd);
}

void gen_div(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_DIV | rs << TO_RS | rt << TO_RT;
    tcode[ir->number++] = op;
    DBGPRINT("div %u, %u\n", rs, rt);
    op = OP_MFLO | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("mflo %u\n", rd);
}

void gen_div2(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_DIV | rs << TO_RS | rt << TO_RT;
    tcode[ir->number++] = op;
    DBGPRINT("div %u, %u\n", rs, rt);
    op = OP_MFHI | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("mfhi %u\n", rd);
}

void gen_slt(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_SLT | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("slt %u, %u, %u\n", rs, rt, rd);
}

void gen_equ(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    gen_subu(rs, rt, rd, ir);
    uint32_t op = OP_SLTIU | ops << TO_RS | treg << TO_RT | 1;
    tcode[ir->number++] = op;
    DBGPRINT("sltiu %u, %u, 1\n", ops, treg);
}

void gen_le(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    gen_subu(rs, rt, rd, ir);
    uint32_t op = OP_SLTI | rd << TO_RS | rd << TO_RT | 1;
    tcode[ir->number++] = op;
    DBGPRINT("slti %u, %u, 1\n", rd, rd);
}

void gen_ge(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    gen_subu(rt, rs, rd, ir);
    uint32_t op = OP_SLTI | rd << TO_RS | rd << TO_RT | 1;
    tcode[ir->number++] = op;
    DBGPRINT("slti %u, %u, 1\n", rd, rd);
}

void gen_and(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_AND | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("and %u, %u, %u\n", rs, rt, rd);
}

void gen_or(uint32_t rs, uint32_t rt, uint32_t rd, struct ir *ir)
{
    uint32_t op = OP_OR | rs << TO_RS | rt << TO_RT | rd << TO_RD;
    tcode[ir->number++] = op;
    DBGPRINT("or %u, %u, %u\n", rs, rt, rd);
}

void gen_addiu(uint32_t rs, uint32_t rt, uint32_t imm, struct ir *ir)
{
    uint32_t op = OP_ADDIU | rs << TO_RS | rt << TO_RT | imm&IMM;
    tcode[ir->number++] = op;
    DBGPRINT("addiu %u, %u, #%u\n", rs, rt, imm);
}

uint32_t get_tmp_arg(uint32_t offset, struct ir *ir)
{
    uint32_t treg = get_tmp_reg();
    gen_lw(REG_STACK, treg, offset, ir);
    return treg;
}

uint32_t get_var(const char *var, struct ir *ir, struct func_detail *func)
{
    uint32_t offset;
    uint32_t reg;
    switch (var[0])
    {
    case '!' :
    {
        reg = in_reg(var, NULL);
        if (reg == -1)
        {
            uint32_t t_num = atoi(var+1);
            offset = func->tvarsize - t_num*PSIZE;
            return get_tmp_arg(offset, ir);
        }
        return reg;
    }
    case '#':
    {
        if (var[1] == 'd')
        {
            reg = in_reg(var, NULL);
            if (reg == -1)
            {
                offset = func->tvarsize + func->uvarsize + func->formalsize;
                return get_tmp_arg(offset, ir);
            }
            return reg;
        }
        if (var[1] == 'a')
        {
            return REG_A+0;
        }
        else
        {
            uint32_t t_num = atoi(var+1);
            return REG_A+t_num;
        }
    }
    case '$':
    {
        if (!strcmp(var+1, "$readinteger"))
        {   
            return gen_readint(ir);
        }
        else if (!strcmp(var+1, "readline"))
        {
            return gen_readstring(ir);
        }
        else
        {
            return gen_imm(var+1, ir);
        }
    }
    case '*':
    {
        uint32_t treg = get_tmp_reg();
        gen_lw(get_var(var+1, ir, func), treg, 0, ir);
        return treg;
    }
    default:
    {
        reg = in_reg(var, ir->env);
        if (reg != -1)
            return reg;
        struct symhash *r = sym_get(ir->env, var);
        struct var_detail *detail = r->detail;
        switch (r->scope)
        {
        case SCOPE_LOCAL:
        {
            offset = func->tvarsize + func->uvarsize - detail->offset;
            return get_tmp_arg(offset, ir);
        }
        case SCOPE_FORMAL:
        {
            offset = func->tvarsize + func->uvarsize + func->formalsize - detail->offset;
            return get_tmp_arg(offset, ir);
        }
        case SCOPE_GLOBAL:
        {
            uint32_t base = gen_realloc_addr(var, ir);
            uint32_t treg = get_tmp_reg();
            gen_lw(base, treg, 0, ir);
            return treg;
        }
        default:
            return 0;
        }
    }
    }
}

void save_var(const char *l, uint32_t r, struct ir *ir, struct func_detail *func)
{
    uint32_t num;
    uint32_t offset;
    uint32_t reg = -1;
    switch (l[0])
    {
    case '!':
    {
        reg = in_reg(var, NULL);
        if (reg == -1)
        {
            num = atoi(l+1);
            offset = func->tvarsize - num*PSIZE;
            gen_sw(REG_STACK, offset, r, ir);
            return;
        }
    }
    case '#':
    {
        num = atoi(l+1);
        if (num < 4)
        {
            gen_addu(r, REG_ZERO, REG_A+num, ir);
        }
        else
        {
            gen_sw(REG_STACK, -(2+num)*PSIZE, r, ir);
        }
        return;
    }
    case '*':
    {
        uint32_t tr = get_var(l+1, ir, func);
        gen_sw(tr, 0, r, ir);
        return;
    }
    default:
    {
        reg = in_reg(var, ir->env);
        if (reg == -1)
        {
            struct symhash *r = sym_get(ir->env, var);
            struct var_detail *detail = r->detail;
            switch (r->scope)
            {
            case SCOPE_LOCAL:
            {
                offset = func->tvarsize + func->uvarsize - detail->offset;
                gen_sw(REG_STACK, offset, r, ir);
                return;
            }
            case SCOPE_FORMAL:
            {
                offset = func->tvarsize + func->uvarsize + func->formalsize - detail->offset;
                gen_sw(REG_STACK, offset, r, ir);
                return;
            }
            case SCOPE_GLOBAL:
            {
                uint32_t base = gen_realloc_addr(var, ir);
                gen_sw(base, 0, r, ir);
                return;
            }
            default:
                return;
            }
        }
    }
    }
    gen_addu(r, REG_ZERO, reg, ir);
}

void gen_call_mem(uint32_t i, struct ir ir[], struct func_detail *func)
{
    char l[20];
    uint32_t rl;
    uint32_t op;
    ir[i].addressed = 1;
    ir[i].generated = 1;
    sscanf(ir[i].code, "%s", l);
    rl = get_var(l, ir+i, func);
    op = OP_JALR | rl << TO_RS | REG_RA << TO_RD;
    tcode[ir[i].number++] = op;
    DBGPRINT("jalr %u\n", rl);
    gen_nop(ir+i);
    return;
}

void gen_call(uint32_t i, struct ir ir[])
{
    char func_name[20];
    uint32_t l;
    uint32_t op;
    ir[i].addressed = 1;
    ir[i].generated = 1;
    sscanf(ir[i].code, "%s", func_name);
    l = gen_realloc_addr(func_name, ir+i);
    op = OP_JALR | l << TO_RS | REG_RA << TO_RD;
    tcode[ir[i].number++] = op;
    DBGPRINT("jalr %u\n", l);
    gen_nop(ir+i);
    return;
}

void gen_print(uint32_t i, struct ir ir[], struct func_detail *func)
{
    int type;
    uint32_t reg, rl;
    uint32_t op;
    char l[20];
    if[i].addressed = 1;
    ir[i].generated = 1;
    sscanf(ir[i].code, "%d%s", &type, l);
    reg = get_var(l, ir+i, func);
    gen_addu(reg, REG_ZERO, REG_A+1, ir+i);
    uint32_t strtab = gen_realloc_addr(".strs", ir+i);
    switch(type)
    {
    case D_INT:
        gen_addiu(strtab, stringlist->next->i, REG_A+0, ir+i);
        break;
    case D_DOUBLE:
        gen_addiu(strtab, stringlist->next->next->i, REG_A+0, ir+i);
        break;
    case D_STRING:
        gen_addiu(strtab, stringlist->next->next->next->i, REG_A+0, ir+i);
        break;
    }
    rl = gen_realloc_addr("printf", ir+i);
    op = OP_JALR | rl << TO_RS | REG_RA << TO_RD;
    tcode[ir[i].number++] = op;
    DBGPRINT("jalr %u\n", rl);
    gen_nop(ir+i);
}

void gen_new(uint32_t i, struct ir ir[], struct func_detail *func)
{
    int type;
    uint32_t rl, rr;
    uint32_t op;
    char l[20], r[20], class[20];
    if[i].addressed = 1;
    ir[i].generated = 1;
    sscanf(ir[i].code, "%s%d%s", l, &type, r);
    rr = get_var(r, ir+i, func);
    
    gen_addu(rr, REG_ZERO, REG_A+0, ir+i);
    rl = gen_realloc_addr("malloc", ir+i);
    op = OP_JALR | rl << TO_RS | REG_RA << TO_RD;
    tcode[ir[i].number++] = op;
    DBGPRINT("jalr %u\n", l);
    gen_nop(ir+i);
    
    save_var(l, REG_A+0, ir+i, func);
    
    if (type == D_CLASS)
    {
        sscanf(ir[i].code, "%s%d%s", l, &type, r, class);
        struct symhash *r = sym_get(&root, class);
        struct class_detail *detail = r->detail;
        uint32_t classtab = gen_realloc_addr(".class", ir+i);
        uint32_t treg = get_tmp_reg();
        gen_addiu(classtab, detail->offset, treg, ir+i);
        gen_sw(REG_A+0, 0, treg, ir+i);
    }
}

void gen_ret(uint32_t i, struct ir ir[])
{
    uint32_t op;
    ir[i].addressed = 1;
    ir[i].generated = 1;
    op = OP_JR | REG_RA << TO_RS;  
    tcode[ir[i].number++] = op;
    DBGPRINT("jr %u\n", REG_RA);
    gen_nop(ir+i)
    return;
}

void gen_save_regs(uint32_t i, struct ir ir[], struct func_detail *func)
{
    uint32_t rsize = get_tmp_reg();
    uint32_t formals;
    ir[i].addressed = 1;
    ir[i].generated = 1;
    gen_addiu(REG_SP, REG_SP, -(func->stacksize+PSIZE), ir+i);
    gen_sw(REG_SP, func->stacksize+PSIZE+PSIZE, REG_RA, ir+i);
    gen_sw(REG_SP, func->stacksize+PSIZE, REG_STACK, ir+i);
    for (formals=0; formals<=func->formalsize; formals+=PSIZE)
    {
        if (formals >= 4)
            break;
        gen_sw(REG_SP, func->stacksize-formals, REG_A+formals/PSIZE, ir+i);
    }
    gen_addu(REG_SP, REG_ZERO, REG_STACK, ir+i);
}

void gen_restore_regs(uint32_t i, struct ir ir[], struct func_detail *func)
{
    uint32_t rsize = get_tmp_reg();
    uint32_t formals;
    ir[i].addressed = 1;
    ir[i].generated = 1;
    gen_lw(REG_SP, REG_STACK, func->stacksize+PSIZE, ir+i);
    gen_lw(REG_SP, REG_RA, func->stacksize+PSIZE+PSIZE, ir+i);
    gen_addiu(REG_SP, REG_SP, func->stacksize+PSIZE, ir+i);
}

void gen_nop(struct ir *ir)
{
    tcode[ir->number++] = 0;
}

void gen_branch(uint32_t i, struct ir ir[], struct func_detail *func)
{
    char e[20];
    uint32_t irnum;
    uint32_t offset=0;
    uint32_t j;
    sscanf(ir[i].code, "%s%u", e, &irnum);
    if (ir[i].generated)
    {
        if (ir[i].addressed)
            return;
        for (j=0; j<ir[i].number; ++j)
        {
            if (ir[i].bcode[j] & 0xfc == OP_BNE)
            {
                offset += ir[i].number - j;
                uint32_t k;
                for (k=i+1; k<=irnum; ++k)
                    offset += ir[k].number;
                --offset;
                ir[i].bcode[j] |= offset;
                break;
            }
        }
    }
    else
    {
        ir[i].generated = 1;
        uint32_t r = get_var(e, ir+i, func);
        uint32_t op = OP_BNE | r << TO_RS | REG_ZERO << TO_RT;
        if (i >= irnum)
        {
            ir[i].addressed = 1;
            offset += ir[i].number;
            for (j=irnum; j<i; ++j)
            {
                offset += ir[j].number;
            }
            offset = -offset;
            --offset;
            op |= offset;
        }
        else
        {
            ir[i].addressed = 0;
        }
        tcode[ir[i].number++] = op;
        BGPRINT("bne %u,0,#%d\n", r, offset);
        gen_nop(ir+i);
    }
}

void gen_j(uint32_t i, struct ir ir[], struct func_detail *func)
{
    uint32_t irnum;
    uint32_t offset=0;
    uint32_t j;
    sscanf(ir[i].code, "%u", &irnum);
    if (ir[i].generated)
    {
        if (ir[i].addressed)
            return;
        for (j=0; j<ir[i].number; ++j)
        {
            if (ir[i].bcode[j] & 0xfc == OP_BEQ)
            {
                offset += ir[i].number - j;
                uint32_t k;
                for (k=i+1; k<=irnum; ++k)
                    offset += ir[k].number;
                --offset;
                ir[i].bcode[j] |= offset;
                break;
            }
        }
    }
    else
    {
        ir[i].generated = 1;
        uint32_t op = OP_BEQ | REG_ZERO << TO_RS | REG_ZERO << TO_RT;
        if (i >= irnum)
        {
            ir[i].addressed = 1;
            offset += ir[i].number;
            for (j=irnum; j<i; ++j)
            {
                offset += ir[j].number;
            }
            offset = -offset;
            --offset;
            op |= offset;
        }
        else
        {
            ir[i].addressed = 0;
        }
        tcode[ir[i].number++] = op;
        DBGPRINT("b #%d\n", offset);
        gen_nop(ir+i);
    }
}

void gen_single(uint32_t i, struct ir ir[], struct func_detail *func)
{
    char l[20], r[20], op[3];
    uint32_t rr,rl;
    ir[i].generated = 1;
    ir[i].addressed = 1;
    sscanf(ir[i].code, "%s%s%s", l, r, op);
    rr = get_var(r, ir[i].env, ir+i);
    if (op[0] == '!')
    {
        rr = gen_not(rr, ir);
    }
    save_var(l, rr, ir+i, func);
}

void gen_double(uint32_t i, struct ir ir[], struct func_detail *func)
{
    char l[20], r[20], r2[20], op[3];
    uint32_t rr, rr2, rl, rm;
    ir[i].generated = 1;
    ir[i].addressed = 1;
    sscanf(ir[i].code, "%s%s%s%s", l, r, r2, op);
    rr = get_var(r, ir[i].env, ir+i);
    rr2 = get_var(r2, ir[i].env, ir+i);
    rm = get_tmp_reg();
    switch(op[0])
    {
        case '+':
            gen_addu(rr, rr2, rm, ir);
            break;
        case '-':
            gen_subu(rr, rr2, rm, ir);
            break;
        case '*':
            gen_mul(rr, rr2, rm, ir);
            break;
        case '/':
            gen_div(rr, rr2, rm, ir);
            break;
        case '%':
            gen_div2(rr, rr2, rm, ir);
            break;
        case '<':
            if (op[1])
                gen_le(rr, rr2, rm, ir);
            else
                gen_slt(rr, rr2, rm, ir);
            break;
        case '>':
            if (op[1])
                gen_ge(rr, rr2, rm, ir);
            else
                gen_slt(rr2, rr, rm, ir);
            break;
        case '=':
            gen_equ(rr, rr2, rm, ir);
            break;
        case '!':
            gen_subu(rr, rr2, rm, ir);
            break;
        case '&':
            gen_and(rr, rr2, rm, ir);
            break;
        case '|':
            gen_or(rr, r2, rm, ir);
            break;
        default:
            ;
    }
    save_var(l, rm, ir+i, func);
}

void gen_code(uint32_t i, struct ir ir[], struct func_detail *func)
{
    uint32_t new = 1;
    if (ir[i].generated && ir[i].addressed)
        return;
    if (ir[i].generated)
        new = 0;
    reset_s();
    switch (ir[i].type)
    {
        case IR_SINGLE:
            gen_single(i, ir, func);
            break;
        case IR_DOUBLE:
            gen_double(i, ir, func);
            break;
        case IR_CALL_MEMBER:
            gen_call_mem(i, ir, func);
            break;
        case IR_CALL:
            gen_call(i, ir);
            break;
        case IR_RET:
            gen_ret(i, ir);
            break;
        case IR_SAVE_REGS:
            gen_save_regs(i, ir, func);
            break;
        case IR_RESTORE_REGS:
            gen_restore_regs(i, ir, func);
            break;
        case IR_PRINT:
            gen_print(i, ir, new);
            break;
        case IR_NEW:
            gen_new(i, ir, new);
            break;
        case IR_B:
            gen_branch(i, ir, func);
            break;
        case IR_J:
            gen_j(i, ir, func);
            break;
        default:
            break;
    }
    if (new)
    {
        uint32_t size = ir[i].number*sizeof(uint32_t);
        ir[i].bcode = malloc(size);
        memcpy(ir[i].bcode, tcode, size);
    }
}
