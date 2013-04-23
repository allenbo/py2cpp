#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "type.h"

enum symtab_entry_kind { UNKNOWN_KIND, VARIABLE_KIND, SK_CONSTANT_KIND };

typedef struct symtab_entry * symtab_entry_ty;
typedef struct symtab * symtab_ty;


struct symtab_entry {
    int se_offset;
    char se_name[128];
    enum symtab_entry_kind se_kind;    
    type_ty se_type;
};



struct symtab {
    int st_size;   /* the size of table entry */
    int st_capability;  /* the whole slot in array */
    symtab_entry_ty * st_symbols; 
   
    /* pointer to out scope*/
    symtab_ty st_parent;
    
    int n_child;
    int child_capability;
    symtab_ty * st_children;
};



#endif
