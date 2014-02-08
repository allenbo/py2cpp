#ifndef _PRIMARY_H_
#define _PRIMARY_H_

#include "struct_common.h"
#include "type_common.h"


int is_primary_type(type_ty t);
char* primary_type_with_lib(expr_ty left, operator_ty op, expr_ty right);

#endif
