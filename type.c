#include "type.h"
#include "symtab.h"

struct type t_integer = {INTEGER_KIND, 4};
struct type t_float = {FLOAT_KIND, 4};



static void assign_type_to_stmt(stmt_ty s);
static void assign_type_to_expr(expr_ty e);


static void
assign_type_to_stmt(stmt_ty s) {
    int i;
    switch(s->kind) {
        case FuncDef_kind:
/*            char funcname[128] = {0};
            strcpy(funcname, s->funcdef.name);

            arguments_ty args = s->fundef->args;
            for(i = 0; i < n_param; i ++ ) {
                expr_ty e = args->params[i]->value;
                assign_type_to_expr(e);
                switch(e->e_type->kind) {
                    case CAHR_KIND:
                        strcat(funcname, "_c"); break;
                    case SHORT_KIND:
                        strcat(funcname, "_s"); break;
                    case INTEGER_KIND:
                        strcat(funcname, "_i"); break;
                    case FLOAT_KIND:
                        strcat(funcname, "_f"); break;
                    case DOUBLE_KIND:
                        strcat(funcname, "_d"); break;
                    case ARRAY_KIND:
                        strcat(funcname, "_a"); break;
                    case STRUCT_KIND:
                        strcat(funcname, "_t"); break;
                    case POINTER_KIND:
                        strcat(funcname, "_p"); break;
                    case FUNCTION_KIND:
                        strcat(funcname, "_n"); break;
                }
            }

            for(i = 0; i < n_default; i ++ ) {
                expr_ty e = args->default_params[i]->value;
                assign_type_to_expr(e);
                switch(e->e_type->kind) {
                    case CAHR_KIND:
                        strcat(funcname, "_c"); break;
                    case SHORT_KIND:
                        strcat(funcname, "_s"); break;
                    case INTEGER_KIND:
                        strcat(funcname, "_i"); break;
                    case FLOAT_KIND:
                        strcat(funcname, "_f"); break;
                    case DOUBLE_KIND:
                        strcat(funcname, "_d"); break;
                    case ARRAY_KIND:
                        strcat(funcname, "_a"); break;
                    case STRUCT_KIND:
                        strcat(funcname, "_t"); break;
                    case POINTER_KIND:
                        strcat(funcname, "_p"); break;
                    case FUNCTION_KIND:
                        strcat(funcname, "_n"); break; }
            }
            
            insert_func_to_current_table(funcname);
            for(i = 0; i < n_default_params; i ++ ) {
                expr_ty e = args->default_params[i]->value;
                char name[128] = {0};
                sprintf(name, "%s_%s", name, funcname);
                insert_to_current_table(name, e->e_type, VARIABLE_KIND);
            }
            enter_new_scope_func(s->funcdef);
            for(i = 0; i < n_default_params; i ++ ) {
                expr_ty e = args->default_params[i]->value;
                char name[128] = {0};
                sprintf(name, "%s_%s", name, funcname);
                insert_to_current_table(name, e->e_type, VARIABLE_KIND);
            }
        
            assign_type_to_ast(s->funcdef->body);
            exit_scope(); 
*/
            break;
        case ClassDef_kind:
            break;
        case Return_kind:
            break;
        case Delete_kind:
            break;
        case Assign_kind:
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
                    break;
                case DECIMAL:
                    e->e_type = &t_float;
                    break;
            }
            break;
        case Str_kind:
            break;
        case Attribute_kind:
            break;
        case Subscript_kind:
            break;
        case Name_kind:
            break;
        case List_kind:
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

