#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Python-ast.h"

char* newTemp() {
    static int i = 0;
    char * tmp = (char* ) malloc( sizeof(char) * 10);
    sprintf(tmp, "_t%d", i++);
    return tmp;
}
char* newIterator() {
    static int i = 0;
    char* tmp = (char*) malloc( sizeof(char) * 4);
    sprintf(tmp, "i%d", i ++ );
    return tmp;
}

int level = 1;

static type_ty func_ret = NULL;
static FILE* fdef[1000];
int fd = 0;

void push_fd(FILE* f) {
    fdef[fd++] = f;
}
FILE* pop_fd() {
    return fdef[--fd];
}

FILE* output = NULL;


struct type t_unknown  = {UNKNOWN_KIND,  "VOID"};
struct type t_char = {CHAR_KIND, "CHAR"};
struct type t_boolean = {BOOLEAN_KIND, "BOOL"};
struct type t_integer = {INTEGER_KIND, "INT"};
struct type t_float = {FLOAT_KIND, "FLOAT"};
struct type t_string = {STRING_KIND,  "STR"};

static void assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);

static void assign_type_to_funcdef_stmt(stmt_ty s);
static void assign_type_to_classdef_stmt(stmt_ty s);
static void assign_type_to_return_stmt(stmt_ty s);
static void assign_type_to_delete_stmt(stmt_ty s);
static void assign_type_to_assign_stmt(stmt_ty s);
static void assign_type_to_augassign_stmt(stmt_ty s);
static void assign_type_to_print_stmt(stmt_ty s);
static void assign_type_to_for_stmt(stmt_ty s);
static void assign_type_to_while_stmt(stmt_ty s);
static void assign_type_to_if_stmt(stmt_ty s);
static void assign_type_to_with_stmt(stmt_ty s);
static void assign_type_to_raise_stmt(stmt_ty s);
static void assign_type_to_try_stmt(stmt_ty s);
static void assign_type_to_assert_stmt(stmt_ty s);
static void assign_type_to_global_stmt(stmt_ty s);
static void assign_type_to_expr_stmt(stmt_ty s);


static void assign_type_to_binop_expr(expr_ty e);
static void assign_type_to_boolop_expr(expr_ty e);
static void assign_type_to_unaryop_expr(expr_ty e);
static void assign_type_to_lambda_expr(expr_ty e);
static void assign_type_to_ifexp_expr(expr_ty e);
static void assign_type_to_listcomp_expr(expr_ty e);
static void assign_type_to_dict_expr(expr_ty e);
static void assign_type_to_set_expr(expr_ty e);
static void assign_type_to_dictcomp_expr(expr_ty e);
static void assign_type_to_setcomp_expr(expr_ty e);
static void assign_type_to_generator_expr(expr_ty e);
static void assign_type_to_yield_expr(expr_ty e);
static void assign_type_to_compare_expr(expr_ty e);
static void assign_type_to_call_expr(expr_ty e);
static void assign_type_to_repr_expr(expr_ty e);
static void assign_type_to_num_expr(expr_ty e);
static void assign_type_to_str_expr(expr_ty e);
static void assign_type_to_attribute_expr(expr_ty e);
static void assign_type_to_subscript_expr(expr_ty e);
static void assign_type_to_name_expr(expr_ty e);
static void assign_type_to_list_expr(expr_ty e);
static void assign_type_to_tuple_expr(expr_ty e);



void indent_output() {
    int i ;

    for(i= 0 ; i < level; i ++ ) {
        fprintf(output, "\t");
    }
}


type_ty
create_list_type(int n) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "LIST");
    tp->kind = LIST_KIND;
    tp->n_elt = n;

    tp->elts = (type_ty*) malloc (sizeof(type_ty) * n);
    return tp;
}

int
list_set_type(type_ty list, int i, type_ty t) {
    if(list == NULL || list->elts == NULL || list->n_elt <= i) {
        return 0;
    }
    list->elts[i] = t;
}

char* get_op_literal(operator_ty op) {
    switch(op) {
        case Add: return "+";
        case Sub: return "-";
        case Mult: return "*";
        case Div:  return "/";
        case Mod:  return "\%";
        case LShift: return "<<";
        case RShift: return ">>";
        case BitOr:  return "|";
        case BitXor:return "^";
        case BitAnd:  return "&";
    }
}


char* get_cmp_literal(compop_ty op) {
    switch(op) {
        case Eq: return "==";
        case NotEq: return "!=";
        case Lt: return "<";
        case LtE: return "<=";
        case Gt: return ">";
        case GtE: return ">=";
    }
}

char* get_unaryop_literal(unaryop_ty op) {
    switch(op) {
        case Invert: return "~";
        case UAdd: return "+";
        case USub: return "-";
        case Not: return "!";
    }
}

