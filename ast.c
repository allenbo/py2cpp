#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "node.h"
#include "token.h"
#include "Python-ast.h"
#include "mod.h"
#include "graminit.h"

static stmt_ty ast_for_stmt(const node* n);
static stmt_ty ast_for_expr_stmt(const node* n);
static expr_ty ast_for_testlist(const node* n);
static expr_ty ast_for_expr(const node* n);
static expr_ty ast_for_lambdef(const node* n);
static expr_ty ast_for_atom(const node* n);
static expr_ty ast_for_name(const node* n);
static comprehension_ty* ast_for_comprehension(const node* n, int *n_com);
static expr_ty ast_for_subscript(const node* n);
static expr_ty ast_for_trailer(const node* n);
static expr_ty ast_for_listcomp(const node* n);
static arguments_ty ast_for_arguments(const node* n);
static stmt_ty ast_for_print_stmt(const node* n);
static stmt_ty ast_for_del_stmt(const node* n);
static stmt_ty ast_for_pass_stmt(const node* n);
static stmt_ty ast_for_flow_stmt(const node* n);
static stmt_ty ast_for_import_stmt(const node* n);
static stmt_ty ast_for_global_stmt(const node* n);
static stmt_ty ast_for_exec_stmt(const node* n);
static stmt_ty ast_for_assert_stmt(const node* n);
static stmt_ty ast_for_if_stmt(const node* n);
static stmt_ty ast_for_for_stmt(const node* n);
static stmt_ty ast_for_while_stmt(const node* n);
static stmt_ty ast_for_try_stmt(const node* n);
static exception_handler_ty ast_for_exception_handler(const node* n);
static stmt_ty ast_for_with_stmt(const node* n);
static stmt_ty ast_for_funcdef_stmt(const node* n);
static stmt_ty ast_for_classdef_stmt(const node* n);
static stmt_ty ast_for_decorated_stmt(const node* n);


unaryop_ty
get_unaryop( const node* n) {
    const node* ch = n;
    switch(TYPE(ch)) {
        case PLUS:
            return UAdd;
        case MINUS:
            return USub;
        case NAME:
            if(strcmp(STR(ch), "not") == 0) 
                return Not;
            else
                return 0;
        case TILDE:
            return Invert;
        default:
            return 0;
    }
}

compop_ty
get_compop(const node* n) {
    node* ch;
    if(NCH(n) == 1) {
        ch = CHILD(n, 0); 
        switch(TYPE(ch)) {
            case LESS:
                return Lt;
            case GREATER:
                return Gt;
            case EQEQUAL:
                return Eq;
            case LESSEQUAL:
                return LtE;
            case GREATEREQUAL:
                return GtE;
            case NOTEQUAL:
                return NotEq;
            case NAME:
                if( strcmp(STR(ch) , "in") == 0) 
                    return In;
                else if( strcmp( STR(ch), "is") == 0) 
                    return Is;
                else {
                    fprintf(stderr, "Not a legal compop\n");
                    return 0;
                } 
        }
    }
    else {
        ch = CHILD(n, 0);
        if ( strcmp( STR(ch) , "not") == 0) 
            return NotIn;
        else 
            return IsNot;
    }
    return 0;
}


operator_ty
get_operator(const node* n) {
    const node * ch = n;
    switch(TYPE(ch)) {
        case PLUS:
            return Add;
        case MINUS:
            return Sub;
        case STAR:
            return Mult;
        case SLASH:
            return Div;
        case PERCENT:
            return Mod;
        case DOUBLESTAR:
            return Pow;
        case LEFTSHIFT:
            return LShift;
        case RIGHTSHIFT:
            return RShift;
        case VBAR:
            return BitOr;
        case AMPER:
            return BitXor;
        case CIRCUMFLEX:
            return BitAnd;
        case DOUBLESLASH:
            return FloorDiv;
        default:
            return 0;
    }
}

int
count_fors(const node* n) {
    int n_fors = 0;
label_list_for:
    n_fors ++;
    if( NCH(n) == 4) return n_fors;
    n = CHILD(n, 4);
    n = CHILD(n, 0);

    if(TYPE(n) == list_for) goto label_list_for;
label_list_if:
    if(NCH(n) == 2) return n_fors;
    n = CHILD(n, 2);
    n = CHILD(n, 0);
    if(TYPE(n) == list_for) goto label_list_for;
    else goto label_list_if;
}

int
count_ifs(const node* n) {
    int n_ifs = 0;
label_list_iter:
    n = CHILD(n,0);
    if(TYPE(n) == list_for) return n_ifs;
    n_ifs ++;
    if( NCH(n) == 2) return n_ifs;
    n = CHILD(n, 2);
    goto label_list_iter;
}


int
num_stmt(const node * n) {
    node* ch = NULL;
    
    int i, l;

    switch(TYPE(n) ) {
        case file_input:
            l = 0;
            for(i = 0; i < NCH(n) -1; i ++ ) {
                ch = CHILD(n, i);
                if(TYPE(ch) == stmt) {
                    l += num_stmt(ch);
                }
            }
            return l;
        case stmt:
            return num_stmt(CHILD(n, 0));
        case compound_stmt:
            return 1;
        case simple_stmt:
            return NCH(n) / 2;
        case suite:
            if( NCH(n) == 1) 
                return num_stmt(CHILD(n, 0));
            else {
                l = 0;
                for(i = 2 ; i < NCH(n) ; i ++ ) {
                    ch = CHILD(n, i);
                    if(TYPE(ch) == stmt) {
                        l += num_stmt(ch);
                    }
                }
                return l;
            }
    }
}

stmt_seq *
ast_from_node(const node* n) {
    stmt_seq* stmts = NULL;
    stmt_ty s;
    int i, j, k; 
    if(TYPE(n) == file_input) {
        k = 0;
        stmts = stmt_seq_new(num_stmt(n));
        node* ch;
        int num;
        for (i = 0; i < NCH(n) - 1; i++) {
            ch = CHILD(n, i);
            if (TYPE(ch) == NEWLINE)
                continue;
            REQ(ch, stmt);
            num = num_stmt(ch);
            if (num == 1) {
                s = ast_for_stmt(ch);
                if (!s)
                    goto error;
                ast_stmt_set(stmts, s, k++);
            }
            else {
                ch = CHILD(ch, 0);
                REQ(ch, simple_stmt);
                for (j = 0; j < num; j++) {
                    s = ast_for_stmt(CHILD(ch, j * 2));
                    if (!s)
                        goto error;
                    ast_stmt_set(stmts, s, k++);
                }
            }
        }
    }
    else if( TYPE(n) == suite) {
        k = 0;
        stmts = stmt_seq_new(num_stmt(n));
        const node* ch;
        if(NCH(n) == 1) {
            ch = CHILD(n, 0);
            stmt_ty s;
            i = 0;
        }else {
            ch = n;
            i = 2;
        }

        for( ; i < NCH(ch) ; i ++ ) {
            if(TYPE(CHILD(ch, i)) == small_stmt || TYPE(CHILD(ch, i)) == stmt) {
                s = ast_for_stmt(CHILD(ch, i));
                if(s == NULL) goto error;

                ast_stmt_set(stmts, s, k++);
            }
        }
    }
    return  stmts;
error:
    fprintf(stderr, "Something is wrong when construct AST\n");
    return 0;
}


