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
static void gen_cpp_for_asign_stmt(stmt_ty s);
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
        fprihtf(stderr, "Can't open file -- %s\n", filename);
    }

    gen_cpp_for_ast(ss);

    fclose(fout);
}


static void
gen_cpp_for_ast(stmt_seq* ss) {
     
}
