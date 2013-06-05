#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Python-ast.h"
#include "context.h"

static char* newTemp() {
    static int i = 0;
    char * tmp = (char* ) malloc( sizeof(char) * 10);
    sprintf(tmp, "_t%d", i++);
    return tmp;
}

static char* newIterator() {
    static int i = 0;
    char* tmp = (char*) malloc( sizeof(char) * 4);
    sprintf(tmp, "i%d", i ++ );
    return tmp;
}

struct type t_unknown  = {UNKNOWN_KIND,  "VOID"};
struct type t_char = {CHAR_KIND, "CHAR"};
struct type t_boolean = {BOOLEAN_KIND, "BOOL"};
struct type t_integer = {INTEGER_KIND, "PINT"};
struct type t_float = {FLOAT_KIND, "PFLOAT"};
struct type t_string = {STRING_KIND,  "PSTR"};

static type_ty assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);

static type_ty assign_type_to_funcdef_stmt(stmt_ty s);
static type_ty assign_type_to_classdef_stmt(stmt_ty s);
static type_ty assign_type_to_return_stmt(stmt_ty s);
static type_ty assign_type_to_delete_stmt(stmt_ty s);
static type_ty assign_type_to_assign_stmt(stmt_ty s);
static type_ty assign_type_to_augassign_stmt(stmt_ty s);
static type_ty assign_type_to_print_stmt(stmt_ty s);
static type_ty assign_type_to_for_stmt(stmt_ty s);
static type_ty assign_type_to_while_stmt(stmt_ty s);
static type_ty assign_type_to_if_stmt(stmt_ty s);
static type_ty assign_type_to_with_stmt(stmt_ty s);
static type_ty assign_type_to_raise_stmt(stmt_ty s);
static type_ty assign_type_to_try_stmt(stmt_ty s);
static type_ty assign_type_to_assert_stmt(stmt_ty s);
static type_ty assign_type_to_global_stmt(stmt_ty s);
static type_ty assign_type_to_expr_stmt(stmt_ty s);


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

static void assign_type_to_comprehension(comprehension_ty com);

static void assemble_installation(expr_ty e, enum symtab_entry_kind kind);
static void push_type_to_arguments(arguments_ty args,
        type_ty ctype, Parameter* param, int n);



type_ty
create_list_type(int n, type_ty t) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "PLIST");
    tp->kind = LIST_KIND;
    tp->base = t;
    return tp;
}


type_ty
create_set_type(int n, type_ty t) {
    type_ty tp = (type_ty) malloc (sizeof(struct type));
    strcpy(tp->name, "PSET");
    tp->kind = SET_KIND;
    tp->base = t;
    return tp;
}


type_ty
create_generator_type(type_ty t) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "PGENERATOR");
    tp->kind = GENERATOR_KIND;
    tp->base = t;
    return tp;
}


type_ty
create_tuple_type(int n) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "PTUPLE");
    tp->kind = TUPLE_KIND;
    tp->n_elt = n;

    tp->elts = (type_ty*) malloc (sizeof(type_ty) * n);
    return tp;
}

int
tuple_set_type(type_ty tuple, int i, type_ty t) {
    if(tuple == NULL || tuple->elts == NULL || tuple->n_elt <= i) {
        return 0;
    }
    tuple->elts[i] = t;
}

type_ty
create_dict_type(int n, type_ty kbase, type_ty vbase) {
    type_ty tp = (type_ty) malloc (sizeof(struct type));
    sprintf(tp->name, "PDICT");
    tp->kind = DICT_KIND;
    tp->kbase = kbase;
    tp->vbase = vbase;
    return tp;
}

type_ty
create_func_type(stmt_ty s) {
    type_ty tp = (type_ty) malloc (sizeof(struct type));
    strcpy(tp->name, "PFUNC");
    tp->kind = FUNCTION_KIND;
    tp->def = s;
    return tp;
}

