#ifndef _PYTH0N_AST_H_
#define _PYTHON_AST_H_
#include "mod.h"


stmt_seq* stmt_seq_new(int n);
void ast_stmt_set(stmt_seq* ss, stmt_ty s, int i);


stmt_ty stmt_new();
stmt_ty stmt_new_full();

expr_ty expr_new();
expr_ty expr_new_full();


stmt_ty Assign_stmt( int , expr_ty* , expr_ty , int , int );
stmt_ty Augassign_stmt(expr_ty , expr_ty , operator_ty , int , int );
stmt_ty Expr_stmt(expr_ty , int , int );
stmt_ty Print_stmt(expr_ty , int , expr_ty* , int , int , int );
stmt_ty Del_stmt(int , expr_ty* , int , int );
stmt_ty Raise_stmt(expr_ty , expr_ty , expr_ty , int , int );
stmt_ty Global_stmt(int , expr_ty* , int , int );
stmt_ty Return_stmt(expr_ty , int , int );
stmt_ty Assert_stmt(expr_ty , expr_ty , int , int );
stmt_ty For_stmt(expr_ty , expr_ty , stmt_seq* , stmt_seq* , int , int );
stmt_ty Import_stmt(int , char** , char** , int , int );
stmt_ty Importfrom_stmt(char* , int , int , char** , char** , int , int );
stmt_ty While_stmt(expr_ty , stmt_seq* , stmt_seq* , int , int );
stmt_ty With_stmt(expr_ty , expr_ty , stmt_seq* , int , int );
stmt_ty Try_stmt(stmt_seq* , int , exception_handler_ty* , stmt_seq* , stmt_seq* , int , int );
stmt_ty If_stmt(expr_ty , stmt_seq* , stmt_seq* , int , int );
stmt_ty Class_stmt(char* , expr_ty , stmt_seq* , int , int );
stmt_ty Func_stmt(char* , arguments_ty , stmt_seq* body , int , int );


expr_ty Tuple_expr(int , expr_ty * , int , int );
expr_ty Ifexp_expr(expr_ty , expr_ty , expr_ty , int , int );
expr_ty Listcomp_expr(expr_ty , int , comprehension_ty* , int , int );
expr_ty Dictcomp_expr(expr_ty , expr_ty , int , comprehension_ty * , int , int );
expr_ty Lambda_expr(arguments_ty , expr_ty , int , int );
expr_ty Boolop_expr(boolop_ty , int , expr_ty* , int , int );
expr_ty Unaryop_expr(unaryop_ty , expr_ty , int , int );
expr_ty Yield_expr(expr_ty , int , int );
expr_ty Compare_expr(expr_ty , int , compop_ty* , expr_ty* , int , int );
expr_ty Binop_expr(expr_ty , operator_ty , expr_ty , int , int );
expr_ty List_expr(int , expr_ty * , int , int );
expr_ty Dict_expr(int , expr_ty* , expr_ty* , int , int );
expr_ty Set_expr(int , expr_ty* , int , int );
expr_ty Attribute_expr(expr_ty , char* , int , int );
expr_ty Subscript_expr(expr_ty , int , slice_ty * , int , int );
expr_ty Call_expr(expr_ty , int , Parameter* , expr_ty , expr_ty , int , int );

#endif
