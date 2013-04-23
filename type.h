#ifndef _TYPE_H_
#define _TYPE_H_

enum type_kind { CHAR_KIND, SHORT_KIND, INTEGER_KIND, ARRAY_KIND, STRUCT_KIND, POINTER_KIND, FLOAT_KIND,
    DOUBLE_KIND, FUNCTION_KIND };


typedef struct symbol_table* symbol_table_ty;

typedef struct type* type_ty;

struct  type{
    enum type_kind kind;
    int length;
    
    /*This is for arrary*/
    type_ty base;  /* <- also for pointer */
    int size;

    /*This is for struct */
    symbol_table_ty fields;

    /*This is for function */
    int n_params;
    int n_default;
    type_ty* params;
    type_ty* defaults;
};




#endif
