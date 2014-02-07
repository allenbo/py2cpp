#include "exception.h"
#include "mod.h"


typedef struct binop_exception {
  operator_ty op; 
  expr_ty right;
}binop_exception;

binop_exception int_binop_exception_list [] = {
  { Add, &t_char },
  { Add, &t_boolean },
  { Add, &t_integer },
  { Add, &t_float },


  { Sub, &t_char },
  { Sub, &t_boolean },
  { Sub, &t_integer },
  { Sub, &t_float },

  { Mult, &t_char },
  { Mult, &t_boolean },
  { Mult, &t_integer },
  { Mult, &t_float },

  { Div, &t_char },
  { Div, &t_boolean },
  { Div, &t_integer },
  { Div, &t_float },

};


int in_exception_list(expr_ty e) {
  return 0;
}


int binop_in_exception_list(expr_ty left, expr_ty right, operator_ty op ){
  if( !is_primitive_type(left->e_type) || !is_primitive_type(right->e_type)) return 0;
  
}
