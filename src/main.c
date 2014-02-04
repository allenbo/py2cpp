#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsetok.h"
#include "node.h"
#include "mod.h"
#include "code.h"

#define Py_single_intput 256
#define Py_file_input 257

extern grammar _PyParser_Grammar;
extern void PyNode_ListTree(node* n);
extern stmt_seq* ast_from_node(const node* n);

FILE* fdef = NULL;

int main(int argc, char** argv) {
    char* filename; 
    char* output_filename = "output.cpp";
    if( argc < 2 ) {
      fprintf(stderr, "Please specify the input file\n");
      return 0;
    }

    filename = argv[1];

    if (argc >= 3) {
      output_filename = argv[2];
    }
    else {
      fprintf(stderr, "No output filename specified, use output.cpp by default\n");
    }

    FILE* fp = fopen(filename, "r");
    perrdetail err_ret;
    int flag = 0;
    node *n = PyParser_ParseFileFlagsEx(fp, filename, &_PyParser_Grammar, Py_file_input, NULL, NULL, &err_ret, &flag);
    fclose(fp);
    stmt_seq* ss = ast_from_node(n);
    assign_type_to_ast(ss);
    generate_cpp_code(output_filename, ss);
    //PyNode_Free(n);
    return 0;
}
