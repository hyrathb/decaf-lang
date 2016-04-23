#ifndef __SYM_H__
#define __SYM_H__

#include <stdlib.h>
#include <stdint.h>

#define YYSTYPE struct semantics

enum decaf_type
{
    D_INT,
    D_DOUBLE,
    D_STRING
};

enum comp_type
{
    C_IDENT,
    C_ICONST,
    C_BCONST,
    C_DCONST,
    C_SCONST,
    C_NULL,
    OTHER
};

struct symres
{
};

struct semantics
{
    char *text;
    uint8_t type_ok;
    enum comp_type type;
    union
    {
        int i_val;
        double d_val;
        char *s_val;
        struct symres *sym;
    };
};

#endif
