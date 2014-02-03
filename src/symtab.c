#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "code.h"

extern struct type t_unknown;
extern struct type t_char;
extern struct type t_boolean;
extern struct type t_integer;
extern struct type t_float;
extern struct type t_string;

static symtab_ty global = NULL;
static symtab_ty cur = NULL;
static symtab_ty file = NULL;

static symtab_ty tables[1000];
int tind = 0;


symtab_ty get_current_symtab() { return cur; }
symtab_ty get_global_symtab() { return global; }
symtab_ty get_curfile_symtab() { return file; }

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

    /* return to the scope in function defination */
    if(tp && tp->scope == NULL)
        tp->scope = t;
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


static funcentry_ty
create_funcentry(char* index, int n_param, type_ty* params, type_ty ret, symtab_ty p) {
    funcentry_ty fe = (funcentry_ty) malloc(sizeof(struct funcentry) );
    strcpy(fe->index, index);
    fe->n_param = n_param;
    fe->params = params;
    fe->ret = ret;

    symtab_ty sp = NULL;
    sp = create_symtab(p, SK_FUNCTION_KIND);

    if(p->child_capacity == 0) {
        p->child_capacity = 8;
        int n  = p->child_capacity;
        p->st_children = (symtab_ty*) malloc (
                sizeof(symtab_ty) * n);
    }else {
        if(p->child_capacity == p->n_child) {
            p->child_capacity += 8;
            int n = p->child_capacity;
            p->st_children = (symtab_ty*) realloc( p->st_children,
                    sizeof(symtab_ty)* n);
        }
    }
    p->st_children[p->n_child++] = sp;
    fe->scope = sp;

    push_table(cur);
    cur = sp;
    return fe;
}


static void
init_global() {
    global = create_symtab(NULL, SK_GLOBAL_KIND);
    cur = global;
    type_ty tp = (type_ty) malloc ( sizeof( struct type));
    tp->kind = MODULE_KIND;
    install_scope_variable("__test__", tp, SE_MODULE_KIND);
    file = tp->scope;
}



void
install_variable(char* name, type_ty tp, enum symtab_entry_kind kind) {
    if(NULL == global) {
        init_global();
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

    symtab_ty sp = NULL;
    switch(kind) {
        case SE_MODULE_KIND:
            push_table(cur);
            cur = global;
            sp = create_symtab(cur, SK_MODULE_KIND);
            break;
        case SE_CLASS_KIND:
            sp = create_symtab(cur, SK_CLASS_KIND);
            break;
        case SE_SCOPE_KIND:
            sp = create_symtab(cur, SK_SCOPE_KIND);
            break;
        case SE_FUNCTION_KIND:
            sp = create_symtab(cur, SK_FUNCTION_KIND);
            break;
        default:
            fprintf(stderr, "This is not a scope kind\n");
    }


    if(cur->child_capacity == 0) {
        cur->child_capacity = 8;
        int n  = cur->child_capacity;
        cur->st_children = (symtab_ty*) malloc (
                sizeof(symtab_ty) * n);
    }else {
        if(cur->child_capacity == cur->n_child) {
            cur->child_capacity += 8;
            int n = cur->child_capacity;
            cur->st_children = (symtab_ty*) realloc( cur->st_children,
                    sizeof(symtab_ty)* n);
        }
    }
    cur->st_children[cur->n_child++] = sp;

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

void
change_type(char* name, type_ty tp ){
    symtab_ty st = cur;
    while(st) {
        int i;
        int n = st->st_size;
        symtab_entry_ty se = NULL;
        for(i = n - 1; i >= 0; i -- ) {
            se = st->st_symbols[i];
            if(strcmp(se->se_name, name) == 0) {
                se->se_type = tp;
                return ;
            }
        }
        st = st->st_parent;
    }
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

void
functable_insert(char* name, int n, Parameter* args, type_ty tp ) {
    type_ty ret = &t_unknown;
    int n_param = n;
    type_ty* params = NULL;
    params = (type_ty*) malloc (sizeof(type_ty) * n);

    int i;
    for(i = 0; i < n; i ++ ) {
        params[i] = args[i].args->e_type;
    }

    tp->tab[tp->ind++] = create_funcentry(name, n_param, params, ret, tp->scope);
}

void
functable_insert_ret(char* name, type_ty ret, type_ty tp) {
    int i, n = tp->ind;
    for(i = 0; i < n;i ++ ) {
        if(strcmp(name, tp->tab[i]->index) == 0) {
            tp->tab[i]->ret = ret;
            return;
        }
    }
}

type_ty
functable_lookup(char* name, type_ty tp) {
    int i, n = tp->ind;
    for(i = 0; i < n;i ++ ) {
        if(strcmp(name, tp->tab[i]->index) == 0) {
            return tp->tab[i]->ret;
        }
    }
    return NULL;
}

int
has_constructor() {
    if(cur->st_kind != SK_CLASS_KIND) return 1;

    int i, n = cur->st_size;
    for (i = 0; i < n; i ++ ) {
        if(strcmp(cur->st_symbols[i]->se_name, "__init__") == 0)
            return 1;
    }
    return 0;
}


void
output_symtab(FILE* fout, symtab_ty st) {
    if(NULL == st) return;

    char buf[128] = "";
    int i, n = st->st_size;
    for(i = 0; i < n; i ++ ){
        symtab_entry_ty se = st->st_symbols[i];
        if(se->se_kind == SE_PARAMETER_KIND || se->se_kind == SE_SCOPE_KIND ||
                se->se_kind == SE_CLASS_KIND)
            continue;
        else if(se->se_kind == SE_FUNCTION_KIND) {
            type_ty t = se->se_type;
            stmt_ty def = t->def;
            int i, n = t->ind, from = 0;
            if((get_context())->inclass != 1) {
                for(i = 0; i < n; i ++ ) {
                    sprintf(buf, "%s %s(", t->tab[i]->ret->name, def->funcdef.name);
                    fprintf(fout, "%s", buf);
                    int j, m = t->tab[i]->n_param;
                    arguments_ty args = def->funcdef.args;
                    for(j = 0; j < m; j++ ){
                        args->params[j + from]->args->e_type = t->tab[i]->params[j];
                        sprintf(buf, "%s %s", args->params[j + from]->args->e_type->name,
                                args->params[j + from]->args->name.id);
                        fprintf(fout, "%s", buf);
                        if(j != m-1) {
                            sprintf(buf, ", ");
                            fprintf(fout, "%s", buf);
                        }
                    }
                    sprintf(buf, ");\n");
                    fprintf(fout, "%s", buf);
                }
            }
        }else {
            if((get_context())->inclass == 1) {
                sprintf(buf, "static ");
                fprintf(fout, "%s", buf);
            }
            type_ty tp = se->se_type;
            sprintf(buf, "%s %s;\n", tp->name, se->se_name);
            fprintf(fout, "%s", buf);
        }
    }
}
