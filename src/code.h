#ifndef _CODE_H_
#define _CODE_H_

#include "mod.h"
#include "symtab.h"
void generete_cpp_code(char* filename, stmt_seq* ss);
void gen_cpp_for_ast(stmt_seq* ss, symtab_ty s);
#endif
