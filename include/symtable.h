#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include "defs.h"
#include "uthash.h"
#include "ir.h"
#include <stdlib.h>

#ifdef DEBUG
#define DBGPRINT(fmt, ...)   printf(fmt, ##__VA_ARGS__)
#define PUTCHAR(c) putchar(c)
#else
#define DBGPRINT(fmt, ...)
#define PUTCHAR(c)
#endif

#define WPRINT(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)
#define ERRPRINT(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)

struct symhash
{
    const char *name;
    enum decaf_type type;
    void *detail;
    UT_hash_handle hh;
};

enum scope
{
    SCOPE_GLOBAL,
    SCOPE_CLASS,
    SCOPE_FORMAL,
    SCOPE_LOCAL
};

struct symres
{
    enum scope scope;
    uint64_t current_var_offset;
    uint64_t current_func_offset;
    struct symhash **table;
    struct symres *parent;
};

struct func_detail
{
    uint64_t size;
    struct semantics *type;
    struct symres *formals;
    struct ir *irlist;
    uint64_t offset;
};

struct interface_detail
{
    struct semantics *protypes;
};


struct interface_details
{
    struct interface_detail *detail;
    struct interface_details *next;
};

struct vtable
{
    const char *name;
    uint64_t offset;
    struct vtable *next;
};

struct class_detail
{
    uint64_t vtable_size;
    struct vtable *vtable;
    uint64_t size;
    struct class_detail *base;
    struct interface_details *interface;
    struct symres *env;
};

struct var_detail
{
    uint8_t is_array;
    uint64_t array_dims;
    uint64_t size;
    enum decaf_type type;
    struct class_detail *class;
    uint64_t offset;
};

int sym_add(struct symres *table, const char * i,enum decaf_type t, void *d);
struct symhash *sym_get(struct symres *table, const char *i);
struct symhash *sym_get_no_recursive(struct symres *table, const char *i);
uint64_t base_size(struct class_detail *base);
enum decaf_type get_basic_type(struct type *type);
uint64_t get_array_dims(struct type *type);


#define parseit(sem) void parse_##sem(int indent, struct semantics *s)

void parse_ident(int indent, struct semantics *s, int type);
parseit(const);
parseit(null);
parseit(tformals);
parseit(formals);
parseit(type);
parseit(var);
parseit(vardefine);
parseit(vardefines);
parseit(expr_with_comma);
parseit(actuals);
parseit(call);
parseit(lvalue);
parseit(expr);
parseit(ifstm);
parseit(whilestm);
parseit(forstm);
parseit(retstm);
parseit(breakstm);
parseit(printstm);
parseit(expr_or_not);
parseit(stm);
parseit(stms);
parseit(stmblock);
parseit(funcdefine);
void parse_field(int indent, struct semantics *s, int no_func);
void parse_fields(int indent, struct semantics *s, int no_func);
parseit(extend);
parseit(ident_with_comma);
parseit(implement);
parseit(classdefine);
parseit(protype);
parseit(protypes);
parseit(interfacedefine);
parseit(define);
parseit(program);

#endif
