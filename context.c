#include "context.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static context_ty con = NULL;

static context_ty
init_con() {
    context_ty con = (context_ty) malloc (sizeof( context));
    memset(con, 0, sizeof(context));
    return con;
}

context_ty
get_context() {
    if(NULL == con) {
        con = init_con();
    }
    return con;
}
