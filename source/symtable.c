#include "defs.h"
#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

static struct symhash *roothash=NULL;
static struct symres root={0, &roothash, NULL};
static struct symres *current=&root;

#define new_field(f) {struct symres *nt = malloc(sizeof(struct symres));\
                      nt->table = malloc(sizeof(struct symhash *)); \
                      *(nt->table) = NULL; \
                      nt->parent = current; \
                      current = nt;}

int sym_add(struct symres *table, const char * i,enum decaf_type t, void *d)
{
    struct symhash *tmp;
    HASH_FIND_STR( *(table->table), i, tmp);
    if (tmp)
        return -1;
    struct symhash *n=malloc(sizeof(struct symhash));
    n->name = i;
    n->type = t;
    n->define = d;
    HASH_ADD_KEYPTR( hh, *(table->table), n->name, strlen(n->name), n);
    return 0;
}

struct symhash *sym_get_no_recursive(struct symres *table, const char *i)
{
    struct symhash *n;
    HASH_FIND_STR(*(table->table), i, n);
    return n;
}

struct symhash *sym_get(struct symres *table, const char *i)
{
    struct symhash *n;
    while (table)
    {
        HASH_FIND_STR( *(table->table), i, n);
        if (n)
            return n;
        table = table->parent;
    }
    return NULL;
}

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
              
void parse_ident(int indent, struct semantics *s, int type)
{
    ind;
    if (type)
    {
        struct symhash *r = sym_get(current, s->text);
        if (r)
            switch(r->type)
            {
            case D_INT:
                DBGPRINT("identifier: %s, type: int\n", s->text);
                break;
            case D_BOOL:
                DBGPRINT("identifier: %s, type: bool\n", s->text);
                break;
            case D_DOUBLE:
                DBGPRINT("identifier: %s, type: double\n", s->text);
                break;
            case D_STRING:
                DBGPRINT("identifier: %s, type: string\n", s->text);
                break;
            case D_CLASS:
                DBGPRINT("identifier: %s, type: class\n", s->text);
                break;
            case D_FUNCTION:
                DBGPRINT("identifier: %s, type: function\n", s->text);
                break;
            case D_TYPE:
                DBGPRINT("identifier: %s, type: class type\n", s->text);
                break;
            case D_INTERFACE:
                DBGPRINT("identifier: %s, type: interface\n", s->text);
                break;
            default:
                DBGPRINT("identifier: %s\n", s->text);
            }
        else
            WPRINT("Fatal: Unknown identifier %s\n", s->text);
    }
    else
    {
        if (sym_get_no_recursive(current, s->text))
            WPRINT("Fatal: re-define of identifier %s\n", s->text);
        else    
            DBGPRINT("new identifier: %s\n", s->text);
    }
}

parseit(const)
{
    ind;
    switch (s->type)
    {
    case C_ICONST:
        DBGPRINT("int const: %d\n", s->i_val);
        break;
    case C_BCONST:
        DBGPRINT("bool const: %s\n", s->i_val?"true":"false");
        break;
    case C_DCONST:
        DBGPRINT("double const: %f\n", s->d_val);
        break;
    case C_SCONST:
        DBGPRINT("string const: %s\n", s->s_val);
        break;
    default:
        return;
    }
}

parseit(null)
{
    ind;
    DBGPRINT("NULL\n");
}

parseit(formals)
{
    ind;
    DBGPRINT("formals:\n");
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
            DBGPRINT("type: int\n");
            break;
        case D_BOOL:
            DBGPRINT("type: bool\n");
            break;
        case D_DOUBLE:
            DBGPRINT("type: double\n");
            break;
        case D_STRING:
            DBGPRINT("type: string\n");
            break;
        default:
            return;
        }
    }
    else if (s->vtype->is_array)
    {
        DBGPRINT("type: array of\n");
        parse_type(indent+2, s->vtype->arr_type);
    }
    else
    {
        DBGPRINT("type: class\n");
        parse_ident(indent+2, s->vtype->id, 0);
    }
}

parseit(var)
{
    in;
    parse_type(indent, s->var->type);
    parse_ident(indent, s->var->id, 0);
    sym_add(current, s->var->id->text, s->var->type->vtype->btype, 0);
}

parseit(vardefine)
{
    ind;
    DBGPRINT("var define:\n");
    parse_var(indent+2, s->vardefine->var);
}

