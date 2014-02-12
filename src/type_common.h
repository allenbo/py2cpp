#ifndef _TYPE_COMMON_H_
#define _TYPE_COMMON_H_

enum type_kind {
    UNKNOWN_KIND,
    BOOLEAN_KIND,
    CHAR_KIND,
    INTEGER_KIND,
    FLOAT_KIND,
    STRING_KIND,
    LIST_KIND,
    SET_KIND,
    DICT_KIND,
    TUPLE_KIND,
    GENERATOR_KIND,
    FUNCTION_KIND,
    LAMBDA_KIND,
    CLASS_KIND,
    MODULE_KIND,
    SCOPE_KIND
};

typedef struct type * type_ty;
typedef struct symtab * symtab_ty;
typedef struct funcentry * funcentry_ty;
typedef struct _stmt * stmt_ty;

struct  type {
    enum type_kind kind;
    char name[128];

    /* This is for list */
    type_ty base;

    /* This is for tuple */
    int n_elt;
    type_ty * elts;


    type_ty kbase;
    type_ty vbase;

    /* for class and module */
    symtab_ty scope;

    /* for function */
    stmt_ty def;  /* corresponding statement*/
    int ind;
    funcentry_ty tab[128];
    type_ty ctype;  /* the type of the class */
};



struct funcentry {
    char index[128]; /* the short of parameters */
    int n_param;  /* number of parameters */
    type_ty * params;
    type_ty ret;
    symtab_ty scope;
};


#endif