static int
type_compare(type_ty t1, type_ty t2) {
    if(t1 == t2) return 0;
    if(t1->kind == t2->kind) return 0;
    if(t1->kind == INTEGER_KIND) return -1;
    return 1;
}


static type_ty
widen_type(type_ty t1, type_ty t2) {
    if(t1 == &t_unknown ) return t2;
    if(t2 == &t_unknown) return t1;
    if(t1 != &t_integer && t1 != &t_float) return t1;
    if(t2 != &t_integer && t2 != &t_float) return t2;
    if(t1 == t2) return t1;
    if(t1 == &t_float || t2 == &t_float) return &t_float;
}

static type_ty
narrow_type(type_ty t1, type_ty t2) {
    if(t1 == t2) return t1;
    if(t1 == &t_unknown) return t1;
    if(t2 == &t_unknown) return t2;
}

static type_ty
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
            return assign_type_to_global_stmt(s);
        case Expr_kind:
            return assign_type_to_expr_stmt(s);
    }
}

static type_ty
assign_type_to_funcdef_stmt(stmt_ty s){
    char* name = s->funcdef.name;
    type_ty tp = create_func_type(s);

    if((get_context())->inclass == 1)
        tp->ctype = (get_context())->ctype;

    install_variable(name, tp, SE_FUNCTION_KIND);
    return &t_unknown;
}

static type_ty
assign_type_to_classdef_stmt(stmt_ty s) {
    type_ty tp = (type_ty) malloc (sizeof(struct type));
    tp->kind = CLASS_KIND;
    char* name = s->classdef.name;
    sprintf(tp->name, "%s*", name);

    /* function defined in the class will get
     * some information of the class
     */
    (get_context())->inclass = 1;
    (get_context())->classname = name;
    (get_context())->ctype = tp;

    install_scope_variable(name, tp, SE_CLASS_KIND);
    assign_type_to_ast(s->classdef.body);

    if(!has_constructor()) {
        type_ty p = create_func_type(NULL);
        p->ctype = tp;
        install_variable("__init__", p, SE_FUNCTION_KIND);
    }

    change_symtab_back();
    (get_context())->inclass = 0;
    return &t_unknown;
}

static type_ty
assign_type_to_return_stmt(stmt_ty s) {
    assign_type_to_expr(s->ret.value);
    return s->ret.value->e_type;
}

static type_ty
assign_type_to_delete_stmt(stmt_ty s) {
    int i, n = s->del.n_target;
    expr_ty * targets = s->del.targets;

    for(i = 0; i < n; i ++ ){
        assign_type_to_expr(targets[i]);
    }
    return &t_unknown;
}

static type_ty
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
            assemble_installation(targets[i], SE_VARIABLE_KIND);
        }else if(targets[i]->e_type == NULL) {
            /* This variable must be the global variable, when it's inserted
             * into symbol table, we haven't know its type
             * So the type is empty
             * Now we can insert the type for variable
             */
            assert(targets[i]->kind == Name_kind);
            targets[i]->e_type = value->e_type;
            change_type(targets[i]->name.id, targets[i]->e_type);
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
                assemble_installation(targets[i], SE_REUSE_KIND);
            }
        }
    }
    return &t_unknown;
}

static type_ty
assign_type_to_augassign_stmt(stmt_ty s){
    expr_ty target = s->augassignstmt.target;
    expr_ty value = s->augassignstmt.value;

    assign_type_to_expr(target);
    assign_type_to_expr(value);
    return &t_unknown;
}

static type_ty
assign_type_to_print_stmt(stmt_ty s){
    expr_ty dest = s->print.dest;
    expr_ty * values = s->print.values;
    int i, n = s->print.n_value;

    if(dest)
        assign_type_to_expr(dest);

    for(i = 0; i < n; i ++ ) {
        assign_type_to_expr(values[i]);
    }
    return &t_unknown;
}


