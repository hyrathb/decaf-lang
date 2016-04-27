#ifndef __SYM_H__
#define __SYM_H__

#include <stdlib.h>
#include <stdint.h>

#include "hash.h"

#define YYSTYPE struct semantics*

enum decaf_type
{
    D_INT,
    D_BOOL,
    D_DOUBLE,
    D_STRING,
    D_FUNCTION
};

enum comp_type
{
    C_EMPTY,
    C_IDENT,
    C_ICONST,
    C_BCONST,
    C_DCONST,
    C_SCONST,
    C_NULL,
    C_TFORMALS,
    C_FORMALS,
    C_TYPE,
    C_VAR,
    C_VARDEFINE,
    C_VARDEFINES,
    C_EXPR_WITH_COMMA,
    C_ACTUALS,
    C_CALL,
    C_LVALUE,
    C_EXPR,
    C_IFSTM,
    C_WHILESTM,
    C_FORSTM,
    C_RETSTM,
    C_BREAKSTM,
    C_PRINTSTM,
    C_EXPR_OR_NOT,
    C_STM,
    C_STMS,
    C_STMBLOCK,
    C_FUNCDEFINE,
    C_FIELD,
    C_FIELDS,
    C_EXTEND,
    C_IDENT_WITH_COMMA,
    C_IMPLEMENT,
    C_CLASSDEFINE,
    C_PROTYPE,
    C_PROTYPES,
    C_INTERFACEDEFINE,
    C_DEFINE,
    C_PROGRAM
    
};

struct semantics;
struct expr;
struct stm;

struct symres
{
    hashtable_t *table;
    struct symres *parent;
};

struct ident
{
    char *name;
    struct symres *sym;
};

struct if_stm
{
    struct semantics *expr;
    struct semantics *stm1;
    struct semantics *stm2;
};

struct var;

struct tformals
{
    struct semantics *var;
    struct tformals *last;
    struct tformals *next;
};

struct formals
{
    struct semantics *tformals;
};

struct type
{
    uint8_t is_basic;
    uint8_t is_array;
    union
    {
        enum decaf_type btype;
        struct semantics *id;
        struct semantics *arr_type;
    };
};

struct var
{
    struct semantics *type;
    struct semantics *id;
};

struct vardefine
{
    struct semantics *var;
};

struct vardefines
{
    struct semantics *vardefine;
    struct vardefines *last;
    struct vardefines *next;
};

struct expr_with_comma
{
    struct semantics *expr;
    struct expr_with_comma *last;
    struct expr_with_comma *next;
};

struct actuals
{
    struct semantics *expr_with_comma;
};

struct call
{
    uint8_t is_member;
    struct semantics *expr;
    struct semantics *id;
    struct semantics *actuals;
};

struct lvalue
{
    enum
    {
        LVAL_IDENT,
        LVAL_MEMBER,
        LVAL_ARRAY
    } lvalue_type;
    struct semantics *id;
    struct semantics *expr1;
    struct semantics *expr2;
};

struct expr
{
    enum
    {
        EXPR_ASSIGN,
        EXPR_CONST,
        EXPR_LVAL,
        EXPR_THIS,
        EXPR_CALL,
        EXPR_PRIORITY,
        EXPR_PLUS,
        EXPR_MINUS,
        EXPR_MUL,
        EXPR_DIV,
        EXPR_IDIV,
        EXPR_LT,
        EXPR_LE,
        EXPR_GT,
        EXPR_GE,
        EXPR_EQU,
        EXPR_NE,
        EXPR_AND,
        EXPR_OR,
        EXPR_NOT,
        EXPR_READINTEGER,
        EXPR_READLINE,
        EXPR_NEW,
        EXPR_NEWARRAY
        
    } expr_type;
    struct semantics *constant;
    struct semantics *lvalue;
    struct semantics *expr1;
    struct semantics *expr2;
    struct semantics *call;
    struct semantics *id;
    struct semantics *type;
};

struct whilestm
{
    struct semantics *expr;
    struct semantics *stm;
};

struct forstm
{
    struct semantics *expr_or_not1;
    struct semantics *expr;
    struct semantics *expr_or_not2;
    struct semantics *stm;
    
};

struct expr_or_not
{
    struct semantics *expr;
};

struct returnstm
{
    struct semantics *expr_or_not;
};

struct printstm
{
    struct semantics *expr;
};

struct stm
{
    enum
    {
        STM_EMPTY,
        STM_EXPR,
        STM_IF,
        STM_WHILE,
        STM_FOR,
        STM_BREAK,
        STM_RET,
        STM_PRINT,
        STM_BLOCK
    } stm_type;
    union
    {
        struct semantics *expr;
        struct semantics *s_stm;
        struct semantics *stmblock;
    };
};

struct stms
{
    struct semantics *stm;
    struct stms *last;
    struct stms *next;
};

struct stmblock
{
    struct semantics *vardefines;
    struct semantics *stms;
};

struct funcdefine
{
    uint8_t is_void;
    struct semantics *type;
    struct semantics *id;
    struct semantics *formals;
    struct semantics *stmblock;
};

struct field
{
    uint8_t is_vardefine;
    union
    {
        struct semantics *vardefine;
        struct semantics *funcdefine;
    };
};

struct fields
{
    struct semantics *field;
    struct fields *last;
    struct fields *next;
};

struct extend
{
    struct semantics *id;
};

struct id_with_comma
{
    struct semantics *id;
    struct id_with_comma *last;
    struct id_with_comma *next;
};

struct implement
{
    struct semantics *id_with_comma;
};

struct classdefine
{
    struct semantics *id;
    struct semantics *extend;
    struct semantics *implement;
    struct semantics *fields;
};

struct protype
{
    uint8_t is_void;
    struct semantics *type;
    struct semantics *id;
    struct semantics *formals;
};

struct protypes
{
    struct semantics *protype;
    struct protypes *last;
    struct protypes *next;
};

struct interfacedefine
{
    struct semantics *id;
    struct semantics *protypes;
};

struct define
{
    enum
    {
        DEFINE_VAR,
        DEFINE_FUNC,
        DEFINE_CLASS,
        DEFINE_INTERFACE
    } define_type;
    struct semantics *s_define;
};

struct program
{
    struct semantics *define;
    struct program *last;
    struct program *next;
};

struct semantics
{
    char *text;
    uint8_t type_ok;
    enum comp_type type;
    int line;
    union
    {
        int i_val;
        double d_val;
        char *s_val;
        struct tformals *tformals;
        struct formals *formals;
        struct type *vtype;
        struct var *var;
        struct vardefine *vardefine;
        struct vardefines *vardefines;
        struct actuals *actuals;
        struct call *call;
        struct lvalue *lvalue;
        struct expr *expr;
        struct expr_with_comma *expr_with_comma;
        struct if_stm *if_stm;
        struct whilestm *whilestm;
        struct expr_or_not *expr_or_not;
        struct forstm *forstm;
        struct returnstm *returnstm;
        struct printstm *printstm;
        struct stm *stm;
        struct stms *stms;
        struct stmblock *stmblock;
        struct funcdefine *funcdefine;
        struct field *field;
        struct fields *fields;
        struct extend *extend;
        struct id_with_comma *id_with_comma;
        struct implement *implement;
        struct classdefine *classdefine;
        struct protype *protype;
        struct protypes *protypes;
        struct interfacedefine *interfacedefine;
        struct define *define;
        struct program *program;
    };
};

#define parseit(sem) void parse_##sem(int indent, struct semantics *s)

parseit(ident);
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
