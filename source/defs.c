#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

void printindent(int indent)
{
    int i;
    for (i=0; i<indent; ++i)
    {
        if (i%2)
            putchar('|');
        else if (i == indent-1)
            putchar('-');
        else 
            putchar(' ');
    }
}

#define ind  if (!s)\
                return;\
              printindent(indent);

printit(ident)
{
    ind;
    printf("identifier:%s\n", s->text);
}

printit(const)
{
    int i;
    ind;
    switch (s->type)
    {
    case C_ICONST:
        printf("int const: %d\n", s->i_val);
        break;
    case C_BCONST:
        printf("bool const: %s\n", s->i_val?"true":"false");
        break;
    case C_DCONST:
        printf("double const: %f\n", s->d_val);
        break;
    case C_SCONST:
        printf("string const: %s\n", s->s_val);
        break;
    default:
        return;
    }
}

printit(null)
{
    ind;
    printf("NULL\n");
}

printit(formals)
{
    struct tformals *i;
    ind;
    for (i=s->formals->tformals->tformals; i; i=i->next)
        print_var(indent, i->var);
}

printit(type)
{
    ind;
    if (s->vtype->is_basic)
    {
        switch(s->vtype->btype)
        {
        case D_INT:
            printf("type: int\n");
        case D_BOOL:
            printf("type: bool\n");
        case D_DOUBLE:
            printf("type: double\n");
        case D_STRING:
            printf("type: string\n");
        default:
            return;
        }
    }
    else if (s->vtype->is_array)
    {
        printf("type: array of\n");
        print_type(indent+2, s->vtype->arr_type);
    }
    else
    {
        printf("type: class\n");
        print_ident(indent+2, s->vtype->id);
    }
}
