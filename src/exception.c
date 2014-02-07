#include "exception.h"
#include "mod.h"

extern struct type t_unknown;
extern struct type t_char;
extern struct type t_boolean;
extern struct type t_integer;
extern struct type t_float;
extern struct type t_string;


typedef struct binop_exception {
  operator_ty op; 
  type_ty right_type;
}binop_exception;

binop_exception int_binop_exception_list [] = {
  { Add, &t_char }, { Add, &t_boolean }, { Add, &t_integer }, { Add, &t_float },
  { Sub, &t_char }, { Sub, &t_boolean }, { Sub, &t_integer }, { Sub, &t_float },
  { Mult, &t_char }, { Mult, &t_boolean }, { Mult, &t_integer }, { Mult, &t_float },
  { Div, &t_char }, { Div, &t_boolean }, { Div, &t_integer }, { Div, &t_float }, 
  { Mod, &t_char }, { Mod, &t_boolean }, { Mod, &t_integer }, { Mod, &t_float },
  { LShift, &t_char }, { LShift, &t_integer },
  { RShift, &t_char }, { RShift, &t_integer },
  { BitOr, &t_char }, { BitOr, &t_integer },
  { BitAnd, &t_char }, { BitAnd, &t_integer },
  { BitXor, &t_char }, { BitXor, &t_integer },
  { FloorDiv, &t_char }, { FloorDiv, &t_boolean }, { FloorDiv, &t_integer}, { FloorDiv, &t_float },
};

binop_exception* char_binop_exception_list = int_binop_exception_list;
binop_exception* bool_binop_exception_list = int_binop_exception_list;

binop_exception float_binop_exception_list[] = {
  { Add, &t_char }, { Add, &t_boolean }, { Add, &t_integer }, { Add, &t_float },
  { Sub, &t_char }, { Sub, &t_boolean }, { Sub, &t_integer }, { Sub, &t_float },
  { Mult, &t_char }, { Mult, &t_boolean }, { Mult, &t_integer }, { Mult, &t_float },
  { Div, &t_char }, { Div, &t_boolean }, { Div, &t_integer }, { Div, &t_float }, 
  { FloorDiv, &t_char }, { FloorDiv, &t_boolean }, { FloorDiv, &t_integer}, { FloorDiv, &t_float },
};

binop_exception string_binop_exception_list [] = {
  {Add, &t_string}, 
};


int in_exception_list(expr_ty e) {
  return 0;
}


int binop_in_exception_list(expr_ty left, expr_ty right, operator_ty op ){
  int size = 0;
  binop_exception * list = NULL;
  if ( is_primitive_type( left->e_type) ) {
    if ( left->e_type == &t_integer || left->e_type == &t_char || left->e_type == &t_boolean ) {
      size = sizeof(int_binop_exception_list) / sizeof(binop_exception );
      list = int_binop_exception_list; 
    }

    else if ( left->e_type == &t_float ) {
      size = sizeof(float_binop_exception_list) / sizeof( binop_exception );
      list = float_binop_exception_list;
    }
    
    else {
      size = sizeof(string_binop_exception_list) / sizeof( binop_exception );
      list = string_binop_exception_list;
    }

    int i = 0;
    for(i = 0; i < size; i ++ ) {
      if (list[i].op == op && list[i].right_type == right->e_type ) return 1;
    }

    return 0;
  }

  return 0;
}
