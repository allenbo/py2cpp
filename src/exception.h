#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "symtab.h"


// check if a expression is in exception list
int in_exception_list(expr_ty e);


// check if a binop expression is in exception list
int binop_in_exception_list(expr_ty left, expr_ty right, operator_ty op );
int compare_in_exception_list(expr_ty left, expr_ty right, operator_ty op );

#endif