static type_ty
assign_type_to_for_stmt(stmt_ty s){
    expr_ty target = s->forstmt.target;
    expr_ty iter = s->forstmt.iter;

    assign_type_to_expr(iter);
    assign_type_to_expr(target);

    type_ty t = NULL;
    if(iter->e_type->kind == DICT_KIND)
        t = iter->e_type->kbase;
    else if( iter->e_type->kind == LIST_KIND || iter->e_type->kind == SET_KIND
            || iter->e_type->kind == GENERATOR_KIND)
        t = iter->e_type->base;
    else if( iter->e_type->kind == TUPLE_KIND)
        /* for tuple kind, I use the first element's type
         * Because we don't generate multitype list, the elements in
         * tuple should have the same type
         */
        t = iter->e_type->elts[0];
    else {
        /* Here is for generator and iterable object */
    }

    if(target->e_type == &t_unknown) {
        target->e_type = t;
        assemble_installation(target, SE_VARIABLE_KIND);
    }else if( type_compare(target->e_type, t) != 0) {
        if(target->dable == 1) {
            target->e_type = t;
            assemble_installation(target, SE_REUSE_KIND);
        }
    }

    type_ty tp = &t_unknown;
    tp = assign_type_to_ast(s->forstmt.body);
    if(s->forstmt.orelse) {
        type_ty t;
        if((t = assign_type_to_ast(s->forstmt.orelse)) != &t_unknown && tp == &t_unknown)
            tp = t;
    }
    return tp;
}


static type_ty
assign_type_to_while_stmt(stmt_ty s){
    expr_ty test = s->whilestmt.test;
    assign_type_to_expr(test);

    type_ty tp = &t_unknown;
    tp = assign_type_to_ast(s->whilestmt.body);
    if(s->whilestmt.orelse) {
        type_ty t;
        if((t = assign_type_to_ast(s->whilestmt.orelse)) != &t_unknown && tp == &t_unknown)
            tp = t;
    }
    return tp;
}

static type_ty
assign_type_to_if_stmt(stmt_ty s){
    expr_ty test = s->ifstmt.test;
    assign_type_to_expr(test);

    type_ty tp = &t_unknown;
    tp = assign_type_to_ast(s->ifstmt.body);
    if(s->ifstmt.orelse) {
        type_ty t;
        if((t = assign_type_to_ast(s->ifstmt.orelse)) != &t_unknown && tp != &t_unknown)
            tp = t;
    }
    return tp;
}
static type_ty
assign_type_to_with_stmt(stmt_ty s){
    expr_ty expr = s->with.context_expr;
    expr_ty vars = s->with.optional_vars;

    stmt_seq* body = s->with.body;

    assign_type_to_expr(expr);
    if(vars != NULL) {
        assign_type_to_expr(vars);

        if(vars->e_type == &t_unknown) {
            vars->e_type = expr->e_type;
            assemble_installation(vars, SE_VARIABLE_KIND);
        }else if( type_compare(vars->e_type, expr->e_type) != 0) {
            if(vars->dable == 1) {
                vars->e_type = expr->e_type;
                assemble_installation(vars, SE_REUSE_KIND);
            }
        }
    }

    assign_type_to_ast(body);

    return &t_unknown;
}

static type_ty
assign_type_to_raise_stmt(stmt_ty s){
    if(s->raise.type != NULL)
        assign_type_to_expr(s->raise.type);
    if(s->raise.inst != NULL)
        assign_type_to_expr(s->raise.inst);
    if(s->raise.tback != NULL)
        assign_type_to_expr(s->raise.tback);
    return &t_unknown;
}

