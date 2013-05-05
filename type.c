#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* newTemp() {
    static int i = 0;
    char * tmp = (char* ) malloc( sizeof(char) * 10);
    sprintf(tmp, "_t%d", i++); return tmp; }
    char* newIterator() {
        static int i = 0;
        char* tmp = (char*) malloc( sizeof(char) * 4);
        sprintf(tmp, "i%d", i ++ );
        return tmp;
    }

int level = 1;

static type_ty func_ret = NULL;
extern FILE* fdef;
FILE* output = NULL;


struct type t_unknown  = {UNKNOWN_KIND, 0 , "void"};
struct type t_char = {CHAR_KIND, 1, "char"};
struct type t_boolean = {BOOLEAN_KIND, 1, "bool"};
struct type t_integer = {INTEGER_KIND, 4, "int"};
struct type t_float = {FLOAT_KIND, 4, "float"};
struct type t_string = {STRING_KIND, 0,  "string"};

static void assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);
static void assign_type_to_comprehension(comprehension_ty com);
static void push_type_to_expr(expr_ty e);
static void push_type_to_arguments(stmt_ty st, expr_ty s);
static int type_compare(type_ty t1, type_ty t2);
static type_ty max_type(type_ty t1, type_ty t2);
static void stmt_for_expr(expr_ty e);
static void eliminate_python_unique_for_expr(expr_ty e);
static void eliminate_python_unique_for_stmt(stmt_ty s);


void indent_output() {
    int i ;

    for(i= 0 ; i < level; i ++ ) {
        fprintf(output, "\t");
    }
}


