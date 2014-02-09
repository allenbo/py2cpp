#include "exception.h"
#include "primary.h"
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



typedef struct compare_exception {
  operator_ty op;
  type_ty right_type;
} compare_exception;


compare_exception numeric_compare_exception_list [] = {
  { Eq, &t_char }, { Eq, &t_boolean }, { Eq, &t_integer }, { Eq, &t_float },
  { NotEq, &t_char }, { NotEq, &t_boolean }, { NotEq, &t_integer }, { NotEq, &t_float },
  { Lt, &t_char }, { Lt, &t_boolean }, { Lt, &t_integer }, { Lt, &t_float },
  { LtE, &t_char }, { LtE, &t_boolean }, { LtE, &t_integer }, { LtE, &t_float },
  { Gt, &t_char }, { Gt, &t_boolean }, { Gt, &t_integer }, { Gt, &t_float },
  { GtE, &t_char }, { GtE, &t_boolean }, { GtE, &t_integer }, { GtE, &t_float },
  { Is, &t_char }, { Is, &t_boolean }, { Is, &t_integer }, { Is, &t_float },
  { IsNot, &t_char }, { IsNot, &t_boolean }, { IsNot, &t_integer }, { IsNot, &t_float },
};


compare_exception string_compare_exception_list [] = {
  { Eq, &t_string },
  { NotEq, &t_string },
  { Lt, &t_string },
  { LtE, &t_string },
  { Gt, &t_string },
  { GtE, &t_string },
  { Is, &t_string },
  { IsNot, &t_string },
};


int in_exception_list(expr_ty e) {
  return 0;
}


int binop_in_exception_list(expr_ty left, expr_ty right, operator_ty op ){
  int size = 0;
  binop_exception * list = NULL;
  if ( is_primary_type( left->e_type) ) {
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


int compare_in_exception_list(expr_ty left, expr_ty right, operator_ty op ) {
  int size = 0;
  compare_exception * list = NULL;
  if ( is_primary_type(left->e_type) ) {
    if ( left->e_type == &t_integer || left->e_type == &t_char || left->e_type == &t_boolean || left->e_type == &t_float ) {
      size = sizeof(numeric_compare_exception_list) / sizeof(compare_exception);
      list = numeric_compare_exception_list;
    }
    else {
      size = sizeof(string_compare_exception_list) / sizeof(compare_exception);
      list = string_compare_exception_list;
    }


    int i = 0;
    for(; i < size; i ++ ) {
      if (list[i].op == op && list[i].right_type == right->e_type ) return 1;
    }
    return 0;
  }

  return 0;
}