static stmt_ty
ast_for_stmt(const node* n) {
    if(TYPE(n) == stmt)
        n = CHILD(n, 0);

    if(TYPE(n) == simple_stmt) {
        assert(num_stmt(n) == 1);
        n = CHILD(n, 0);
    }

    if(TYPE(n) == small_stmt) {
        /* small_stmt: expr_stmt | print_stmt  | del_stmt | pass_stmt
                     | flow_stmt | import_stmt | global_stmt | exec_stmt
                     | assert_stmt
        */
        node* ch = CHILD(n, 0);
        switch(TYPE(ch)) {
            case expr_stmt:
                return ast_for_expr_stmt(ch);
            case print_stmt:
                return ast_for_print_stmt(ch);
            case del_stmt:
                return ast_for_del_stmt(ch);
            case pass_stmt:
                return ast_for_pass_stmt(ch);
            case flow_stmt:
                return ast_for_flow_stmt(ch);
            case import_stmt:
                return ast_for_import_stmt(ch);
            case global_stmt:
                return ast_for_global_stmt(ch);
            case assert_stmt:
                return ast_for_assert_stmt(ch);
        }
    }
    else {
        /*compound_stmt: if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated */
        node* ch = CHILD(n, 0);
        switch(TYPE(ch)) {
            case if_stmt:
                return ast_for_if_stmt(ch);
            case while_stmt:
                return ast_for_while_stmt(ch);
            case for_stmt:
                return ast_for_for_stmt(ch);
            case try_stmt:
                return ast_for_try_stmt(ch);
            case with_stmt:
                return ast_for_with_stmt(ch);
            case funcdef:
                return ast_for_funcdef_stmt(ch);
            case classdef:
                return ast_for_classdef_stmt(ch);
            case decorated:
                return ast_for_decorated_stmt(ch);
        }
    }
}


static stmt_ty
ast_for_expr_stmt(const node* n) {
    /*    
    expr_stmt: testlist (augassign (yield_expr|testlist) |
                     ('=' (yield_expr|testlist))*)
    */
    stmt_ty s;
    node* ch; 

    if( NCH(n) == 1) {
        expr_ty e = ast_for_testlist(CHILD(n, 0));
        if(e == NULL) {
            return NULL;
        }
        return Expr_stmt(e, LINENO(n), n->n_col_offset);
    }
    else if( TYPE(CHILD(n, 1)) == augassign ) {
        expr_ty target;
        expr_ty value;
        operator_ty op;

        target = ast_for_testlist(CHILD(n, 0));
        if(target == NULL) return NULL;
        enum expr_kind kind = target->kind;
        if( kind != Name_kind && kind != Attribute_kind && kind != Subscript_kind) {
            fprintf(stderr, "Illegal expression for augmented assignment\n");
            return NULL;
        }

        op = get_operator(CHILD(n, 1));
        ch = CHILD(n, 2);
        if(TYPE(ch) == testlist) 
            value = ast_for_testlist(ch);
        else
            value = ast_for_expr(ch);
        if(value == NULL) return NULL;
        
        return Augassign_stmt(target, value, op, LINENO(n), n->n_col_offset);
    }
    else {
        expr_ty value;
        expr_ty* targets;        
        int n_target;
        n_target = NCH(n) / 2;
        targets = (expr_ty*) malloc( sizeof(expr_ty) * n_target);
        int i ;
        for(i = 0 ; i < NCH(n) -1; i += 2) {
            if(TYPE(CHILD(n, i)) == yield_expr) {
                fprintf(stderr, "Assignment to yield expression not possible\n");
                return NULL;
            }
            targets[i/2] = ast_for_testlist(CHILD(n, i));
        }

        value = ast_for_testlist(CHILD(n , NCH(n) -1));
        if(value == NULL) return NULL;
        return Assign_stmt(n_target, targets, value, LINENO(n), n->n_col_offset);
    }
}


static expr_ty
ast_for_testlist(const node* n) {

    /* 
        testlist: test (',' test)* [',']
        testlist1: test (',' test)*
        testlist_safe: old_test [(',' old_test)+ [',']]
        testlist_comp: test  (',' test)* [',']
        listmaker: test (',' test)* [',']
        exprlist: expr (',' expr)* [',']
        */
    expr_ty e;

    if(TYPE(n) == testlist_comp) {
        if( NCH(n) > 1) 
            assert(TYPE(CHILD(n, 1)) != comp_for);
    }

    if(TYPE(n) == listmaker) {
        if(NCH(n) > 1)
            assert(TYPE(CHILD(n, 1)) != list_for);
    }

    if( NCH(n) == 1) 
        return ast_for_expr(CHILD(n, 0));
    else {
        int size = (NCH(n) + 1) / 2;
        expr_ty * elts = (expr_ty*) malloc ( sizeof( expr_ty) * size) ;
        int i ;
        for(i = 0; i < NCH(n); i += 2 ) {
            elts[i/2] = ast_for_expr(CHILD(n, i));
        }
        return Tuple_expr(size, elts, LINENO(n), n->n_col_offset); 
    }
}


