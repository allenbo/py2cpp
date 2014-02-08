#ifndef _OPERATOR_H_
#define _OPERATOR_H_

#include "struct_common.h"

char* get_binop_fake_literal(operator_ty op) ;
char* get_binop_true_literal(operator_ty op ) ;

char* get_augop_true_literal(operator_ty op) ;
char* get_augop_fake_literal(operator_ty op) ;

char* get_cmpop_true_literal(compop_ty op) ;
char* get_cmpop_fake_literal(compop_ty op) ;

char* get_unaryop_true_literal(unaryop_ty op) ;
char* get_unaryop_fake_literal(unaryop_ty op) ;

char* get_boolop_literal(boolop_ty op) ;


int is_precedent(operator_ty f, operator_ty s);
#endif
