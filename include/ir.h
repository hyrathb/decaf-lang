#ifndef __IR_H__
#define __IR_H__

#include <stdlib.h>
#include "symtable.h"

enum ir_type
{
    IR_SINGLE,
    IR_DOUBLE,
    IR_B,
    IR_J
}

struct ir
{
    enum ir_type type;
    char *code;
};


#endif