static expr_ty
ast_for_expr(const node* n) {
    /* 
     * test: or_test ['if' or_test 'else' test] | lambdef
     * or_test: and_test ('or' and_test)*
     * and_test: not_test ('and' not_test)*
     * not_test: 'not' not_test | comparison
     * comparison: expr (comp_op expr)*
     * comp_op: '<'|'>'|'=='|'>='|'<='|'<>'|'!='|'in'|'not' 'in'|'is'|'is' 'not'
     * expr: xor_expr ('|' xor_expr)*
     * xor_expr: and_expr ('^' and_expr)*
     * and_expr: shift_expr ('&' shift_expr)*
     * shift_expr: arith_expr (('<<'|'>>') arith_expr)*
     * arith_expr: term (('+'|'-') term)*
     * term: factor (('*'|'/'|'%'|'//') factor)*
     * factor: ('+'|'-'|'~') factor | power
     */

    const node* ch = n;
loop:
    switch(TYPE(ch)) {
        case test:
        case old_test:
            if(TYPE(CHILD(ch, 0)) == lambdef || TYPE(CHILD(ch, 0)) == old_lambdef)
                return ast_for_lambdef(CHILD(n,0));
            else {
                if( NCH(ch) == 1) {
                    ch = CHILD(ch, 0);
                    goto loop;
                }
                else {
                    /* IfExp */
                    expr_ty expression, body, orelse;
                    body = ast_for_expr(CHILD(ch, 0));
                    
                    if(NULL == body) return NULL;
                    
                    expression = ast_for_expr(CHILD(ch, 2));
                    if(NULL == expression) return NULL;

                    orelse = ast_for_expr(CHILD(ch, 4));
                    if(NULL == orelse ) return NULL;

                    return Ifexp_expr(expression, body, orelse, LINENO(ch), ch->n_col_offset);
                }
            }
            break;
        case or_test:
        case and_test:
            if( NCH(ch) == 1) {
                ch = CHILD(ch, 0);
                goto loop;
            }
            else {
                /* boolop */
                expr_ty * values;
                boolop_ty op;
                int n_value = (NCH(ch) + 1) /2;
                values = (expr_ty* ) malloc( sizeof( expr_ty)  * n_value);
                int i = 0;
                for(; i < NCH(ch) ; i += 2) {
                    values[i/2] = ast_for_expr(CHILD(ch, i));
                    if( values[i/2] == NULL) return NULL;
                }
                
                if(strcmp(STR(CHILD(ch, 1)), "and") == 0)
                    op = And;
                else 
                    op = Or;

                return Boolop_expr(op, n_value, values, LINENO(ch), ch->n_col_offset);

            }
            break;
        case not_test:
        case factor:
            /* unary op */
            if(NCH(ch) == 1) {
                ch = CHILD(ch, 0);
                goto loop;
            }
            else {
                unaryop_ty op;
                expr_ty operand;
                
                op = get_unaryop(CHILD(ch, 0));
                if(op == 0) return NULL;
                
                operand = ast_for_expr(CHILD(ch, 1));
                if(NULL == operand) return NULL;
                
                return Unaryop_expr(op, operand, LINENO(ch), ch->n_col_offset);
            }
            break;
        case comparison:
            /* compop */
            if(NCH(ch) == 1) {
                ch = CHILD(ch, 0);
                goto loop;
            }
            else {
                expr_ty left;
                compop_ty* ops;
                int n_comparator;
                expr_ty* comparators;

                n_comparator = NCH(ch) / 2; 
                
                ops = (compop_ty*) malloc( sizeof( compop_ty) * n_comparator);
                comparators = (expr_ty*) malloc( sizeof(expr_ty) * n_comparator);
                
                left = ast_for_expr(CHILD(ch,0));
                if(NULL == left) return NULL;
                int i;
                for(i = 1; i < NCH(ch) ; i += 2) {
                    ops[i/2] = get_compop(CHILD(ch, i));
                    comparators[i/2] = ast_for_expr(CHILD(ch, i+1));
                    if(NULL == comparators[i/2] ) return NULL;
                }

                return Compare_expr(left, n_comparator, ops, comparators, LINENO(ch), ch->n_col_offset);
            }
            break;
        case expr:
        case xor_expr:
        case and_expr:
        case shift_expr:
        case arith_expr:
        case term:
            if( NCH(ch) == 1) {
                ch = CHILD(ch, 0);
                goto loop;
            }
            else {
                /* binop */
                expr_ty left, right;
                operator_ty op;
                expr_ty tmp;
                int i;
                left = ast_for_expr(CHILD(ch, 0));
                if(left == NULL) return NULL;

                for(i = 1; i < NCH(ch) ; i +=2 ) {
                    op = get_operator(CHILD(ch, i));
                    if(op == 0) return NULL;

                    right = ast_for_expr(CHILD(ch, i+1));
                    if(right == NULL) return NULL;

                    tmp = Binop_expr(left, op, right, LINENO(ch), ch->n_col_offset);
                    left = tmp;
                }
                return tmp; 
            }
            break;
        case power:
            if( NCH(ch) == 1)
                return ast_for_atom(CHILD(ch, 0));

            else {
                int i;
                expr_ty value = ast_for_atom(CHILD(ch, 0));
                if(value == NULL) return NULL;
                
                expr_ty tra = NULL; 
                for(i = 1; i < NCH(ch); i ++ ) {
                    if(TYPE(CHILD(ch, i)) != trailer)
                        break;
                    else {
                        tra = ast_for_trailer(CHILD(ch, i));
                        if(NULL == tra) return NULL;
                        switch(tra->kind) {
                            case Call_kind:
                                tra->call.func = value;
                                break;
                            case Attribute_kind:
                                tra->attribute.value = value;
                                break;
                            case Subscript_kind:
                                tra->sub.value = value;
                                break;
                        }
                        value = tra;
                    }
                }
                if(i != NCH(ch)){
                    REQ(CHILD(ch, i), DOUBLESTAR);
                    
                    expr_ty right;
                    operator_ty op = Pow;
                    
                    right = ast_for_expr(CHILD(ch, i+1));
                    if(NULL == right) return NULL;

                    return Binop_expr(value, op, right, LINENO(ch), ch->n_col_offset);
                }
                else
                    return tra;
            }
            break;
        case yield_expr:
            {
                expr_ty value = NULL;
                if( NCH(ch) == 2 ) {
                    value = ast_for_testlist(CHILD(ch, 1));
                }
                return Yield_expr(value, LINENO(ch), ch->n_col_offset);
            }
    }
}


static expr_ty 
ast_for_name(const node* n) {
    expr_ty e = NULL;
    e = expr_new_full(Name_kind, LINENO(n), n->n_col_offset);
    strcpy(e->name.id, STR(n));
    return e;
}