static type_ty
create_list_type(type_ty t) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "vector< %s >", t->name);
    tp->kind = LIST_KIND;
    tp->base = t;
    return tp;
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
            indent_output();
            fprintf(output, "break;\n");
            break;
        case Continue_kind:
            indent_output();
            fprintf(output, "continue;\n");
            break;
        case FuncDef_kind:
            insert_incomplete_func_to_table(s->funcdef.name,s);
            break;
        case ClassDef_kind:
            break;
        case Return_kind:
            assign_type_to_expr(s->ret.value);
            if(s->ret.value->isplain)
                stmt_for_expr(s->ret.value);
            else
                eliminate_python_unique_for_expr(s->ret.value);
            indent_output();
            fprintf(output, "return %s;\n", s->ret.value->addr);
            func_ret = s->ret.value->e_type;
            strcpy(s->ret.value->addr, "");
            break;
        case Delete_kind:
            break;
        case Assign_kind:
            {
                int n_target = s->assign.n_target;
                expr_ty * targets = s->assign.targets;
                expr_ty value = s->assign.value;
                assign_type_to_expr(value);
                strcpy(value->addr, targets[0]->name.id);

                if(value->isplain)
                    stmt_for_expr(value);
                else
                    eliminate_python_unique_for_expr(value);

                insert_to_current_table(targets[0]->name.id,
                        value->e_type, SE_VARIABLE_KIND);
                int i ;
                for(i= 1; i < n_target; i ++ ){
                    assign_type_to_expr(targets[i]);
                    insert_to_current_table(targets[i]->name.id,
                            value->e_type, SE_VARIABLE_KIND);

                    indent_output();
                    switch(value->e_type->kind) {
                        case INTEGER_KIND:
                        case FLOAT_KIND:
                        case STRING_KIND:
                        case BOOLEAN_KIND:
                            fprintf(output, "%s %s = %s;\n", value->e_type->name, targets[i]->addr, value->addr);
                            break;
                        case LIST_KIND:
                            fprintf(output, "%s & %s = %s;\n", value->e_type->name,
                                    targets[i]->addr, value->addr);
                            break;
                    }
                }
            }
            break;
        case AugAssign_kind:
            {
                expr_ty value = s->augassignstmt.value;
                expr_ty target = s->augassignstmt.target;
                assign_type_to_expr(value);
                assign_type_to_expr(target);
                value->aug = 1;
                strcpy(value->addr, target->name.id);
                strcpy(value->augop, get_op_literal(s->augassignstmt.op));
                if(value->isplain)
                    stmt_for_expr(value);
                else
                    eliminate_python_unique_for_expr(value);
            }
            break;
        case Print_kind:
            {
                int i ;
                for(i = 0; i < s->print.n_value; i ++ ) {
                    assign_type_to_expr(s->print.values[i]);
                }
                char* dest = "cout";
                if(s->print.dest!= NULL) {
                    eliminate_python_unique_for_expr(s->print.dest);
                    dest = s->print.dest->addr;
                }

                for(i = 0; i < s->print.n_value; i ++ ) {
                    if(s->print.values[i]->isplain)
                        stmt_for_expr(s->print.values[i]);
                    else
                        eliminate_python_unique_for_expr(s->print.values[i]);
                }

                int fcout = 0;
                int end = 0;
                for(i = 0; i < s->print.n_value; i ++ ) {
                    if(s->print.values[i]->e_type->kind == LIST_KIND) {
                        if(end == 1)  printf(";\n");
                        indent_output();
                        fprintf(output, "%s << \"[\";\n", dest);
                        indent_output();
                        fprintf(output, "for_each(%s.begin(), %s.end() - 1, output<%s, %s>);\n",
                                s->print.values[i]->addr, s->print.values[i]->addr,
                                s->print.values[i]->e_type->base->name, dest);
                        indent_output();
                        fprintf(output, "%s << *(%s.end() -1);\n", dest, s->print.values[i]->addr);
                        indent_output();
                        fprintf(output, "%s << \"] \";\n", dest);
                        fcout = 0;
                        end = 1;
                    }
                    else {
                        if(fcout == 0 ) {
                            indent_output();
                            fprintf(output, "%s << %s << \" \"", dest, s->print.values[i]->addr);
                            fcout = 1;
                        }else {
                            fprintf(output, " << %s << \" \"", s->print.values[i]->addr);
                        }
                    }
                }
                if(fcout == 0) {
                    indent_output();
                    fprintf(output, "cout");
                }
                if(s->print.newline_mark == 1) {
                    fprintf(output, " << endl;\n");
                }else {
                    fprintf(output, ";\n");
                }

            }
            break;
        case For_kind:
            assign_type_to_expr(s->forstmt.target);
            assign_type_to_expr(s->forstmt.iter);
            s->forstmt.target->e_type = s->forstmt.iter->e_type->base;
            insert_to_current_table(s->forstmt.target->name.id,
                    s->forstmt.target->e_type, SE_VARIABLE_KIND);

            if(s->forstmt.target->isplain)
                stmt_for_expr(s->forstmt.target);
            else
                eliminate_python_unique_for_expr(s->forstmt.target);
            if(s->forstmt.iter->isplain)
                stmt_for_expr(s->forstmt.iter);
            else
                eliminate_python_unique_for_expr(s->forstmt.iter);


            char* iter = newIterator();
            indent_output();
            fprintf(output, "int %s = 0;\n", iter);
            indent_output();
            fprintf(output, "for(%s %s : %s) {\n", s->forstmt.target->e_type->name,
                    s->forstmt.target->addr, s->forstmt.iter->addr);
            level ++;
            assign_type_to_ast(s->forstmt.body);

            indent_output();
            fprintf(output, "%s ++;\n", iter);
            level --;
            indent_output();
            fprintf(output, "}\n");

            if(s->forstmt.orelse != NULL) {
                indent_output();
                fprintf(output, "if(%s == %s.size()) {\n", iter, s->forstmt.iter->addr);
                level++;
                assign_type_to_ast(s->forstmt.orelse);
                level--;
                indent_output();
                fprintf(output, "}\n");
            }
            break;
        case While_kind:
            {
                assign_type_to_expr(s->whilestmt.test);
                if(s->whilestmt.test->isplain)
                    stmt_for_expr(s->whilestmt.test);
                else
                    eliminate_python_unique_for_expr(s->whilestmt.test);

                char* iter = newIterator();
                indent_output();
                fprintf(output, "int %s = 1;\n", iter);
                indent_output();
                fprintf(output, "while( %s ) {\n", s->whilestmt.test->addr);
                level ++;
                indent_output();
                fprintf(output, "%s = 0;\n", iter);
                assign_type_to_ast(s->whilestmt.body);
                indent_output();
                fprintf(output, "%s = 1;\n", iter);
                level --;
                indent_output();
                fprintf(output, "}\n");

                if(s->whilestmt.orelse != NULL) {
                    indent_output();
                    fprintf(output, "if(%s == 1) {\n", iter);
                    level ++;
                    assign_type_to_ast(s->whilestmt.orelse);
                    level --;
                    indent_output();
                    fprintf(output, "}\n");
                }
            }
            break;
        case If_kind:
            assign_type_to_expr(s->ifstmt.test);
            if(s->ifstmt.test->isplain)
                stmt_for_expr(s->ifstmt.test);
            else
                eliminate_python_unique_for_expr(s->ifstmt.test);
            indent_output();
            fprintf(output, "if(%s) {\n", s->ifstmt.test->addr);
            level ++;
            assign_type_to_ast(s->ifstmt.body);
            level --;
            indent_output();
            fprintf(output, "}\n");
            if(s->ifstmt.orelse != NULL) {
                indent_output();
                fprintf(output, "else {\n");
                level++;
                assign_type_to_ast(s->ifstmt.orelse);
                level --;
                indent_output();
                fprintf(output, "}\n");
            }
            break;
        case With_kind:
            break;
        case Raise_kind:
            break;
        case Try_kind:
            break;
        case Assert_kind:
            assign_type_to_expr(s->assert.test);
            if(s->assert.test->isplain)
                stmt_for_expr(s->assert.test);
            else
                eliminate_python_unique_for_expr(s->assert.test);
            indent_output();
            fprintf(output, "assert(%s);\n", s->assert.test->addr);
            break;
        case Global_kind:
            break;
        case Expr_kind:
            assign_type_to_expr(s->exprstmt.value);
            if(s->exprstmt.value->isplain) {
                stmt_for_expr(s->exprstmt.value);
            }
            else {
                eliminate_python_unique_for_expr(s->exprstmt.value);
            }
            break;
    }
}


