#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static symtab_ty global_table = NULL;
static symtab_ty cur_table = NULL;
static symtab_ty func_table = NULL;

static symtab_ty tables[1000];
int tind = 0;


void push_table(symtab_ty s) {
    tables[tind ++ ] = s;
}
symtab_ty pop_table() {
    return tables[--tind];
}


static symtab_entry_ty
create_incomplete_func_symtab_entry(char* name, stmt_ty s, symtab_ty t) {
    symtab_entry_ty se = (symtab_entry_ty) malloc( sizeof(struct symtab_entry));
    memset(se, 0, sizeof(struct symtab_entry));
    strcpy(se->se_name, name);
    se->se_node  = s;
    se->se_table = t;
    return se;
}


static symtab_entry_ty
create_symtab_entry(char* name, type_ty tp, enum symtab_entry_kind kind, symtab_ty t) {
    symtab_entry_ty se = (symtab_entry_ty) malloc (sizeof(struct symtab_entry));
    memset(se, 0, sizeof(struct symtab_entry));
    strcpy(se->se_name, name);
    se->se_type = tp;
    se->se_kind = kind;
    se->se_table = t;
    return se;
}

static symtab_ty
create_symtab(symtab_ty p) {
    symtab_ty st  = (symtab_ty) malloc (sizeof(struct symtab));
    memset(st, 0, sizeof(struct symtab));
    st->st_capacity = 8;
    st->st_parent = p;
    st->st_symbols = (symtab_entry_ty*) malloc (sizeof(symtab_entry_ty) * st->st_capacity);
    return st;
}

static void
init_global_symtab() {
    if(NULL == global_table)
        global_table = create_symtab(NULL);

    insert_incomplete_func_to_table("abs", NULL);

}



static void
expand_cur_table_for_entry() {
    cur_table->st_capacity += 8;
    cur_table->st_symbols = (symtab_entry_ty*)realloc (cur_table->st_symbols, sizeof(symtab_entry_ty) * cur_table->st_capacity);
}

symtab_ty get_current_symtab() { return cur_table; }
symtab_ty get_global_table() { return global_table; }


type_ty
search_type_for_name(char* name) {
    if(global_table == NULL) {
        global_table = create_symtab(NULL);
        cur_table = global_table;
        return NULL;
    }
    char* p = name;
    if((p = strchr(name, '[') ) != NULL) {
        return (type_ty)1;
    }
    int i;
    symtab_ty st = cur_table;
    while(st) {
        for(i = st->st_size - 1 ; i >= 0 ; -- i) {
            symtab_entry_ty se = st->st_symbols[i];
            if(strcmp(se->se_name, name) == 0) {
                return se->se_type;
            }
        }
        st = st->st_parent;
    }
    return NULL;
}


void
change_func_ret_type(char* name, type_ty t) {
    int i;
    symtab_ty st = cur_table;
    while(st) {
        for(i = st->st_size - 1 ; i >= 0 ; -- i) {
            symtab_entry_ty se = st->st_symbols[i];
            if(strcmp(se->se_name, name) == 0) {
                se->se_type = t;
                break;
            }
        }
        st = st->st_parent;
    }

}

stmt_ty
search_stmt_for_name(char* name) {
    int i;
    symtab_ty st = cur_table;
    while(st) {
        for(i = st->st_size - 1 ; i >= 0 ; -- i) {
            symtab_entry_ty se = st->st_symbols[i];
            if(strcmp(se->se_name, name) == 0) {
                func_table = se->se_table;
                return se->se_node;
            }
        }
        st = st->st_parent;
    }
    return NULL;
}

int
insert_to_current_table(char* name, type_ty tp, enum symtab_entry_kind kind) {
    if(global_table == NULL) {
        global_table = create_symtab(NULL);
        cur_table = global_table;
    }

    if(cur_table->st_size == cur_table->st_capacity) {
        expand_cur_table_for_entry();
    }
    cur_table->st_symbols[cur_table->st_size ++ ] = create_symtab_entry(name, tp, kind, cur_table);
    return 1;
}


int
insert_to_func_table(char* name, type_ty tp, enum symtab_entry_kind kind) {
    if(global_table == NULL) {
        global_table = create_symtab(NULL);
        cur_table = global_table;
    }
    push_table(cur_table);
    cur_table = func_table;
    if(cur_table->st_size == cur_table->st_capacity) {
        expand_cur_table_for_entry();
    }
    cur_table->st_symbols[cur_table->st_size ++ ] = create_symtab_entry(name, tp, kind, cur_table);
    cur_table = pop_table();
    return 1;
}


int
insert_incomplete_func_to_table(char* name, stmt_ty node) {
    if(global_table == NULL) {
        global_table = create_symtab(NULL);
        cur_table = global_table;
    }

    if(cur_table->st_size == cur_table->st_capacity) {
        expand_cur_table_for_entry();
    }
    cur_table->st_symbols[cur_table->st_size ++ ] = create_incomplete_func_symtab_entry(name, node, cur_table);
    return 1;
}



void
enter_new_scope_for_func() {
    if(0 == func_table->child_capacity ) {
        func_table->child_capacity = 8;
        func_table->st_children = (symtab_ty*) malloc( func_table->child_capacity * sizeof(symtab_ty));
    }
    if(func_table->child_capacity == func_table->n_child) {
        func_table->child_capacity += 8;
        func_table->st_children = (symtab_ty*) realloc (func_table->st_children,
                sizeof(symtab_ty) * func_table->child_capacity) ;
    }
    func_table->st_children[func_table->n_child ++ ] = create_symtab(func_table);
    push_table(cur_table);
    cur_table = func_table->st_children[func_table->n_child - 1];
}

void
exit_scope_from_func() {
    cur_table = pop_table();
}