static expr_ty
ast_for_atom(const node * n) {
    const node* ch = n;
    expr_ty e = NULL;
    switch(TYPE(CHILD(ch, 0))) {
        case LPAR:
            /* generator */
            if(NCH(n) == 2) {
                return Tuple_expr(0, NULL, LINENO(n), n->n_col_offset);
            }
            else {
                n = CHILD(n, 1);
                if(TYPE(n) == yield_expr) {
                    return ast_for_expr(n); 
                }else if (NCH(n) == 1 || TYPE(CHILD(n, 1)) == COMMA) {
                    return ast_for_testlist(n);
                }
                else {
                   expr_ty lcom = ast_for_listcomp(n);
                   if(NULL == lcom) return NULL;
                   lcom->kind = GeneratorExp_kind;
                   return lcom;
                }
            }
            break;
        case LSQB:
            if( NCH(ch) == 2) {
                REQ(CHILD(ch, 1), RSQB);
                return List_expr(0, NULL, LINENO(ch), ch->n_col_offset);
            }
            else {
                REQ(CHILD(ch, 1), listmaker);
                REQ(CHILD(ch, 2), RSQB);
                ch = CHILD(ch, 1);

                if(NCH(ch) != 1 && TYPE(CHILD(ch, 1)) == list_for) {
                    /* listcomprehension */
                    return ast_for_listcomp(ch);
                }
                else {
                    expr_ty e = ast_for_testlist(ch); 
                    if(e == NULL) return NULL;
                    expr_ty* elts;
                    int n_elt;
                    if( e->kind != Tuple_kind) {
                        n_elt = 1;
                        elts = (expr_ty*) malloc (sizeof( expr_ty) * n_elt);
                        elts[0] = e;
                    }
                    else {
                        /* should assign tuple's elements to list's elements
                         * so the tuple is no longer usefull
                         */
                        elts = e->tuple.elts;
                        n_elt = e->tuple.n_elt;
                        free(e);
                    }
                    return List_expr(n_elt, elts, LINENO(ch), ch->n_col_offset);
                }
            }
            break;
        case LBRACE:
            if(NCH(ch) == 2) {
                return Dict_expr(0, NULL, NULL, LINENO(ch), ch->n_col_offset);
            }
            else {
                ch = CHILD(ch, 1);
                if( TYPE(CHILD(ch, 1)) == COLON ) {
                    if( TYPE(CHILD(ch, 3)) == comp_for) {
                        expr_ty key = NULL, value = NULL;
                        key = ast_for_expr(CHILD(ch, 0));
                        if(NULL == key) return NULL;
                        value = ast_for_expr(CHILD(ch, 2));
                        if(NULL == value) return NULL;
                        comprehension_ty* generators = NULL;
                        int n_com = 0;
                        generators = ast_for_comprehension(CHILD(ch, 3), &n_com);
                        if(NULL == generators) return NULL;

                        return Dictcomp_expr(key, value, n_com, generators, LINENO(ch), ch->n_col_offset);
                    }
                    else {
                        int i = 0;
                        int n_key = 0;
                        for(; i < NCH(ch); i++ ) {
                            if(TYPE (CHILD(ch, i)) == COLON) n_key ++;
                        }

                        expr_ty* keys = (expr_ty* ) malloc( sizeof(expr_ty) * n_key);
                        expr_ty* values = (expr_ty* ) malloc( sizeof(expr_ty) * n_key);
                        
                        for(i = 0; i < n_key ; i++ ) {
                            keys[i] = ast_for_expr(CHILD(ch, i * 4));
                            if(NULL == keys[i]) return NULL;
                            values[i] = ast_for_expr(CHILD(ch, i * 4 + 2));
                            if(NULL == values[i] ) return NULL;
                        }

                        return Dict_expr(n_key, keys, values, LINENO(ch), ch->n_col_offset);

                    }

                }
                else {
                    if( TYPE( CHILD(ch, 1)) == comp_for ) {
                        expr_ty lcom = ast_for_listcomp(ch);
                        if(NULL == lcom) return NULL;
                        lcom->kind = SetComp_kind;
                        return lcom;
                    }
                    else {
                        expr_ty e = ast_for_testlist(ch); 
                        if(e == NULL) return NULL;
                        expr_ty* elts;
                        int n_elt;
                        if( e->kind != Tuple_kind) {
                            n_elt = 1;
                            elts = (expr_ty*) malloc (sizeof( expr_ty) * n_elt);
                            elts[0] = e;
                        }
                        else {
                            /* should assign tuple's elements to list's elements
                             * so the tuple is no longer usefull
                             */
                            elts = e->tuple.elts;
                            n_elt = e->tuple.n_elt;
                            free(e);
                        }
                        return Set_expr(n_elt, elts, LINENO(ch), ch->n_col_offset);   
                    }
                }
            }
            break;
        case BACKQUOTE:
            e = expr_new_full(Repr_kind, LINENO(n), n->n_col_offset);
            e->repr.value = ast_for_testlist(CHILD(n, 1));
            if(NULL == e->repr.value) return NULL;
            return e;
            break;
        case NAME:
            return ast_for_name(CHILD(n, 0));
            break;
        case NUMBER:
            ch = CHILD(ch, 0);
            e = expr_new_full(Num_kind, LINENO(ch), n->n_col_offset); 
            if(strchr(STR(ch), '.') != NULL) {
                e->num.kind = DECIMAL;
                e->num.fvalue = atof(STR(ch));
            }
            else {
                e->num.kind = INTEGER;
                e->num.ivalue = atoi(STR(ch));
            }
            return e;
            break;
        case STRING:
            e = expr_new_full(Str_kind, LINENO(n), n->n_col_offset);
            int len = 0;
            int i, j;
            for(i = 0; i < NCH(n) ; i ++ ) {
                len += strlen(STR(CHILD(n, i)));
            }
            e->str.s = (char*) malloc (sizeof(char) * len + 1);
            e->str.s[0] = 0;
            char* p = e->str.s;
            for(i = 0; i < NCH(n); i ++ ) {
                int delim = 0;
                char* str = STR(CHILD(n, i));
                delim = str[0];
                for(j = 1; str[j]; j ++ ) {
                    if(str[j] == '\\') {
                        if(str[j + 1] == 't') *p ++ = '\t';
                        else if(str[j + 1] == 'n') *p ++ = '\n';
                        else if(str[j + 1] == 'r') *p ++ = '\r';
                        else if(str[j + 1] == 'v') *p ++ = '\v';
                        else if(str[j + 1] == delim) *p ++ = (char)delim;
                        j ++;
                    }
                    else {
                        *p ++ = str[j];
                    }
                }
                *(--p) = 0;
                
            }
            return e;
            break;
    }
}


