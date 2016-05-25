#ifndef __IR_H__
#define __IR_H__

#include <stdlib.h>
#include "symtable.h"

enum ir_type
{
    IR_BREAK, //not in final ir
    IR_SINGLE,
    IR_DOUBLE,
    IR_SAVE,
    IR_RESTORE,
    IR_NEW,
    IR_B,
    IR_J
};

struct ir
{
    enum ir_type type;
    char *code;
    struct symres *env;
};


#endif