static void
assign_type_to_expr(expr_ty e) {
    int i;
    switch(e->kind) {
        case BoolOp_kind:
            {
                int i, n = e->boolop.n_value;
                e->isplain = 1;
                for(i = 0; i < n; i ++ )  {
                    assign_type_to_expr(e->boolop.values[i]);
                    e->isplain &= e->boolop.values[i]->isplain;
                }
                e->e_type = e->boolop.values[0]->e_type;
            }
            break;
        case BinOp_kind:
            assign_type_to_expr(e->binop.left);
            assign_type_to_expr(e->binop.right);
            e->isplain = e->binop.left->isplain & e->binop.right->isplain;

            if(e->binop.op == Mult &&
                    (e->binop.left->e_type->kind == STRING_KIND ||
                     e->binop.right->e_type->kind == STRING_KIND)) {
                e->isplain = 0;
            }
            if(e->binop.op == Pow) {
                e->isplain = 0;
            }
            if(e->binop.left->e_type->kind == LIST_KIND) {
                e->isplain = 0;
                e->e_type = e->binop.left->e_type;
            }
            else {
                e->e_type = max_type(e->binop.left->e_type, e->binop.right->e_type);
            }
            break;
        case UnaryOp_kind:
            assign_type_to_expr(e->unaryop.operand);
            if(e->unaryop.op == Not)
                e->e_type = &t_boolean;
            else
                e->e_type = e->unaryop.operand->e_type;
            e->isplain = 1;
            break;
        case Lambda_kind:
            break;
        case IfExp_kind:
            break;
        case ListComp_kind:
            e->isplain = 0;
            for(i = 0; i < e->listcomp.n_com; i ++ ) {
                assign_type_to_comprehension(e->listcomp.generators[i]);
            }
            assign_type_to_expr(e->listcomp.elt);
            e->e_type = create_list_type(e->listcomp.elt->e_type);
            break;
        case Dict_kind:
            break;
        case Set_kind:
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
            assign_type_to_expr(e->compare.left);
            e->isplain = e->compare.left->isplain;
            for(i = 0; i< e->compare.n_comparator; i ++ ) {
                assign_type_to_expr(e->compare.comparators[i]);
                switch(e->compare.ops[i]) {
                    case Is:
                    case IsNot:
                    case In:
                    case NotIn:
                        e->isplain = 0;break;
                    default:
                        e->isplain = e->isplain && e->compare.comparators[i]->isplain;break;
                }
            }
            e->e_type = &t_boolean;
            if(e->compare.n_comparator != 1) {
                e->isplain = 0;
            }
            break;
        case Call_kind:
            {
                char fullname[50] = "";
                strcpy(fullname, e->call.func->name.id);
                int i;
                e->isplain = 1;
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    assign_type_to_expr(e->call.args[i].args);
                    if(e->call.args[i].args->isplain == 0)
                        e->isplain = 0;
                    switch(e->call.args[i].args->e_type->kind) {
                        case BOOLEAN_KIND:
                            strcat(fullname, "_b");break;
                        case CHAR_KIND:
                            strcat(fullname, "_c");break;
                        case INTEGER_KIND:
                            strcat(fullname, "_i");break;
                        case FLOAT_KIND:
                            strcat(fullname, "_f");break;
                        case STRING_KIND:
                            strcat(fullname, "_s");break;
                        case LIST_KIND:
                            strcat(fullname, "_l");break;
                        default:
                            break;
                    }
                }

                type_ty tp = search_type_for_name(fullname);
                if(tp == NULL) {
                    stmt_ty st = search_stmt_for_name(e->call.func->name.id);
                    insert_to_current_table(fullname, &t_unknown, SE_FUNCTION_KIND);
                    strcpy(st->funcdef.fullname, fullname);
                    push_type_to_arguments(st, e);
                    eliminate_python_unique_for_stmt(st);
                    e->e_type = func_ret;
                    change_func_ret_type(fullname, func_ret);
                    func_ret = NULL;
                }
                else {
                    e->e_type = tp;
                }
                strcpy(e->call.fullname, fullname);
            }
            break;
        case Repr_kind:
            break;
        case Num_kind:
            switch(e->num.kind) {
                case INTEGER:
                    e->e_type = &t_integer;
                    break;
                case DECIMAL:
                    e->e_type = &t_float;
                    break;
            }
            e->isplain = 1; break;
        case Str_kind:
            e->e_type = &t_string;
            e->isplain = 1;
            break;
        case Attribute_kind:
            break;
        case Subscript_kind:
            {
                assign_type_to_expr(e->sub.value);
                e->e_type = e->sub.value->e_type;
                e->isplain = 1;
                int n = e->sub.n_slice;
                int i;
                for(i = 0; i < n; i ++ ) {
                    slice_ty s = e->sub.slices[i];
                    if(s->kind == Slice_kind) {
                        if(s->slice.lower)
                            assign_type_to_expr(s->slice.lower);
                        if(s->slice.upper)
                            assign_type_to_expr(s->slice.upper);
                        if(s->slice.step)
                            assign_type_to_expr(s->slice.step);
                        e->isplain = 0;
                    }else {
                        assign_type_to_expr(s->index.value);
                        e->e_type = e->e_type->base;
                        e->isplain = s->index.value->isplain;
                    }
                }
                if(e->sub.value->isplain == 0) {
                    e->isplain = 0;
                }
            }
            break;
        case Name_kind:
            {
                type_ty tp = search_type_for_name(e->name.id);
                if(tp == NULL) {
                    e->e_type = &t_unknown;
                }else {
                    e->e_type = tp;
                }
                e->isplain = 1;
            }
            break;
        case List_kind:
            assign_type_to_expr(e->list.elts[0]);
            e->e_type = create_list_type(e->list.elts[0]->e_type);
            e->isplain = 0;
            for(i = 0; i < e->list.n_elt; i ++ ) {
                assign_type_to_expr(e->list.elts[i]);
            }
            break;
        case Tuple_kind:
            break;
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

    if(iter->e_type->kind = LIST_KIND) {
        target->e_type = iter->e_type->base;
    }else if(iter->e_type->kind ==DICT_KIND) {
        target->e_type = iter->e_type->kbase;
    }

    push_type_to_expr(target);
}