static int
type_compare(type_ty t1, type_ty t2) {
    if(t1 == t2) return 0;
    if(t1->kind == t2->kind) return 0;
    if(t1->kind == INTEGER_KIND) return -1;
    return 1;
}

static type_ty
max_type(type_ty t1, type_ty t2) {
    if(t1 == &t_unknown ) return t2;
    if(t2 == &t_unknown) return t1;
    if(t1 != &t_integer && t1 != &t_float) return t1;
    if(t2 != &t_integer && t2 != &t_float) return t2;
    if(t1 == t2) return t1;
    if(t1 == &t_float || t2 == &t_float) return &t_float;
}

static void
assign_type_to_stmt(stmt_ty s) {
    int i;
    switch(s->kind) {
        case Break_kind:
            break;
        case Continue_kind:
            break;
        case FuncDef_kind:
            return assign_type_to_funcdef_stmt(s);
        case ClassDef_kind:
            return assign_type_to_classdef_stmt(s);
        case Return_kind:
            return assign_type_to_return_stmt(s);
        case Delete_kind:
            return assign_type_to_delete_stmt(s);
        case Assign_kind:
            return assign_type_to_assign_stmt(s);
        case AugAssign_kind:
            return assign_type_to_augassign_stmt(s);
        case Print_kind:
            return assign_type_to_print_stmt(s);
        case For_kind:
            return assign_type_to_for_stmt(s);
        case While_kind:
            return assign_type_to_while_stmt(s);
        case If_kind:
            return assign_type_to_if_stmt(s);
        case With_kind:
            return assign_type_to_with_stmt(s);
        case Raise_kind:
            return assign_type_to_raise_stmt(s);
        case Try_kind:
            return assign_type_to_try_stmt(s);
        case Assert_kind:
            return assign_type_to_assert_stmt(s);
        case Global_kind:
            return assign_type_to_assert_stmt(s);
        case Expr_kind:
            return assign_type_to_expr_stmt(s);
    }
}

static void
assign_type_to_funcdef_stmt(stmt_ty s){
}

static void
assign_type_to_classdef_stmt(stmt_ty s) {
}

static void
assign_type_to_return_stmt(stmt_ty s) {
}

static void
assign_type_to_delete_stmt(stmt_ty s) {
}

static void
assign_type_to_assign_stmt(stmt_ty s) {
    expr_ty value = s->assign.value;
    int n = s->assign.n_target;
    expr_ty * targets = s->assign.targets;

    assign_type_to_expr(value);

    int i;
    for(i = 0; i < n; i ++ ) {
        assign_type_to_expr(targets[i]);
        if(targets[i]->e_type == &t_unknown) {
            /* The the variable is not in the symbol table
             * which means we has to declare the variable
             */
            targets[i]->e_type = value->e_type;
            install_variable(targets[i]);
        }else if(type_compare(targets[i]->e_type, value->e_type) != 0){
            /* Sometimes types of value and target are not the same
             * Like:
             *   x = 1      <== type of x is int
             *   x = 'str'  <== type of x is int and type of 'str' is str
             * In this case, we have to declare x aggin, but not use the same
             * name of x, so I attach a flag to x, when it's declared, it
             * should look like PSTR sx = Str('str');
             */
            if(targets[i]->dable == 1) {
                targets[i]->e_type = value->e_type;
                install_variable_full(targets[i], SE_REUSE_KIND);
            }
        }
    }
}

static void
assign_type_to_augassign_stmt(stmt_ty s){
}
static void
assign_type_to_print_stmt(stmt_ty s){
}
static void
assign_type_to_for_stmt(stmt_ty s){
}
static void
assign_type_to_while_stmt(stmt_ty s){
}
static void
assign_type_to_if_stmt(stmt_ty s){
}
static void
assign_type_to_with_stmt(stmt_ty s){
}
static void
assign_type_to_raise_stmt(stmt_ty s){
}
static void
assign_type_to_try_stmt(stmt_ty s){
}
static void
assign_type_to_assert_stmt(stmt_ty s){
}
static void
assign_type_to_global_stmt(stmt_ty s){
}
static void
assign_type_to_expr_stmt(stmt_ty s){
}

