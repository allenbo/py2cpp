#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#include "symtab.h"
#include "util.h"
#include "type_common.h"

typedef struct {
    /* whether the function is a class member or not */
    int inclass;
    char* classname;
    type_ty ctype;

    int inmember;
    char* selfname;

    int setitem;
    char* itemname;
    hashtable_ty ht;

    /* for yield statement */
    int is_yield;
    type_ty yield_type;
    
    funcentry_ty fe;

    char prefix[128];

}context;

typedef context * context_ty;

context_ty get_context();

#endif