static expr_ty
ast_for_trailer(const node* n) {
    if(TYPE(CHILD(n, 0)) == DOT) {
        return Attribute_expr(NULL, STR(CHILD(n, 1)), LINENO(n), n->n_col_offset);
    }
    else if(TYPE(CHILD(n, 0)) == LSQB) {
        n = CHILD(n, 1);
        return ast_for_subscript(n);
    }
    else if( TYPE(CHILD(n, 0)) == LPAR) {
        if( NCH(n) == 2) {
            return Call_expr(NULL, 0,  NULL, NULL, NULL, LINENO(n), n->n_col_offset);
        }
        else {
            n = CHILD(n, 1);
            int n_arg = 0;
            Parameter* args;
            expr_ty varg = NULL, karg = NULL;
            int has_v = 0, has_k = 0;
            int i = 0;
            for(; i < NCH(n); i ++ ) {
                if(TYPE(CHILD(n,i)) == argument) n_arg ++;
                if(TYPE(CHILD(n, i)) == STAR) has_v = 1;
                if(TYPE(CHILD(n, i)) == DOUBLESTAR) has_k = 1;
            }

            args = (Parameter*) malloc( sizeof(Parameter) * n_arg);
            node* ch;
            int j = 0;
            for(i = 0; i < NCH(n) ; i ++ ) {
                if(TYPE(CHILD(n, i)) == argument) {
                    ch = CHILD(n, i);
                    args[j].args = ast_for_expr(CHILD(ch, 0));
                    if(args[j].args == NULL) return NULL;
                    if( NCH(ch) == 3) {
                        args[j].value = ast_for_expr(CHILD(ch, 2));
                        if(NULL == args[i].value) return  NULL;
                    }
                    j ++;
                }
                if(TYPE(CHILD(n, i)) == STAR) {
                    varg = ast_for_expr(CHILD(n, i+1));
                    if(NULL == varg) return NULL;
                    i ++;
                }
                if(TYPE(CHILD(n, i)) == DOUBLESTAR) {
                    karg = ast_for_expr(CHILD(n, i+1));
                    if(NULL == karg) return NULL;
                    i ++;
                }
            }
            return Call_expr(NULL, n_arg, args, varg, karg, LINENO(n), n->n_col_offset);
        }
    }
}


static expr_ty
ast_for_subscript(const node* n) {
    int i; 
    int n_slice = (NCH(n)+1) / 2;
    slice_ty* slices = (slice_ty*) malloc( sizeof(slice_ty) * n_slice);
    node* ch; 
    
    for(i = 0; i < n_slice; i ++ ) {
        ch = CHILD(n, i* 2);
        slices[i] = (slice_ty) malloc( sizeof(struct slice) );
        if(TYPE(CHILD(ch, 0)) == DOT) {
            slices[i]->kind = Ellipsis_kind;
        }else if( NCH(ch) == 1 && TYPE(CHILD(ch, 0) ) == test) {
            slices[i]->kind = Index_kind;
            slices[i]->index.value = ast_for_expr(CHILD(ch, 0));
            if(slices[i]->index.value == NULL) return NULL;
        }else  {
            expr_ty lower = NULL;
            expr_ty upper = NULL;
            expr_ty step = NULL;
            
            node* c;
            if(TYPE(CHILD(ch, NCH(ch) - 1)) == sliceop) {
                c = CHILD(ch, NCH(ch) - 1);
                if( NCH(c) == 2)  {
                    step = ast_for_expr(CHILD(c, 1));
                    if(NULL == step) return NULL;
                }
            }

            if( TYPE(CHILD(ch, 0)) == test) {
                lower = ast_for_expr(CHILD(ch, 0));
                if(NULL == lower) return NULL;
                if( TYPE(CHILD(ch, 2)) == test) {
                    upper = ast_for_expr(CHILD(ch, 2));
                    if(NULL == upper) return NULL;
                }
            }
            else {
                if(TYPE(CHILD(ch, 1)) == test) {
                    upper = ast_for_expr(CHILD(ch, 1));
                    if(NULL == upper) return NULL;
                }
            }

            slices[i]->kind = Slice_kind;
            slices[i]->slice.lower = lower;
            slices[i]->slice.upper = upper;
            slices[i]->slice.step = step;
        }
                
    }

    return Subscript_expr(NULL, n_slice, slices, LINENO(n), n->n_col_offset);
}


static expr_ty
ast_for_listcomp(const node* n) {
    /*
        listmaker: test list_for
        testlist_comp: test ( comp_for | (',' test)* [','] )
    */
    
    expr_ty elt;
    comprehension_ty * generators = NULL;
    elt = ast_for_expr(CHILD(n, 0));
    if(NULL == elt) return NULL;

    int n_com = 0;
    generators = ast_for_comprehension(CHILD(n, 1), &n_com); 
    if(generators == NULL) return NULL;
    return Listcomp_expr(elt, n_com, generators, LINENO(n), n->n_col_offset);
}

static comprehension_ty*
ast_for_comprehension(const node* n, int * pn_com) {
    comprehension_ty * generators = NULL;
    int n_com = count_fors(n);
    if(n_com == -1) return NULL;

    generators = (comprehension_ty*) malloc(sizeof(comprehension_ty) * n_com);
    if( NULL == generators) return NULL;
    
    int i = 0;
    for(; i < n_com; i ++ ) {
        comprehension_ty com = (comprehension_ty) malloc( sizeof( struct comprehension) );
        expr_ty target = NULL, iter = NULL;
        int n_test = 0;
        expr_ty* tests = NULL;
        
        target = ast_for_testlist(CHILD(n, 1));
        if(NULL == target) return NULL;
        
        if(TYPE(CHILD(n, 3)) == or_test) 
            iter = ast_for_expr(CHILD(n, 3));
        else
            iter = ast_for_testlist(CHILD(n, 3));

        if(NULL == iter) return NULL;

        if(NCH(n) == 5) {
            n = CHILD(n, 4);
            n_test = count_ifs(n);
            if(n_test == -1) return NULL;
            if(n_test != 0) {
                tests = (expr_ty*) malloc (sizeof(expr_ty) * n_test);
                
                int j = 0;
                for(; j < n_test; j ++ ) {
                    n = CHILD(n, 0);
                    tests[j] = ast_for_expr( CHILD(n, 1) );
                    if(NULL == tests[j] ) return NULL;
                    if(NCH(n) == 3) n = CHILD(n, 2);
                }
            }
        }
        com->target = target;
        com->iter = iter;
        com->n_test = n_test;
        com->tests = tests;

        generators[i] = com;
        /* n is list_iter */
        n = CHILD(n, 0); 
    } 
    *pn_com = n_com;
    return generators;
}


