#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mod.h"
#include "Python-ast.h"


stmt_ty
stmt_new() {
    stmt_ty s;
    s = (stmt_ty) malloc (sizeof( struct _stmt ) );
    if( NULL == s ) {
        fprintf(stderr, "Memor Error\n");
        exit(-1);
    }
    return s;
}


stmt_ty
stmt_new_full(enum stmt_kind kind, int lineno, int col_offset) {
    stmt_ty s = stmt_new();
    s->kind = kind;
    s->lineno = lineno;
    s->col_offset;
    return s;
}


expr_ty
expr_new() {
    expr_ty e = NULL;
    if( ( e = (expr_ty) malloc ( sizeof( struct _expr) )) == NULL) {
        fprintf(stderr, "Memory Error\n");
        exit(-1);
    }
    return e;
}

expr_ty
expr_new_full(enum expr_kind kind, int lineno, int col_offset) {
    expr_ty e = expr_new();
    e->kind = kind;
    e->lineno = lineno;
    e->col_offset;
    return e;
}

stmt_ty 
Assign_stmt( int n_target, expr_ty* targets, expr_ty value, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Assign_kind, lineno, col_offset);
    s->assign.n_target = n_target;
    s->assign.targets = targets;
    s->assign.value = value;
    return s;
}


stmt_ty
Return_stmt(expr_ty value, int lineno, int col_offset) {
    stmt_ty s= stmt_new_full(Return_kind, lineno, col_offset);
    s->ret.value = value;
    return s;
}

stmt_ty
Augassign_stmt(expr_ty target, expr_ty value, operator_ty op, int lineno, int col_offset) {
    stmt_ty s =stmt_new_full(AugAssign_kind, lineno, col_offset);
    s->augassignstmt.target = target;
    s->augassignstmt.value = value;
    s->augassignstmt.op = op;
    return s;
}

stmt_ty
Expr_stmt(expr_ty value, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Expr_kind, lineno, col_offset);
    s->exprstmt.value = value;
}

stmt_ty
Print_stmt(expr_ty dest, int n_value, expr_ty* values, int newline_mark, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Print_kind, lineno, col_offset);
    s->print.dest = dest;
    s->print.n_value = n_value;
    s->print.values = values;
    s->print.newline_mark = newline_mark;
    return s;
}


stmt_ty
Del_stmt(int n_target, expr_ty* targets, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Delete_kind, lineno, col_offset);
    s->del.n_target = n_target;
    s->del.targets = targets;
    return s;
}


stmt_ty
Raise_stmt(expr_ty type, expr_ty inst, expr_ty tback, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Raise_kind, lineno, col_offset);
    s->raise.type = type;
    s->raise.inst = inst;
    s->raise.tback = tback;
    return s;
}

stmt_ty
Global_stmt(int n_name, expr_ty* names, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Global_kind, lineno, col_offset);
    s->global.n_name = n_name;
    s->global.names = names;
    return s;
}

stmt_ty
Assert_stmt(expr_ty test, expr_ty msg, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Assert_kind, lineno, col_offset);
    s->assert.test = test;
    s->assert.msg = msg;
    return s;
}



stmt_ty
For_stmt(expr_ty target, expr_ty iter, stmt_seq* body, stmt_seq* orelse, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(For_kind, lineno, col_offset);
    s->forstmt.target = target;
    s->forstmt.iter = iter;
    s->forstmt.body = body;
    s->forstmt.orelse = orelse;
    return s;
}


stmt_ty
Import_stmt(int n_module, char** modules, char** names, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Import_kind, lineno, col_offset);
    s->import.n_module = n_module;
    s->import.modules = modules;
    s->import.names = names;
    return s;
}



stmt_ty
Importfrom_stmt(char* from, int all, int n_module, char** modules, char** names, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(ImportFrom_kind, lineno, col_offset);
    s->importfrom.from = strdup(from);
    s->importfrom.all = all;
    s->importfrom.n_module = n_module;
    s->importfrom.modules = modules;
    s->importfrom.names = names;
    return s;
}


stmt_ty
While_stmt(expr_ty test, stmt_seq* body, stmt_seq* orelse, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(While_kind, lineno, col_offset);
    s->whilestmt.test = test;
    s->whilestmt.body = body;
    s->whilestmt.orelse = orelse;
    return s;
}


stmt_ty
With_stmt(expr_ty context_expr, expr_ty optional_vars, stmt_seq* body, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(With_kind, lineno, col_offset);
    s->with.context_expr = context_expr;
    s->with.optional_vars = optional_vars;
    s->with.body = body;
    return s;
}


stmt_ty
Try_stmt(stmt_seq* body, int n_handler, exception_handler_ty* handlers, stmt_seq* orelse, stmt_seq* final, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(Try_kind, lineno, col_offset);
    s->trystmt.body = body;
    s->trystmt.n_handler = n_handler;
    s->trystmt.handlers = handlers;
    s->trystmt.orelse = orelse;
    s->trystmt.final = final;
    return s;

}

stmt_ty
If_stmt(expr_ty test, stmt_seq* body, stmt_seq* orelse, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(If_kind, lineno, col_offset);
    s->ifstmt.test = test;
    s->ifstmt.body = body;
    s->ifstmt.orelse = orelse;
    return s;
}


stmt_ty
Class_stmt(char* name, expr_ty super, stmt_seq* body, int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(ClassDef_kind, lineno, col_offset);
    strcpy(s->classdef.name , name);
    s->classdef.super = super;
    s->classdef.body = body;
    return s;
}


