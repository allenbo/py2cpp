#include "symtab.h"
#include <stdio.h>
static symtab_ty global_table = NULL;
static symtab_ty cur_table = NULL;


symtab_ty get_current_symtab() { return cur_table; }
symtab_ty get_global_table() { return global_table; }


