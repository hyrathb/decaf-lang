#include "defs.h"
#include "ir.h"
#include "symtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/elf.h>

struct symhash *roothash=NULL;
struct symres root={SCOPE_GLOBAL, 0, 0, 0, &roothash, NULL};

Elf32_Ehdr elfheader;
Elf32_Shdr segheader[9];
uint8_t text[10240];
uint8_t data[2048];     //use data seg for vtable, bss seg for globals
Elf32_Rel textrealloc[2048];
Elf32_Rel datarealloc[2048];
Elf32_Sym symtab[2048];
char shstrtab[10240];
char strtab[10240];
char prostrtab[10240];


uint32_t current_text_reallocs=0;
uint32_t current_data_reallocs=0;
uint32_t current_syms=1;
uint32_t current_string_offset=0;
uint32_t current_shstring_offset=0;
uint32_t current_text_offset=0;

static struct symres *current=&root;
static struct class_detail *current_class = NULL;
static const char *current_class_name = NULL;
static struct func_detail *current_func = NULL;
static char tir[50];
static struct ir tirs[2048];
static uint32_t tmp_var_i=0;
static uint32_t current_prostring_offset=0;

void gen_code(uint32_t i, struct ir ir[], struct func_detail *func);

#define new_field(s) {struct symres *nt = malloc(sizeof(struct symres));\
                      nt->table = malloc(sizeof(struct symhash *)); \
                      nt->scope = s; \
                      nt->current_var_offset = 0; \
                      nt->current_func_offset = 0; \
                      *(nt->table) = NULL; \
                      nt->parent = current; \
                      current = nt;}

#define new_ir(t)         {tirs[current_func->ircount].type = t; \
                            tirs[current_func->ircount].env = current; \
                            tirs[current_func->ircount].code = malloc(strlen(tir) + 1); \
                            strcpy(tirs[current_func->ircount].code, tir); \
                            ++current_func->ircount;} 

#define set_type(a, x)       {a->t = x->t; \
                              a->bt = x->bt; \
                              a->class = x->class; \
}
                            
#define set_check_type(a, x, y) {if (((x)->t != (y)->t) || (((x)->t == D_CLASS) && ((y)->class) && check_base((x)->class, (y)->class))) \
                                ERRPRINT("Type Mismatch %d, %d.\n", x->t, y->t); \
                                set_type(a, x); \
}


#ifdef SYMDEBUG
#define DPRINTSYM(x)  if (current->table)   \
                      {for (x = *(current->table); x; x = x->hh.next) \
                      { \
                            DBGPRINT("name %s, type %d, detail %p.\n", x->name, x->type, x->detail); \
                      } \
                      DBGPRINT("\n");}
#else
#define DPRINTSYM(x)
#endif

#ifdef IRDEBUG
#define DPRINTIR(x)   \
                      { \
                          for (x = 0; x < current_func->ircount; ++x) \
                      { \
                            if (current_func->irlist[x].code) \
                                \
                            {DBGPRINT("%u %d %s \n", x, current_func->irlist[x].type, current_func->irlist[x].code);} \
                            else \
                            {DBGPRINT("%u %d\n", x, current_func->irlist[x].type);}    \
                      } \
                      DBGPRINT("\n");}
#else
#define DPRINTIR(x)
#endif

static void new_string(const char *s)
{
    strcpy(prostrtab+current_prostring_offset, s+1);
    current_prostring_offset += strlen(s)-1;
    prostrtab[current_prostring_offset-1] = 0;
}

static void fill_seg(int i, const char *name, uint32_t type, uint32_t flags, uint32_t size, uint32_t link, uint32_t info, uint32_t al, uint32_t entsize)
{
    strcpy(shstrtab+current_shstring_offset, name);
    segheader[i].sh_name = current_shstring_offset;
    current_shstring_offset += strlen(shstrtab+current_shstring_offset)+1;
    segheader[i].sh_type = type;
    segheader[i].sh_flags = flags;
    segheader[i].sh_addr = 0;
    segheader[i].sh_size = size;
    segheader[i].sh_link = link;
    segheader[i].sh_info = info;
    segheader[i].sh_addralign = al;
    segheader[i].sh_entsize = entsize;
}

static void fill_sym(const char *name, uint32_t value, uint32_t size, uint8_t info, uint8_t other, uint16_t sh)
{
    strcpy(strtab+current_string_offset, name);
    symtab[current_syms].st_name = current_string_offset;
    current_string_offset += strlen(name)+1;
    symtab[current_syms].st_value = value;
    symtab[current_syms].st_size = size;
    symtab[current_syms].st_info = info;
    symtab[current_syms].st_other = other;
    symtab[current_syms].st_shndx = sh;
    ++current_syms;
}

static void init_symtab()
{
    fill_sym("default.decaf", 0, 0, STB_LOCAL<<4|STT_FILE, 0, SHN_ABS);
    fill_sym("", 0, 0, STB_LOCAL<<4|STT_SECTION, 0, 1);
    fill_sym("", 0, 0, STB_LOCAL<<4|STT_SECTION, 0, 3);
    fill_sym("", 0, 0, STB_LOCAL<<4|STT_SECTION, 0, 5);
    fill_sym("printf", 0, 0, STB_GLOBAL<<4|STT_NOTYPE, 0, SHN_UNDEF);
    fill_sym("malloc", 0, 0, STB_GLOBAL<<4|STT_NOTYPE, 0, SHN_UNDEF);
    fill_sym(".vtable", 0, 0, STB_GLOBAL<<4|STT_OBJECT, 0, 3);
    fill_sym(".slist", 0, 0, STB_GLOBAL<<4|STT_OBJECT, 0, 3);
    fill_sym(".textbegin", 0, 0, STB_GLOBAL<<4|STT_OBJECT, 0, 1);
}

