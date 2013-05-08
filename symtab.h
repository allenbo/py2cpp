#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "mod.h"

enum symtab_entry_kind { SE_UNKNOWN_KIND, SE_FUNCTION_KIND, SE_VARIABLE_KIND, SE_CONSTANT_KIND , SE_GLOBAL_KIND,
    SE_DEFAULT_KIND, SE_TEMP, SE_CLASS_KIND};

enum symtab_kind { SK_FILE_KIND, SK_FUNCTION_KIND, SK_CLASS_KIND};

enum type_kind { UNKNOWN_KIND,BOOLEAN_KIND,  CHAR_KIND, INTEGER_KIND, FLOAT_KIND, STRING_KIND, LIST_KIND, DICT_KIND, POINTER_KIND,
    TUPLE_KIND, FUNCTION_KIND, CLASS_KIND};


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




struct  type{
    enum type_kind kind;
    char name[128];
    
    /*This is for arrary*/
    type_ty base;  /* <- also for pointer */
    int size;
    
    type_ty kbase;
    type_ty vbase;
};


void assign_type_to_ast(stmt_seq* ss);


int insert_to_current_table(char *name, type_ty t, enum symtab_entry_kind kind);
int insert_to_func_table(char *name, type_ty t, enum symtab_entry_kind kind);
int insert_to_global_table(char *name, type_ty t, enum symtab_entry_kind kind);
int insert_incomplete_func_to_table(char* name, stmt_ty node);
int insert_class_to_table(char* name);


type_ty search_type_for_name(char* name);
type_ty search_type_for_name_and_class(char* name, char* class);
stmt_ty search_stmt_for_name(char* name);



void change_func_ret_type(char* fullname, type_ty func_ret);


void enter_new_scope_for_func();
void exit_scope_from_func();

type_ty create_list_type(type_ty t);
#endif