stmt_ty
Func_stmt(char* name, arguments_ty args, stmt_seq* body , int lineno, int col_offset) {
    stmt_ty s = stmt_new_full(FuncDef_kind, lineno, col_offset);
    strcpy(s->funcdef.name, name) ;
    s->funcdef.args = args;
    s->funcdef.body = body;
    return s;
}



expr_ty
Tuple_expr(int n_elt, expr_ty * elts, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Tuple_kind, lineno, col_offset);
    e->tuple.n_elt = n_elt;
    e->tuple.elts = elts;
    return e;
}



expr_ty
Ifexp_expr(expr_ty test, expr_ty body, expr_ty orelse, int lineno, int col_offset) {
    expr_ty e = expr_new_full(IfExp_kind, lineno, col_offset);
    e->ifexp.test = test;
    e->ifexp.body = body;
    e->ifexp.orelse = orelse;
    return e;
}

expr_ty
Listcomp_expr(expr_ty elt, int n_com, comprehension_ty* generators, int lineno, int col_offset) {
    expr_ty e = expr_new_full(ListComp_kind, lineno, col_offset);
    e->listcomp.elt = elt;
    e->listcomp.n_com = n_com;
    e->listcomp.generators = generators;
    return e;
}

expr_ty
Dictcomp_expr(expr_ty key, expr_ty value, int n_com, comprehension_ty * generators, int lineno, int col_offset) {
    expr_ty e = expr_new_full(DictComp_kind, lineno, col_offset);
    e->dictcomp.key = key;
    e->dictcomp.value = value;
    e->dictcomp.n_com = n_com;
    e->dictcomp.generators = generators;
    return e;
}


expr_ty
Lambda_expr(arguments_ty args, expr_ty body, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Lambda_kind, lineno, col_offset);
    e->lambda.args = args;
    e->lambda.body = body;
    return e;
}


expr_ty
Boolop_expr(boolop_ty op, int n_value, expr_ty* values, int lineno, int col_offset) {
    expr_ty e = expr_new_full(BoolOp_kind, lineno, col_offset);
    e->boolop.op = op;
    e->boolop.values = values;
    e->boolop.n_value = n_value;
    return e;
}


expr_ty
Unaryop_expr(unaryop_ty op, expr_ty operand, int lineno, int col_offset) {
    expr_ty e = expr_new_full(UnaryOp_kind, lineno, col_offset);
    e->unaryop.op = op;
    e->unaryop.operand = operand;
    return e;
}


expr_ty
Yield_expr(expr_ty value, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Yield_kind, lineno, col_offset);
    e->yield.value = value;
    return e;
}

expr_ty
Compare_expr(expr_ty left, int n_comparator, compop_ty* ops, expr_ty* comparators, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Compare_kind, lineno, col_offset);
    e->compare.left = left;
    e->compare.n_comparator = n_comparator;
    e->compare.ops = ops;
    e->compare.comparators = comparators;
    return e;
}


expr_ty
Binop_expr(expr_ty left, operator_ty op, expr_ty right, int lineno, int col_offset) {
    expr_ty e = expr_new_full(BinOp_kind, lineno, col_offset);
    e->binop.left= left;
    e->binop.op = op;
    e->binop.right = right;
    return e;
}


expr_ty
List_expr(int n_elt, expr_ty * elts, int lineno, int col_offset) {
    expr_ty e = expr_new_full(List_kind, lineno, col_offset);
    e->list.n_elt = n_elt;
    e->list.elts =elts;
    return e;
}


expr_ty
Dict_expr(int n_key, expr_ty* keys, expr_ty* values, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Dict_kind, lineno, col_offset);
    e->dict.n_key = n_key;
    e->dict.keys = keys;
    e->dict.values = values;
    return e;
}

expr_ty
Set_expr(int n_elt, expr_ty* elts, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Set_kind, lineno, col_offset);
    e->set.n_elt = n_elt;
    e->set.elts = elts;
    return e;
}


expr_ty
Attribute_expr(expr_ty value, char* attr, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Attribute_kind, lineno, col_offset);
    e->attribute.value = value;
    strcpy(e->attribute.attr, attr);
    return e;
}

expr_ty
Subscript_expr(expr_ty value, int n_slice, slice_ty * slices, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Subscript_kind, lineno, col_offset);
    e->sub.value = value;
    e->sub.n_slice = n_slice;
    e->sub.slices = slices;
    return e;
}

expr_ty
Call_expr(expr_ty func, int n_arg, Parameter* args, expr_ty varg, expr_ty karg, int lineno, int col_offset) {
    expr_ty e = expr_new_full(Call_kind, lineno, col_offset);
    e->call.func = func;
    e->call.n_arg = n_arg;
    e->call.args = args;
    e->call.varg = varg;
    e->call.karg = karg;
    return e;
}



stmt_seq *
stmt_seq_new(int n ) {
    if( n <= 0 ) {
        fprintf(stderr, "number of statement is less than 0\n");
        return NULL;
    }

    stmt_seq* ss = NULL;
    if( (ss = (stmt_seq* ) malloc( sizeof(stmt_seq) )) == NULL) {
        fprintf(stderr, "Memory Fail\n");
        exit(-1);
    }
    
    ss->size = n;
    
    if(( ss->seqs = (stmt_ty*)  malloc (sizeof( stmt_ty) * ss->size) ) == NULL) {
        fprintf(stderr, "Memory Fail\n");
        exit(-1);
    }

    return ss;
}

void
ast_stmt_set(stmt_seq* ss, stmt_ty s, int i) {
    if( i < 0 || i > ss->size) {
        fprintf(stderr, "Index error when set stmt seq\n");
        exit(-1);
    }
    ss->seqs[i] = s;
}

