#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "mod.h"

enum symtab_entry_kind {
    SE_UNKNOWN_KIND,
    SE_FUNCTION_KIND,
    SE_VARIABLE_KIND,
    SE_GLOBAL_KIND,
    SE_DEFAULT_KIND,
    SE_CLASS_KIND,
    SE_MODULE_KIND,
    SE_REUSE_KIND
};

enum symtab_kind {
    SK_GLOBAL_KIND,
    SK_MODULE_KIND,
    SK_FUNCTION_KIND,
    SK_CLASS_KIND
};

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
    FUNCTION_KIND,
    CLASS_KIND,
    MODULE_KIND
};


typedef struct symtab_entry * symtab_entry_ty;
typedef struct symtab * symtab_ty;


struct symtab_entry {
    symtab_ty se_table;
    char se_name[128];
    char c_name[128];
    enum symtab_entry_kind se_kind;
    type_ty se_type;
    stmt_ty se_node;
    symtab_ty se_scope;
};



struct symtab {
    enum symtab_kind st_kind;
    int st_size;   /* the size of table entry */
    int st_capacity;  /* the whole slot in array */
    symtab_entry_ty * st_symbols;

    /* pointer to out scope*/
    symtab_ty st_parent;

    int n_child;
    int child_capacity;
    symtab_ty * st_children;
};




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

};


void install_variable(expr_ty e);
void install_variable_full(expr_ty e, enum symtab_entry_kind kind);
void install_scope_variable(char* name, type_ty tp, enum symtab_entry_kind kind);
type_ty lookup_variable(char* name);
type_ty lookup_scope_variable(char* name);

void change_symtab(symtab_ty st);
void change_symtab_back();


void assign_type_to_ast(stmt_seq* ss);
int insert_to_current_table(char *name, type_ty t, enum symtab_entry_kind kind);

void enter_new_scope_for_func();
void enter_new_scope_for_class();
void exit_scope();

//type_ty create_list_type(type_ty t);
#endif