static void
assign_type_to_expr(expr_ty e) {
    int i;
    switch(e->kind) {
        case BoolOp_kind:
            return assign_type_to_boolop_expr(e);
        case BinOp_kind:
            return assign_type_to_binop_expr(e);
        case UnaryOp_kind:
            return assign_type_to_unaryop_expr(e);
        case Lambda_kind:
            return assign_type_to_lambda_expr(e);
        case IfExp_kind:
            return assign_type_to_ifexp_expr(e);
        case ListComp_kind:
            return assign_type_to_listcomp_expr(e);
        case Dict_kind:
            return assign_type_to_dict_expr(e);
        case Set_kind:
            return assign_type_to_set_expr(e);
        case SetComp_kind:
            return assign_type_to_setcomp_expr(e);
        case DictComp_kind:
            return assign_type_to_dictcomp_expr(e);
        case GeneratorExp_kind:
            return assign_type_to_generator_expr(e);
        case Yield_kind:
            return assign_type_to_yield_expr(e);
        case Compare_kind:
            return assign_type_to_compare_expr(e);
        case Call_kind:
            return assign_type_to_call_expr(e);
        case Repr_kind:
            return assign_type_to_repr_expr(e);
        case Num_kind:
            return assign_type_to_num_expr(e);
        case Str_kind:
            return assign_type_to_str_expr(e);
        case Attribute_kind:
            return assign_type_to_attribute_expr(e);
        case Subscript_kind:
            return assign_type_to_subscript_expr(e);
        case Name_kind:
            return assign_type_to_name_expr(e);
        case List_kind:
            return assign_type_to_list_expr(e);
        case Tuple_kind:
            return assign_type_to_tuple_expr(e);
    }
}


static void
assign_type_to_boolop_expr(expr_ty e){
}
static void
assign_type_to_binop_expr(expr_ty e) {
    expr_ty left = e->binop.left;
    expr_ty right = e->binop.right;
    operator_ty op = e->binop.op;

    assign_type_to_expr(left);
    assign_type_to_expr(right);

    if(type_compare(left->e_type, right->e_type) == 0)
        e->e_type = left->e_type;
    else {
        if(op == Div) e->e_type = &t_float;
        else e->e_type = max_type(left->e_type, right->e_type);
    }
}
static void
assign_type_to_unaryop_expr(expr_ty e){
}
static void
assign_type_to_lambda_expr(expr_ty e){
}
static void
assign_type_to_ifexp_expr(expr_ty e){
}
static void
assign_type_to_listcomp_expr(expr_ty e){
}
static void
assign_type_to_dict_expr(expr_ty e){
}
static void
assign_type_to_set_expr(expr_ty e){
}
static void
assign_type_to_dictcomp_expr(expr_ty e){
}
static void
assign_type_to_setcomp_expr(expr_ty e){
}
static void
assign_type_to_generator_expr(expr_ty e){
}
static void
assign_type_to_yield_expr(expr_ty e){
}
static void
assign_type_to_compare_expr(expr_ty e){
}
static void
assign_type_to_call_expr(expr_ty e){
}
static void
assign_type_to_repr_expr(expr_ty e){
}

static void
assign_type_to_num_expr(expr_ty e){
    if(e->num.kind == INTEGER)
        e->e_type = &t_integer;
    else
        e->e_type = &t_float;
}

static void
assign_type_to_str_expr(expr_ty e){
    e->e_type = &t_string;
}
static void
assign_type_to_attribute_expr(expr_ty e){
}
static void
assign_type_to_subscript_expr(expr_ty e){
}
static void
assign_type_to_name_expr(expr_ty e){
    type_ty tp = NULL;
    tp = lookup_variable(e->name.id);
    e->e_type = tp;
    e->dable = 1;
}
static void
assign_type_to_list_expr(expr_ty e){
    type_ty tp = NULL;
    int i, n = e->list.n_elt;
    e->e_type = create_list_type(n);

    expr_ty t = NULL;
    for(i = 0; i < n; i ++ ) {
        t = e->list.elts[i];
        assign_type_to_expr(t);
        list_set_type(e->e_type, i, t->e_type);
    }
}
static void
assign_type_to_tuple_expr(expr_ty e){
}


static void
push_type_to_expr(expr_ty e) {
    if(e->kind == Tuple_kind) {
        if(e->e_type->kind == TUPLE_KIND) {

        }
    }
    else if(e->kind == Name_kind) {
        //insert_to_current_table(e->name.id, e->e_type, SE_TEMP);
        strcpy(e->addr, e->name.id);
    }
}

static void
assign_type_to_comprehension(comprehension_ty com) {
    expr_ty iter = com->iter;
    expr_ty target = com->target;

    int i;
    for(i = 0; i < com->n_test; i ++ ) {
        assign_type_to_expr(com->tests[i]);
    }
    assign_type_to_expr(iter);
/*
    if(iter->e_type->kind == LIST_KIND) {
        target->e_type = iter->e_type->base;
    }else if(iter->e_type->kind ==DICT_KIND) {
        target->e_type = iter->e_type->kbase;
    }
*/
    push_type_to_expr(target);
}



static void
push_type_to_arguments(stmt_ty st, expr_ty e) {
    int i;
    for(i = 0; i < e->call.n_arg; i ++ ) {
        st->funcdef.args->params[i]->args->e_type = e->call.args[i].args->e_type;
    }
}



void
assign_type_to_ast(stmt_seq* ss) {
    if(NULL == output) output = stdout;
    int i = 0;
    for(; i < ss->size; i ++ ) {
        assign_type_to_stmt(ss->seqs[i]);
    }
}