static expr_ty
ast_for_lambdef(const node* n) {
    /* lambdef: 'lambda' [varargslist] ':' test
     * old_lambdef: 'lambda' [varargslist] ':' old_test
     */
    expr_ty body;
    arguments_ty args = NULL;
    if(NCH(n) == 4) {
        args = ast_for_arguments(CHILD(n, 1));
        if(NULL == args) return NULL;
        body = ast_for_expr(CHILD(n, 3));
        if(NULL == body) return NULL;
    }else {
        body = ast_for_expr(CHILD(n, 2));
    }

    return Lambda_expr(args, body, LINENO(n), n->n_col_offset);
}


static stmt_ty
ast_for_print_stmt(const node* n) {
    const node* ch = n;
    expr_ty dest = NULL;
    int n_value = NCH(ch) / 2; 
    int newline_mark = 0;
    
    int i = 1;
    int j = 0;


    if( TYPE(CHILD(ch, 1)) == RIGHTSHIFT) {
        REQ(CHILD(ch, 2), test);
        dest = ast_for_expr(CHILD(ch, 2));
        if(NULL == dest ) return NULL;

        i = 4; 
        n_value = (NCH(ch) - 3) / 2;
    }
    expr_ty* values = (expr_ty*) malloc ( sizeof( expr_ty) * n_value);
    for(; i < NCH(ch); i += 2 , j++) {
        values[j] = ast_for_expr(CHILD(ch, i));
        if(values[j] == NULL) return NULL;
    }

    if( TYPE(CHILD(ch, NCH(ch) - 1)) == COMMA)
        newline_mark = 1;
    
    return Print_stmt(dest, n_value, values, newline_mark, LINENO(ch), ch->n_col_offset);
}


static stmt_ty
ast_for_del_stmt(const node* n) {
    REQ(CHILD(n, 1), exprlist);
    expr_ty e = ast_for_testlist(CHILD(n, 1));
    expr_ty* targets = NULL;
    int n_target;
    if(e->kind != Tuple_kind) {
        targets = (expr_ty*) malloc ( sizeof( expr_ty) );
        targets[0] = e; 
    }
    else {
        targets = e->tuple.elts;
        n_target = e->tuple.n_elt;
        free(e);
    }

    return Del_stmt(n_target, targets, LINENO(n), n->n_col_offset);
}

static stmt_ty
ast_for_pass_stmt(const node* n) {
    stmt_ty s = stmt_new_full(Pass_kind, LINENO(n), n->n_col_offset);
    return s;
}

static stmt_ty
ast_for_flow_stmt(const node* n) {
    node* ch =  CHILD(n, 0);
    stmt_ty s = stmt_new();
    s->lineno = LINENO(ch);
    s->col_offset = ch->n_col_offset;
    
    switch(TYPE(ch)) {
        case break_stmt:
            s->kind = Break_kind;
            break;
        case continue_stmt:
            s->kind = Continue_kind;
            break;
        case yield_stmt:
            s->kind = Expr_kind;
            s->exprstmt.value = ast_for_expr(CHILD(ch, 0));
            break;
        case raise_stmt:
            s->kind = Raise_kind;
            if( NCH(ch) == 1) 
                return Raise_stmt(NULL, NULL, NULL, LINENO(ch), ch->n_col_offset);
            if( NCH(ch) == 2) {
                expr_ty type = ast_for_expr(CHILD(ch, 1));
                if(type == NULL) return NULL;

                return Raise_stmt(type, NULL, NULL, LINENO(ch), ch->n_col_offset);
            }
            if( NCH(ch) == 4) {
                expr_ty type = ast_for_expr(CHILD(ch, 1));
                if(type == NULL) return NULL;

                expr_ty inst = ast_for_expr(CHILD(ch, 3));
                if(NULL == inst) return NULL;

                return Raise_stmt(type, inst, NULL, LINENO(ch), ch->n_col_offset);
            }
            if( NCH(ch) == 6) {
                 expr_ty type = ast_for_expr(CHILD(ch, 1));
                if(type == NULL) return NULL;

                expr_ty inst = ast_for_expr(CHILD(ch, 3));
                if(NULL == inst) return NULL;
                    
                expr_ty tback = ast_for_expr(CHILD(ch, 5));
                if(NULL == tback) return NULL;

                return Raise_stmt(type, inst, tback, LINENO(ch), ch->n_col_offset);
            }
            break;
        case return_stmt:
            if( NCH(ch) == 1) 
                return Return_stmt(NULL, LINENO(ch), ch->n_col_offset);
            else {
                expr_ty value = ast_for_testlist(CHILD(ch, 1));
                if(NULL == value) return NULL;

                return Return_stmt(value, LINENO(ch), ch->n_col_offset);
            }
            break;
    }
    return s;
}

static stmt_ty
ast_for_import_stmt(const node* n) {
    if(TYPE(CHILD(n, 0)) == import_name) {
        n = CHILD(n, 0);
        n = CHILD(n, 1); //dotted_as_names

        int n_module = (NCH(n) + 1)/ 2;
        char** modules = (char**) malloc(sizeof(char*) * n_module);
        char** names = (char**) malloc(sizeof(char*) * n_module);
        memset(modules, 0, sizeof(char*) * n_module);
        memset(names, 0, sizeof(char*) * n_module);

        int i;
        node* ch;
        for(i = 0; i < n_module; i ++) {
            ch = CHILD(n, i * 2);
            if(NCH(ch) == 3) {
                names[i] = strdup(STR(CHILD(ch, 2)));
            }
            ch = CHILD(ch, 0);
            modules[i] = (char* ) malloc( sizeof(char) * NAME_MAX_SIZE);
            memset(modules[i], 0, sizeof(char) * NAME_MAX_SIZE);
            int j = 0;
            for(; j < NCH(ch); j ++ ) {
                strcat(modules[i], STR(CHILD(ch, j)));
            }
        }

        return Import_stmt(n_module, modules, names, LINENO(n), n->n_col_offset);
    }else {
        n = CHILD(n, 0);
        REQ(n, import_from);
        char from[NAME_MAX_SIZE] = "";
        int i;
        node* ch;
        for(i = 1; 1 ; i ++ ) {
            ch = CHILD(n, i);
            if(TYPE(ch) == DOT) strcat(from, ".");
            else break;
        }

        if(TYPE(ch) == dotted_name) {
            int j = 0;
            for(; j < NCH(ch) ; j ++ ) {
                strcat(from, STR(CHILD(ch, j)));
            }
        }

        i ++;
        REQ(CHILD(n, i), NAME);

        if(TYPE(CHILD(n , ++i)) == STAR) {
            return Importfrom_stmt(from, 1, 0, NULL, NULL, LINENO(n), n->n_col_offset);
        }else {
            if( TYPE(CHILD(n, i)) == LPAR) {
                ch = CHILD(n, ++i);
            }else {
                ch = CHILD(n, i);
            }

            REQ(ch, import_as_names);
            int n_module = (NCH(ch) + 1)/ 2;
            char** modules = (char**) malloc(sizeof(char*) * n_module);
            char** names = (char**) malloc(sizeof(char*) * n_module);
            memset(modules, 0, sizeof(char*) * n_module);
            memset(names, 0, sizeof(char*) * n_module);
                
            int i;
            node* c;
            for(i = 0; i < n_module; i ++ ) {
                c = CHILD(ch, i * 2);
                modules[i] = strdup(STR(CHILD(c, 0)));
                if(NCH(c) == 3) {
                    names[i] = strdup(STR(CHILD(c, 2)));
                }
            }
            return  Importfrom_stmt(from, 0, n_module, modules, names, LINENO(n), n->n_col_offset);
        }

    }
}

