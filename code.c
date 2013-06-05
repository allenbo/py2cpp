#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "code.h"
#include "symtab.h"
#include "context.h"
#include "util.h"
FILE* fout = NULL;

char* get_binop_literal(operator_ty op) {
    switch(op) {
        case Add: return "__add__";
        case Sub: return "__sub__";
        case Mult: return "__mul__";
        case Div:  return "__div__";
        case Mod:  return "__mod__";
        case Pow: return "__pow__";
        case LShift: return "__lshift__";
        case RShift: return "__rshift__";
        case BitOr:  return "__or__";
        case BitXor:return "__xor__";
        case BitAnd:  return "__and__";
    }
}
char* get_augop_literal(operator_ty op) {
    switch(op) {
        case Add: return "__iadd__";
        case Sub: return "__isub__";
        case Mult: return "__imul__";
        case Div:  return "__idiv__";
        case Mod:  return "__imod__";
        case Pow: return "__ipow__";
        case LShift: return "__ilshift__";
        case RShift: return "__irshift__";
        case BitOr:  return "__ior__";
        case BitXor:return "__ixor__";
        case BitAnd:  return "__iand__";
    }
}

char* get_cmpop_literal(compop_ty op) {
    switch(op) {
        case Eq: return "__eq__";
        case NotEq: return "__ne__";
        case Lt: return "__lt__";
        case LtE: return "__le__";
        case Gt: return "__gt__";
        case GtE: return "__ge__";
        case Is: return "==";
        case IsNot: return "!=";
        case In: return "__contains__";
        case NotIn: return "__contains__";
    }
}

char* get_unaryop_literal(unaryop_ty op) {
    switch(op) {
        case Invert: return "__invert__";
        case UAdd: return "__pos__";
        case USub: return "__neg__";
        case Not: return "!";
    }
}

char* get_boolop_literal(boolop_ty op) {
    switch(op) {
        case And: return "&&";
        case Or: return "||";
    }
}

char* new_temp() {
    static int ind = 0;
    char* name = (char*) malloc (sizeof(char)*12);
    sprintf(name, "_t%d", ind++);
    return name;
}


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

    gen_cpp_for_ast(ss, get_current_symtab());

    fclose(fout);
}


void
gen_cpp_for_ast(stmt_seq* ss, symtab_ty s) {
    /* first we need to output the variables */
    output_symtab(fout, s);

    int i, n = ss->size;
    for(i = 0; i < n; i ++ ) {
        gen_cpp_for_stmt(ss->seqs[i]);
    }
}