static void write_byte(void *x, FILE *out)
{
    fwrite(x, sizeof(uint8_t), 1, out);
}

static void write_half(void *x, FILE *out)
{
    fwrite((uint8_t *)x +1, sizeof(uint8_t), 1, out);
    fwrite(x, sizeof(uint8_t), 1, out);
}

static void write_word(void *x, FILE *out)
{
    fwrite((uint8_t *)x +3, sizeof(uint8_t), 1, out);
    fwrite((uint8_t *)x +2, sizeof(uint8_t), 1, out);
    fwrite((uint8_t *)x +1, sizeof(uint8_t), 1, out);
    fwrite(x, sizeof(uint8_t), 1, out);
}

static void outputheader(FILE *out)
{
    fwrite(elfheader.e_ident, sizeof(uint8_t), EI_NIDENT, out);
    write_half(&elfheader.e_type, out);
    write_half(&elfheader.e_machine, out);
    write_word(&elfheader.e_version, out);
    write_word(&elfheader.e_entry, out);
    write_word(&elfheader.e_phoff, out);
    write_word(&elfheader.e_shoff, out);
    write_word(&elfheader.e_flags, out);
    write_half(&elfheader.e_ehsize, out);
    write_half(&elfheader.e_phentsize, out);
    write_half(&elfheader.e_phnum, out);
    write_half(&elfheader.e_shentsize, out);
    write_half(&elfheader.e_shnum, out);
    write_half(&elfheader.e_shstrndx, out);
}

static void outputtext(FILE *out)
{
    uint8_t *t = (void *)text;
    uint32_t off;
    for (off = 0; off<current_text_offset; off+=4)
    {
        write_word(t+off, out);
    }
}

static void outputdata(FILE *out)
{
    uint8_t *t = (void *)data;
    uint32_t off;
    for (off = 0; off<root.current_class_offset; off+=4)
    {
        write_word(t+off, out);
    }
    fwrite(prostrtab, sizeof(uint8_t), current_prostring_offset, out);
}

static void outputsym(Elf32_Sym *sym, FILE *out)
{
    write_word(&sym->st_name, out);
    write_word(&sym->st_value, out);
    write_word(&sym->st_size, out);
    write_byte(&sym->st_info, out);
    write_byte(&sym->st_other, out);
    write_half(&sym->st_shndx, out);
}

static void outputrel(Elf32_Rel *rel, FILE *out)
{
    write_word(&rel->r_offset, out);
    write_word(&rel->r_info, out);
}

static void outputseg(Elf32_Shdr *seg, FILE *out)
{
    write_word(&seg->sh_name, out);
    write_word(&seg->sh_type, out);
    write_word(&seg->sh_flags, out);
    write_word(&seg->sh_addr, out);
    write_word(&seg->sh_offset, out);
    write_word(&seg->sh_size, out);
    write_word(&seg->sh_link, out);
    write_word(&seg->sh_info, out);
    write_word(&seg->sh_addralign, out);
    write_word(&seg->sh_entsize, out);
}

static void outputelf(FILE *out)
{
    uint32_t i;
    outputheader(out);
    outputtext(out);
    outputdata(out);
    fwrite(shstrtab, sizeof(uint8_t), current_shstring_offset, out);
    for (i=0; i<current_syms; ++i)
        outputsym(symtab+i, out);
    fwrite(strtab, sizeof(uint8_t), current_string_offset, out);
    for (i=0; i<current_text_reallocs; ++i)
        outputrel(textrealloc+i, out);
    for (i=0; i<current_data_reallocs; ++i)
        outputrel(datarealloc+i, out);
    for (i=0; i<9; ++i)
        outputseg(segheader+i, out);
}

void init_string()
{
    new_string("\"%s\n\"");
    new_string("\"%f\n\"");
    new_string("\"%x\n\"");
    strtab[0] = 0;
    ++current_string_offset;
    shstrtab[0] = 0;
    ++current_shstring_offset;
}

/*******NEED TO BE FREED*******/
char *get_tmp_var()
{
    static char tmpname[20];
    char *s;
    sprintf(tmpname, "!%u", tmp_var_i++);
    s = malloc(strlen(tmpname) + 1);
    strcpy(s, tmpname);
    current_func->stacksize += PSIZE;
    current_func->tvarsize += PSIZE;
    return s;
}
                      
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

int check_base(struct class_detail *x, struct class_detail *y)
{
    while (y)
    {
        if (y == x)
            return 0;
        y = y->base;
    }
    return 1;
    
}

uint32_t base_size(struct class_detail *base)
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

uint32_t get_array_dims(struct type *type)
{
    uint32_t ret = 0;
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
            DBGPRINT("identifier: %s\n", s->text);
    }
    else
    {
        DBGPRINT("new identifier: %s\n", s->text);
    }
}

