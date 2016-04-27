#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define PRINT(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#define PUTCHAR(c) putchar(c)
#else
#define PRINT(fmt, ...)
#define PUTCHAR(c)
#endif

void printindent(int indent)
{
    int i;
    for (i=0; i<indent; ++i)
    {/*
        if (indent-i <= 2)
            PUTCHAR('-');
        else */
            PUTCHAR(' ');
    }
}

#define ind  if (!s)\
                return;\
              printindent(indent);

#define in  if (!s)\
              return;
              
parseit(ident)
{
    ind;
    PRINT("identifier: %s\n", s->text);
}

parseit(const)
{
    ind;
    switch (s->type)
    {
    case C_ICONST:
        PRINT("int const: %d\n", s->i_val);
        break;
    case C_BCONST:
        PRINT("bool const: %s\n", s->i_val?"true":"false");
        break;
    case C_DCONST:
        PRINT("double const: %f\n", s->d_val);
        break;
    case C_SCONST:
        PRINT("string const: %s\n", s->s_val);
        break;
    default:
        return;
    }
}

parseit(null)
{
    ind;
    PRINT("NULL\n");
}

parseit(formals)
{
    ind;
    PRINT("formals:\n");
    struct tformals *i;
    if (s->formals->tformals)
    {
        for (i=s->formals->tformals->tformals; i; i=i->next)
            parse_var(indent+2, i->var);
    }
}

parseit(type)
{
    ind;
    if (s->vtype->is_basic)
    {
        switch(s->vtype->btype)
        {
        case D_INT:
            PRINT("type: int\n");
            break;
        case D_BOOL:
            PRINT("type: bool\n");
            break;
        case D_DOUBLE:
            PRINT("type: double\n");
            break;
        case D_STRING:
            PRINT("type: string\n");
            break;
        default:
            return;
        }
    }
    else if (s->vtype->is_array)
    {
        PRINT("type: array of\n");
        parse_type(indent+2, s->vtype->arr_type);
    }
    else
    {
        PRINT("type: class\n");
        parse_ident(indent+2, s->vtype->id);
    }
}

parseit(var)
{
    in;
    parse_type(indent, s->var->type);
    parse_ident(indent, s->var->id);
}

parseit(vardefine)
{
    ind;
    PRINT("var define:\n");
    parse_var(indent+2, s->vardefine->var);
}

parseit(vardefines)
{
    struct vardefines *i;
    ind;
    PRINT("var define section:\n");
    for (i=s->vardefines; i; i=i->next)
        parse_vardefine(indent+2, i->vardefine);
}

parseit(expr_with_comma)
{
    in;
    struct expr_with_comma *i;
    for (i=s->expr_with_comma; i; i=i->next)
        parse_expr(indent+2, i->expr);
}

parseit(actuals)
{
    ind;
    PRINT("actuals:\n");
    parse_expr_with_comma(indent+2, s->actuals->expr_with_comma);
}

parseit(call)
{
    ind;
    if (s->call->is_member)
    {
        PRINT("member function call:\n");
        parse_expr(indent+2, s->call->expr);
        parse_ident(indent+2, s->call->id);
        parse_actuals(indent+2, s->call->actuals);
    }
    else
    {
        PRINT("function call:\n");
        parse_ident(indent+2, s->call->id);
        parse_actuals(indent+2, s->call->actuals);
    }
}

parseit(lvalue)
{
    ind;
    switch (s->lvalue->lvalue_type)
    {
    case LVAL_IDENT:
        PRINT("identifier as lvalue: \n");
        parse_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_MEMBER:
        PRINT("member as lvalue: \n");
        parse_expr(indent+2, s->lvalue->expr1);
        parse_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_ARRAY:
        PRINT("array elem as lvalue: \n");
        parse_expr(indent+2, s->lvalue->expr1);
        parse_expr(indent+2, s->lvalue->expr2);
    }
}

