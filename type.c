#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* newTemp() {
    static int i = 0;
    char * tmp = (char* ) malloc( sizeof(char) * 10);
    sprintf(tmp, "_t%d", i++);
    return tmp;
}


struct type t_unknown  = {UNKNOWN_KIND, 0};
struct type t_char = {CHAR_KIND, 1};
struct type t_integer = {INTEGER_KIND, 4};
struct type t_float = {FLOAT_KIND, 4};
struct type t_string = {POINTER_KIND, 4,  &t_char};

static void assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);
static int type_compare(type_ty t1, type_ty t2);
static type_ty max_type(type_ty t1, type_ty t2);

static type_ty 
create_list_type(type_ty t) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    tp->kind = LIST_KIND;
    tp->base = t;
    return tp;
}


static int
type_compare(type_ty t1, type_ty t2) {
}

static type_ty
max_type(type_ty t1, type_ty t2) {
    if(t1 == t2) return t1;
    if(t1 == &t_float || t2 == &t_float) return &t_float;
}


static void
assign_type_to_stmt(stmt_ty s) {
    int i;
    switch(s->kind) {
        case FuncDef_kind:
            break;
        case ClassDef_kind:
            break;
        case Return_kind:
            break;
        case Delete_kind:
            break;
        case Assign_kind:
            {
                int n_target = s->assign.n_target;
                expr_ty * targets = s->assign.targets;
                expr_ty value = s->assign.value;
                assign_type_to_expr(targets[0]);
                assign_type_to_expr(value);
                insert_to_current_table(targets[0]->name.id,
                        value->e_type, SE_VARIABLE_KIND);

                switch(value->e_type->kind) {
                    case INTEGER_KIND:
                        printf("int %s = %s;\n", targets[0]->addr, value->addr);
                        break;
                    case FLOAT_KIND:
                        printf("float %s = %s;\n", targets[0]->addr, value->addr);
                        break;
                    case LIST_KIND:
                        printf("vector<%s> & %s= %s;\n", value->list.elts[0]->e_type == &t_integer?"int":"float",
                                targets[0]->addr, value->addr);
                        break;
                }
            }
            break;
        case AugAssign_kind:
            break;
        case Print_kind:
            break;
        case For_kind:
            break;
        case While_kind:
            break;
        case If_kind:
            break;
        case With_kind:
            break;
        case Raise_kind:
            break;
        case Try_kind:
            break;
        case Assert_kind:
            break;
        case Global_kind:
            break;
        case Expr_kind:
            assign_type_to_expr(s->exprstmt.value);
            break;
    }
}


static void
assign_type_to_expr(expr_ty e) {
    int i;
    switch(e->kind) {
        case BoolOp_kind:
            break;
        case BinOp_kind:
            assign_type_to_expr(e->binop.left);
            assign_type_to_expr(e->binop.right); 
            { 
                expr_ty l = e->binop.left;
                expr_ty r = e->binop.right;

                if(l->e_type->kind == LIST_KIND) {
                    e->e_type = create_list_type(l->list.elts[0]->e_type);
                }else if( l->e_type->kind = STRING_KIND) {
                    e->e_type = &t_string;
                }else {
                    e->e_type = max_type(l->e_type, r->e_type);
                }
            }
            break;
        case UnaryOp_kind:
            break;
        case Lambda_kind:
            break;
        case IfExp_kind:
            break;
        case Dict_kind:
            break;
        case Set_kind:
            break;
        case ListComp_kind:
            break;
        case SetComp_kind:
            break;
        case DictComp_kind:
            break;
        case GeneratorExp_kind:
            break;
        case Yield_kind:
            break;
        case Compare_kind:
            break;
        case Call_kind:
            break;
        case Repr_kind:
            break;
        case Num_kind:
            switch(e->num.kind) {
                case INTEGER:
                    e->e_type = &t_integer;
                    sprintf(e->addr, "%d", e->num.ivalue);
                    break;
                case DECIMAL:
                    e->e_type = &t_float;
                    sprintf(e->addr, "%f", e->num.fvalue);
                    break;
            }
            break;
        case Str_kind:
            e->e_type = &t_string;
            break;
        case Attribute_kind:
            break;
        case Subscript_kind:
            break;
        case Name_kind:
            {
                type_ty tp = search_type_for_name(e->name.id);
                if(tp == NULL) {
                    e->e_type = &t_unknown;
                }else {
                    e->e_type = tp;
                }
                strcpy(e->addr, e->name.id);
            }
            break;
        case List_kind:
            assign_type_to_expr(e->list.elts[0]);
            e->e_type = create_list_type(e->list.elts[0]->e_type);
            strcpy(e->addr, newTemp());
            printf("vector<%s> %s;\n", e->list.elts[0]->e_type == &t_integer?"int":"float", e->addr);
            printf("%s.push_back(%s)\n", e->addr, e->list.elts[0]->addr);
            for(i = 1; i < e->list.n_elt;  i ++ ) {
                assign_type_to_expr(e->list.elts[i]);
                printf("%s.push_back(%s)\n", e->addr, e->list.elts[i]->addr);
            }
            break;
        case Tuple_kind:
            break;
    }
}


void
assign_type_to_ast(stmt_seq* ss) {
    int i = 0;
    for(; i < ss->size; i ++ ) {
        assign_type_to_stmt(ss->seqs[i]);
    }
}