static void
push_type_to_expr(expr_ty e) {
    if(e->kind == Tuple_kind) {
        if(e->e_type->kind == TUPLE_KIND) {

        }
    }
    else if(e->kind == Name_kind) {
        insert_to_current_table(e->name.id, e->e_type, SE_TEMP);
        strcpy(e->addr, e->name.id);
    }
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



static void
stmt_for_expr(expr_ty e) {
    if(e->isplain == 0) return ;
    switch(e->kind) {
        case BoolOp_kind:
            {
                /* x = a and b or c not supported*/
                /*
                int i, n = e->boolop.n_value;
                if(e->addr[0] != 0) {
                    indent_output();
                    if(search_type_for_name(e->addr) == NULL)
                        fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
                    indent_output();
                    for(i = 0; i < n; i ++ ) {
                        expr_ty te = e->boolop.values[i];
                        if(te->kind == BoolOp_kind) {
                            strcpy(te->addr, e->addr);
                        }
                        stmt_for_expr(te);
                        fprintf(output, "if(%s = %s) ", e->addr, te->addr);
                        if(e->boolop.op == Or && i != n - 1)
                            fprintf(output, "; else { ");
                    }
                    if(e->boolop.op == Or) {
                        for(i = 0; i < n -1; i ++ )
                            if(i == 0) fprintf(output, "; ");
                            fprintf(output, "} ");
                        fprintf(output, "\n");
                    }else
                        fprintf(output, ";\n");
                }else {
                }
                */

                int i, n = e->boolop.n_value;
                char* op = e->boolop.op == And ? " && ": " || ";
                for(i = 0; i < n; i ++) {
                    expr_ty te = e->boolop.values[i];
                    stmt_for_expr(te);
                    strcat(e->addr, te->addr);
                    if(n-1 != i) {
                        strcat(e->addr, op);
                    }
                }
            }
            break;
        case BinOp_kind:
            {
                expr_ty l = e->binop.left;
                expr_ty r = e->binop.right;
                char* oper = get_op_literal(e->binop.op);
                stmt_for_expr(e->binop.left);
                stmt_for_expr(e->binop.right);
                if(e->addr[0] != 0) {
                    indent_output();
                    if( search_type_for_name(e->addr) == NULL) {
                        fprintf(output, "%s %s = %s %s %s;\n", e->e_type->name, e->addr,
                                l->addr, oper, r->addr);
                    }else {
                        fprintf(output, "%s = %s %s %s;\n", e->addr,l->addr, oper, r->addr);
                    }
                }else {
                    sprintf(e->addr, "(%s %s %s)", l->addr, oper, r->addr);
                }
            }
            break;
        case Num_kind:
            if(e->addr[0] != 0) {
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    fprintf(output, "%s %s = ", e->e_type->name, e->addr);
                else
                    fprintf(output, "%s = ", e->addr);

                if(e->num.kind == INTEGER)  {
                    fprintf(output, "%d;\n", e->num.ivalue);
                }
                else {
                    fprintf(output, "%f;\n", e->num.fvalue);
                }
            }
            else {
                if(e->num.kind == INTEGER) {
                    sprintf(e->addr, "%d", e->num.ivalue);
                }else {
                    sprintf(e->addr, "%f", e->num.fvalue);
                }
            }

            break;
        case Name_kind:
            if(e->addr[0] != 0) {
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    if(e->e_type->kind == LIST_KIND)
                        fprintf(output, "%s &%s = %s;\n", e->e_type->name, e->addr, e->name.id);
                    else
                        fprintf(output, "%s %s = %s;\n", e->e_type->name, e->addr, e->name.id);
                else
                    fprintf(output, "%s = %s;\n", e->addr, e->name.id);
            }
            else {
                strcpy(e->addr, e->name.id);
            }
            break;
        case Str_kind:
            if(e->addr[0] != 0) {
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    fprintf(output, "%s %s = \"%s\";\n", e->e_type->name, e->addr,  e->str.s);
                else
                    fprintf(output, "%s = \"%s\";\n", e->addr,  e->str.s);
            }else {
                sprintf(e->addr, "\"%s\"", e->str.s);
            }
            break;
        case Compare_kind:
            {
                stmt_for_expr(e->compare.left);
                stmt_for_expr(e->compare.comparators[0]);
                char* left = e->compare.left->addr;
                char* op = get_cmp_literal(e->compare.ops[0]);
                char* right = e->compare.comparators[0]->addr;

                if(e->addr[0] != 0 ) {
                    indent_output();

                    if(search_type_for_name(e->addr) == NULL)
                        fprintf(output, "%s %s = %s %s %s;\n", e->e_type->name, e->addr, left, op, right);
                    else
                        fprintf(output, "%s = %s %s %s;\n",e->addr, left, op, right);

                }else {
                    sprintf(e->addr, "(%s %s %s)", left, op, right);
                }
            }
            break;
        case Call_kind:
            if(e->addr[0] != 0) {
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    fprintf(output, "%s %s = %s(", e->e_type->name,  e->addr, e->call.fullname);
                else
                    fprintf(output, "%s = %s(",  e->addr, e->call.fullname);
                int i ;
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    stmt_for_expr(e->call.args[i].args);
                    fprintf(output, "%s", e->call.args[i].args->addr);
                    if(i != e->call.n_arg -1) printf(", ");
                }
                fprintf(output, ");\n");
            }
            else {
                sprintf(e->addr, "%s(", e->call.fullname);
                int i ;
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    stmt_for_expr(e->call.args[i].args);
                    strcat(e->addr,e->call.args[i].args->addr);
                    if(i != e->call.n_arg -1) strcat(e->addr, ", ");
                }
                strcat(e->addr, ") ");
            }
            break;
        case Subscript_kind:
            {
                stmt_for_expr(e->sub.value);
                int n = e->sub.n_slice;
                int i;
                for(i = 0; i < n; i ++ ) {
                    slice_ty s = e->sub.slices[i];
                    stmt_for_expr(s->index.value);
                }
                if(e->addr[0] != 0) {
                    indent_output();
                    if(search_type_for_name(e->addr) == NULL)
                        fprintf(output, "%s %s = %s", e->e_type->name, e->addr, e->sub.value->addr);
                    else
                        fprintf(output, "%s = %s", e->addr, e->sub.value->addr);

                    for(i =0; i < n; i ++ ) {
                        fprintf(output, "[%s]", e->sub.slices[i]->index.value->addr);
                    }
                    fprintf(output, ";\n");
                }else {
                    sprintf(e->addr, "%s", e->sub.value->addr);
                    for(i = 0; i < n; i ++ ) {
                        sprintf(e->addr + strlen(e->addr), "[%s]", e->sub.slices[i]->index.value->addr);
                    }
                }
            }
            break;
        case UnaryOp_kind:
            stmt_for_expr(e->unaryop.operand);
            if(e->addr[0]!= 0) {
                if(search_type_for_name(e->addr) == NULL) {
                    indent_output();
                    fprintf(output, "%s ", e->e_type->name);
                }
                fprintf(output, "%s = %s%s;\n", e->addr, get_unaryop_literal(e->unaryop.op), e->unaryop.operand->addr);
            }
            else {
                sprintf(e->addr, "%s%s", get_unaryop_literal(e->unaryop.op), e->unaryop.operand->addr);
            }
    }
}


