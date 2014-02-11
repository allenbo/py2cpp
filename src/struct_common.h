#ifndef _STRUCT_COMMON_H_
#define _STRUCT_COMMON_H_

typedef struct type* type_ty;

typedef struct _stmt * stmt_ty;

typedef struct _expr* expr_ty;

typedef struct Variable Variable;

typedef Variable Parameter;

typedef struct stmt_seq stmt_seq;

typedef struct arguments * arguments_ty;

typedef struct exception_handler * exception_handler_ty;

typedef struct slice * slice_ty;

typedef struct comprehension * comprehension_ty;

typedef enum _expr_context { Load = 1, Store = 2, Del = 3, AugLoad = 4, AugStore = 5,
    Param = 6} expr_context_ty;

typedef enum _boolop { Or = 1, And = 2 } boolop_ty;

typedef enum _operator { BitOr = 3, BitXor = 4, BitAnd= 5, Add, Sub, Mult, Div, Mod, FloorDiv, Pow, LShift,
    RShift  } operator_ty;

typedef enum _unaryop { Invert = 15, Not = 16, UAdd = 17, USub = 18 } unaryop_ty;

typedef enum _compop { Eq = 19, NotEq = 20 ,Lt = 21, LtE = 22, Gt = 23, GtE = 24, Is = 25, IsNot = 26,
    In = 27, NotIn = 28 } compop_ty;

#endif
