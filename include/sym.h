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

struct symres
{
};

struct semantics
{
    char *text;
    uint8_t type_OK;
    enum decaf_type type;
    union
    {
        int i_val;
        double d_val;
        char *s_val;
        struct symres *sym;
    };
};

#endif
