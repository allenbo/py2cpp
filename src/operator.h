#ifndef _OPERATOR_H_
#define _OPERATOR_H_

typedef enum _boolop { Or = 1, And = 2 } boolop_ty;

typedef enum _operator { BitOr = 3, BitXor = 4, BitAnd= 5, Add, Sub, Mult, Div, Mod, FloorDiv, Pow, LShift,
    RShift  } operator_ty;

typedef enum _unaryop { Invert = 15, Not = 16, UAdd = 17, USub = 18 } unaryop_ty;

typedef enum _compop { Eq = 19, NotEq = 20 ,Lt = 21, LtE = 22, Gt = 23, GtE = 24, Is = 25, IsNot = 26,
    In = 27, NotIn = 28 } compop_ty;


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