char *parse_const(int indent, struct semantics *s)
{
    if (!s)
        return NULL;
    printindent(indent);
    char *l = get_tmp_var();
    switch (s->type)
    {
    case C_ICONST:
        DBGPRINT("int const: %d\n", s->i_val);
        sprintf(tir, "%s $%d =", l, s->i_val);
        break;
    case C_BCONST:
        DBGPRINT("bool const: %s\n", s->i_val?"true":"false");
        sprintf(tir, "%s $%d =", l, s->i_val);
        break;
    case C_DCONST:
        DBGPRINT("double const: %f\n", s->d_val);
        sprintf(tir, "%s $%lf =", l, s->d_val);
        break;
    case C_NULL:
        DBGPRINT("null const\n");
        sprintf(tir, "%s $0 =", l);
        break;        
    case C_SCONST:
        DBGPRINT("string const: %s\n", s->s_val);
        sprintf(tir, "%s $s%u =", l, current_prostring_offset);
        new_string(s->s_val);
        break;
    default:
        ;
    }
    new_ir(IR_SINGLE);
    return l;
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
    if (s->formals->tformals && current->scope == SCOPE_FORMAL)
    {
        for (i=s->formals->tformals->tformals; i; i=i->next)
        {
            parse_var(indent+2, i->var);
            current_func->stacksize += PSIZE;
            current_func->formalsize += PSIZE;
        }
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
    new_var->size = PSIZE; ///hehe
    new_var->scope = current->scope;
    sym_add(current, s->var->id->text, s->var->type->vtype->btype, new_var);
    if (current->scope == SCOPE_GLOBAL)
    {
        strcpy(strtab+current_string_offset, s->var->id->text);
        fill_sym(s->var->id->text, current->current_var_offset, PSIZE, (STB_GLOBAL << 4) | STT_OBJECT, 0, 5);
        new_var->symnum = current_syms-1;
        ++current_syms;
    }
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
    {
        parse_vardefine(indent+2, i->vardefine);
        current_func->stacksize += PSIZE;
        current_func->uvarsize += PSIZE;
    }
}

parseit(expr_with_comma)
{
    in;
    struct expr_with_comma *i;
    for (i=s->expr_with_comma; i; i=i->next)
        parse_expr(indent+2, i->expr);
}

void parse_actuals(int indent, struct semantics *s, int arg)
{
    ind;
    DBGPRINT("actuals:\n");
    char *l, *r;
    char *tmpnames[20], *tmpexprs[20];
    char tmpname[20];
    int j=arg;
    //parse_expr_with_comma(indent+2, s->actuals->expr_with_comma);
    struct expr_with_comma *i;
    if (s->actuals->expr_with_comma)
    {
        for (i=s->actuals->expr_with_comma->expr_with_comma; i; i=i->next)
        {
            r = parse_expr(indent+2, i->expr);
            sprintf(tmpname, "#%d", arg);
            l = malloc(strlen(tmpname)+1);
            strcpy(l, tmpname);
            tmpexprs[arg] = r;
            tmpnames[arg++] = l;
        }
        for (; j<arg; ++j)
        {
            sprintf(tir, "%s %s =", tmpnames[j], tmpexprs[j]);
            new_ir(IR_SINGLE);
            mfree(tmpnames[j]);
            mfree(tmpexprs[j]);
        }
    }
}

char *parse_call(int indent, struct semantics *s, struct semantics *expr)
{
    if (!s)
        return NULL;
    struct symhash *sym;
    char *l, *r=NULL;
    int is_member;
    if (s->call->is_member)
    {
        r = parse_lvalue(indent+2, s->call->lvalue);
        is_member = 1;
        if (s->call->lvalue->lvalue->t != D_CLASS)
        {
            ERRPRINT("Requesting member from a non-class object.\n");
            sym = NULL;
        }
        else
        {
            sym = sym_class_get(s->call->lvalue->lvalue->class, s->call->id->text);
        }
    }
    else
    {
        sym = sym_class_get(current_class, s->call->id->text);
        if (sym)
        {
            is_member = 1;
            r = malloc(strlen("#dx")+1);
            strcpy(r, "#dx");
        }
        else
        {
            sym = sym_get(current, s->call->id->text);
            is_member = 0;
        }
    }
    if (is_member)
        DBGPRINT("member function call expr:\n");
    else
        DBGPRINT("function call expr:\n");
    parse_ident(indent+2, s->call->id, 1);
    if (sym)
    {
        struct func_detail *detail = sym->detail;
        if (detail->type)
        {
            struct type *t = detail->type->vtype;
            expr->expr->t = t->is_array?D_ARRAY:t->btype;
            expr->expr->bt = get_basic_type(t);
            if (expr->expr->bt == D_TYPE)
            {
                while (t->btype == D_ARRAY)
                    t = t->arr_type->vtype;
                struct symhash *sym2 = sym_get(&root, t->id->text);
                expr->expr->class = sym2->detail;
            }
            else
            {
                expr->expr->class = NULL;
            }
        }
        else
        {
            expr->expr->t = D_VOID;
            expr->expr->bt = D_VOID;
            expr->expr->class = NULL;
        }
        if (sym->type == D_FUNCTION)
        {
            if (is_member)
            {
                parse_actuals(indent+2, s->call->actuals, 1);
                sprintf(tir, "#0 %s =", r);
                new_ir(IR_SINGLE);
                l = get_tmp_var();
                sprintf(tir, "%s *%s =", l, r);
                new_ir(IR_SINGLE);
                mfree(r);
                r = get_tmp_var();
                sprintf(tir, "%s %s $%u +", r, l, detail->offset);
                new_ir(IR_DOUBLE);
                mfree(l);
                l = get_tmp_var();
                sprintf(tir, "%s *%s =", l, r);
                new_ir(IR_SINGLE);
                sprintf(tir, "%s", l);
                new_ir(IR_CALL_MEMBER);
                mfree(r);
                mfree(l);
            }
            else
            {
                parse_actuals(indent+2, s->call->actuals, 0);
                sprintf(tir, "%s", s->call->id->text);
                new_ir(IR_CALL);
            }
        }
        else
            ERRPRINT("Not a function.\n");
    }
    l = malloc(strlen("#ar")+1);
    strcpy(l, "#ar");
    return l;
    
}

char *parse_lvalue(int indent, struct semantics *s)
{
    if (!s)
        return NULL;
    printindent(indent);
    char *l, *tl, *r, *r2;
    struct symhash *id;
    struct var_detail *detail;
    switch (s->lvalue->lvalue_type)
    {
    case LVAL_IDENT:
        DBGPRINT("identifier as lvalue: \n");
        parse_ident(indent+2, s->lvalue->id, 1);
        id = sym_class_get(current_class, s->lvalue->id->text);
        if (!id)
        {
            id = sym_get(current, s->lvalue->id->text);
        }
        detail = (struct var_detail *)id->detail;
        
        s->lvalue->t = detail->is_array?D_ARRAY:detail->type;
        s->lvalue->bt = detail->type;
        s->lvalue->class = detail->class;
        
        if (detail->scope == SCOPE_CLASS)
        {
            r = malloc(strlen("#dx")+1);
            strcpy(r, "#dx");
            r2 = get_tmp_var();
            sprintf(tir, "%s %s $%u +", r2, r, PSIZE + detail->offset);
            new_ir(IR_DOUBLE);
            mfree(r);
            l = malloc(strlen(r2) + 2);
            l[0] = '*';
            l[1] = 0;
            strcat(l, r2);
            free(r2);
            return l;
        }
        else
        {
            l = malloc(strlen(s->lvalue->id->text) + 1);
            strcpy(l, s->lvalue->id->text);
            return l;
        }
    case LVAL_MEMBER:
        DBGPRINT("member as lvalue: \n");
        r = parse_lvalue(indent+2, s->lvalue->lvalue);
        if (s->lvalue->lvalue->lvalue->t != D_CLASS)
            ERRPRINT("Not a class.\n");
        parse_ident(indent+2, s->lvalue->id, 1);
        detail = sym_class_get(s->lvalue->lvalue->lvalue->class, s->lvalue->id->text)->detail;
        
        s->lvalue->t = detail->is_array?D_ARRAY:detail->type;
        s->lvalue->bt = detail->type;
        s->lvalue->class = detail->class;
        
        r2 = get_tmp_var();
        sprintf(tir, "%s %s $%u +", r2, r, PSIZE + detail->offset);
        new_ir(IR_DOUBLE);
        mfree(r);
        l = malloc(strlen(r2) + 2);
        l[0] = '*';
        l[1] = 0;
        strcat(l, r2);
        free(r2);
        return l;
    case LVAL_ARRAY:
        DBGPRINT("array elem as lvalue: \n");
        r = parse_lvalue(indent+2, s->lvalue->lvalue);
        r2 = parse_expr(indent+2, s->lvalue->expr);
        
        if (s->lvalue->lvalue->lvalue->t != D_ARRAY)
        {
            ERRPRINT("%d is not an array.\n", s->lvalue->lvalue->lvalue->t);
        }
        
        s->lvalue->t = s->lvalue->lvalue->lvalue->bt;
        s->lvalue->bt = s->lvalue->t;
        s->lvalue->class = s->lvalue->lvalue->lvalue->class;
        
        tl = get_tmp_var();
        sprintf(tir, "%s %s $%d *", tl, r2, PSIZE);
        new_ir(IR_DOUBLE);
        
        sprintf(tir, "%s %s %s +", r2, r, tl);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(tl);
        l = malloc(strlen(r2) + 2);
        l[0] = '*';
        l[1] = 0;
        strcat(l, r2);
        free(r2);
        return l;
    default:
        return NULL;
    }
}

char *parse_expr(int indent, struct semantics *s)
{
    if (!s)
        return NULL;
    printindent(indent);
    char *l, *r, *r2;
    switch (s->expr->expr_type)
    {
    case EXPR_ASSIGN:
        DBGPRINT("assign expr:\n");
        l= parse_lvalue(indent+2, s->expr->lvalue);
        r = parse_expr(indent+2, s->expr->expr1);
        
        set_check_type(s->expr, s->expr->lvalue->lvalue, s->expr->expr1->expr);
        
        sprintf(tir, "%s %s =", l, r);
        new_ir(IR_SINGLE);
        mfree(r);
        return l;
    case EXPR_CONST:
        DBGPRINT("const expr:\n");
        l = parse_const(indent+2, s->expr->constant);
        
        switch (s->expr->constant->type)
        {
            case C_ICONST:
            s->expr->t = D_INT;
            break;
        case C_BCONST:
            s->expr->t = D_BOOL;
            break;
        case C_SCONST:
            s->expr->t = D_STRING;
            break;
        case C_DCONST:
            s->expr->t = D_DOUBLE;
            break;
        default:
            s->expr->t = D_CLASS;
        }
        s->expr->bt = s->expr->t;
        s->expr->class = NULL;
        
        return l;
    case EXPR_LVAL:
        DBGPRINT("lvalue expr:\n");
        l = parse_lvalue(indent+2, s->expr->lvalue);
        
        set_type(s->expr, s->expr->lvalue->lvalue);
        
        return l;
    case EXPR_THIS:
        DBGPRINT("this pointer expr:\n");
        l = malloc(strlen("#dx") + 1);
        strcpy(l, "#dx");
        
        s->expr->t = D_CLASS;
        s->expr->bt = D_CLASS;
        s->expr->class = current_class;
        
        return l;
    case EXPR_CALL:
        l = parse_call(indent+2, s->expr->call, s);
        return l;
    case EXPR_PRIORITY:
        DBGPRINT("(expr):\n");
        l = parse_expr(indent+2, s->expr->expr1);
        
        set_type(s->expr, s->expr->expr1->expr);
        
        return l;
    case EXPR_PLUS:
        DBGPRINT("+ expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s +", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_MINUS:
        DBGPRINT("- expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s -", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_MUL:
        DBGPRINT("* expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s *", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_DIV:
        DBGPRINT("/ expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s /", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_IDIV:
        PUTCHAR('%');
        DBGPRINT(" expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s %%", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_LT:
        DBGPRINT("< expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s <", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_LE:
        DBGPRINT("<= expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s <=", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_GT:
        DBGPRINT("> expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s >", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_GE:
        DBGPRINT(">= expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s >=", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_EQU:
        DBGPRINT("== expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s ==", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_NE:
        DBGPRINT("!= expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s !=", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_AND:
        DBGPRINT("&& expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s &&", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_OR:
        DBGPRINT("|| expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        r2 = parse_expr(indent+2, s->expr->expr2);
        l = get_tmp_var();
        sprintf(tir, "%s %s %s ||", l, r, r2);
        new_ir(IR_DOUBLE);
        mfree(r);
        mfree(r2);
        
        set_check_type(s->expr, s->expr->expr1->expr, s->expr->expr2->expr);
        
        return l;
    case EXPR_NOT:
        DBGPRINT("! expr:\n");
        r = parse_expr(indent+2, s->expr->expr1);
        l = get_tmp_var();
        sprintf(tir, "%s %s !", l, r);
        new_ir(IR_SINGLE);
        mfree(r);
        
        set_type(s->expr, s->expr->expr1->expr);
        
        return l;
    case EXPR_READINTEGER:
        DBGPRINT("read integer expr:\n");
        l = malloc(strlen("$readinteger") + 1);
        strcpy(l, "$readinteger");
        
        s->expr->t = D_INT;
        s->expr->bt = D_INT;
        s->expr->class = NULL;
        
        return l;
    case EXPR_READLINE:
        DBGPRINT("read line expr:\n");
        l = malloc(strlen("$readline") + 1);
        strcpy(l, "$readline");
        
        s->expr->t = D_STRING;
        s->expr->bt = D_STRING;
        s->expr->class = NULL;
        
        return l;
    case EXPR_NEW:
        DBGPRINT("new expr:\n");
        struct symhash *sym;
        parse_ident(indent+2, s->expr->id, 1);
        l = get_tmp_var();
        sprintf(tir, "%s %s", l, s->expr->id->text);
        new_ir(IR_NEW);
        
        sym = sym_get(current, s->expr->id->text);
        if (!sym || sym->type != D_TYPE)
        {
            ERRPRINT("%s is not a class type.\n", s->expr->id->text);
            s->expr->t = D_CLASS;
            s->expr->bt = D_CLASS;
            s->expr->class = NULL; 
        }
        else
        {
            s->expr->t = D_CLASS;
            s->expr->bt = D_CLASS;
            s->expr->class = sym->detail;
        }
        
        return l;
    case EXPR_NEWARRAY:
        DBGPRINT("newarray expr:\n");
        l = get_tmp_var();
        r = parse_expr(indent+2, s->expr->expr1);
        
        sprintf(tir, "%s %s $%u *", r, r, PSIZE);
        new_ir(IR_DOUBLE);
        
        parse_type(indent+2, s->expr->type);
        int btype = get_basic_type(s->expr->type->vtype);
        if (btype == D_CLASS)
        {
            struct type *t = s->expr->type->vtype;
            while (t->btype == D_ARRAY)
                t = t->arr_type->vtype;
            sprintf(tir, "%s %d %s %s", l, D_TYPE, r, t->id->text);
            
            sym = sym_get(current, t->id->text);
            if (!sym || sym->type != D_TYPE)
            {
                ERRPRINT("%s is not a class type.\n", s->expr->id->text);
                s->expr->t = D_CLASS;
                s->expr->bt = D_CLASS;
                s->expr->class = NULL; 
            }
            else
            {
                s->expr->t = D_CLASS;
                s->expr->bt = D_CLASS;
                s->expr->class = sym->detail;
            }
        }
        else
        {
            sprintf(tir, "%s %d %s", l, btype, r);
            s->expr->t = btype;
            s->expr->bt = btype;
            s->expr->class = NULL;
        }
        new_ir(IR_NEW_ARRAY);
        return l;
    default:
        return NULL;
    }
}

parseit(ifstm)
{
    ind;
    char *e1;
    uint32_t irplace1, irplace2;
    DBGPRINT("if statement:\n");
    e1 = parse_expr(indent+2, s->if_stm->expr);
    irplace1 = current_func->ircount;
    ++current_func->ircount;
    
    ind;
    DBGPRINT("else:\n");
    parse_stm(indent+2, s->if_stm->stm2);
    irplace2 = current_func->ircount;
    ++current_func->ircount;
    
    sprintf(tir, "%s %u", e1, current_func->ircount);
    tirs[irplace1].type = IR_B;
    tirs[irplace1].env = current;
    tirs[irplace1].code = malloc(strlen(tir)+1);
    strcpy(tirs[irplace1].code, tir);
    
    ind;
    DBGPRINT("then:\n");
    parse_stm(indent+2, s->if_stm->stm1);
    sprintf(tir, "%u", current_func->ircount);
    tirs[irplace2].type = IR_J;
    tirs[irplace2].env = current;
    tirs[irplace2].code = malloc(strlen(tir)+1);
    strcpy(tirs[irplace2].code, tir);
}

parseit(whilestm)
{
    ind;
    uint32_t loop_entery = current_func->ircount, loop_cond_end, loop_body_end;
    char *e;
    DBGPRINT("while statement:\n");
    
    e = parse_expr(indent+2, s->whilestm->expr);
    loop_cond_end = current_func->ircount;
    current_func->ircount += 2;
    
    parse_stm(indent+2, s->whilestm->stm);
    sprintf(tir, "%u", loop_entery);
    new_ir(IR_J);
    
    loop_body_end = current_func->ircount;
    sprintf(tir, "%s %u", e, loop_cond_end+2);
    tirs[loop_cond_end].type = IR_B;
    tirs[loop_cond_end].env = current;
    tirs[loop_cond_end].code = malloc(strlen(tir)+1);
    strcpy(tirs[loop_cond_end].code, tir);
    sprintf(tir, "%u", loop_body_end);
    tirs[loop_cond_end+1].type = IR_J;
    tirs[loop_cond_end+1].env = current;
    tirs[loop_cond_end+1].code = malloc(strlen(tir)+1);
    strcpy(tirs[loop_cond_end+1].code, tir);
    
    uint32_t i;
    for (i = loop_cond_end; i < loop_body_end; ++i)
        if (tirs[i].type == IR_BREAK)
        {
            tirs[i].type = IR_J;
            tirs[i].code = malloc(strlen(tir)+1);
            strcpy(tirs[i].code, tir);
        }
}

parseit(forstm)
{
    ind;
    DBGPRINT("for statement:\n");
    ind;
    char *e;
    DBGPRINT("INIT:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not1);
    
    ind;
    uint32_t loop_entery = current_func->ircount, loop_cond_end, loop_body_end;
    DBGPRINT("COND:\n");
    e = parse_expr(indent+2, s->forstm->expr);
    loop_cond_end = current_func->ircount;
    current_func->ircount += 2;
    
    ind;
    DBGPRINT("STATEMENT:\n");
    parse_stm(indent+2, s->forstm->stm);
    
    ind;
    DBGPRINT("ACC:\n");
    parse_expr_or_not(indent+2, s->forstm->expr_or_not2);
    sprintf(tir, "%u", loop_entery);
    new_ir(IR_J);
    
    loop_body_end = current_func->ircount;
    sprintf(tir, "%s %u", e, loop_cond_end+2);
    tirs[loop_cond_end].type = IR_B;
    tirs[loop_cond_end].env = current;
    tirs[loop_cond_end].code = malloc(strlen(tir)+1);
    strcpy(tirs[loop_cond_end].code, tir);
    sprintf(tir, "%u", loop_body_end);
    tirs[loop_cond_end+1].type = IR_J;
    tirs[loop_cond_end+1].env = current;
    tirs[loop_cond_end+1].code = malloc(strlen(tir)+1);
    strcpy(tirs[loop_cond_end+1].code, tir);
    
    uint32_t i;
    for (i = loop_cond_end; i < loop_body_end; ++i)
        if (tirs[i].type == IR_BREAK)
        {
            tirs[i].type = IR_J;
            tirs[i].code = malloc(strlen(tir)+1);
            strcpy(tirs[i].code, tir);
        }
}

parseit(retstm)
{
    ind;
    DBGPRINT("return statement:\n");
    //parse_expr_or_not(indent+2, s->returnstm->expr_or_not);
    
    if (current_func->type)
    {
        if (!s->returnstm->expr_or_not->expr_or_not->expr)
        {
            ERRPRINT("No return value in non-void function.");
        }
        else
        {
            sprintf(tir, "#ar %s =", parse_expr(indent+2, s->returnstm->expr_or_not->expr_or_not->expr));
            new_ir(IR_SINGLE);
        }
    }
    
    tirs[current_func->ircount].type = IR_RESTORE_REGS;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    tirs[current_func->ircount].type = IR_RET;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    
}

parseit(breakstm)
{
    ind;
    tirs[current_func->ircount].type = IR_BREAK;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    DBGPRINT("break statement\n");
}

parseit(printstm)
{
    ind;
    DBGPRINT("print statement:\n");
    char *e = parse_expr(indent+2, s->printstm->expr);
    sprintf(tir, "%d %s", s->printstm->expr->expr->bt, e);
    new_ir(IR_PRINT);
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
        char *e = parse_expr(indent+2, s->stm->expr);
        mfree(e);
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
    new_field(SCOPE_LOCAL);
    parse_vardefines(indent+2, s->stmblock->vardefines);
    parse_stms(indent+2, s->stmblock->stms);
    
    struct symhash *si;
    DPRINTSYM(si);
    
    current = current->parent;
}

void parse_funcdefine_reg_only(int indent, struct semantics *s)
{
    in;
    struct func_detail *new_func = malloc(sizeof(struct func_detail));
    struct symhash *override = sym_class_get(current_class, s->funcdefine->id->text);
    new_func->generated = 0;
    new_func->is_member = 1;
    if (override)
    {
        new_func->override = 1;
        new_func->offset = ((struct func_detail *)override->detail)->offset;
        
    }
    else
    {
        new_func->override = 0;
        if (current_class->base)
            new_func->offset = current_class->base->vtable_size + current->current_func_offset;
        else
            new_func->offset = current->current_func_offset;
        current->current_func_offset += PSIZE;
        current_class->vtable_size += PSIZE;
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
    {
        new_func = malloc(sizeof(struct func_detail));
        new_func->is_member = 0;
        sym_add(&root, s->funcdefine->id->text, D_FUNCTION, new_func);
    }
    current_func = new_func;
    new_func->ircount = 0;
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
        new_func->generated = 0;
    }
    parse_ident(indent+2, s->funcdefine->id, 0);
    new_field(SCOPE_FORMAL);
    if (new_func->is_member)
    {
        current->current_var_offset = PSIZE;
        new_func->formalsize = PSIZE;
    }
    else
    {
        new_func->formalsize = 0;
    }
    new_func->formals = current;
    new_func->size = 0;
    new_func->stacksize = 0;
    new_func->uvarsize = 0;
    new_func->tvarsize = 0;
    parse_formals(indent+2, s->funcdefine->formals);
    struct symhash *si;
    DPRINTSYM(si);
    
    tirs[current_func->ircount].type = IR_SAVE_REGS;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    
    tmp_var_i = 0;
    parse_stmblock(indent+2, s->funcdefine->stmblock);
    
    tirs[current_func->ircount].type = IR_RESTORE_REGS;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    
    tirs[current_func->ircount].type = IR_RET;
    tirs[current_func->ircount].env = current;
    tirs[current_func->ircount].code = NULL;
    ++current_func->ircount;
    
    uint32_t ri;
    for (ri=0; ri<current_func->ircount; ++ri)
    {
        tirs[ri].generated = 0;
        tirs[ri].addressed = 0;
        tirs[ri].number = 0;
    }
        
    new_func->irlist = malloc(new_func->ircount * sizeof(struct ir));
    memcpy(new_func->irlist, tirs, new_func->ircount * sizeof(struct ir));
    current = current->parent;
        
    DPRINTIR(ri);
    
    for (ri=0; ri<current_func->ircount; ++ri)
    {
        gen_code(ri, current_func->irlist, current_func);
    }
    
    for (ri=0; ri<current_func->ircount; ++ri)
    {
        gen_code(ri, current_func->irlist, current_func);
        memcpy(text+root.current_func_offset+current_func->size, current_func->irlist[ri].bcode, current_func->irlist[ri].number*PSIZE);
        DBGPRINT("COPYING TO %u FROM %u, SIZE %u\n", root.current_func_offset+current_func->size, ri, current_func->irlist[ri].number*PSIZE);
        current_func->size += current_func->irlist[ri].number*PSIZE;
    }
    
    if (current->scope != SCOPE_CLASS)
    {
        new_func->offset = root.current_func_offset;
        strcpy(strtab+current_string_offset, s->funcdefine->id->text);
    }
    else
    {
        current_class->vtable[new_func->offset/PSIZE] = root.current_func_offset;
        sprintf(strtab+current_string_offset, "%s.%s", current_class_name, s->funcdefine->id->text);
    }
    symtab[current_syms].st_name = current_string_offset;
    symtab[current_syms].st_value = root.current_func_offset;
    symtab[current_syms].st_size = current_func->size;
    symtab[current_syms].st_info = (STB_GLOBAL << 4) | STT_FUNC;
    symtab[current_syms].st_other = 0;
    symtab[current_syms].st_shndx = 1;
    new_func->symnum = current_syms;
    ++current_syms;
    current_string_offset += strlen(strtab+current_string_offset)+1;
    root.current_func_offset += new_func->size + new_func->size % ROUNDSIZE;
}

void parse_field(int indent, struct semantics *s, int no_func)
{
    ind;
    if (no_func)
    {
        if (s->field->is_vardefine)
            parse_vardefine(indent+2, s->field->vardefine);
        else
            parse_funcdefine_reg_only(indent+2, s->field->funcdefine);
    }
    else if(!s->field->is_vardefine)
    {
        parse_funcdefine(indent+2, s->field->funcdefine);
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
    current_class = new_class;
    current_class_name = s->classdefine->id->text;
    sym_add(current, s->classdefine->id->text, D_TYPE, new_class);
    if (s->classdefine->extend && s->classdefine->extend->extend && s->classdefine->extend->extend->id)
    {
        struct symhash *r = sym_get(current, s->classdefine->extend->extend->id->text);
        if (r->type == D_TYPE)
        {
            new_class->base = r->detail;
        }
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
    if (new_class->base)
    {
        new_class->vtable_size = new_class->base->vtable_size;
        current->current_var_offset = new_class->base->size - PSIZE;
    }
    else
        new_class->vtable_size = 0;
    new_class->env = current;
    new_class->offset = root.current_class_offset;
    parse_fields(indent+2, s->classdefine->fields, 1);
    new_class->size = current->current_var_offset + PSIZE;
    new_class->vtable = malloc(new_class->vtable_size);
    if (new_class->base)
        memcpy(new_class->vtable, new_class->base->vtable, new_class->base->vtable_size);
    parse_fields(indent+2, s->classdefine->fields, 0);
    struct symhash *si;
    DPRINTSYM(si);
    
    current = current->parent;
    memcpy(data+root.current_class_offset, current_class->vtable, current_class->vtable_size);
    root.current_class_offset += new_class->vtable_size;
    current_class = NULL;
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
    init_string();
    init_symtab();
    struct program *i;
    for (i=s->program; i; i=i->next)
    {
        parse_define(indent+2, i->define);
    }
    struct symhash *si;
    DPRINTSYM(si);
    elfheader.e_ident[0] = ELFMAG0;
    elfheader.e_ident[1] = ELFMAG1;
    elfheader.e_ident[2] = ELFMAG2;
    elfheader.e_ident[3] = ELFMAG3;
    elfheader.e_ident[4] = 1;
    elfheader.e_ident[5] = 2;
    elfheader.e_ident[6] = 1;
    elfheader.e_type = ET_REL;
    elfheader.e_machine = EM_MIPS;
    elfheader.e_version = 1;
    elfheader.e_entry = 0;
    elfheader.e_phoff = 0;
    elfheader.e_flags = 0x1005;
    elfheader.e_ehsize = 52;
    elfheader.e_phentsize = 0;
    elfheader.e_phnum = 0;
    elfheader.e_shentsize = 40;
    elfheader.e_shnum = 9;
    elfheader.e_shstrndx = 6;
    
    fill_seg(1, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, current_text_offset, 0, 0, 16, 0);
    fill_seg(3, ".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, root.current_class_offset+current_prostring_offset, 0, 0, 16, 0);
    fill_seg(5, ".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE, root.current_var_offset, 0, 0, 16, 0);
    fill_seg(6, ".shstrtab", SHT_STRTAB, 0, current_shstring_offset, 0, 0, 1, 0);
    fill_seg(7, ".symtab", SHT_SYMTAB, 0, current_syms*sizeof(Elf32_Sym), 8, 0, 4, sizeof(Elf32_Sym));
    fill_seg(8, ".strtab", SHT_STRTAB, 0, current_string_offset, 0, 0, 1, 0);
    fill_seg(2, ".text.rel", SHT_REL, 0, current_text_reallocs*sizeof(Elf32_Rel), 7, 1, 4, sizeof(Elf32_Rel));
    fill_seg(4, ".data.rel", SHT_REL, 0, current_data_reallocs*sizeof(Elf32_Rel), 7, 3, 4, sizeof(Elf32_Rel));
    
    uint32_t seg_offset=52;
    segheader[1].sh_offset = seg_offset;
    DBGPRINT("TEXT: 0x%x\n", seg_offset);
    seg_offset += current_text_offset;
    
    segheader[3].sh_offset = seg_offset;
    DBGPRINT("DATA: 0x%x\n", seg_offset);
    seg_offset += root.current_class_offset+current_prostring_offset;
    
    segheader[5].sh_offset = seg_offset;
    DBGPRINT("BSS: 0x%x\n", seg_offset);
    
    segheader[6].sh_offset = seg_offset;
    segheader[6].sh_size = current_shstring_offset;
    DBGPRINT("SHSTRTAB: 0x%x\n", seg_offset);
    seg_offset += current_shstring_offset;
    
    segheader[7].sh_offset = seg_offset;
    DBGPRINT("SYMTAB: 0x%x\n", seg_offset);
    seg_offset += current_syms*sizeof(Elf32_Sym);
    
    segheader[8].sh_offset = seg_offset;
    DBGPRINT("STRTAB: 0x%x\n", seg_offset);
    seg_offset += current_string_offset;
    
    segheader[2].sh_offset = seg_offset;
    DBGPRINT("TEXT REL: 0x%x\n", seg_offset);
    seg_offset += current_text_reallocs*sizeof(Elf32_Rel);
    
    segheader[4].sh_offset = seg_offset;
    DBGPRINT("DATA REL: 0x%x\n", seg_offset);
    seg_offset += current_data_reallocs*sizeof(Elf32_Rel);
    
    symtab[7].st_size = root.current_class_offset;
    symtab[8].st_value = root.current_class_offset;
    symtab[8].st_size = current_prostring_offset;
    
    elfheader.e_shoff = seg_offset;
    
    DBGPRINT("TEXT LENGTH: %u\n", current_text_offset);
    DBGPRINT("DATA LENGTH: %u\n", root.current_class_offset);
    DBGPRINT("TEXT REL NUMBER: %u\n", current_text_reallocs);
    DBGPRINT("DATA REL NUMBER: %u\n", current_data_reallocs);
    FILE *out = fopen("a.o", "wb");
    outputelf(out);
    fclose(out);
}
