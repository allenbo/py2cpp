#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static symtab_ty global_table = NULL;
static symtab_ty cur_table = NULL;


static symtab_entry_ty
create_symtab_entry(char* name, type_ty tp, enum symtab_entry_kind kind) {
    symtab_entry_ty se = (symtab_entry_ty) malloc (sizeof(struct symtab_entry));
    memset(se, 0, sizeof(struct symtab_entry));
    strcpy(se->se_name, name);
    se->se_type = tp;
    se->se_kind = kind;
}


static symtab_ty 
create_symtab() {
    symtab_ty st  = (symtab_ty) malloc (sizeof(struct symtab));
    memset(st, 0, sizeof(struct symtab));
    st->st_capability = 8;
    st->st_symbols = (symtab_entry_ty*) malloc (sizeof(symtab_entry_ty) * st->st_capability);
    return st;
}


static void
expand_cur_table_for_entry() {
    cur_table->st_capability += 8;
    cur_table->st_symbols = (symtab_entry_ty*)realloc (cur_table->st_symbols, sizeof(symtab_entry_ty) * cur_table->st_capability);
}

symtab_ty get_current_symtab() { return cur_table; }
symtab_ty get_global_table() { return global_table; }



type_ty
search_type_for_name(char* name) {
    if(global_table == NULL) {
        global_table = create_symtab();
        cur_table = global_table;
        return NULL;
    }

    int i;
    for(i = cur_table->st_size - 1 ; i >= 0 ; -- i) {
        symtab_entry_ty se = cur_table->st_symbols[i];
        if(strcmp(se->se_name, name) == 0) {
            return se->se_type;
        }
    }
    return NULL;
}

int
insert_to_current_table(char* name, type_ty tp, enum symtab_entry_kind kind) {
    if(cur_table->st_size == cur_table->st_capability) {
        expand_cur_table_for_entry();
    }
    cur_table->st_symbols[cur_table->st_size ++ ] = create_symtab_entry(name, tp, kind);
    return 1;
}