static void gen_cpp_for_stmt(stmt_ty s){
    switch(s->kind) {
        case Break_kind:
            fprintf(fout, "break;\n");
            break;
        case Continue_kind:
            fprintf(fout, "continue;\n");
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

static void
gen_cpp_for_classdef_stmt(stmt_ty s){
}

static void
gen_cpp_for_return_stmt(stmt_ty s){
    expr_ty value = s->ret.value;
    annotate_for_expr(value);

    char buf[512] = "";
    sprintf(buf, "return %s;\n", value->ann);
    fprintf(fout, "%s", buf);
}

static void
gen_cpp_for_delete_stmt(stmt_ty s){
}

static void
gen_cpp_for_assign_stmt(stmt_ty s){
    int i, n = s->assign.n_target;
    expr_ty value = s->assign.value;
    expr_ty * targets = s->assign.targets;

    char buf[512] = "";
    annotate_for_expr(value);
    for(i = 0; i < n ; i ++){
        if(targets[i]->kind == Subscript_kind) {
            (get_context())->setitem = 1;
            (get_context())->itemname = value->ann;
        }
        annotate_for_expr(targets[i]);
        switch(targets[i]->kind) {
            case Tuple_kind:
                break;
            case Name_kind:
                sprintf(buf, "%s = %s;\n", targets[i]->ann, value->ann);
                fprintf(fout, "%s", buf);
                break;
            case Subscript_kind:
                 sprintf(buf, "%s;\n", targets[i]->ann);
                 fprintf(fout, "%s", buf);
                break;
        }
    }
    return;
}

static void
gen_cpp_for_augassign_stmt(stmt_ty s){
    expr_ty target = s->augassignstmt.target;
    operator_ty op = s->augassignstmt.op;
    expr_ty value = s->augassignstmt.value;

    annotate_for_expr(value);
    annotate_for_expr(target);
    char* lop = get_augop_literal(op);

    char buf[512] = "";
    sprintf(buf, "%s->%s(%s);\n", target->ann, lop, value->ann);
    fprintf(fout, "%s", buf);
}

static void
gen_cpp_for_print_stmt(stmt_ty s){
    expr_ty dest = s->print.dest;
    expr_ty* values = s->print.values;
    int i, n = s->print.n_value;

    char buf[512] = "";
    sprintf(buf, "print(");
    if(dest) {
        annotate_for_expr(dest);
        strcat(buf, dest->ann);
    }else {
        strcat(buf, "NULL");
    }
    strcat(buf, ", ");
    sprintf(buf + strlen(buf), "%d, %d, ", s->print.newline_mark, n);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(values[i]);
        strcat(buf, values[i]->ann);
        if(i != n-1)
            strcat(buf, ", ");
    }
    strcat(buf, ");\n");
    fprintf(fout, "%s", buf);
}

static void
gen_cpp_for_for_stmt(stmt_ty s){
}

static void
gen_cpp_for_while_stmt(stmt_ty s){
}

static void
gen_cpp_for_if_stmt(stmt_ty s){
}

static void
gen_cpp_for_with_stmt(stmt_ty s){
}

static void
gen_cpp_for_raise_stmt(stmt_ty s){
}

static void
gen_cpp_for_try_stmt(stmt_ty s){
}

static void
gen_cpp_for_assert_stmt(stmt_ty s){
}

static void
gen_cpp_for_global_stmt(stmt_ty s){
}

static void
gen_cpp_for_expr_stmt(stmt_ty s){
    annotate_for_expr(s->exprstmt.value);
    char buf[512] = "";
    sprintf(buf, "%s;\n", s->exprstmt.value->ann);
    fprintf(fout, "%s", buf);
}

static void
annotate_for_binop_expr(expr_ty e){
    expr_ty left = e->binop.left;
    expr_ty right = e->binop.right;
    operator_ty op = e->binop.op;

    annotate_for_expr(left);
    annotate_for_expr(right);

    char* fake = get_binop_literal(op);
    sprintf(e->ann, "(%s)->%s(%s)", left->ann, fake, right->ann);
}
static void
annotate_for_boolop_expr(expr_ty e){
    char* tmp = new_temp();
    strcpy(e->ann, tmp);

    expr_ty* values = e->boolop.values;
    int i, n = e->boolop.n_value;
    boolop_ty op = e->boolop.op;

    char buf[512] = "";
    sprintf(buf, "%s %s;\n", values[0]->e_type->name, tmp);
    fprintf(fout, "%s", buf);

    buf[0] = '\0';
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(values[i]);
        sprintf(buf + strlen(buf), "(%s = (%s))->__nonzero__()", tmp, values[i]->ann);
        if(i != n -1) {
            sprintf(buf + strlen(buf) , " %s ", get_boolop_literal(op));
        }
    }
    fprintf(fout, "%s;\n", buf);

}
static void
annotate_for_unaryop_expr(expr_ty e){
    expr_ty operand = e->unaryop.operand;
    unaryop_ty op = e->unaryop.op;

    char* fake = get_unaryop_literal(op);
    annotate_for_expr(operand);
    if(fake[0] == '!') {
        sprintf(e->ann, "!%s", operand->ann);
    }else {
        sprintf(e->ann, "(%s)->%s()", operand->ann, fake);
    }
}
static void
annotate_for_lambda_expr(expr_ty e){
}
static void
annotate_for_ifexp_expr(expr_ty e){
    expr_ty test = e->ifexp.test;
    expr_ty body = e->ifexp.body;
    expr_ty orelse = e->ifexp.orelse;

    annotate_for_expr(test);
    annotate_for_expr(body);
    annotate_for_expr(orelse);

    sprintf(e->ann, "%s ? %s : %s", test->ann, body->ann, orelse->ann);
}
static void
annotate_for_listcomp_expr(expr_ty e){
    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    char* ann = search_hashtable((get_context())->ht,  scope_name);
    strcpy(e->ann, ann);

    type_ty tp = lookup_variable(scope_name);

    char line[512] = "";
    sprintf(line, "{\n");
    fprintf(fout, "%s", line);


    output_symtab(fout, tp->scope);

    expr_ty elt = e->listcomp.elt;
    sprintf(line, "%s = List<%s>(0);\n", ann, elt->e_type->name);
    fprintf(fout, "%s", line);
    int i, n = e->listcomp.n_com;
    comprehension_ty* coms = e->listcomp.generators;

    annotate_for_expr(elt);


    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        annotate_for_expr(target);
        annotate_for_expr(iter);
        if(iter->kind != Name_kind) {
            char* tmp = newTemp();
            sprintf(line, "%s %s = %s;\n", iter->e_type->name, tmp, iter->ann);
            fprintf(fout, "%s", line);
            strcpy(iter->ann, tmp);
        }
        for(j = 0; j < m; j ++) {
            annotate_for_expr(tests[j]);
        }
    }

    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        sprintf(line, "for(; %s->has_next();) {\n", iter->ann);
        fprintf(fout, "%s", line);

        sprintf(line, "%s = %s->next();\n",  target->ann, iter->ann);
        fprintf(fout, "%s", line);

        for(j = 0; j < m; j++ ) {
            sprintf(line, "if (%s)\n", tests[j]->ann);
            fprintf(fout, "%s", line);
        }
    }

    sprintf(line, "%s->add(%s);\n", ann, elt->ann);
    fprintf(fout, "%s", line);
    for(i = 0; i < n ;i ++ ) {
        sprintf(line, "}\n");fprintf(fout, "%s", line);
    }

    sprintf(line, "}\n");fprintf(fout, "%s", line);

}
static void
annotate_for_dict_expr(expr_ty e){
    int i, n = e->dict.n_key;
    expr_ty* keys = e->dict.keys;
    expr_ty* values = e->dict.values;

    sprintf(e->ann, "Dict<%s, %s>( %d, ",
            e->e_type->kbase->name,e->e_type->vbase->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(values[i]);
        annotate_for_expr(keys[i]);

        strcat(e->ann, keys[i]->ann);
        strcat(e->ann, ", ");
        strcat(e->ann, values[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");

}
static void
annotate_for_set_expr(expr_ty e){
    int i, n = e->set.n_elt;
    expr_ty* elts = e->set.elts;

    sprintf(e->ann, "Set<%s>( %d, ", e->e_type->base->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
static void
annotate_for_dictcomp_expr(expr_ty e){

}
static void
annotate_for_setcomp_expr(expr_ty e){
    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    char* ann = search_hashtable((get_context())->ht,  scope_name);
    strcpy(e->ann, ann);

    type_ty tp = lookup_variable(scope_name);

    char line[512] = "";
    sprintf(line, "{\n");
    fprintf(fout, "%s", line);


    output_symtab(fout, tp->scope);

    expr_ty elt = e->setcomp.elt;
    sprintf(line, "%s = Set<%s>(0);\n", ann, elt->e_type->name);
    fprintf(fout, "%s", line);
    int i, n = e->setcomp.n_com;
    comprehension_ty* coms = e->setcomp.generators;

    annotate_for_expr(elt);


    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        annotate_for_expr(target);
        annotate_for_expr(iter);
        if(iter->kind != Name_kind) {
            char* tmp = newTemp();
            sprintf(line, "%s %s = %s;\n", iter->e_type->name, tmp, iter->ann);
            fprintf(fout, "%s", line);
            strcpy(iter->ann, tmp);
        }
        for(j = 0; j < m; j ++) {
            annotate_for_expr(tests[j]);
        }
    }

    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        sprintf(line, "for(; %s->has_next();) {\n", iter->ann);
        fprintf(fout, "%s", line);

        sprintf(line, "%s = %s->next();\n",  target->ann, iter->ann);
        fprintf(fout, "%s", line);

        for(j = 0; j < m; j++ ) {
            sprintf(line, "if (%s)\n", tests[j]->ann);
            fprintf(fout, "%s", line);
        }
    }

    sprintf(line, "%s->append(%s);\n", ann, elt->ann);
    fprintf(fout, "%s", line);
    for(i = 0; i < n ;i ++ ) {
        sprintf(line, "}\n");fprintf(fout, "%s", line);
    }

    sprintf(line, "}\n");fprintf(fout, "%s", line);

}
static void
annotate_for_generator_expr(expr_ty e){
}
static void
annotate_for_yield_expr(expr_ty e){
}
static void
annotate_for_compare_expr(expr_ty e){
    expr_ty left = e->compare.left;
    expr_ty* comps = e->compare.comparators;
    int i, n = e->compare.n_comparator;
    compop_ty* ops = e->compare.ops;

    char* op;
    char* prev;
    char* p;
    annotate_for_expr(left);

    for(i = 0; i < n; i ++ ) {
        op = get_cmpop_literal(ops[i]);
        annotate_for_expr(comps[i]);
        if(i == 0) {
            prev = left->ann;
            p = comps[i]->ann;
        }else {
            prev = comps[i-1]->ann;
            p = comps[i]->ann;
        }
        char tmp[128];

        if(ops[i] == In) {
            sprintf(tmp, "(%s)->%s(%s)", p, op, prev);
        }else if(ops[i] == NotIn) {
            sprintf(tmp, "!(%s)->%s(%s)", p, op, prev);
            sprintf(tmp, "!(%s)->%s(%s)", p, op, prev);
        }else if(ops[i] == Is || ops[i] == IsNot) {
            sprintf(tmp, "!(%s)%s(%s)", p, op, prev);
        }else
            sprintf(tmp, "(%s)->%s(%s)", prev, op, p);
        strcat(e->ann, tmp);

        if(i != n-1) {
            strcat(e->ann, " && ");
        }
    }
}
static void
annotate_for_call_expr(expr_ty e){
    expr_ty func = e->call.func;
    Parameter * args = e->call.args;
    int i, n = e->call.n_arg;
    expr_ty varg = e->call.varg;
    expr_ty karg = e->call.karg;

    annotate_for_expr(func);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(args[i].args);
    }

    if(func->e_type->kind != FUNCTION_KIND &&
            func->e_type->kind != LAMBDA_KIND) {
        sprintf(e->ann, "DLambda(%s)(", func->ann);
    }else {
        sprintf(e->ann, "%s(", func->ann);
    }

    for(i = 0;i < n; i ++ ) {
        strcat(e->ann, args[i].args->ann);
        if(i != n-1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, ")");
}
static void
annotate_for_repr_expr(expr_ty e){
}
static void
annotate_for_num_expr(expr_ty e){
    if(e->num.kind == INTEGER) {
        sprintf(e->ann, "Int(%d)", e->num.ivalue);
    }else {
        sprintf(e->ann, "Int(%f)", e->num.fvalue);
    }
}
static void
annotate_for_str_expr(expr_ty e){
    sprintf(e->ann, "Str(\"%s\")", e->str.s);
}
static void
annotate_for_attribute_expr(expr_ty e){
    annotate_for_expr(e->attribute.value);
    sprintf(e->ann, "%s->%s", e->attribute.value->ann, e->attribute.attr);
}
static void
annotate_for_subscript_expr(expr_ty e){
    expr_ty value = e->sub.value;
    int i, n = e->sub.n_slice;
    slice_ty * slices = e->sub.slices;

    if((get_context())->setitem) {
        (get_context())->setitem = 0;
        annotate_for_expr(value);
        /* Some case there are multiple subscript in python
         * but we don't consider it here
         * So what we are gonna do is we're gonna take n equals to 1
         */
        slice_ty slice = slices[0];
        if(slice->kind == Index_kind) {
            annotate_for_expr(slice->index.value);
            sprintf(e->ann, "(%s)->__setitem__( %s , %s)", value->ann,
                    slice->index.value->ann, (get_context())->itemname);
        }else if(slice->kind == Slice_kind) {
            char* lower = "NULL";
            char* upper = "NULL";
            char* step = "NULL";

            expr_ty l = slice->slice.lower;
            expr_ty u = slice->slice.upper;
            expr_ty s = slice->slice.step;

            if(NULL != l ) {
                annotate_for_expr(l);
                lower = l->ann;
            }
            if(NULL != u ) {
                annotate_for_expr(u);
                upper = u->ann;
            }

            sprintf(e->ann, "(%s)->__setslice__( %s , %s, %s)", value->ann,
                    lower, upper, (get_context())->itemname);
        }
    }else {
        annotate_for_expr(value);
        slice_ty slice = slices[0];
        if(slice->kind == Index_kind) {
            annotate_for_expr(slice->index.value);
            sprintf(e->ann, "(%s)->__getitem__( %s )", value->ann,
                    slice->index.value->ann);
        }else if(slice->kind == Slice_kind) {
            char* lower = "NULL";
            char* upper = "NULL";
            char* step = "NULL";

            expr_ty l = slice->slice.lower;
            expr_ty u = slice->slice.upper;
            expr_ty s = slice->slice.step;

            if(NULL != l ) {
                annotate_for_expr(l);
                lower = l->ann;
            }
            if(NULL != u ) {
                annotate_for_expr(u);
                upper = u->ann;
            }

            sprintf(e->ann, "(%s)->__getslice__( %s , %s)", value->ann,
                    lower, upper);
        }
    }
}
static void
annotate_for_name_expr(expr_ty e){
    strcpy(e->ann, e->name.id);
}
static void
annotate_for_list_expr(expr_ty e){
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
static void
annotate_for_tuple_expr(expr_ty e){
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
