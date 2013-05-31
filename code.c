#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "code.h"
#include "symtab.h"
#include "context.h"

FILE* fout = NULL;

static void gen_cpp_for_ast(stmt_seq* ss);

static void gen_cpp_for_stmt(stmt_ty s);
static void gen_cpp_for_funcdef_stmt(stmt_ty s);
static void gen_cpp_for_classdef_stmt(stmt_ty s);
static void gen_cpp_for_return_stmt(stmt_ty s);
static void gen_cpp_for_delete_stmt(stmt_ty s);
static void gen_cpp_for_assign_stmt(stmt_ty s);
static void gen_cpp_for_augassign_stmt(stmt_ty s);
static void gen_cpp_for_print_stmt(stmt_ty s);
static void gen_cpp_for_for_stmt(stmt_ty s);
static void gen_cpp_for_while_stmt(stmt_ty s);
static void gen_cpp_for_if_stmt(stmt_ty s);
static void gen_cpp_for_with_stmt(stmt_ty s);
static void gen_cpp_for_raise_stmt(stmt_ty s);
static void gen_cpp_for_try_stmt(stmt_ty s);
static void gen_cpp_for_assert_stmt(stmt_ty s);
static void gen_cpp_for_global_stmt(stmt_ty s);
static void gen_cpp_for_expr_stmt(stmt_ty s);


static void annotate_for_expr(expr_ty e);
static void annotate_for_binop_expr(expr_ty e);
static void annotate_for_boolop_expr(expr_ty e);
static void annotate_for_unaryop_expr(expr_ty e);
static void annotate_for_lambda_expr(expr_ty e);
static void annotate_for_ifexp_expr(expr_ty e);
static void annotate_for_listcomp_expr(expr_ty e);
static void annotate_for_dict_expr(expr_ty e);
static void annotate_for_set_expr(expr_ty e);
static void annotate_for_dictcomp_expr(expr_ty e);
static void annotate_for_setcomp_expr(expr_ty e);
static void annotate_for_generator_expr(expr_ty e);
static void annotate_for_yield_expr(expr_ty e);
static void annotate_for_compare_expr(expr_ty e);
static void annotate_for_call_expr(expr_ty e);
static void annotate_for_repr_expr(expr_ty e);
static void annotate_for_num_expr(expr_ty e);
static void annotate_for_str_expr(expr_ty e);
static void annotate_for_attribute_expr(expr_ty e);
static void annotate_for_subscript_expr(expr_ty e);
static void annotate_for_name_expr(expr_ty e);
static void annotate_for_list_expr(expr_ty e);
static void annotate_for_tuple_expr(expr_ty e);