static type_ty
assign_type_to_try_stmt(stmt_ty s){
    type_ty tp = NULL;

    tp = assign_type_to_ast(s->trystmt.body);
    int i, n = s->trystmt.n_handler;
    exception_handler_ty* handlers = s->trystmt.handlers;

    for(i = 0; i < n; i ++ ) {
        type_ty tmp;
        assign_type_to_expr(handlers[i]->type);

        /* install the variable of value */
        if(handlers[i]->value) {
            handlers[i]->value->e_type = handlers[i]->type->e_type;
            assemble_installation(handlers[i]->value, SE_VARIABLE_KIND);
        }
        tmp = assign_type_to_ast(handlers[i]->body);
        if(tmp != &t_unknown && tp == &t_unknown)
            tp = tmp;
    }

    if(s->trystmt.orelse != NULL) {
        type_ty tmp;
        tmp = assign_type_to_ast(s->trystmt.orelse);
        if(tmp != &t_unknown && tp == &t_unknown)
            tp = tmp;
    }
    if(s->trystmt.final != NULL) {
        type_ty tmp;
        tmp = assign_type_to_ast(s->trystmt.final);
        if(tmp != &t_unknown && tp == &t_unknown)
            tp = tmp;
    }
    return tp;
}

static type_ty
assign_type_to_assert_stmt(stmt_ty s){
    assign_type_to_expr(s->assert.test);
    if(s->assert.msg != NULL)
        assign_type_to_expr(s->assert.msg);
    return &t_unknown;
}

static type_ty
assign_type_to_global_stmt(stmt_ty s){
    int i, n = s->global.n_name;
    expr_ty* names = s->global.names;
    change_symtab(get_curfile_symtab());

    for(i = 0; i < n; i ++ ) {
        assert(names[i]->kind == Name_kind);
        type_ty tp = NULL;
        tp = lookup_scope_variable(names[i]->name.id);

        /* hasn't declared , and we still don't know the type of the variable*/
        if(tp == &t_unknown) {
            assemble_installation(names[i], SE_VARIABLE_KIND);
        }
    }
    change_symtab_back();
    return &t_unknown;
}

static type_ty
assign_type_to_expr_stmt(stmt_ty s){
    assign_type_to_expr(s->exprstmt.value);
    return &t_unknown;
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
    int i, n = e->boolop.n_value;
    expr_ty * values = e->boolop.values;
    for(i = 0; i < n; i ++ ) {
        assign_type_to_expr(values[i]);
    }
    e->e_type = values[0]->e_type;
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
        else e->e_type = widen_type(left->e_type, right->e_type);
    }
}

static void
assign_type_to_unaryop_expr(expr_ty e){
    unaryop_ty op = e->unaryop.op;
    expr_ty operand = e->unaryop.operand;

    assign_type_to_expr(operand);
    if(op == Invert) {
        assert(operand->e_type == &t_integer);
        e->e_type = &t_integer;
    }else if(op == UAdd || op == USub) {
        e->e_type = operand->e_type;
    }else {
        e->e_type = &t_boolean;
    }
}

static void
assign_type_to_lambda_expr(expr_ty e){
    arguments_ty args = e->lambda.args;
    expr_ty body = e->lambda.body;
}

static void
assign_type_to_ifexp_expr(expr_ty e){
    expr_ty test = e->ifexp.test;
    expr_ty body = e->ifexp.body;
    expr_ty orelse = e->ifexp.orelse;

    assign_type_to_expr(test);
    assign_type_to_expr(body);
    assign_type_to_expr(orelse);

    e->e_type = narrow_type(body->e_type, orelse->e_type);
}

static void
assign_type_to_listcomp_expr(expr_ty e){
    expr_ty elt = e->listcomp.elt;
    int i, n = e->listcomp.n_com;
    comprehension_ty* gens = e->listcomp.generators;

    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    type_ty tp = (type_ty) malloc ( sizeof ( struct type) );
    tp->kind = SCOPE_KIND;
    install_scope_variable(scope_name, tp, SE_SCOPE_KIND);

    for(i = 0; i < n; i ++ ) {
        assign_type_to_comprehension(gens[i]);
    }

    /* Now identifier in elt will appear in symbol table */
    assign_type_to_expr(elt);
    change_symtab_back(); /* change to current table */

    e->e_type = create_list_type(0, elt->e_type); /* Cannot figure out the number of elements */

    char* compname = newTemp();
    insert_hashtable((get_context())->ht, scope_name, compname);
    install_variable(compname, e->e_type, SE_VARIABLE_KIND);
}

