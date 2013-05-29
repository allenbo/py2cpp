#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#include "symtab.h"

typedef struct {
    int inclass;
    char* classname;
    type_ty ctype;
}context;

typedef context * context_ty;

context_ty get_context();

#endif