void
generate_cpp_code(char* filename, stmt_seq* ss) {
    if(ss == NULL || ss->size == 0) {
        fprintf(stderr, "stmt seq is not ready\n");
        return ;
    }

    if(NULL == filename){
        filename = "test.cpp";
        fprintf(stderr, "Haven't spectify the output file, Set the filename to test.cpp\n");
    }

    if((fout = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Can't open file -- %s\n", filename);
    }

    gen_cpp_for_ast(ss);

    fclose(fout);
}


static void
gen_cpp_for_ast(stmt_seq* ss) {
    /* first we need to output the variables */
    output_symtab(fout, get_current_symtab());

    int i, n = ss->size;
    for(i = 0; i < n; i ++ ) {
        gen_cpp_for_stmt(ss->seqs[i]);
    }
}

static void gen_cpp_for_stmt(stmt_ty s){
    switch(s->kind) {
        case Break_kind:
            break;
        case Continue_kind:
            break;
        case FuncDef_kind:
            return gen_cpp_for_funcdef_stmt(s);
        case ClassDef_kind:
            return gen_cpp_for_classdef_stmt(s);
        case Return_kind:
            return gen_cpp_for_return_stmt(s);
        case Delete_kind:
            return gen_cpp_for_delete_stmt(s);
        case Assign_kind:
            return gen_cpp_for_assign_stmt(s);
        case AugAssign_kind:
            return gen_cpp_for_augassign_stmt(s);
        case Print_kind:
            return gen_cpp_for_print_stmt(s);
        case For_kind:
            return gen_cpp_for_for_stmt(s);
        case While_kind:
            return gen_cpp_for_while_stmt(s);
        case If_kind:
            return gen_cpp_for_if_stmt(s);
        case With_kind:
            return gen_cpp_for_with_stmt(s);
        case Raise_kind:
            return gen_cpp_for_raise_stmt(s);
        case Try_kind:
            return gen_cpp_for_try_stmt(s);
        case Assert_kind:
            return gen_cpp_for_assert_stmt(s);
        case Global_kind:
            return gen_cpp_for_global_stmt(s);
        case Expr_kind:
            return gen_cpp_for_expr_stmt(s);
    }
}

static void annotate_for_expr(expr_ty e){
    switch(e->kind) {
        case BoolOp_kind:
            return annotate_for_boolop_expr(e);
        case BinOp_kind:
            return annotate_for_binop_expr(e);
        case UnaryOp_kind:
            return annotate_for_unaryop_expr(e);
        case Lambda_kind:
            return annotate_for_lambda_expr(e);
        case IfExp_kind:
            return annotate_for_ifexp_expr(e);
        case ListComp_kind:
            return annotate_for_listcomp_expr(e);
        case Dict_kind:
            return annotate_for_dict_expr(e);
        case Set_kind:
            return annotate_for_set_expr(e);
        case SetComp_kind:
            return annotate_for_setcomp_expr(e);
        case DictComp_kind:
            return annotate_for_dictcomp_expr(e);
        case GeneratorExp_kind:
            return annotate_for_generator_expr(e);
        case Yield_kind:
            return annotate_for_yield_expr(e);
        case Compare_kind:
            return annotate_for_compare_expr(e);
        case Call_kind:
            return annotate_for_call_expr(e);
        case Repr_kind:
            return annotate_for_repr_expr(e);
        case Num_kind:
            return annotate_for_num_expr(e);
        case Str_kind:
            return annotate_for_str_expr(e);
        case Attribute_kind:
            return annotate_for_attribute_expr(e);
        case Subscript_kind:
            return annotate_for_subscript_expr(e);
        case Name_kind:
            return annotate_for_name_expr(e);
        case List_kind:
            return annotate_for_list_expr(e);
        case Tuple_kind:
            return annotate_for_tuple_expr(e);
    }
}
static void
gen_cpp_for_funcdef_stmt(stmt_ty s){
}

static void gen_cpp_for_classdef_stmt(stmt_ty s){
}

static void gen_cpp_for_return_stmt(stmt_ty s){
}

static void gen_cpp_for_delete_stmt(stmt_ty s){
}

static void gen_cpp_for_assign_stmt(stmt_ty s){
    int i, n = s->assign.n_target;
    expr_ty value = s->assign.value;
    expr_ty * targets = s->assign.targets;

    char buf[512] = "";
    annotate_for_expr(value);
    for(i = 0; i < n ; i ++){
        annotate_for_expr(targets[i]);
        switch(targets[i]->kind) {
            case Tuple_kind:
                break;
            case Name_kind:
                sprintf(buf, "%s = %s;\n", targets[i]->ann, value->ann);
                fprintf(fout, "%s", buf);
                break;
        }
    }
    return;
}

static void gen_cpp_for_augassign_stmt(stmt_ty s){
}

static void gen_cpp_for_print_stmt(stmt_ty s){
}

static void gen_cpp_for_for_stmt(stmt_ty s){
}

static void gen_cpp_for_while_stmt(stmt_ty s){
}

static void gen_cpp_for_if_stmt(stmt_ty s){
}

static void gen_cpp_for_with_stmt(stmt_ty s){
}

static void gen_cpp_for_raise_stmt(stmt_ty s){
}

static void gen_cpp_for_try_stmt(stmt_ty s){
}

static void gen_cpp_for_assert_stmt(stmt_ty s){
}

static void gen_cpp_for_global_stmt(stmt_ty s){
}

static void gen_cpp_for_expr_stmt(stmt_ty s){
}

static void annotate_for_binop_expr(expr_ty e){
}
static void annotate_for_boolop_expr(expr_ty e){
}
static void annotate_for_unaryop_expr(expr_ty e){
}
static void annotate_for_lambda_expr(expr_ty e){
}
static void annotate_for_ifexp_expr(expr_ty e){
}
static void annotate_for_listcomp_expr(expr_ty e){
}
static void annotate_for_dict_expr(expr_ty e){
}
static void annotate_for_set_expr(expr_ty e){
}
static void annotate_for_dictcomp_expr(expr_ty e){
}
static void annotate_for_setcomp_expr(expr_ty e){
}
static void annotate_for_generator_expr(expr_ty e){
}
static void annotate_for_yield_expr(expr_ty e){
}
static void annotate_for_compare_expr(expr_ty e){
}
static void annotate_for_call_expr(expr_ty e){

}
static void annotate_for_repr_expr(expr_ty e){
}
static void annotate_for_num_expr(expr_ty e){
    if(e->num.kind == INTEGER) {
        sprintf(e->ann, "Int(%d)", e->num.ivalue);
    }else {
        sprintf(e->ann, "Int(%f)", e->num.fvalue);
    }
}
static void annotate_for_str_expr(expr_ty e){
    sprintf(e->ann, "Str(\"%s\")", e->str.s);
}
static void annotate_for_attribute_expr(expr_ty e){
    annotate_for_expr(e->attribute.value);
    sprintf(e->ann, "%s->%s", e->attribute.value->ann, e->attribute.attr);
}
static void annotate_for_subscript_expr(expr_ty e){
}
static void annotate_for_name_expr(expr_ty e){
    strcpy(e->ann, e->name.id);
}
static void annotate_for_list_expr(expr_ty e){
    int i, n = e->list.n_elt;
    expr_ty* elts = e->list.elts;

    sprintf(e->ann, "List<%s>( %d, ", e->e_type->base->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
static void annotate_for_tuple_expr(expr_ty e){
    int i, n = e->tuple.n_elt;
    expr_ty * elts = e->tuple.elts;

    sprintf(e->ann, "Tuple( %d, ", n);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
