#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#include "symtab.h"
#include "util.h"

typedef struct {
    /* whether the function is a class member or not */
    int inclass;
    char* classname;
    type_ty ctype;

    int inmember;
    char* selfname;
    int indent;

    int setitem;
    char* itemname;
    hashtable_ty ht;
}context;

typedef context * context_ty;

context_ty get_context();

#endif