static void
assign_type_to_dict_expr(expr_ty e){
    int i, n = e->dict.n_key;
    expr_ty* keys = e->dict.keys;
    expr_ty* values = e->dict.values;

    for(i = 0; i < n ; i ++) {
        assign_type_to_expr(keys[i]);
        assign_type_to_expr(values[i]);
    }

    e->e_type = create_dict_type(n, keys[0]->e_type, values[0]->e_type);
}

static void
assign_type_to_set_expr(expr_ty e){
    int i, n = e->set.n_elt;
    expr_ty * elts = e->set.elts;

    for(i = 0; i < n; i ++) {
        assign_type_to_expr(elts[i]);
    }
    e->e_type = create_set_type(n, elts[0]->e_type);
}

static void
assign_type_to_dictcomp_expr(expr_ty e){
    expr_ty key = e->dictcomp.key;
    expr_ty value = e->dictcomp.value;

    int i, n = e->dictcomp.n_com;
    comprehension_ty * gens = e->dictcomp.generators;

    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    type_ty tp = (type_ty) malloc ( sizeof ( struct type) );
    tp->kind = SCOPE_KIND;
    install_scope_variable(scope_name, tp, SE_SCOPE_KIND);

    for(i = 0; i < n; i ++ ) {
        assign_type_to_comprehension(gens[i]);
    }

    assign_type_to_expr(key);
    assign_type_to_expr(value);

    change_symtab_back();

    e->e_type = create_dict_type(0, key->e_type, value->e_type);
}

static void
assign_type_to_setcomp_expr(expr_ty e){
    expr_ty elt = e->setcomp.elt;
    int i, n = e->setcomp.n_com;
    comprehension_ty* gens = e->setcomp.generators;

    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    type_ty tp = (type_ty) malloc ( sizeof ( struct type) );
    tp->kind = SCOPE_KIND;
    install_scope_variable(scope_name, tp, SE_SCOPE_KIND);

    for(i = 0; i < n; i ++ ) {
        assign_type_to_comprehension(gens[i]);
    }

    /* Now identifier in elt will appear in symbol table */
    assign_type_to_expr(elt);
    change_symtab_back(); /* change to current table */

    e->e_type = create_set_type(0, elt->e_type); /* Cannot figure out the number of elements */
}

static void
assign_type_to_generator_expr(expr_ty e){
    expr_ty elt = e->generatorexp.elt;
    int i, n = e->generatorexp.n_com;
    comprehension_ty* gens = e->generatorexp.generators;

    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    type_ty tp = (type_ty) malloc ( sizeof ( struct type) );
    tp->kind = SCOPE_KIND;
    install_scope_variable(scope_name, tp, SE_SCOPE_KIND);

    for(i = 0; i < n; i ++ ) {
        assign_type_to_comprehension(gens[i]);
    }

    /* Now identifier in elt will appear in symbol table */
    assign_type_to_expr(elt);
    change_symtab_back(); /* change to current table */

    e->e_type = create_generator_type(elt->e_type); /* Cannot figure out the number of elements */

}

static void
assign_type_to_yield_expr(expr_ty e){
    assign_type_to_expr(e->yield.value);
    e->e_type = create_generator_type(e->yield.value->e_type);
}