parseit(vardefines)
{
    struct vardefines *i;
    ind;
    DBGPRINT("var define section:\n");
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
    DBGPRINT("actuals:\n");
    parse_expr_with_comma(indent+2, s->actuals->expr_with_comma);
}

parseit(call)
{
    ind;
    if (s->call->is_member)
    {
        DBGPRINT("member function call:\n");
        parse_expr(indent+2, s->call->expr);
        parse_ident(indent+2, s->call->id, 1);
        parse_actuals(indent+2, s->call->actuals);
    }
    else
    {
        DBGPRINT("function call:\n");
        parse_ident(indent+2, s->call->id, 1);
        parse_actuals(indent+2, s->call->actuals);
    }
}

parseit(lvalue)
{
    ind;
    switch (s->lvalue->lvalue_type)
    {
    case LVAL_IDENT:
        DBGPRINT("identifier as lvalue: \n");
        parse_ident(indent+2, s->lvalue->id, 1);
        break;
    case LVAL_MEMBER:
        DBGPRINT("member as lvalue: \n");
        parse_expr(indent+2, s->lvalue->expr1);
        parse_ident(indent+2, s->lvalue->id, 1);
        break;
    case LVAL_ARRAY:
        DBGPRINT("array elem as lvalue: \n");
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
        DBGPRINT("assign expr:\n");
        parse_lvalue(indent+2, s->expr->lvalue);
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_CONST:
        DBGPRINT("const expr:\n");
        parse_const(indent+2, s->expr->constant);
        break;
    case EXPR_LVAL:
        DBGPRINT("lvalue expr:\n");
        parse_lvalue(indent+2, s->expr->lvalue);
        break;
    case EXPR_THIS:
        DBGPRINT("this pointer expr:\n");
        break;
    case EXPR_CALL:
        DBGPRINT("function call expr:\n");
        parse_call(indent+2, s->expr->call);
        break;
    case EXPR_PRIORITY:
        DBGPRINT("(expr):\n");
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_PLUS:
        DBGPRINT("+ expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MINUS:
        DBGPRINT("- expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_MUL:
        DBGPRINT("* expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_DIV:
        DBGPRINT("/ expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_IDIV:
        PUTCHAR('%');
        DBGPRINT(" expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LT:
        DBGPRINT("< expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_LE:
        DBGPRINT("<= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GT:
        DBGPRINT("> expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_GE:
        DBGPRINT(">= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_EQU:
        DBGPRINT("== expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NE:
        DBGPRINT("!= expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_AND:
        DBGPRINT("&& expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_OR:
        DBGPRINT("|| expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_expr(indent+2, s->expr->expr2);
        break;
    case EXPR_NOT:
        DBGPRINT("! expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        break;
    case EXPR_READINTEGER:
        DBGPRINT("read integer expr:\n");
        break;
    case EXPR_READLINE:
        DBGPRINT("read line expr:\n");
        break;
    case EXPR_NEW:
        DBGPRINT("new expr:\n");
        parse_ident(indent+2, s->expr->id, 1);
        break;
    case EXPR_NEWARRAY:
        DBGPRINT("newarray expr:\n");
        parse_expr(indent+2, s->expr->expr1);
        parse_type(indent+2, s->expr->id);
        break;
    }
}

parseit(ifstm)
{
    ind;
    DBGPRINT("if statement:\n");
    parse_expr(indent+2, s->if_stm->expr);
    ind;
    DBGPRINT("then:\n");
    parse_stm(indent+2, s->if_stm->stm1);
    ind;
    DBGPRINT("else:\n");
    parse_stm(indent+2, s->if_stm->stm2);
}

parseit(whilestm)
{
    ind;
    DBGPRINT("while statement:\n");
    parse_expr(indent+2, s->whilestm->expr);
    parse_stm(indent+2, s->whilestm->stm);
}

parseit(forstm)
{
    ind;
    DBGPRINT("for statement:\n");
    ind;
    DBGPRINT("INIT:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not1);
    ind;
    DBGPRINT("COND:\n");
    parse_expr(indent+2, s->forstm->expr);
    ind;
    DBGPRINT("ACC:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not2);
    ind;
    DBGPRINT("STATEMENT:\n");
    parse_stm(indent+2, s->forstm->stm);
}

parseit(retstm)
{
    ind;
    DBGPRINT("return statement:\n");
    parse_expr_or_not(indent+2, s->returnstm->expr_or_not);
}

parseit(breakstm)
{
    ind;
    DBGPRINT("break statement\n");
}

parseit(printstm)
{
    ind;
    DBGPRINT("print statement:\n");
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
        DBGPRINT("expr statement:\n");
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
    DBGPRINT("statement section:\n");
    struct stms *i;
    for (i=s->stms; i; i=i->next)
    {
        parse_stm(indent+2, i->stm);
    }
}

parseit(stmblock)
{
    ind;
    DBGPRINT("statement block:\n");
    parse_vardefines(indent+2, s->stmblock->vardefines);
    parse_stms(indent+2, s->stmblock->stms);
}

parseit(funcdefine)
{
    ind;
    if (s->funcdefine->is_void)
    {
        DBGPRINT("void function define:\n");
        parse_ident(indent+2, s->funcdefine->id, 0);
        new_field(current);
        parse_formals(indent+2, s->funcdefine->formals);
        parse_stmblock(indent+2, s->funcdefine->stmblock);   
        sym_add(current->parent, s->funcdefine->id->text, D_FUNCTION, current);
        current = current->parent;
    }
    else
    {
        DBGPRINT("function define:\n");
        parse_type(indent+2, s->funcdefine->type);
        parse_ident(indent+2, s->funcdefine->id, 0);
        new_field(current);
        parse_formals(indent+2, s->funcdefine->formals);
        parse_stmblock(indent+2, s->funcdefine->stmblock);
        sym_add(current->parent, s->funcdefine->id->text, D_FUNCTION, 0);
    }
}

void parse_field(int indent, struct semantics *s, int no_func)
{
    in;
    //DBGPRINT("class field:\n");
    if (s->field->is_vardefine && no_func)
    {
        parse_vardefine(indent, s->field->vardefine);
    }
    else if (!no_func)
    {
        parse_funcdefine(indent, s->field->funcdefine);
    }
}

void parse_fields(int indent, struct semantics *s, int no_func)
{
    in;
    struct fields *i;
    for (i=s->fields; i; i=i->next)
    {
        parse_field(indent, i->field, no_func);
    }
}

parseit(extend)
{
    ind;
    DBGPRINT("class extends:\n");
    parse_ident(indent+2, s->extend->id, 1);
}

parseit(id_with_comma)
{
    in;
    struct id_with_comma *i;
    for (i=s->id_with_comma; i; i=i->next)
    {
        parse_ident(indent, i->id, 1);
    }
}

parseit(implement)
{
    ind;
    DBGPRINT("class implements protypes:\n");
    parse_id_with_comma(indent+2 ,s->implement->id_with_comma);
}

/*******TO DO: NEED TWO ROUND: THE FIRST TO ACQUIRE ALL IDENTIFIER, FUNCTION AND VTABLE; THE SECOND TO GENERATE CODE FOR FUNCTION****************/

parseit(classdefine)
{
    ind;
    DBGPRINT("class define:\n");
    parse_ident(indent+2, s->classdefine->id, 0);
    struct class_detail *new_func=malloc(sizeof(struct class_detail));
    parse_extend(indent+2, s->classdefine->extend);
    parse_implement(indent+2, s->classdefine->implement);
    new_field(current);
    parse_fields(indent+2, s->classdefine->fields, 1);
    parse_fields(indent+2, s->classdefine->fields, 0);
    sym_add(current->parent, s->classdefine->id->text, D_TYPE, current);
    current = current->parent;
}

parseit(protype)
{
    ind;
    if (s->protype->is_void)
    {
        DBGPRINT("void protype:\n");
        parse_ident(indent+2, s->protype->id, 0);
        parse_formals(indent+2, s->protype->formals);
    }
    else
    {
        DBGPRINT("protype:\n");
        parse_type(indent+2, s->protype->type);
        parse_ident(indent+2, s->protype->id, 0);
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
    DBGPRINT("interface define:\n");
    parse_ident(indent+2, s->interfacedefine->id, 0);
    sym_add(current, s->interfacedefine->id->text, D_INTERFACE, 0);
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
    DBGPRINT("PROGRAM:\n");
    struct program *i;
    for (i=s->program; i; i=i->next)
    {
        parse_define(indent+2, i->define);
    }
}
