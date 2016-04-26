#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

void printindent(int indent)
{
    int i;
    for (i=0; i<indent; ++i)
    {
        if (i%2)
            putchar('|');
        else if (i == indent-1)
            putchar('-');
        else 
            putchar(' ');
    }
}

#define ind  if (!s)\
                return;\
              printindent(indent);

printit(ident)
{
    ind;
    printf("identifier:%s\n", s->text);
}

printit(const)
{
    ind;
    switch (s->type)
    {
    case C_ICONST:
        printf("int const: %d\n", s->i_val);
        break;
    case C_BCONST:
        printf("bool const: %s\n", s->i_val?"true":"false");
        break;
    case C_DCONST:
        printf("double const: %f\n", s->d_val);
        break;
    case C_SCONST:
        printf("string const: %s\n", s->s_val);
        break;
    default:
        return;
    }
}

printit(null)
{
    ind;
    printf("NULL\n");
}

printit(formals)
{
    struct tformals *i;
    ind;
    if (s->formals->tformals)
        for (i=s->formals->tformals->tformals; i; i=i->next)
            print_var(indent, i->var);
}

printit(type)
{
    ind;
    if (s->vtype->is_basic)
    {
        switch(s->vtype->btype)
        {
        case D_INT:
            printf("type: int\n");
        case D_BOOL:
            printf("type: bool\n");
        case D_DOUBLE:
            printf("type: double\n");
        case D_STRING:
            printf("type: string\n");
        default:
            return;
        }
    }
    else if (s->vtype->is_array)
    {
        printf("type: array of\n");
        print_type(indent+2, s->vtype->arr_type);
    }
    else
    {
        printf("type: class\n");
        print_ident(indent+2, s->vtype->id);
    }
}

printit(var)
{
    ind;
    print_type(indent, s->var->type);
    print_ident(indent, s->var->id);
}

printit(vardefine)
{
    ind;
    printf("var define:\n");
    print_var(indent+2, s->vardefine->var);
}

printit(vardefines)
{
    struct vardefines *i;
    ind;
    printf("var define section:\n");
    for (i=s->vardefines; i; i=i->next)
        print_vardefine(indent+2, i->vardefine);
}

printit(expr_with_comma)
{
    struct expr_with_comma *i;
    ind;
    for (i=s->expr_with_comma; i; i=i->next)
        print_expr(indent+2, i->expr);
}

printit(actuals)
{
    ind;
    printf("actuals:\n");
    print_expr_with_comma(indent+2, s->actuals->expr_with_comma);
}

printit(call)
{
    ind;
    if (s->call->is_member)
    {
        printf("member function call:");
        print_expr(indent+2, s->call->expr);
        print_ident(indent+2, s->call->id);
        print_actuals(indent+2, s->call->actuals);
    }
    else
    {
        printf("function call:");
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
        printf("identifier as lvalue: \n");
        print_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_MEMBER:
        printf("member as lvalue: \n");
        print_expr(indent+2, s->lvalue->expr1);
        print_ident(indent+2, s->lvalue->id);
        break;
    case LVAL_ARRAY:
        printf("array elem as lvalue: \n");
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
        printf("assign expr:\n");
        print_lvalue(indent+2, s->expr->lvalue);
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_CONST:
        printf("const expr:\n");
        print_const(indent+2, s->expr->constant);
        break;
    case EXPR_LVAL:
        printf("lvalue expr:\n");
        print_lvalue(indent+2, s->expr->expr1);
        break;
    case EXPR_THIS:
        printf("this pointer expr:\n");
        break;
    case EXPR_CALL:
        printf("function call expr:\n");
        print_call(indent+2, s->expr->call);
        break;
    case EXPR_PRIORITY:
        printf("(expr):\n");
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_PLUS:
        printf("+ expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MINUS:
        printf("- expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MUL:
        printf("* expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_DIV:
        printf("/ expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_IDIV:
        putchar('%');
        printf(" expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LT:
        printf("< expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LE:
        printf("<= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GT:
        printf("> expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GE:
        printf(">= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_EQU:
        printf("== expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NE:
        printf("!= expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_AND:
        printf("&& expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_OR:
        printf("|| expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NOT:
        printf("! expr:\n");
        print_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_READINTEGER:
        printf("read integer expr:\n");
        break;
    case EXPR_READLINE:
        printf("read line expr:\n");
        break;
    case EXPR_NEW:
        printf("new expr:\n");
        print_ident(indent+2, s->expr->id);
        break;
    case EXPR_NEWARRAY:
        printf("newarray expr:\n");
        print_expr(indent+2, s->expr->expr1);
        print_type(indent+2, s->expr->id);
        break;
    }
}