static stmt_ty
ast_for_global_stmt(const node* n) {
    const node* ch = n;
    int n_name = NCH(ch) / 2;
    expr_ty* names = (expr_ty* ) malloc ( sizeof( expr_ty) * n_name);
    int i;
    for(i = 1; i < NCH(ch); i += 2) {
        names[i/2] = ast_for_name(CHILD(ch, i));
        if(NULL == names[i/2] ) return NULL;
        if(names[i/2]->kind != Name_kind)  {
            fprintf(stderr, "Global stmt illegal\n");
            return NULL;
        }
    }
    return Global_stmt(n_name, names, LINENO(ch), ch->n_col_offset);
}


static stmt_ty
ast_for_assert_stmt(const node *n ) {
    const node* ch = n;
    expr_ty expression = NULL;
    expr_ty msg = NULL;
    if( NCH(ch) >= 2) {
        expression = ast_for_expr(CHILD(ch, 1));
        if( NULL == expression) return NULL;
    }
    if( NCH(ch) == 4) {
        msg = ast_for_expr(CHILD(ch, 3));
        if( NULL == msg) return NULL;
    }

    return Assert_stmt(expression, msg, LINENO(ch), ch->n_col_offset);
}

static stmt_ty
ast_for_for_stmt(const node* n) {
    expr_ty target, iter;
    target = ast_for_testlist(CHILD(n, 1));
    if(NULL == target) return NULL;

    iter = ast_for_testlist(CHILD(n, 3));
    if(NULL == iter) return NULL;

    stmt_seq * body, *orelse = NULL;
    body = ast_from_node(CHILD(n, 5));
    if(NULL == body) return NULL;
    
    if(NCH(n) == 9)  {
        orelse  = ast_from_node(CHILD(n, 8));
        if(NULL == orelse) return NULL;
    }

    return For_stmt(target, iter, body, orelse, LINENO(n), n->n_col_offset);
}

static stmt_ty
ast_for_while_stmt(const node* n) {
    expr_ty expression;
    stmt_seq* body,* orelse = NULL;

    expression = ast_for_expr(CHILD(n, 1));
    if(NULL == expression) return NULL;

    body = ast_from_node(CHILD(n, 3));
    if(NULL == body) return NULL;

    if(NCH(n) == 7) {
        orelse = ast_from_node(CHILD(n, 6));
        if(NULL == orelse) return NULL;
    }

    return While_stmt(expression, body, orelse, LINENO(n), n->n_col_offset);
}


static stmt_ty
ast_for_with_item(const node *n, stmt_seq* body) {
    expr_ty context_expr, optional_vars = NULL;
    
    context_expr = ast_for_expr(CHILD(n, 0));
    if(NULL == context_expr) return NULL;

    if(NCH(n) == 3) {
        optional_vars = ast_for_expr(CHILD(n, 2));
        if(NULL == optional_vars) return NULL;
    }

    return With_stmt(context_expr, optional_vars, body, LINENO(n), n->n_col_offset);
}


static stmt_ty 
ast_for_with_stmt(const node* n) {

    stmt_seq* body;
    stmt_ty s;
    body = ast_from_node(CHILD(n, NCH(n) - 1));

    if(body == NULL) return NULL;

    int i;
    for(i = NCH(n) - 3; i >= 1; i -= 2) {
        s = ast_for_with_item(CHILD(n, i), body);
        if(NULL == s) return NULL;

        body = stmt_seq_new(1);
        ast_stmt_set(body, s, 0);
    }

    free(body->seqs);
    free(body);
    return s;
}



static stmt_ty
ast_for_try_stmt(const node* n) {
    stmt_seq* body, *orelse = NULL, *final = NULL;
    exception_handler_ty * handlers = NULL;
    int n_clause = NCH(n) / 3  - 1;
    
    body = ast_from_node(CHILD(n, 2));
    if(NULL == body) return NULL;

    if(TYPE(CHILD(n, NCH(n) - 3)) == NAME ) {
        if( strcmp(STR(CHILD(n, NCH(n) - 3)), "finally") == 0) {
            final = ast_from_node(CHILD(n, NCH(n) -1));
            if(NULL == final) return NULL;
            n_clause --;
            if((TYPE(CHILD(n, NCH(n) - 6)) == NAME) && strcmp(STR(CHILD(n, NCH(n) - 3)), "else") == 0) {
                orelse = ast_from_node(CHILD(n, NCH(n) - 6));
                if(NULL == orelse ) return NULL;
                n_clause --;
            }
        }else if( strcmp(STR(CHILD(n, NCH(n) - 3)), "else") == 0) {
            orelse  = ast_from_node(CHILD(n, NCH(n) - 1));
            if(NULL == orelse ) return NULL; 
            n_clause --;
        }
    }
    
    if(n_clause) {
        int i;
        stmt_seq* exp_body = NULL;
        handlers = (exception_handler_ty*) malloc (sizeof( exception_handler_ty) * n_clause);
        for(i = 0; i < n_clause; i ++ ) {
            handlers[i] = ast_for_exception_handler(CHILD(n, i*3 + 3));
            if(NULL == handlers[i]) return NULL;
            exp_body = ast_from_node(CHILD(n, i*3+5));
            if(NULL == exp_body) return NULL;

            handlers[i]->body = exp_body;
        }
    }

    return Try_stmt(body, n_clause, handlers, orelse, final, LINENO(n), n->n_col_offset);
}