parseit(expr)
{
    ind;
    switch (s->expr->expr_type)
    {
    case EXPR_ASSIGN:
        PRINT("assign expr:\n");
        parse_lvalue(indent+2, s->expr->lvalue);
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_CONST:
        PRINT("const expr:\n");
        parse_const(indent+2, s->expr->constant);
        break;
    case EXPR_LVAL:
        PRINT("lvalue expr:\n");
        parse_lvalue(indent+2, s->expr->lvalue);
        break;
    case EXPR_THIS:
        PRINT("this pointer expr:\n");
        break;
    case EXPR_CALL:
        PRINT("function call expr:\n");
        parse_call(indent+2, s->expr->call);
        break;
    case EXPR_PRIORITY:
        PRINT("(expr):\n");
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_PLUS:
        PRINT("+ expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MINUS:
        PRINT("- expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MUL:
        PRINT("* expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_DIV:
        PRINT("/ expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_IDIV:
        PUTCHAR('%');
        PRINT(" expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LT:
        PRINT("< expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LE:
        PRINT("<= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GT:
        PRINT("> expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GE:
        PRINT(">= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_EQU:
        PRINT("== expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NE:
        PRINT("!= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_AND:
        PRINT("&& expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_OR:
        PRINT("|| expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NOT:
        PRINT("! expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_READINTEGER:
        PRINT("read integer expr:\n");
        break;
    case EXPR_READLINE:
        PRINT("read line expr:\n");
        break;
    case EXPR_NEW:
        PRINT("new expr:\n");
        parse_ident(indent+2, s->expr->id);
        break;
    case EXPR_NEWARRAY:
        PRINT("newarray expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_type(indent+2, s->expr->id);
        break;
    }
}

parseit(ifstm)
{
    ind;
    PRINT("if statement:\n");
    parse_expr(indent+2, s->if_stm->expr);
    ind;
    PRINT("then:\n");
    parse_stm(indent+2, s->if_stm->stm1);
    ind;
    PRINT("else:\n");
    parse_stm(indent+2, s->if_stm->stm2);
}

parseit(whilestm)
{
    ind;
    PRINT("while statement:\n");
    parse_expr(indent+2, s->whilestm->expr);
    parse_stm(indent+2, s->whilestm->stm);
}

parseit(forstm)
{
    ind;
    PRINT("for statement:\n");
    ind;
    PRINT("INIT:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not1);
    ind;
    PRINT("COND:\n");
    parse_expr(indent+2, s->forstm->expr);
    ind;
    PRINT("ACC:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not2);
}

parseit(retstm)
{
    ind;
    PRINT("return statement:\n");
    parse_expr_or_not(indent+2, s->returnstm->expr_or_not);
}

parseit(breakstm)
{
    ind;
    PRINT("break statement\n");
}

parseit(printstm)
{
    ind;
    PRINT("print statement:\n");
    parse_expr(indent+2, s->printstm->expr);
}

parseit(expr_or_not)
{
    in;
    parse_expr(indent, s->expr_or_not->expr);
}

parseit(stm)
{
    in;
    switch (s->stm->stm_type)
    {
    case STM_EMPTY:
        break;
    case STM_EXPR:
        ind;
        PRINT("expr statement:\n");
        parse_expr(indent+2, s->stm->expr);
        break;
    case STM_IF:
        parse_ifstm(indent, s->stm->s_stm);
        break;
    case STM_WHILE:
        parse_whilestm(indent, s->stm->s_stm);
        break;
    case STM_FOR:
        parse_forstm(indent, s->stm->s_stm);
        break;
    case STM_BREAK:
        parse_breakstm(indent, s->stm->s_stm);
        break;
    case STM_RET:
        parse_retstm(indent, s->stm->s_stm);
        break;
    case STM_PRINT:
        parse_printstm(indent, s->stm->s_stm);
        break;
    case STM_BLOCK:
        parse_stmblock(indent, s->stm->stmblock);
        break;
    }
}

parseit(stms)
{
    ind;
    PRINT("statement section:\n");
    struct stms *i;
    for (i=s->stms; i; i=i->next)
    {
        parse_stm(indent+2, i->stm);
    }
}

parseit(stmblock)
{
    ind;
    PRINT("statement block:\n");
    parse_vardefines(indent+2, s->stmblock->vardefines);
    parse_stms(indent+2, s->stmblock->stms);
}

parseit(funcdefine)
{
    ind;
    if (s->funcdefine->is_void)
    {
        PRINT("void function define:\n");
        parse_ident(indent+2, s->funcdefine->id);
        parse_formals(indent+2, s->funcdefine->formals);
        parse_stmblock(indent+2, s->funcdefine->stmblock);   
    }
    else
    {
        PRINT("function define:\n");
        parse_type(indent+2, s->funcdefine->type);
        parse_ident(indent+2, s->funcdefine->id);
        parse_formals(indent+2, s->funcdefine->formals);
        parse_stmblock(indent+2, s->funcdefine->stmblock);
    }
}

parseit(field)
{
    in;
    //PRINT("class field:\n");
    if (s->field->is_vardefine)
    {
        parse_vardefine(indent, s->field->vardefine);
    }
    else
    {
        parse_funcdefine(indent, s->field->funcdefine);
    }
}

parseit(fields)
{
    in;
    struct fields *i;
    for (i=s->fields; i; i=i->next)
    {
        parse_field(indent, i->field);
    }
}

parseit(extend)
{
    ind;
    PRINT("class extends:\n");
    parse_ident(indent+2, s->extend->id);
}

parseit(id_with_comma)
{
    in;
    struct id_with_comma *i;
    for (i=s->id_with_comma; i; i=i->next)
    {
        parse_ident(indent, i->id);
    }
}

parseit(implement)
{
    ind;
    PRINT("class implements protypes:\n");
    parse_id_with_comma(indent+2 ,s->implement->id_with_comma);
}

parseit(classdefine)
{
    ind;
    PRINT("class define:\n");
    parse_ident(indent+2, s->classdefine->id);
    parse_extend(indent+2, s->classdefine->extend);
    parse_implement(indent+2, s->classdefine->implement);
    parse_fields(indent+2, s->classdefine->fields);
}

parseit(protype)
{
    ind;
    if (s->protype->is_void)
    {
        PRINT("void protype:\n");
        parse_ident(indent+2, s->protype->id);
        parse_formals(indent+2, s->protype->formals);
    }
    else
    {
        PRINT("protype:\n");
        parse_type(indent+2, s->protype->type);
        parse_ident(indent+2, s->protype->id);
        parse_formals(indent+2, s->protype->formals);
    }
    
}

parseit(protypes)
{
    in;
    struct protypes *i;
    for (i=s->protypes; i; i=i->next)
    {
        parse_protype(indent, i->protype);
    }
}

parseit(interfacedefine)
{
    ind;
    PRINT("interface define:\n");
    parse_ident(indent+2, s->interfacedefine->id);
    parse_protypes(indent+2, s->interfacedefine->protypes);
}

parseit(define)
{
    in;
    switch (s->define->define_type)
    {
    case DEFINE_VAR:
        parse_vardefine(indent, s->define->s_define);
        break;
    case DEFINE_FUNC:
        parse_funcdefine(indent, s->define->s_define);
        break;
    case DEFINE_CLASS:
        parse_classdefine(indent, s->define->s_define);
        break;
    case DEFINE_INTERFACE:
        parse_interfacedefine(indent, s->define->s_define);
        break;
    }
}

parseit(program)
{
    ind;
    PRINT("PROGRAM:\n");
    struct program *i;
    for (i=s->program; i; i=i->next)
    {
        parse_define(indent+2, i->define);
    }
}
