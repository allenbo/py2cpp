#include <string.h>
#include <stdio.h>
#include "primary.h"
#include "type_common.h"
#include "mod.h"

extern struct type t_unknown;
extern struct type t_char;
extern struct type t_boolean;
extern struct type t_integer;
extern struct type t_float;
extern struct type t_string;


typedef struct library_function_map {
  operator_ty op;
  type_ty t;
  int flirt;
  char *funcname;
}func_map;

#define BUF_SIZE 1024
static char buf[BUF_SIZE];


func_map int_func_map[]  = {
  {Pow, NULL, 0, "lib_pow"}, // NULL means every primary type
  {Mult, &t_string, 1, "lib_string_mult"}
};

func_map string_func_map[] = {
  {Mult, &t_integer, 0, "lib_string_mult"},
  {Mult, &t_char, 0, "lib_string_mult"}
};


int is_primary_type(type_ty t) {
  if ( t == &t_char || t == &t_boolean || t == &t_integer || t == &t_float || t == &t_string ) return 1;
  return 0;
}


char* primary_type_with_lib(expr_ty left, operator_ty op, expr_ty right) {
  func_map * map = NULL;
  if (left->e_type == &t_string ) {
    map = string_func_map;
  }else {
    map = int_func_map;
  }

  int i = 0;
  int size = 2; // two maps both have two items;
  for(; i < size; i ++ ) {
    if( op == map[i].op && (map[i].t == NULL || map[i].t == right->e_type) ) {
      char* funcname = map[i].funcname;
      int flirt = map[i].flirt;

      if( flirt ) {
        sprintf(buf, "%s( %s, %s )", funcname, right->ann, left->ann);
      }
      else {
        sprintf(buf, "%s( %s, %s )", funcname, left->ann, right->ann);
      }

      return buf;
    }
  }

  return NULL;

}