static void
assign_type_to_compare_expr(expr_ty e){
    expr_ty left = e->compare.left;
    int i, n = e->compare.n_comparator;
    expr_ty* coms = e->compare.comparators;

    assign_type_to_expr(left);
    for(i = 0; i < n; i ++ )
        assign_type_to_expr(coms[i]);
    e->e_type = &t_boolean;
}
static void
assign_type_to_repr_expr(expr_ty e){
}
static void
assign_type_to_call_expr(expr_ty e){

    expr_ty func = e->call.func;
    int i, n = e->call.n_arg;
    Parameter* args = e->call.args;
    expr_ty varg = e->call.varg;
    expr_ty karg = e->call.karg;

    assign_type_to_expr(func);

    if(func->e_type->kind == CLASS_KIND) {
        type_ty tmp = func->e_type;

        change_symtab(func->e_type->scope);
        func->e_type = lookup_scope_variable("__init__");
        change_symtab_back();

        if(func->e_type->def == NULL) {
            e->e_type = tmp;
            return;
        }
    }


    char name[128] = "";
    for(i = 0 ; i < n; i ++ ) {
        expr_ty ar = args[i].args;
        assign_type_to_expr(ar);
        strcat(name, ar->e_type->name);
    }
    type_ty ret = functable_lookup(name, func->e_type);
    if(NULL == ret) {
        functable_insert(name, n, args, func->e_type);
        stmt_ty s = func->e_type->def;

        /* push the type to arguments and insert them to the table */
        push_type_to_arguments(s->funcdef.args, func->e_type->ctype, args, n);

        arguments_ty args = s->funcdef.args;

        int n = args->n_param;
        for(i = 0; i < n; i ++) {
            assemble_installation(args->params[i]->args, SE_PARAMETER_KIND);
        }

        n = args->n_default;
        for(i = 0; i < n; i ++ ) {
            assemble_installation(args->default_params[i]->args, SE_PARAMETER_KIND);
        }
        if(NULL != args->vargs) {
            assemble_installation(args->vargs, SE_PARAMETER_KIND);
        }
        if(NULL != args->kargs) {
            assemble_installation(args->kargs, SE_PARAMETER_KIND);
        }


        ret = assign_type_to_ast(func->e_type->def->funcdef.body);
        functable_insert_ret(name, ret, func->e_type);

        change_symtab_back();
    }
    e->e_type = ret;
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
    expr_ty value = e->attribute.value;
    char* attr = e->attribute.attr;

    assign_type_to_expr(value);
    if(value->e_type == &t_unknown) {
        fprintf(stderr, "The type of %s is noknown\n", attr);
        e->e_type = &t_unknown;
    }else {
        change_symtab(value->e_type->scope);
        e->e_type = lookup_scope_variable(attr);
        change_symtab_back();
    }
}
static void
assign_type_to_subscript_expr(expr_ty e){
    expr_ty value = e->sub.value;
    int i, n = e->sub.n_slice;
    slice_ty * slices = e->sub.slices;

    assign_type_to_expr(value);
    /* for now , I just deal with the case n_slice = 1 */
    for(i = 0; i < n; i ++ ) {
        slice_ty s = slices[i];
        if(s->kind == Index_kind) {
            expr_ty v = s->index.value;
            assign_type_to_expr(v);
            if(value->e_type->kind == LIST_KIND
                    || value->e_type->kind == SET_KIND) {
                e->e_type = value->e_type->base;
            }else if(value->e_type->kind == DICT_KIND) {
                e->e_type = value->e_type->vbase;
            }else if(value->e_type->kind == TUPLE_KIND) {
                if(v->kind == Num_kind) {
                    int iv = v->num.ivalue;
                    e->e_type = value->e_type->elts[iv];
                }else {
                    e->e_type = &t_unknown;
                }
            }
        }
        else {
            expr_ty lower = s->slice.lower;
            expr_ty upper = s->slice.upper;
            expr_ty step = s->slice.step;

            if(NULL != lower) assign_type_to_expr(lower);
            if(NULL != upper) assign_type_to_expr(upper);
            if(NULL != step) assign_type_to_expr(step);

            if(value->e_type->kind == LIST_KIND ||
                    value->e_type->kind == SET_KIND) {
                e->e_type = value->e_type->base;
            }else if(value->e_type->kind == DICT_KIND) {
                e->e_type = value->e_type->vbase;
            }else if(value->e_type->kind = TUPLE_KIND) {
                int l = 0, u = 0, t = 1;
                if(NULL != lower)
                    if(lower->kind == Num_kind)
                        l = lower->num.ivalue;
                    else
                        l = -1;
                if(NULL != upper)
                    if(upper->kind == Num_kind)
                        u = upper->num.ivalue;
                    else
                        u = -2;
                if(u == -1) u == value->e_type->n_elt;

                if(NULL != step)
                    if(step->kind == Num_kind)
                        t = step->num.ivalue;
                    else
                        t = -1;
                if(l < 0 || u < 0 || t < 0) {
                    e->e_type = create_tuple_type(1);
                    e->e_type->elts[0] = &t_unknown;
                }else {
                    int j;
                    e->e_type = create_tuple_type((u-l)/t);
                    for(j = l; j < u; j += t) {
                        tuple_set_type(e->e_type, j, value->e_type->elts[j]);
                    }
                }
            }
        }
    }
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

    expr_ty t = NULL;
    for(i = 0; i < n; i ++ ) {
        t = e->list.elts[i];
        assign_type_to_expr(t);
    }
    e->e_type = create_list_type(n, t->e_type);
}
static void
assign_type_to_tuple_expr(expr_ty e){
    type_ty tp = NULL;
    int i, n = e->tuple.n_elt;
    e->e_type = create_tuple_type(n);
    expr_ty t = NULL;
    for(i = 0; i < n; i ++ ) {
        t = e->tuple.elts[i];
        assign_type_to_expr(t);
        if(t->dable == 1)
            e->dable = 1;
        if(t->e_type == &t_unknown) {
            free(e->e_type);
            e->e_type = &t_unknown;
            break;
        }
        tuple_set_type(e->e_type, i, t->e_type);
    }
}


