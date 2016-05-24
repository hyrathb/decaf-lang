#include "defs.h"
#include "ir.h"
#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>

static struct symhash *roothash=NULL;
static struct symres root={SCOPE_GLOBAL, 0, 0, &roothash, NULL};
static struct symres *current=&root;

#define new_field(s) {struct symres *nt = malloc(sizeof(struct symres));\
                      nt->table = malloc(sizeof(struct symhash *)); \
                      nt->scope = s; \
                      nt->current_var_offset = 0; \
                      nt->current_func_offset = 0; \
                      *(nt->table) = NULL; \
                      nt->parent = current; \
                      current = nt;}

int sym_add(struct symres *table, const char * i,enum decaf_type t, void *d)
{
    struct symhash *tmp;
    HASH_FIND_STR( *(table->table), i, tmp);
    if (tmp)
    {
        return -1;
    }
    struct symhash *n=malloc(sizeof(struct symhash));
    n->name = i;
    n->type = t;
    n->detail = d;
    HASH_ADD_KEYPTR( hh, *(table->table), n->name, strlen(n->name), n);
    return 0;
}

struct symhash *sym_class_get(struct class_detail *class, const char *i)
{
    struct symhash *n;
    while (class)
    {
        struct symres *table = class->env;
        HASH_FIND_STR(*(table->table), i, n);
        if (n)
            return n;
        class = class->base;
    }
    return NULL;
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

struct symhash *sym_get_no_recursive(struct symres *table, const char *i)
{
    struct symhash *n;
    HASH_FIND_STR( *(table->table), i, n);
    return n;
}

struct class_detail *sym_get_class(struct class_detail *class, const char *i)
{
    struct symhash *n;
    while (class)
    {
        struct symres *table = class->env;
        HASH_FIND_STR(*(table->table), i, n);
        if (n)
            return class;
        class = class->base;
    }
    return NULL;
}


uint64_t base_size(struct class_detail *base)
{
    if (base)
        return base->size;
    return 0;
}

enum decaf_type get_basic_type(struct type *type)
{
    while (type->btype == D_ARRAY)
        type = type->arr_type->vtype;
    return type->btype;
}

uint64_t get_array_dims(struct type *type)
{
    uint64_t ret = 0;
    while (type->btype == D_ARRAY)
    {
        ++ret;
        type = type->arr_type->vtype;
    }
    return ret;
}

struct class_detail * get_array_class(struct type *type)
{
    while (type->btype == D_ARRAY)
        type = type->arr_type->vtype;
    struct symhash *r = sym_get(current, type->id->text);
    if (r && r->type == D_TYPE)
        return r->detail;
    else
    {
        ERRPRINT("class %s not defined.\n", type->id->text);
        return NULL;
    }
        
        
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
        {
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
            return;
        }
        else
            WPRINT("Fatal: Unknown identifier %s\n", s->text);
    }
    else
    {
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
    struct var_detail *new_var = malloc(sizeof(struct var_detail));
    new_var->is_array = s->var->type->vtype->is_array;
    new_var->array_dims = get_array_dims(s->var->type->vtype);
    new_var->type = get_basic_type(s->var->type->vtype);
    new_var->offset = current->current_var_offset;
    new_var->class = NULL;
    switch (new_var->type)
    {
    case D_INT:
        new_var->size = INTSIZE;
        break;
    case D_BOOL:
        new_var->size = BOOLSIZE;
        break;
    case D_DOUBLE:
        new_var->size = DOUBLESIZE;
        break;
    case D_STRING:
        new_var->size = STRINGSIZE;
        break;
    case D_CLASS:
        new_var->class = get_array_class(s->var->type->vtype);
        if (!new_var->class)
            return;
        new_var->size = new_var->class->size;
        break;
    default:
        ERRPRINT("Unknown type.\n");
        
    }
    sym_add(current, s->var->id->text, s->var->type->vtype->btype, new_var);
    current->current_var_offset += new_var->size + (new_var->size % ROUNDSIZE);
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

void parse_funcdefine_reg_only(int indent, struct semantics *s, struct class_detail *class)
{
    in;
    struct func_detail *new_func = malloc(sizeof(struct func_detail));
    struct symhash *override = sym_class_get(class, s->funcdefine->id->text);
    new_func->generated = 0;
    if (override)
    {
        new_func->override = 1;
        new_func->offset = ((struct func_detail *)override->detail)->offset;
        
    }
    else
    {
        new_func->override = 0;
        if (class->base)
            new_func->offset = class->base->vtable_size + current->current_func_offset;
        else
            new_func->offset = current->current_func_offset;
        current->current_func_offset += PSIZE;
        class->vtable_size += PSIZE;
    }
    sym_add(current, s->funcdefine->id->text, D_FUNCTION, new_func);
}

parseit(funcdefine)
{
    ind;
    struct func_detail *new_func;
    struct symhash *r = sym_get_no_recursive(current, s->funcdefine->id->text);
    if (r)
    {
        if (r->type == D_FUNCTION)
            new_func = r->detail;
        else
        {
            ERRPRINT("%s is not a function.\n", s->funcdefine->id->text);
            return;
        }
    }    
    else
        new_func = malloc(sizeof(struct func_detail));
    new_func->generated = 1;
    if (!s->funcdefine->is_void)
    {
        DBGPRINT("function define:\n");
        parse_type(indent+2, s->funcdefine->type);
        new_func->type = s->funcdefine->type;
    }
    else
    {
        DBGPRINT("void function define:\n");
        new_func->type = NULL;
    }
    parse_ident(indent+2, s->funcdefine->id, 0);
    new_field(SCOPE_FORMAL);
    parse_formals(indent+2, s->funcdefine->formals);
    new_func->formals = current;
    new_field(SCOPE_LOCAL);
    parse_stmblock(indent+2, s->funcdefine->stmblock);
    current = current->parent->parent;
    if (current->scope != SCOPE_CLASS)
    {
        new_func->offset = current->current_func_offset;
        current->current_func_offset += new_func->size + new_func->size % ROUNDSIZE;
        sym_add(current, s->funcdefine->id->text, D_FUNCTION, new_func);
    }
}

void parse_field(int indent, struct semantics *s, int no_func, struct class_detail *class)
{
    ind;
    DBGPRINT("class field:\n");
    if (no_func)
    {
        if (s->field->is_vardefine)
            parse_vardefine(indent+2, s->field->vardefine);
        else
            parse_funcdefine_reg_only(indent+2, s->field->funcdefine, class);
    }
    else if(!s->field->is_vardefine)
        parse_funcdefine(indent+2, s->field->funcdefine);
}

void parse_fields(int indent, struct semantics *s, int no_func, struct class_detail *class)
{
    in;
    struct fields *i;
    for (i=s->fields; i; i=i->next)
    {
        parse_field(indent, i->field, no_func, class);
    }
}

parseit(extend)
{
    ind;
    if (!(s->extend && s->extend->id))
        return;
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

struct interface_details *get_implements(struct semantics *s)
{
    if (!s->implement->id_with_comma)
        return NULL;
    struct id_with_comma *i = s->implement->id_with_comma->id_with_comma;
    struct interface_details *ret = NULL;
    if (i)
    {
        ret = malloc(sizeof(struct interface_details));
        struct symhash *r = sym_get(current, i->id->text);
        if (r && r->type == D_INTERFACE)
            ret->detail = r->detail;
        else
        {
            ERRPRINT("interface %s not defined.\n", i->id->text);
            free(ret);
            return NULL;
        }
        ret->next = NULL;
        i = i->next;
    }
    while (i)
    {
        struct interface_details *next_ret = malloc(sizeof(struct interface_details));
        struct symhash *r = sym_get(current, i->id->text);
        if (r && r->type == D_INTERFACE)
            next_ret->detail = r->detail;
        else
        {
            ERRPRINT("interface %s not defined.\n", i->id->text);
            free(next_ret);
            return ret;
        }
        next_ret->next = ret->next;
        ret->next = next_ret;
        i = i->next;
    }
    return ret;
}


parseit(classdefine)
{
    ind;
    DBGPRINT("class define:\n");
    parse_ident(indent+2, s->classdefine->id, 0);
    struct class_detail *new_class=malloc(sizeof(struct class_detail));
    if (s->classdefine->extend && s->classdefine->extend->extend && s->classdefine->extend->extend->id)
    {
        struct symhash *r = sym_get(current, s->classdefine->extend->extend->id->text);
        if (r->type == D_TYPE)
            new_class->base = r->detail;
        else
        {
            ERRPRINT("class %s not defined.\n", s->classdefine->extend->extend->id->text);
            new_class->base = NULL;
        }
    }
    else
        new_class->base = NULL;
    parse_extend(indent+2, s->classdefine->extend);
    new_class->interfaces = get_implements(s->classdefine->implement);
    parse_implement(indent+2, s->classdefine->implement);
    new_field(SCOPE_CLASS);
    new_class->vtable_size = 0;
    new_class->env = current;
    parse_fields(indent+2, s->classdefine->fields, 1, new_class);
    new_class->size = current->current_var_offset + new_class->vtable_size;
    parse_fields(indent+2, s->classdefine->fields, 0, new_class);
    current = current->parent;
    sym_add(current, s->classdefine->id->text, D_TYPE, new_class);
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
    parse_protypes(indent+2, s->interfacedefine->protypes);
    struct interface_detail *new_interface = malloc(sizeof(struct interface_detail));
    new_interface->protypes = s->interfacedefine->protypes;
    sym_add(current, s->interfacedefine->id->text, D_INTERFACE, new_interface);
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