static void
eliminate_python_unique_for_expr(expr_ty e) {
    if(e->isplain) return ;
    int i;
    switch(e->kind) {
        case BoolOp_kind:
            {
                int i, n = e->boolop.n_value;
                char* op = e->boolop.op == And ? " && ": " || ";
                for(i = 0; i < n;i ++) {
                    expr_ty te = e->boolop.values[i];
                    if(te->isplain)
                        stmt_for_expr(te);
                    else
                        eliminate_python_unique_for_expr(te);
                    strcat(e->addr, te->addr);
                    if(n-1 != i) {
                        strcat(e->addr, op);
                    }
                }
            }
            break;
        case BinOp_kind:
            {
                expr_ty l = e->binop.left;
                expr_ty r = e->binop.right;
                if(e->binop.left->e_type->kind == LIST_KIND) {
                    if(e->addr[0] == 0)
                        strcpy(e->addr, newTemp());
                    if(e->binop.op == Add) {
                        if(l->isplain)
                            stmt_for_expr(l);
                        else
                            eliminate_python_unique_for_expr(l);
                        if(r->isplain)
                            stmt_for_expr(r);
                        else
                            eliminate_python_unique_for_expr(r);

                        type_ty tp = search_type_for_name(e->addr);
                        char* temp = NULL;
                        if(NULL == tp) {
                            indent_output();
                            fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
                        }else if(strcmp(l->addr, e->addr) != 0 && strcmp(r->addr, e->addr)!= 0){
                            indent_output();
                            fprintf(output, "%s.clear();\n", e->addr);
                        }else if(strcmp(r->addr, e->addr) == 0) {
                            indent_output();
                            temp = newTemp();
                            fprintf(output, "%s %s = %s;\n", e->e_type->name, temp, e->addr);
                            strcpy(r->addr, temp);
                            if(strcmp(l->addr, e->addr) != 0) {
                                indent_output();
                                fprintf(output, "%s.clear();\n", e->addr);
                            }
                        }
                        char* iter = newIterator();
                        int self = 1;
                        if(strcmp(l->addr, e->addr) != 0) {
                            indent_output();
                            fprintf(output, "for( int %s = 0; %s < %s.size(); %s ++ )\n", iter, iter, l->addr, iter);
                            level ++;
                            indent_output();
                            fprintf(output, "%s.push_back(%s[%s]);\n\n", e->addr, l->addr, iter);
                            level --;
                            self = 0;
                        }
                        if(strcmp(r->addr, e->addr) != 0 || self == 1) {
                            indent_output();
                            fprintf(output, "for( int %s = 0; %s < %s.size(); %s ++ )\n", iter, iter, r->addr, iter);
                            level ++;
                            indent_output();
                            fprintf(output, "%s.push_back(%s[%s]);\n", e->addr, r->addr, iter);
                            level --;
                        }
                        free(iter);
                    }
                    else if(e->binop.op == Mult && (r->e_type->kind == LIST_KIND || l->e_type->kind == LIST_KIND)) {
                        if(e->addr[0] == 0)
                            strcpy(e->addr, newTemp());
                        if(r->e_type->kind == LIST_KIND) {
                            expr_ty t = r; r = l; l = t;
                        }
                        if(r->isplain)
                            stmt_for_expr(r);
                        else
                            eliminate_python_unique_for_expr(r);
                        if(l->isplain)
                            stmt_for_expr(l);
                        else
                            eliminate_python_unique_for_expr(l);

                        indent_output();
                        type_ty tp = search_type_for_name(e->addr);
                        if(NULL == tp)
                            fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
                        else
                            if(strcmp(e->addr, l->addr) == 0) {
                                char *temp = newTemp();
                                fprintf(output, "%s %s = %s;\n", e->e_type->name, temp, e->addr);
                                strcpy(l->addr, temp);
                            }
                            else
                                fprintf(output, "%s.clear();\n", e->addr);

                        char* iter = newIterator();
                        char* iter1 = newIterator();
                        indent_output();
                        fprintf(output, "for( int %s = 0; %s < %s; %s ++ )\n", iter, iter, r->addr, iter);
                        level ++;
                        indent_output();
                        fprintf(output, "for( int %s = 0; %s < %s.size(); %s ++ )\n", iter1, iter1, l->addr, iter1);
                        level ++;
                        indent_output();
                        fprintf(output, "%s.push_back(%s[%s]);\n\n", e->addr, l->addr, iter1);
                        level -= 2;
                        free(iter);
                        free(iter1);
                    }else {
                        fprintf(stderr, "List Error\n");
                        exit(-1);
                    }
                }
                else if(e->binop.op == Pow) {
                    if(e->addr[0] == 0)
                        strcpy(e->addr, newTemp());
                    if(l->isplain) {
                        stmt_for_expr(l);
                    }
                    else {
                        eliminate_python_unique_for_expr(l);
                    }
                    if(r->isplain) {
                        stmt_for_expr(r);
                    }
                    else {
                        eliminate_python_unique_for_expr(r);
                    }
                    type_ty tp = search_type_for_name(e->addr);
                    indent_output();
                    if(NULL == tp)  {
                        fprintf(output, "%s %s = pow(%s, %s);\n", e->e_type->name, e->addr,
                                l->addr, r->addr);
                    }
                    else
                        fprintf(output, "%s = pow(%s, %s);\n",e->addr, l->addr, r->addr);
                }else if(e->binop.op == Mult && (l->e_type->kind == STRING_KIND || r->e_type->kind == STRING_KIND)) {
                    if(e->addr[0] == 0)
                        strcpy(e->addr, newTemp());
                    if(r->kind == Str_kind) {
                        expr_ty t = l;
                        l = r;
                        r = t;
                    }
                    if(r->isplain)
                        stmt_for_expr(r);
                    else
                        eliminate_python_unique_for_expr(r);
                    if(l->isplain)
                        stmt_for_expr(l);
                    else
                        eliminate_python_unique_for_expr(l);
                    indent_output();

                    if(search_type_for_name(e->addr) == NULL) {
                        fprintf(output, "%s %s;\n", "string", e->addr);
                    }else if(strcmp(e->addr, l->addr) != 0) {
                        fprintf(output, "%s.clear();\n", e->addr);
                    }else {
                        char * temp = newTemp();
                        fprintf(output, "%s %s = %s;\n", "string", temp, e->addr);
                        strcpy(l->addr, temp);
                    }
                    char * iter = newIterator();
                    indent_output();
                    fprintf(output, "for(int %s = 0; %s < %s; %s ++ )\n", iter, iter, r->addr, iter);
                    level ++;
                    indent_output();
                    fprintf(output, "%s += %s;\n", e->addr, l->addr);
                    level --;
                    free(iter);
                }else {
                    if(l->isplain)
                        stmt_for_expr(l);
                    else
                        eliminate_python_unique_for_expr(l);
                    if(r->isplain)
                        stmt_for_expr(r);
                    else
                        eliminate_python_unique_for_expr(r);

                    char* oper = get_op_literal(e->binop.op);
                    if(e->addr[0] != 0) {
                        indent_output();
                        if( search_type_for_name(e->addr) == NULL) {
                            fprintf(output, "%s %s = %s %s %s;\n", e->e_type->name, e->addr,
                                    l->addr, oper, r->addr);
                        }else {
                            fprintf(output, "%s = %s %s %s;\n", e->addr,l->addr, oper, r->addr);
                        }
                    }else {
                        sprintf(e->addr, "(%s %s %s)", l->addr, oper, r->addr);

                    }
                }
            }
            break;
        case List_kind:
            if(e->addr[0] == 0)
                strcpy(e->addr, newTemp());
            type_ty tp = search_type_for_name(e->addr);
            indent_output();
            if(tp == NULL) {
                fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
            }else{
                fprintf(output, "%s.clear();\n", e->addr);
            }

            int plain = e->list.elts[0]->isplain;
            if(plain)
                stmt_for_expr(e->list.elts[0]);
            else
                eliminate_python_unique_for_expr(e->list.elts[0]);
            indent_output();
            fprintf(output, "%s.push_back(%s);\n", e->addr, e->list.elts[0]->addr);
            for(i = 1; i < e->list.n_elt;  i ++ ) {
                plain = e->list.elts[i]->isplain;
                if(plain)
                    stmt_for_expr(e->list.elts[i]);
                else
                    eliminate_python_unique_for_expr(e->list.elts[i]);
                indent_output();
                fprintf(output, "%s.push_back(%s);\n", e->addr, e->list.elts[i]->addr);
            }
            fprintf(output, "\n");
            break;
        case ListComp_kind:
            {
                indent_output();
                if(e->listcomp.elt->isplain)
                    stmt_for_expr(e->listcomp.elt);
                else
                    eliminate_python_unique_for_expr(e->listcomp.elt);
                if(search_type_for_name(e->addr) == NULL) {
                    fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
                }else {
                    fprintf(output, "%s.clear();\n", e->addr);
                }
                for(i = 0; i < e->listcomp.n_com; i ++ )  {
                    eliminate_python_unique_for_expr(e->listcomp.generators[i]->iter);
                    int j;
                    for(j = 0; j < e->listcomp.generators[i]->n_test; j ++ ) {
                        expr_ty t = e->listcomp.generators[i]->tests[j];
                        if(t->isplain)
                            stmt_for_expr(t);
                        else
                            eliminate_python_unique_for_expr(t);
                    }
                }
                int oldlevel = level;
                for(i = 0; i < e->listcomp.n_com; i ++ ) {
                    indent_output();
                    fprintf(output, "for(%s %s: %s)\n", e->listcomp.elt->e_type->name,
                            e->listcomp.generators[i]->target->addr,
                            e->listcomp.generators[i]->iter->addr);
                    level ++;
                    if(e->listcomp.generators[i]->n_test) {
                        indent_output();
                        fprintf(output, "if( ");
                        int j;
                        for(j = 0; j < e->listcomp.generators[i]->n_test; j ++ ) {
                            fprintf(output, "%s", e->listcomp.generators[i]->tests[j]->addr);
                            if(j != e->listcomp.generators[i]->n_test - 1)
                                fprintf(output, " && ");
                        }
                        fprintf(output, " )\n");
                        level ++;
                    }
                }
                indent_output();
                fprintf(output, "%s.push_back(%s);\n", e->addr, e->listcomp.elt->addr);
                level = oldlevel;
            }
            break;
        case Compare_kind:
            if(e->compare.left->isplain)
                stmt_for_expr(e->compare.left);
            else
                eliminate_python_unique_for_expr(e->compare.left);
            if(e->addr[0] != 0) {
                for(i = 0; i < e->compare.n_comparator; i ++ ) {
                    if(e->compare.comparators[i]->isplain)
                        stmt_for_expr(e->compare.comparators[i]);
                    else
                        eliminate_python_unique_for_expr(e->compare.comparators[i]);
                }
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    fprintf(output, "%s %s = ", e->e_type->name, e->addr);
                else
                    fprintf(output, "%s = ", e->addr);
                for(i = 0; i< e->compare.n_comparator; i ++ ) {
                    char* prev = NULL;
                    char* p = NULL;
                    if(i == 0) {
                        prev = e->compare.left->addr;
                    }else {
                        prev = e->compare.comparators[i-1]->addr;
                    }
                    p = e->compare.comparators[i]->addr;
                    switch(e->compare.ops[i]) {
                        case NotIn:
                            fprintf(output, "find(%s.begin(), %s.end(), %s) == %s.end()", p, p, prev, p);
                            break;
                        case In:
                            fprintf(output, "find(%s.begin(), %s.end(), %s) != %s.end()", p, p, prev, p);
                            break;
                        default:
                            fprintf(output, "%s", prev);
                            fprintf(output, " %s %s", get_cmp_literal(e->compare.ops[i]), p);
                    }
                    if(i != e->compare.n_comparator -1 ) {
                        fprintf(output, " && ");
                    }
                }
                fprintf(output, ";\n");
            }
            else {
                for(i = 0; i< e->compare.n_comparator; i ++ ) {
                    char* prev = NULL;
                    char* p = NULL;
                    if(i == 0) prev = e->compare.left->addr;
                    else prev = e->compare.comparators[i - 1]->addr;

                    if(e->compare.comparators[i]->isplain)
                        stmt_for_expr(e->compare.comparators[i]);
                    else
                        eliminate_python_unique_for_expr(e->compare.comparators[i]);
                    p = e->compare.comparators[i]->addr;
                    switch(e->compare.ops[i]) {
                        case NotIn:
                            sprintf(e->addr, "find(%s.begin(), %s.end(), %s) == %s.end()", p, p, prev, p);
                            break;
                        case In:
                            sprintf(e->addr, "find(%s.begin(), %s.end(), %s) != %s.end()", p, p, prev, p);
                            break;
                        default:
                            sprintf(e->addr, "(%s %s %s)", prev,
                                    get_cmp_literal(e->compare.ops[i]), e->compare.comparators[i]->addr);
                            break;
                    }
                    if(i != e->compare.n_comparator -1 ) {
                        strcat(e->addr, " && ");
                    }
                }
            }
            break;
        case Call_kind:
            if(e->addr[0] != 0) {
                int i ;
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    if(e->call.args[i].args->isplain)
                        stmt_for_expr(e->call.args[i].args);
                    else
                        eliminate_python_unique_for_expr(e->call.args[i].args);
                }
                indent_output();
                if(search_type_for_name(e->addr) == NULL)
                    fprintf(output, "%s %s = %s(", e->e_type->name,  e->addr, e->call.fullname);
                else
                    fprintf(output, "s = %s(",  e->addr, e->call.fullname);

                for(i = 0; i < e->call.n_arg; i ++ ) {
                    fprintf(output, "%s", e->call.args[i].args->addr);
                    if(i != e->call.n_arg -1) printf(", ");
                }
                fprintf(output, ");\n");
            }
            else {
                int i ;
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    if(e->call.args[i].args->isplain)
                        stmt_for_expr(e->call.args[i].args);
                    else
                        eliminate_python_unique_for_expr(e->call.args[i].args);
                }
                sprintf(e->addr, "%s(", e->call.fullname);
                for(i = 0; i < e->call.n_arg; i ++ ) {
                    strcat(e->addr,e->call.args[i].args->addr);
                    if(i != e->call.n_arg -1) strcat(e->addr, ", ");
                }
                strcat(e->addr, ") ");
            }
            break;
        case Subscript_kind:
            if(e->sub.value->isplain)
                stmt_for_expr(e->sub.value);
            else
                eliminate_python_unique_for_expr(e->sub.value);

            {
                int n = e->sub.n_slice;
                int i;
                if(n == 1 && e->sub.slices[0]->kind == Index_kind) {
                    if(e->sub.slices[0]->index.value->isplain)
                        stmt_for_expr(e->sub.slices[0]->index.value);
                    else
                        eliminate_python_unique_for_expr(e->sub.slices[0]->index.value);

                    if(e->addr[0] == 0) {
                        sprintf(e->addr, "%s[%s]", e->sub.value->addr,
                                e->sub.slices[0]->index.value->addr);
                    }else {
                        indent_output();
                        if(search_type_for_name(e->addr) == NULL)
                            fprintf(output, "%s %s  = %s[%s];\n", e->e_type->name,
                                    e->addr, e->sub.value->addr, e->sub.slices[0]->index.value->addr);
                    }

                }else if(n == 1 && e->sub.slices[0]->kind == Slice_kind) {
                    slice_ty s = e->sub.slices[0];
                    char lower[100] = "";
                    char upper[100] = "";
                    char step[100] = "";
                    if(s->slice.lower) {
                        if(s->slice.lower->isplain)
                            stmt_for_expr(s->slice.lower);
                        else
                            eliminate_python_unique_for_expr(s->slice.lower);
                        strcpy(lower, s->slice.lower->addr);
                    }
                    else
                        strcpy(lower,"0");

                    if(s->slice.upper) {
                        if(s->slice.upper->isplain)
                            stmt_for_expr(s->slice.upper);
                        else
                            eliminate_python_unique_for_expr(s->slice.upper);
                        strcpy(upper, s->slice.upper->addr);
                    }
                    else
                        sprintf(upper, "%s.size()", e->sub.value->addr);

                    if(strcmp(upper, "-1") == 0) {
                        sprintf(upper, "%s.size()", e->sub.value->addr);
                    }
                    if(s->slice.step) {
                        if(s->slice.step->isplain)
                            stmt_for_expr(s->slice.step);
                        else
                            eliminate_python_unique_for_expr(s->slice.step);
                        strcpy(step, s->slice.step->addr);
                    }
                    else
                        strcpy(step, "1");

                    if(e->addr[0] == 0) {
                        strcpy(e->addr, newTemp());
                    }
                    if(search_type_for_name(e->addr) == NULL) {
                        indent_output();
                        fprintf(output, "%s %s;\n", e->e_type->name, e->addr);
                    }

                    indent_output();
                    char* iter = newIterator();
                    fprintf(output, "for(int %s = %s; %s < %s; %s += %s)\n",
                            iter, lower, iter, upper,iter, step);
                    level ++;
                    indent_output();
                    fprintf(output, "%s.push_back(%s[%s]);\n", e->addr,
                            e->sub.value->addr, iter);
                    level --;
                    free(iter);
                }
            }
            break;
    }
}

static void
eliminate_python_unique_for_stmt(stmt_ty s) {
    switch(s->kind) {
        case FuncDef_kind:
            {
                enter_new_scope_for_func();
                output = fdef;
                fprintf(output, "%s(", s->funcdef.fullname);
                int n = s->funcdef.args->n_param;
                int i;
                for(i = 0; i < n; i ++ ) {
                    char* name = s->funcdef.args->params[i]->args->name.id;
                    type_ty tp = s->funcdef.args->params[i]->args->e_type;
                    insert_to_current_table(name, tp, SE_VARIABLE_KIND);
                    fprintf(output, "%s %s", tp->name, name);
                    if(i != n-1) {
                        fprintf(output, ", ");
                    }
                }
                fprintf(output, ") {\n");
                assign_type_to_ast(s->funcdef.body);
                fprintf(output, "}\n");

                exit_scope_from_func();

                fprintf(output, "%s\n", func_ret->name);
                output = stdout;
            }
            break;
    }
}
