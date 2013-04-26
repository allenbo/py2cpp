#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsetok.h"
#include "node.h"
#include "mod.h"

#define Py_single_intput 256
#define Py_file_input 257

extern grammar _PyParser_Grammar;
extern void PyNode_ListTree(node* n);
extern stmt_seq* ast_from_node(const node* n);
int main() {
    char* filename = "test.py";
    FILE* fp = fopen(filename, "r");
    perrdetail err_ret;
    int flag = 0;
    node *n = PyParser_ParseFileFlagsEx(fp, filename, &_PyParser_Grammar, Py_file_input, NULL, NULL, &err_ret, &flag);
    stmt_seq* ss = ast_from_node(n);
    printf("#include <iostream>\n#include <vector>\nusing namespace std;\n\nint main(int argc, char** argv) {\n");
    assign_type_to_ast(ss);
    printf("\treturn 0;\n}");
    PyNode_Free(n);
    fclose(fp);
    return 0;
}