static exception_handler_ty
ast_for_exception_handler( const node * n) {
    expr_ty type = NULL, value = NULL;
    stmt_seq* body = NULL;

    if( NCH(n) >= 2) {
        type = ast_for_expr(CHILD(n, 1));
        if(NULL == type) return NULL;
    }

    if( NCH(n) >= 4) {
        value = ast_for_expr(CHILD(n, 3));
        if(NULL == value) return NULL;
    }

    exception_handler_ty e = (exception_handler_ty) malloc ( sizeof(struct exception_handler));
    if( NULL ==  e ) {
        fprintf(stderr, "Memory Error\n");
        exit(-1);
    }
    e->type = type;
    e->value = value;
    e->body = body;
    return e;

}


static stmt_ty
ast_for_if_stmt(const node* n) {
    expr_ty expression;
    stmt_seq* body = NULL;
    stmt_seq* orelse = NULL;

    expression = ast_for_expr(CHILD(n, 1));
    if(NULL == expression) return NULL;

    body = ast_from_node(CHILD(n, 3));
    if(NULL == body ) return NULL;
    
    int n_elif;
    if( TYPE(CHILD(n, NCH(n) - 3)) == NAME && strcmp(STR(CHILD(n, NCH(n) - 3)), "else") == 0) {
        orelse = ast_from_node(CHILD(n, NCH(n) - 1));
        if(NULL == orelse) return NULL;
        n_elif = (NCH(n) - 7) / 4;
    }else {
        n_elif = (NCH(n) - 4) / 4;
    }

    if(n_elif == 0) {
        return If_stmt(expression, body, orelse, LINENO(n), n->n_col_offset);
    }
    else {
        int i;
        expr_ty eltest = NULL;
        stmt_seq* elbody = NULL; 
        
        stmt_ty  elorelse = NULL;
        for(i = 0; i < n_elif; i ++ ) {
            eltest = ast_for_expr(CHILD(n, NCH(n) - i * 4 - 6));         
            if(NULL == eltest) return NULL;
            
            elbody = ast_from_node(CHILD(n, NCH(n) - i * 4 - 4));
            if(NULL == elbody) return NULL;

            elorelse= If_stmt(eltest, elbody, orelse, LINENO(n), n->n_col_offset); 
            if(NULL == elorelse) return NULL;

            orelse = stmt_seq_new(1);
            ast_stmt_set(orelse, elorelse, 0);
        }
        return If_stmt(expression, body, orelse, LINENO(n), n->n_col_offset);
    }
}


static stmt_ty
ast_for_decorated_stmt(const node* n) {
    if( TYPE(CHILD(n, 1)) == classdef)
        return ast_for_classdef_stmt(CHILD(n, 1));
    else
        return ast_for_funcdef_stmt(CHILD(n, 1));
}

static stmt_ty
ast_for_funcdef_stmt(const node* n) {
    arguments_ty args = NULL;
    stmt_seq* body = NULL;
    
    args = ast_for_arguments(CHILD(n, 2));
    if(NULL == args) return NULL;

    body = ast_from_node(CHILD(n, 4));
    if(NULL == body) return NULL;

    return Func_stmt(STR(CHILD(n, 1)), args, body, LINENO(n), n->n_col_offset);
}


static arguments_ty
ast_for_arguments(const node* n) {
    arguments_ty args = NULL;
    
    if(NCH(n) == 3) {
        n = CHILD(n, 1);
        int n_param = 0, n_default = 0;
        int i ;
        int has_vargs = 0;
        int has_kargs = 0; 
        for(i = 0; i < NCH(n); i ++ ) {
            if(TYPE(CHILD(n, i)) == fpdef) n_param ++;
            if(TYPE(CHILD(n, i)) == test) n_default ++;
            if(TYPE(CHILD(n, i)) == STAR) has_vargs = 1;
            if(TYPE(CHILD(n, i)) == DOUBLESTAR) has_kargs = 1;
        }
        n_param -= n_default;
        Parameter** params = NULL;
        Parameter** default_params = NULL;
        
        if(n_param)
            params = (Parameter** ) malloc( sizeof(Parameter*) * n_param);
        if(n_default)
            default_params = (Parameter**) malloc( sizeof( Parameter*) * n_default);

        int j_param = 0;
        int j_default = 0;
        expr_ty vargs = NULL, kargs = NULL;
        node* ch;
        for(i = 0; i < NCH(n); i ++ ) {
            ch = CHILD(n, i);
            if(TYPE( ch ) == fpdef ) {
                Variable * var = (Variable*) malloc ( sizeof(Variable));
                memset(var, 0, sizeof(Variable));
                ch = CHILD(ch, 0);
                var->args = ast_for_name(ch);
                if(NULL == var->args) return NULL;
                
                if( i + 1 >= NCH(n) || (i + 1 < NCH(n) && TYPE( CHILD(n, i + 1)) != EQUAL)) {
                    params[j_param ++] = var;
                }
                else if( i + 2 < NCH(n) && TYPE(CHILD(n, i + 2)) == test) {
                    var->value = ast_for_expr(CHILD(n, i + 2));
                    if(NULL == var->value) return NULL;
                    default_params[j_default++] = var;
                    i += 2;
                }
                else return NULL;
            }
            else if (TYPE(ch) == STAR) {
                if( i + 1 < NCH(n) && TYPE(CHILD(n, i + 1)) == NAME) {
                    vargs = ast_for_name(CHILD(n, i + 1));
                    if(NULL == vargs ) return NULL;
                }else return NULL;
            }
            else if( TYPE(ch) == DOUBLESTAR) {
                if( i + 1 < NCH(n) &&  TYPE(CHILD(n, i + 1)) == NAME) {
                    kargs = ast_for_name(CHILD(n, i + 1));
                    if(NULL == kargs) return NULL;
                }else return NULL;
            }
        }
        args = (arguments_ty) malloc( sizeof(struct arguments) );
        args->n_param = n_param;
        args->params = params;
        args->n_default = n_default;
        args->default_params = default_params;
        args->vargs = vargs;
        args->kargs = kargs;
    }

    return args;
}


static stmt_ty
ast_for_classdef_stmt(const node* n) {
    expr_ty super = NULL;
    stmt_seq* body = NULL;

    if(NCH(n) == 7) {
        super = ast_for_testlist(CHILD(n, 3));
        if(NULL == super) return NULL;
    }
    
    body = ast_from_node(CHILD(n, NCH(n) -1));
    if(NULL == body) return NULL;

    return Class_stmt(STR(CHILD(n, 1)), super, body, LINENO(n), n->n_col_offset);

}
