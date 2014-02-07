#include "symtable.h"


// check if a expression is in exception list
int in_exception_list(expr_ty e);


// check if a binop expression is in exception list
int binop_in_exception_list(expr_ty left, expr_ty right, operator_ty op );

