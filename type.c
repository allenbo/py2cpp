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

struct type t_unknown  = {UNKNOWN_KIND, 0};
struct type t_char = {CHAR_KIND, 1, "char"};
struct type t_integer = {INTEGER_KIND, 4, "int"};
struct type t_float = {FLOAT_KIND, 4, "float"};
struct type t_string = {STRING_KIND, 0,  "string"};

static void assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);
static void assign_type_to_comprehension(comprehension_ty com);
static void push_type_to_expr(expr_ty e);
static int type_compare(type_ty t1, type_ty t2);
static type_ty max_type(type_ty t1, type_ty t2);
static void stmt_for_expr(expr_ty e);
static void eliminate_python_unique(expr_ty e);

static type_ty 
create_list_type(type_ty t) {
    type_ty tp = (type_ty) malloc ( sizeof(struct type) );
    sprintf(tp->name, "vector< %s >", t->name);
    tp->kind = LIST_KIND;
    tp->base = t;
    return tp;
}


static int
type_compare(type_ty t1, type_ty t2) {
}

static type_ty
max_type(type_ty t1, type_ty t2) {
    if(t1 != &t_integer && t1 != &t_float) return t1;
    if(t2 != &t_integer && t2 != &t_float) return t2;
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
                assign_type_to_expr(value);

                strcpy(value->addr, targets[0]->name.id);
                insert_to_current_table(targets[0]->name.id,
                        value->e_type, SE_VARIABLE_KIND);
                    
                eliminate_python_unique(value);
                if(value->isplain)
                    stmt_for_expr(value);
                
                int i ;
                for(i= 1; i < n_target; i ++ ){
                    assign_type_to_expr(targets[i]);
                    insert_to_current_table(targets[i]->name.id,
                            value->e_type, SE_VARIABLE_KIND);
            
                    switch(value->e_type->kind) {
                        case INTEGER_KIND:
                            printf("int %s = %s;\n", targets[i]->addr, value->addr);
                            break;
                        case FLOAT_KIND:
                            printf("float %s = %s;\n", targets[i]->addr, value->addr);
                            break;
                        case STRING_KIND:
                            printf("string %s = %s;\n", targets[i]->addr, value->addr);
                            break;
                        case LIST_KIND:
                            printf("%s & %s = %s;\n", value->e_type->name, 
                                    targets[i]->addr, value->addr);
                            break;
                    }
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
            e->isplain = e->binop.left->isplain & e->binop.right->isplain; 
           
            if(e->binop.op == Mult && (e->binop.left->e_type->kind == STRING_KIND || e->binop.right->e_type->kind == STRING_KIND)) {
                e->isplain = 0;
            }
            if(e->binop.op == Pow) {
                e->isplain = 0;
            }
            if(e->binop.left->e_type->kind == LIST_KIND) {
                e->e_type = e->binop.left->e_type;
            }
            else {
                e->e_type = max_type(e->binop.left->e_type, e->binop.right->e_type);
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
            e->isplain = 0;
            for(i = 0; i < e->listcomp.n_com; i ++ ) {
                assign_type_to_comprehension(e->listcomp.generators[i]);
            }
            assign_type_to_expr(e->listcomp.elt);
            e->e_type = create_list_type(e->listcomp.elt->e_type);
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
                    break;
                case DECIMAL:
                    e->e_type = &t_float;
                    break;
            }
            e->isplain = 1;
            break;
        case Str_kind:
            e->e_type = &t_string;
            e->isplain = 1;
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
                if(e->e_type->kind == LIST_KIND ) {
                    e->isplain = 0;
                }else {
                    e->isplain = 1;
                }
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



void
assign_type_to_ast(stmt_seq* ss) {
    int i = 0;
    for(; i < ss->size; i ++ ) {
        assign_type_to_stmt(ss->seqs[i]);
    }
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


static void
stmt_for_expr(expr_ty e) {
    if(e->isplain == 0) return ;
    switch(e->kind) {
        case BinOp_kind:
        {
            expr_ty l = e->binop.left;
            expr_ty r = e->binop.right;
            char* oper = get_op_literal(e->binop.op);
            stmt_for_expr(e->binop.left);
            stmt_for_expr(e->binop.right);
            if(e->addr[0] != 0) {
                printf("%s %s = %s %s %s;\n", e->e_type->name, e->addr,
                        l->addr, oper, r->addr);
            }else {
                sprintf(e->addr, "(%s %s %s)", l->addr, oper, r->addr);
            }
        }
            break;
        case Num_kind:
            if(e->addr[0] != 0) {
                printf("%s %s = ", e->e_type->name, e->addr);
                if(e->num.kind == INTEGER)  {
                    printf("%d;\n", e->num.ivalue);
                }
                else {
                    printf("%f;\n", e->num.fvalue);
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
        case Str_kind:
            if(e->addr[0] != 0) {
                printf("%s %s = \"%s\";\n", e->e_type->name, e->addr,  e->str.s);
            }else {
                sprintf(e->addr, "\"%s\"", e->str.s);
            }
            break;
    }
}


static void
eliminate_python_unique(expr_ty e) {
    if(e->isplain) return ;
    int i;
    switch(e->kind) {
        case BinOp_kind:
        {
            if(e->addr[0] == 0)
                strcpy(e->addr, newTemp());
            expr_ty l = e->binop.left;
            expr_ty r = e->binop.right;
            if(e->binop.left->e_type->kind == LIST_KIND) {
                if(e->binop.op == Add) {
                    printf("%s %s;\n", e->e_type->name, e->addr);
                    eliminate_python_unique(e->binop.left);
                     
                    char* iter = newIterator();
                    printf("for( int %s = 0; %s < %s.size(); %s ++ )\n", iter, iter, l->addr, iter);
                    printf("\t%s.push_back(%s[%s]);\n\n", e->addr, l->addr, iter);

                    eliminate_python_unique(e->binop.right);
                    
                    printf("for( int %s = 0; %s < %s.size(); %s ++ )\n", iter, iter, r->addr, iter);
                    printf("\t%s.push_back(%s[%s]);\n", e->addr, r->addr, iter);
                    free(iter);
                }
                else if(e->binop.op == Mult) {
                    printf("%s %s;\n", e->e_type->name, e->addr);
                    char* iter = newIterator();
                    char* iter1 = newIterator();
                    printf("for( int %s = 1; %s < %s; %s ++ )\n", iter, iter, r->addr, iter);
                    printf("\tfor( int %s = 1; %s < %s.size(); %s ++ )\n", iter1, l->addr, r->addr, iter1);
                    printf("\t\t%s.push_back(%s[%s]);\n\n", e->addr, l->addr, iter1);
                    free(iter);
                    free(iter1);
                }else {
                    fprintf(stderr, "List Error\n");
                    exit(-1);
                }
            }
            else if(e->binop.op == Pow) {
                if(l->isplain) {
                    stmt_for_expr(l);
                }
                else {
                    eliminate_python_unique(l);
                }
                if(r->isplain) {
                    stmt_for_expr(r);
                }
                else {
                    eliminate_python_unique(r);
                }
                printf("%s %s = pow(%s, %s);\n", e->e_type->name, e->addr,
                        l->addr, r->addr);
                //e->isplain = 1;
            }else if(e->binop.op == Mult) {
                if(l->e_type->kind == STRING_KIND || r->e_type->kind == STRING_KIND) {
                    if(r->kind == Str_kind) {
                        expr_ty t = l;
                        l = r;
                        r = t;
                    }
                        
                    printf("%s %s;\n", "string", e->addr);
                    char * iter = newIterator();
                    if(r->isplain) {
                        stmt_for_expr(r);
                    }
                    else {
                        eliminate_python_unique(r);
                    }
                    printf("for(int %s = 0; %s < %s; %s ++ ) {\n", iter, iter, r->addr, iter);
                    printf("\t%s += %s;\n", e->addr, l->addr);
                    free(iter);
                }
            }else {
                eliminate_python_unique(l);
                eliminate_python_unique(r);
                e->isplain = 1;
            }
        }
            break;
        case List_kind:
            if(e->addr[0] == 0)
                strcpy(e->addr, newTemp());
            printf("%s %s;\n", e->e_type->name, e->addr);
            
            int plain = e->list.elts[0]->isplain;
            if(plain)
                stmt_for_expr(e->list.elts[0]);
            else 
                eliminate_python_unique(e->list.elts[0]);
            printf("%s.push_back(%s);\n", e->addr, e->list.elts[0]->addr);
            for(i = 1; i < e->list.n_elt;  i ++ ) {
                plain = e->list.elts[i]->isplain;
                if(plain)
                    stmt_for_expr(e->list.elts[i]);
                else 
                    eliminate_python_unique(e->list.elts[i]);
                printf("%s.push_back(%s);\n", e->addr, e->list.elts[i]->addr);
            } 
            printf("\n");
            break;
        case ListComp_kind:
            {
                printf("%s %s;\n", e->e_type->name, e->addr);
                for(i = 0; i < e->listcomp.n_com; i ++ )                 
                    eliminate_python_unique(e->listcomp.generators[i]->iter);
                for(i = 0; i < e->listcomp.n_com; i ++ ) {
                    printf("for(%s %s: %s)\n", e->listcomp.elt->e_type->name, e->listcomp.generators[i]->target->addr,
                            e->listcomp.generators[i]->iter->addr);
                }
                printf("\t%s.push_back(%s);\n", e->addr, e->listcomp.elt->addr);
                
            }
            break;
    }
}