static void
assign_type_to_comprehension(comprehension_ty com) {
    expr_ty iter = com->iter;
    expr_ty target = com->target;

    assign_type_to_expr(iter);

    if(iter->e_type->kind == DICT_KIND)
        target->e_type = iter->e_type->kbase;
    else if( iter->e_type->kind == LIST_KIND)
        target->e_type = iter->e_type->base;
    else if( iter->e_type->kind == TUPLE_KIND)
        /* for tuple kind, I use the first element's type
         * Because we don't generate multitype list, the elements in
         * tuple should have the same type
         */
        target->e_type = iter->e_type->elts[0];
    else {
        /* Here is for generator and iterable object */
    }

    assemble_installation(target, SE_VARIABLE_KIND);

    int i;
    for(i = 0; i < com->n_test; i ++ ) {
        assign_type_to_expr(com->tests[i]);
    }
}

static void
assemble_installation(expr_ty e, enum symtab_entry_kind kind) {
    if(e->kind == Name_kind && strcmp(e->name.id , "_") != 0) {
        install_variable(e->name.id, e->e_type, kind);
    }else if(e->kind == Tuple_kind){
        int i, n = e->tuple.n_elt;
        expr_ty * elts = e->tuple.elts;
        for(i = 0; i < n; i ++ ) {
            elts[i]->e_type = e->e_type->elts[i];
            assemble_installation(elts[i], SE_VARIABLE_KIND);
        }
    }
}


static void
push_type_to_arguments(arguments_ty args,
        type_ty ctype, Parameter* params, int n) {
    /* deal with position index and no default arguments */
    int i, from;
    if(ctype != NULL) {
        from = 1;
        args->params[0]->args->e_type = ctype;
    }
    else
        from = 0;
    for(i = 0; i < n; i ++) {
        args->params[i + from]->args->e_type = params[i].args->e_type;
    }

}


type_ty
assign_type_to_ast(stmt_seq* ss) {
    int i = 0;
    type_ty tp = &t_unknown;
    type_ty t;
    for(; i < ss->size; i ++ ) {
        stmt_ty s = ss->seqs[i];
        t = assign_type_to_stmt(s);

        if(t != &t_unknown && tp == &t_unknown)
            tp = t;
    }
    return tp;
}
