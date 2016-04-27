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
              
printit(ident)
{
    ind;
    PRINT("identifier: %s\n", s->text);
}

printit(const)
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

printit(null)
{
    ind;
    PRINT("NULL\n");
}

printit(formals)
{
    ind;
    PRINT("formals:\n");
    struct tformals *i;
    if (s->formals->tformals)
    {
        for (i=s->formals->tformals->tformals; i; i=i->next)
            print_var(indent+2, i->var);
    }
}

printit(type)
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
        print_type(indent+2, s->vtype->arr_type);
    }
    else
    {
        PRINT("type: class\n");
        print_ident(indent+2, s->vtype->id);
    }
}

printit(var)
{
    in;
    print_type(indent, s->var->type);
    print_ident(indent, s->var->id);
}

printit(vardefine)
{
    ind;
    PRINT("var define:\n");
    print_var(indent+2, s->vardefine->var);
}

printit(vardefines)
{
    struct vardefines *i;
    ind;
    PRINT("var define section:\n");
    for (i=s->vardefines; i; i=i->next)
        print_vardefine(indent+2, i->vardefine);
}

printit(expr_with_comma)
{
    in;
    struct expr_with_comma *i;
    for (i=s->expr_with_comma; i; i=i->next)
        print_expr(indent+2, i->expr);
}

printit(actuals)
{
    ind;
    PRINT("actuals:\n");
    print_expr_with_comma(indent+2, s->actuals->expr_with_comma);
}

printit(call)
{
    ind;
    if (s->call->is_member)
    {
        PRINT("member function call:\n");
        print_expr(indent+2, s->call->expr);
        print_ident(indent+2, s->call->id);
        print_actuals(indent+2, s->call->actuals);
    }
    else
    {
        PRINT("function call:\n");
        print_ident(indent+2, s->call->id);
        print_actuals(indent+2, s->call->actuals);
    }
}

printit(lvalue)
{
    ind;
    switch (s->lvalue->lvalue_type)
    {
    case LVAL_IDENT:
        PRINT("identifier as lvalue: \n");
        print_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_MEMBER:
        PRINT("member as lvalue: \n");
        print_expr(indent+2, s->lvalue->expr1);
        print_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_ARRAY:
        PRINT("array elem as lvalue: \n");
        print_expr(indent+2, s->lvalue->expr1);
        print_expr(indent+2, s->lvalue->expr2);
    }
}

