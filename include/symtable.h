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


struct symhash
{
    const char *name;
    enum decaf_type type;
    void *define;
    UT_hash_handle hh;
};

struct symres
{
    uint8_t is_class_scope;
    struct symhash **table;
    struct symres *parent;
};

struct func_detail
{
    uint8_t is_member_function;
    struct semantics *type;
    struct semantics *formals;
    struct ir *irlist;
    uint64_t offset;
};

struct class_detail
{
    struct class_detail *base;
    struct symres *env;
};

struct interface_detail
{
    struct semantics *protypes;
};

struct var_detail
{
    enum decaf_type type;
    struct class_detail *class;
    uint64_t offset;
};

int sym_add(struct symres *table, const char * i,enum decaf_type t, void *d);
struct symhash *sym_get(struct symres *table, const char *i);
struct symhash *sym_get_no_recursive(struct symres *table, const char *i);

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
parseit(field);
parseit(fields);
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
