#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern struct type t_unknown;
extern struct type t_char;
extern struct type t_boolean;
extern struct type t_integer;
extern struct type t_float;
extern struct type t_string;

static symtab_ty global = NULL;
static symtab_ty cur = NULL;
static symtab_ty func_table = NULL;

static symtab_ty tables[1000];
int tind = 0;


symtab_ty get_current_symtab() { return cur; }
symtab_ty get_global_symtab() { return global; }


void push_table(symtab_ty s) {
    tables[tind ++ ] = s;
}
symtab_ty pop_table() {
    return tables[--tind];
}


static void
expand_cur_for_entry() {
    cur->st_capacity += 8;
    cur->st_symbols =
        (symtab_entry_ty*)realloc(
                cur->st_symbols,
                sizeof(symtab_entry_ty)*cur->st_capacity
                );
}



static symtab_entry_ty
create_symtab_entry(
        char* name,
        type_ty tp,
        enum symtab_entry_kind kind,
        symtab_ty t)
{
    symtab_entry_ty se =
        (symtab_entry_ty) malloc (sizeof(struct symtab_entry));
    memset(se, 0, sizeof(struct symtab_entry));
    strcpy(se->se_name, name);
    se->se_type = tp;
    se->se_kind = kind;
    se->se_table = t;
    return se;
}

static symtab_ty
create_symtab(symtab_ty p, enum symtab_kind kind) {
    symtab_ty st  = (symtab_ty) malloc (sizeof(struct symtab));
    memset(st, 0, sizeof(struct symtab));
    st->st_capacity = 8;
    st->st_parent = p;
    st->st_kind = kind;
    st->st_symbols =
        (symtab_entry_ty*) malloc (
                sizeof(symtab_entry_ty) * st->st_capacity
                );
    return st;
}


static void
init_global() {
    global = create_symtab(NULL, SK_GLOBAL_KIND);
    cur = global;
    type_ty tp = (type_ty) malloc ( sizeof( struct type));
    tp->kind = MODULE_KIND;
    install_scope_variable("__test__", tp, SE_MODULE_KIND);
}


int
insert_to_current_table(char* name, type_ty tp, enum symtab_entry_kind kind) {
    if(global == NULL) {
        init_global();
    }

    if(cur->st_size == cur->st_capacity) {
        expand_cur_for_entry();
    }
    cur->st_symbols[cur->st_size ++ ] =
        create_symtab_entry(name, tp, kind, cur);
    return 1;
}



void
enter_new_scope_for_func() {
    if(0 == func_table->child_capacity ) {
        func_table->child_capacity = 8;
        func_table->st_children =
            (symtab_ty*) malloc(
                    func_table->child_capacity*sizeof(symtab_ty)
                    );
    }
    if(func_table->child_capacity == func_table->n_child) {
        func_table->child_capacity += 8;
        func_table->st_children =
            (symtab_ty*) realloc (
                    func_table->st_children,
                    sizeof(symtab_ty) * func_table->child_capacity
                    );
    }
    func_table->st_children[func_table->n_child ++ ] =
        create_symtab(func_table, SK_FUNCTION_KIND);
    push_table(cur);
    cur = func_table->st_children[func_table->n_child - 1];
}


void
enter_new_scope_for_class() {
    push_table(cur);
    cur = cur->st_symbols[cur->st_size -1]->se_scope;
}

void
exit_scope() {
    cur = pop_table();
}


void
install_variable(expr_ty e){
    install_variable_full(e, SE_VARIABLE_KIND);
}

void
install_variable_full(expr_ty e, enum symtab_entry_kind kind) {
    if(NULL == global) {
        init_global();
    }

    char* name = NULL;
    type_ty tp = NULL;
loop:
    if(e->kind == Name_kind) {
        name = e->name.id;
        tp = e->e_type;
    }

    if(cur->st_size == cur->st_capacity)
        expand_cur_for_entry();
    cur->st_symbols[cur->st_size ++ ] =
        create_symtab_entry(name, tp, kind, cur);
}

void
install_scope_variable(char* name, type_ty tp,
        enum symtab_entry_kind kind) {
    if(NULL == global)
        init_global();

    if(kind == SE_MODULE_KIND ) {
        push_table(cur);
        cur = global;
    }

    symtab_ty sp = create_symtab(cur, SK_MODULE_KIND);

    cur->child_capacity = 8;
    int n  = cur->child_capacity;
    cur->st_children = (symtab_ty*) malloc (
            sizeof(symtab_ty) * n);
    cur->st_children[global->n_child++] = sp;

    tp->scope = sp;
    if(cur->st_size == cur->st_capacity)
        expand_cur_for_entry();
    cur->st_symbols[cur->st_size ++ ] =
        create_symtab_entry(name, tp, kind, cur);
    push_table(cur);
    cur = sp;
}

type_ty
lookup_variable(char* name) {
    if(global == NULL) {
        init_global();
        return &t_unknown;
    }

    symtab_ty st = cur;
    while(st) {
        int i;
        int n = st->st_size;
        symtab_entry_ty se = NULL;
        for(i = n - 1; i >= 0; i -- ) {
            se = st->st_symbols[i];
            if(strcmp(se->se_name, name) == 0) {
                return se->se_type;
            }
        }
        st = st->st_parent;
    }
    return &t_unknown;
}

type_ty
lookup_scope_variable(char* name) {
    symtab_ty st = cur;
    int i;
    int n = st->st_size;
    symtab_entry_ty se = NULL;
    for(i = n - 1; i >= 0; i -- ) {
        se = st->st_symbols[i];
        if(strcmp(se->se_name, name) == 0) {
            return se->se_type;
        }
    }
    return &t_unknown;
}

void
change_symtab(symtab_ty st) {
    push_table(cur);
    cur = st;
}

void change_symtab_back() {
    cur = pop_table();
}