printit(expr)
{
    ind;
    switch (s->expr->expr_type)
    {
    case EXPR_ASSIGN:
        PRINT("assign expr:\n");
        print_lvalue(indent+2, s->expr->lvalue);
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_CONST:
        PRINT("const expr:\n");
        print_const(indent+2, s->expr->constant);
        break;
    case EXPR_LVAL:
        PRINT("lvalue expr:\n");
        print_lvalue(indent+2, s->expr->lvalue);
        break;
    case EXPR_THIS:
        PRINT("this pointer expr:\n");
        break;
    case EXPR_CALL:
        PRINT("function call expr:\n");
        print_call(indent+2, s->expr->call);
        break;
    case EXPR_PRIORITY:
        PRINT("(expr):\n");
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_PLUS:
        PRINT("+ expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MINUS:
        PRINT("- expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MUL:
        PRINT("* expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_DIV:
        PRINT("/ expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_IDIV:
        PUTCHAR('%');
        PRINT(" expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LT:
        PRINT("< expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LE:
        PRINT("<= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GT:
        PRINT("> expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GE:
        PRINT(">= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_EQU:
        PRINT("== expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NE:
        PRINT("!= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_AND:
        PRINT("&& expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_OR:
        PRINT("|| expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NOT:
        PRINT("! expr:\n");
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_READINTEGER:
        PRINT("read integer expr:\n");
        break;
    case EXPR_READLINE:
        PRINT("read line expr:\n");
        break;
    case EXPR_NEW:
        PRINT("new expr:\n");
        print_ident(indent+2, s->expr->id);
        break;
    case EXPR_NEWARRAY:
        PRINT("newarray expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_type(indent+2, s->expr->id);
        break;
    }
}

printit(ifstm)
{
    ind;
    PRINT("if statement:\n");
    print_expr(indent+2, s->if_stm->expr);
    ind;
    PRINT("then:\n");
    print_stm(indent+2, s->if_stm->stm1);
    ind;
    PRINT("else:\n");
    print_stm(indent+2, s->if_stm->stm2);
}

printit(whilestm)
{
    ind;
    PRINT("while statement:\n");
    print_expr(indent+2, s->whilestm->expr);
    print_stm(indent+2, s->whilestm->stm);
}

printit(forstm)
{
    ind;
    PRINT("for statement:\n");
    ind;
    PRINT("INIT:\n");
    print_expr_or_not(indent+2, s->forstm->expr_or_not1);
    ind;
    PRINT("COND:\n");
    print_expr(indent+2, s->forstm->expr);
    ind;
    PRINT("ACC:\n");
    print_expr_or_not(indent+2, s->forstm->expr_or_not2);
}

printit(retstm)
{
    ind;
    PRINT("return statement:\n");
    print_expr_or_not(indent+2, s->returnstm->expr_or_not);
}

printit(breakstm)
{
    ind;
    PRINT("break statement\n");
}

printit(printstm)
{
    ind;
    PRINT("print statement:\n");
    print_expr(indent+2, s->printstm->expr);
}

printit(expr_or_not)
{
    in;
    print_expr(indent, s->expr_or_not->expr);
}

printit(stm)
{
    in;
    switch (s->stm->stm_type)
    {
    case STM_EMPTY:
        break;
    case STM_EXPR:
        ind;
        PRINT("expr statement:\n");
        print_expr(indent+2, s->stm->expr);
        break;
    case STM_IF:
        print_ifstm(indent, s->stm->s_stm);
        break;
    case STM_WHILE:
        print_whilestm(indent, s->stm->s_stm);
        break;
    case STM_FOR:
        print_forstm(indent, s->stm->s_stm);
        break;
    case STM_BREAK:
        print_breakstm(indent, s->stm->s_stm);
        break;
    case STM_RET:
        print_retstm(indent, s->stm->s_stm);
        break;
    case STM_PRINT:
        print_printstm(indent, s->stm->s_stm);
        break;
    case STM_BLOCK:
        print_stmblock(indent, s->stm->stmblock);
        break;
    }
}

printit(stms)
{
    ind;
    PRINT("statement section:\n");
    struct stms *i;
    for (i=s->stms; i; i=i->next)
    {
        print_stm(indent+2, i->stm);
    }
}

printit(stmblock)
{
    ind;
    PRINT("statement block:\n");
    print_vardefines(indent+2, s->stmblock->vardefines);
    print_stms(indent+2, s->stmblock->stms);
}

printit(funcdefine)
{
    ind;
    if (s->funcdefine->is_void)
    {
        PRINT("void function define:\n");
        print_ident(indent+2, s->funcdefine->id);
        print_formals(indent+2, s->funcdefine->formals);
        print_stmblock(indent+2, s->funcdefine->stmblock);   
    }
    else
    {
        PRINT("function define:\n");
        print_type(indent+2, s->funcdefine->type);
        print_ident(indent+2, s->funcdefine->id);
        print_formals(indent+2, s->funcdefine->formals);
        print_stmblock(indent+2, s->funcdefine->stmblock);
    }
}

printit(field)
{
    in;
    //PRINT("class field:\n");
    if (s->field->is_vardefine)
    {
        print_vardefine(indent, s->field->vardefine);
    }
    else
    {
        print_funcdefine(indent, s->field->funcdefine);
    }
}

printit(fields)
{
    in;
    struct fields *i;
    for (i=s->fields; i; i=i->next)
    {
        print_field(indent, i->field);
    }
}

printit(extend)
{
    ind;
    PRINT("class extends:\n");
    print_ident(indent+2, s->extend->id);
}

printit(id_with_comma)
{
    in;
    struct id_with_comma *i;
    for (i=s->id_with_comma; i; i=i->next)
    {
        print_ident(indent, i->id);
    }
}

printit(implement)
{
    ind;
    PRINT("class implements protypes:\n");
    print_id_with_comma(indent+2 ,s->implement->id_with_comma);
}

printit(classdefine)
{
    ind;
    PRINT("class define:\n");
    print_ident(indent+2, s->classdefine->id);
    print_extend(indent+2, s->classdefine->extend);
    print_implement(indent+2, s->classdefine->implement);
    print_fields(indent+2, s->classdefine->fields);
}

printit(protype)
{
    ind;
    if (s->protype->is_void)
    {
        PRINT("void protype:\n");
        print_ident(indent+2, s->protype->id);
        print_formals(indent+2, s->protype->formals);
    }
    else
    {
        PRINT("protype:\n");
        print_type(indent+2, s->protype->type);
        print_ident(indent+2, s->protype->id);
        print_formals(indent+2, s->protype->formals);
    }
    
}

printit(protypes)
{
    in;
    struct protypes *i;
    for (i=s->protypes; i; i=i->next)
    {
        print_protype(indent, i->protype);
    }
}

printit(interfacedefine)
{
    ind;
    PRINT("interface define:\n");
    print_ident(indent+2, s->interfacedefine->id);
    print_protypes(indent+2, s->interfacedefine->protypes);
}

printit(define)
{
    in;
    switch (s->define->define_type)
    {
    case DEFINE_VAR:
        print_vardefine(indent, s->define->s_define);
        break;
    case DEFINE_FUNC:
        print_funcdefine(indent, s->define->s_define);
        break;
    case DEFINE_CLASS:
        print_classdefine(indent, s->define->s_define);
        break;
    case DEFINE_INTERFACE:
        print_interfacedefine(indent, s->define->s_define);
        break;
    }
}

printit(program)
{
    ind;
    PRINT("PROGRAM:\n");
    struct program *i;
    for (i=s->program; i; i=i->next)
    {
        print_define(indent+2, i->define);
    }
}
