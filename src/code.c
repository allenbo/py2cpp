#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "code.h"
#include "symtab.h"
#include "context.h"
#include "util.h"

#include "operator.h"
#include "primary.h"

#include "writer.h"

extern struct type t_unknown;
extern struct type t_char;
extern struct type t_boolean;
extern struct type t_integer;
extern struct type t_float;
extern struct type t_string;



static void gen_cpp_for_stmt(stmt_ty s);
static void gen_cpp_for_funcdef_stmt(stmt_ty s);
static void gen_cpp_for_classdef_stmt(stmt_ty s);
static void gen_cpp_for_return_stmt(stmt_ty s);
static void gen_cpp_for_delete_stmt(stmt_ty s);
static void gen_cpp_for_assign_stmt(stmt_ty s);
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


static void annotate_for_expr(expr_ty e);
static void annotate_for_binop_expr(expr_ty e);
static void annotate_for_boolop_expr(expr_ty e);
static void annotate_for_compare_expr(expr_ty e); 
static void annotate_for_unaryop_expr(expr_ty e);
static void annotate_for_ifexp_expr(expr_ty e);
static void annotate_for_listcomp_expr(expr_ty e);
static void annotate_for_dictcomp_expr(expr_ty e);
static void annotate_for_setcomp_expr(expr_ty e);
static void annotate_for_generator_expr(expr_ty e); //waiting
static void annotate_for_yield_expr(expr_ty e); //waiting
static void annotate_for_lambda_expr(expr_ty e); // waiting
static void annotate_for_call_expr(expr_ty e); // waiting
static void annotate_for_repr_expr(expr_ty e);
static void annotate_for_num_expr(expr_ty e);
static void annotate_for_str_expr(expr_ty e);
static void annotate_for_attribute_expr(expr_ty e);
static void annotate_for_subscript_expr(expr_ty e);
static void annotate_for_name_expr(expr_ty e);
static void annotate_for_list_expr(expr_ty e);
static void annotate_for_tuple_expr(expr_ty e);
static void annotate_for_dict_expr(expr_ty e);
static void annotate_for_set_expr(expr_ty e);


void
generate_cpp_code( char* filename, stmt_seq* ss) {
    if(ss == NULL || ss->size == 0) {
        fprintf(stderr, "stmt seq is not ready\n");
        return ;
    }

    if(NULL == filename){
        filename = "test.cpp";
        fprintf(stderr, "Haven't spectify the output file, Set the filename to test.cpp\n");
    }
    writer_set_filename(filename);
    writer_ready();
    /*
    if((fout = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "Can't open file -- %s\n", filename);
    }
    */

    gen_cpp_for_ast(ss, get_current_symtab());
    close_writer();
    /*
    fclose(fout);
    */
}


void
gen_cpp_for_ast(stmt_seq* ss, symtab_ty s) {
    /* first we need to output the variables */
    if(NULL != s)
      if(get_context()->prefix[0] != 0)
        output_symtab_with_prefix(s, get_context()->prefix);
      else
        output_symtab(s);


    int i, n = ss->size;
    for(i = 0; i < n; i ++ ) {
        gen_cpp_for_stmt(ss->seqs[i]);
    }
}

static void gen_cpp_for_stmt(stmt_ty s){
    switch(s->kind) {
        case Break_kind:
            fprintf(writer_get_file(), "break;\n");
            break;
        case Continue_kind:
            fprintf(writer_get_file(), "continue;\n");
            break;
        case FuncDef_kind:
            return gen_cpp_for_funcdef_stmt(s);
        case ClassDef_kind:
            return gen_cpp_for_classdef_stmt(s);
        case Return_kind:
            return gen_cpp_for_return_stmt(s);
        case Delete_kind:
            return gen_cpp_for_delete_stmt(s);
        case Assign_kind:
            return gen_cpp_for_assign_stmt(s);
        case AugAssign_kind:
            return gen_cpp_for_augassign_stmt(s);
        case Print_kind:
            return gen_cpp_for_print_stmt(s);
        case For_kind:
            return gen_cpp_for_for_stmt(s);
        case While_kind:
            return gen_cpp_for_while_stmt(s);
        case If_kind:
            return gen_cpp_for_if_stmt(s);
        case With_kind:
            return gen_cpp_for_with_stmt(s);
        case Raise_kind:
            return gen_cpp_for_raise_stmt(s);
        case Try_kind:
            return gen_cpp_for_try_stmt(s);
        case Assert_kind:
            return gen_cpp_for_assert_stmt(s);
        case Global_kind:
            return gen_cpp_for_global_stmt(s);
        case Expr_kind:
            return gen_cpp_for_expr_stmt(s);
    }
}

static void annotate_for_expr(expr_ty e){
    switch(e->kind) {
        case BoolOp_kind:
            return annotate_for_boolop_expr(e);
        case BinOp_kind:
            return annotate_for_binop_expr(e);
        case UnaryOp_kind:
            return annotate_for_unaryop_expr(e);
        case Lambda_kind:
            return annotate_for_lambda_expr(e);
        case IfExp_kind:
            return annotate_for_ifexp_expr(e);
        case ListComp_kind:
            return annotate_for_listcomp_expr(e);
        case Dict_kind:
            return annotate_for_dict_expr(e);
        case Set_kind:
            return annotate_for_set_expr(e);
        case SetComp_kind:
            return annotate_for_setcomp_expr(e);
        case DictComp_kind:
            return annotate_for_dictcomp_expr(e);
        case GeneratorExp_kind:
            return annotate_for_generator_expr(e);
        case Yield_kind:
            return annotate_for_yield_expr(e);
        case Compare_kind:
            return annotate_for_compare_expr(e);
        case Call_kind:
            return annotate_for_call_expr(e);
        case Repr_kind:
            return annotate_for_repr_expr(e);
        case Num_kind:
            return annotate_for_num_expr(e);
        case Str_kind:
            return annotate_for_str_expr(e);
        case Attribute_kind:
            return annotate_for_attribute_expr(e);
        case Subscript_kind:
            return annotate_for_subscript_expr(e);
        case Name_kind:
            return annotate_for_name_expr(e);
        case List_kind:
            return annotate_for_list_expr(e);
        case Tuple_kind:
            return annotate_for_tuple_expr(e);
    }
}

static void
gen_cpp_for_funcdef_stmt(stmt_ty s){
    char buf[512] = "";
    type_ty t = lookup_variable(s->funcdef.name);
    stmt_ty def = t->def;
    int i, n = t->ind;
    int from = 0;
    if((get_context())->inclass == 1) {
        from = 1;
        (get_context())->inmember = 1;
    }
    for(i = 0; i < n; i ++ ) {
        if ( t->is_yield ) {
          sprintf(buf, "%s %s(", t->tab[i]->ret->base->name, def->funcdef.name);
        }
        else {
          sprintf(buf, "%s %s(", t->tab[i]->ret->name, def->funcdef.name);
        }
        int j, m = t->tab[i]->n_param;
        arguments_ty args = def->funcdef.args;
        if(from == 1) {
            (get_context())->selfname = args->params[0]->args->name.id;
        }
        for(j = 0; j < m; j++ ){
            args->params[j + from]->args->e_type = t->tab[i]->params[j];
            sprintf(buf + strlen(buf), "%s %s", args->params[j + from]->args->e_type->name,
                    args->params[j + from]->args->name.id);
            if(j != m-1) {
              strcat(buf, ", ");
            }
        }
        strcat(buf, ") {");
        smart_write_buffer(buf);
        symtab_ty st = t->tab[i]->scope;
        if ( ! t->is_yield ) {
          change_symtab(st);
          assign_type_to_ast(def->funcdef.body);
          gen_cpp_for_ast(def->funcdef.body, st);
          smart_write_buffer("}");
          change_symtab_back();
        }
        else {
          strcpy(get_context()->prefix, "stacit");
          gen_cpp_for_ast(def->funcdef.body, st);
          smart_write_buffer("}");
          get_context()->prefix[0] = 0;
        }
        (get_context())->inmember = 0;
    }
}

static void
gen_cpp_for_classdef_stmt(stmt_ty s){
    char buf[512] = "";
    char* name = s->classdef.name;
    expr_ty super = s->classdef.super;

    stmt_seq* body = s->classdef.body;

    type_ty tp = lookup_variable(name);
    sprintf(buf, "class %s {", name);
    smart_write_buffer(buf);

    write_bufferln("public:");

    change_symtab(tp->scope);
    (get_context())->inclass = 1;
    gen_cpp_for_ast(body, tp->scope);
    smart_write_buffer("};");
    (get_context())->inclass = 0;
    change_symtab_back();
}

static void
gen_cpp_for_return_stmt(stmt_ty s){
    expr_ty value = s->ret.value;
    annotate_for_expr(value);

    char buf[512] = "";
    sprintf(buf, "return %s;", value->ann);
    write_bufferln(buf);
}

static void
gen_cpp_for_delete_stmt(stmt_ty s){
    int i, n = s->del.n_target;
    expr_ty* targets = s->del.targets;

    char buf[512];
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(targets[i]);
        if(targets[i]->kind == Subscript_kind) {
            sprintf(buf, "%s->__delitem__(%s);", targets[i]->sub.value->ann, targets[i]->sub.slices[0]->index.value->ann);
        }else if(targets[i]->kind == Attribute_kind) {
            sprintf(buf, "%s->__delattr__(%s);", targets[i]->attribute.value->ann, targets[i]->attribute.attr);
        }else {
            sprintf(buf, "%s->__del__();", targets[i]->ann);
        }
        write_bufferln(buf);
    }
}

static void
gen_cpp_for_assign_stmt(stmt_ty s){
    int i, n = s->assign.n_target;
    expr_ty value = s->assign.value;
    expr_ty * targets = s->assign.targets;

    char buf[512] = "";
    annotate_for_expr(value);
    for(i = 0; i < n ; i ++){
        if(targets[i]->kind == Subscript_kind) {
            (get_context())->setitem = 1;
            (get_context())->itemname = value->ann;
        }
        annotate_for_expr(targets[i]);
        switch(targets[i]->kind) {
            case Tuple_kind:
                break;
            case Subscript_kind:
                 sprintf(buf, "%s;", targets[i]->ann);
                 write_bufferln(buf);
                break;
            default:
                sprintf(buf, "%s = %s;", targets[i]->ann, value->ann);
                 write_bufferln(buf);
        }
    }
    return;
}

static void
gen_cpp_for_augassign_stmt(stmt_ty s){
    expr_ty target = s->augassignstmt.target;
    operator_ty op = s->augassignstmt.op;
    expr_ty value = s->augassignstmt.value;

    annotate_for_expr(value);
    annotate_for_expr(target);
    char* lop = get_augop_fake_literal(op);

    char buf[512] = "";
    sprintf(buf, "%s->%s(%s);", target->ann, lop, value->ann);
    write_bufferln(buf);
}

static void
gen_cpp_for_print_stmt(stmt_ty s){
    expr_ty dest = s->print.dest;
    expr_ty* values = s->print.values;
    int i, n = s->print.n_value;

    char buf[512] = "";
    sprintf(buf, "print(");
    if(dest) {
        annotate_for_expr(dest);
        strcat(buf, dest->ann);
    }else {
        strcat(buf, "NULL");
    }
    strcat(buf, ", ");
    sprintf(buf + strlen(buf), "%d, %d, ", s->print.newline_mark, n);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(values[i]);
        strcat(buf, values[i]->ann);
        if(i != n-1)
            strcat(buf, ", ");
    }
    strcat(buf, ");");
    write_bufferln(buf);
}

static void
gen_cpp_for_for_stmt(stmt_ty s){
    expr_ty target = s->forstmt.target;
    expr_ty iter = s->forstmt.iter;
    stmt_seq* body = s->forstmt.body;
    stmt_seq* orelse = s->forstmt.orelse;


    annotate_for_expr(target);
    annotate_for_expr(iter);

    char buf[512] = "";

    char* ef;
    if(orelse != NULL) {
        ef = newTemp();
        sprintf(buf, "int %s = NOTINT;", ef);
        write_bufferln(buf);
    }
    if(iter->kind != Name_kind) {
        char* tmp = newTemp();
        sprintf(buf, "%s %s = %s;", iter->e_type->name, tmp, iter->ann);
        write_bufferln(buf);
        strcpy(iter->ann, tmp);
    }
    sprintf(buf, "for(; %s->has_next(); ) {", iter->ann);
    write_bufferln(buf);

    if(orelse != NULL) {
        sprintf(buf, "%s = INT", ef);
        write_bufferln(buf);
    }

    /* just deal with the case when target is not a tuple */
    sprintf(buf, "%s %s = %s->next();",
            target->e_type->name, target->ann, iter->ann);
    write_bufferln(buf);

    gen_cpp_for_ast(body, NULL);

    if(NULL != orelse ) {
        sprintf(buf, "%s = NOTINT", ef);
        write_bufferln(buf);
    }

    smart_write_buffer("}");

    if(orelse != NULL){
        sprintf(buf, "if( %s == NOTINT) {", ef);
        write_bufferln(buf);

        gen_cpp_for_ast(orelse, NULL);

        smart_write_buffer("}");
    }
}

static void
gen_cpp_for_while_stmt(stmt_ty s){
    expr_ty test = s->whilestmt.test;
    stmt_seq* body = s->whilestmt.body;
    stmt_seq* orelse = s->whilestmt.orelse;

    annotate_for_expr(test);

    char* ef;
    char buf[512] = "";
    if(orelse != NULL) {
        ef = newTemp();
        sprintf(buf, "int %s = NOTINT;", ef);
        write_bufferln(buf);
    }

    sprintf(buf, "while( %s ) {", test->ann);
    smart_write_buffer(buf);

    if(orelse != NULL) {
        sprintf(buf, "%s = INT", ef);
        write_buffer(buf);
    }
    gen_cpp_for_ast(body, NULL);

    if(NULL != orelse ) {
        sprintf(buf, "%s = NOTINT", ef);
        write_bufferln(buf);
    }

    smart_write_buffer("}");

    if(orelse != NULL){
        sprintf(buf, "if( %s == NOTINT) {", ef);
        write_bufferln(buf);

        gen_cpp_for_ast(orelse, NULL);

        smart_write_buffer("}");
    }
}

static void
gen_cpp_for_if_stmt(stmt_ty s){
    expr_ty test = s->ifstmt.test;
    stmt_seq* body = s->ifstmt.body;
    stmt_seq* orelse = s->ifstmt.orelse;

    annotate_for_expr(test);
    char buf[512] = "";
    sprintf(buf, "if(%s) {", test->ann);
    smart_write_buffer(buf);

    gen_cpp_for_ast(body, NULL);
    smart_write_buffer("}");

    if(orelse != NULL) {
        smart_write_buffer("else {");

        gen_cpp_for_ast(orelse, NULL);
        smart_write_buffer("}");
    }
}

static void
gen_cpp_for_with_stmt(stmt_ty s){
}

static void
gen_cpp_for_raise_stmt(stmt_ty s){
}

static void
gen_cpp_for_try_stmt(stmt_ty s){
}

static void
gen_cpp_for_assert_stmt(stmt_ty s){
}

static void
gen_cpp_for_global_stmt(stmt_ty s){

}

static void
gen_cpp_for_expr_stmt(stmt_ty s){
    annotate_for_expr(s->exprstmt.value);
    char buf[512] = "";
    sprintf(buf, "%s;", s->exprstmt.value->ann);
    write_bufferln(buf);
}

static void
annotate_for_binop_expr(expr_ty e){
    expr_ty left = e->binop.left;
    expr_ty right = e->binop.right;
    operator_ty op = e->binop.op;

    annotate_for_expr(left);
    annotate_for_expr(right);
    
    char* literal = NULL;

    // binary operation can convert to c++ language directly
    // it's exceptional
    if (binop_in_exception_list(left, right, op)) {
      literal = get_binop_true_literal(op);
      if ( left->kind == BinOp_kind && !is_precedent(left->binop.op, op) )  {
        sprintf(e->ann, "(%s) %s ", left->ann, literal);
      }
      else {
        sprintf(e->ann, "%s %s ", left->ann, literal);
      }

      if( right->kind == BinOp_kind && !is_precedent(right->binop.op, op)) {
        sprintf(e->ann + strlen(e->ann), "(%s)", right->ann);
      }
      else {
        sprintf(e->ann + strlen(e->ann), "%s", right->ann);
      }
    }

    else if ( is_primary_type(left->e_type ) && is_primary_type(right->e_type)){
      sprintf(e->ann, "%s", primary_type_with_lib(left, op, right));
    }
    else{
      literal = get_binop_fake_literal(op);
      sprintf(e->ann, "(%s)->%s(%s)", left->ann, literal, right->ann);
    }
}
static void
annotate_for_boolop_expr(expr_ty e){
    expr_ty* values = e->boolop.values;
    int i, n = e->boolop.n_value;
    boolop_ty op = e->boolop.op;
    
    char buf[512] = "";

    if ( values[0]->e_type == &t_boolean ) {
      for(i = 0; i < n; i ++ ) {
        annotate_for_expr(values[i]);
        sprintf(buf+strlen(buf), "%s", values[i]->ann);

        if ( i != n - 1 ) {
          sprintf(buf + strlen(buf),  " %s ", get_boolop_literal(op) );
        }
      }

      strcpy(e->ann, buf);
    }
    else {
      char* tmp = newTemp();
      strcpy(e->ann, tmp);

      sprintf(buf, "%s %s;", values[0]->e_type->name, tmp);
      write_bufferln(buf);

      buf[0] = '\0';
      for(i = 0; i < n; i ++ ) {
          annotate_for_expr(values[i]);
          sprintf(buf + strlen(buf), "(%s = %s) ", tmp, values[i]->ann);
          if(i != n -1) {
              sprintf(buf + strlen(buf) , " %s ", get_boolop_literal(op));
          }
      }
      write_bufferln(buf);
    }

}
static void
annotate_for_unaryop_expr(expr_ty e){
    expr_ty operand = e->unaryop.operand;
    unaryop_ty op = e->unaryop.op;

    char* literal = NULL;
    if ( is_primary_type(operand->e_type) ) {
      literal = get_unaryop_true_literal(op);
    }
    else {
      literal = get_unaryop_fake_literal(op);
    }
    annotate_for_expr(operand);
    if(literal[0] == '!') {
        sprintf(e->ann, "! %s", operand->ann);
    }else {
        if ( is_primary_type(operand->e_type) ) {
          if ( operand->kind == BinOp_kind ) {
            sprintf(e->ann, "%s (%s)", literal, operand->ann);
          }
          else {
            sprintf(e->ann, "%s %s", literal, operand->ann);
          }
        }
        else {
          sprintf(e->ann, "(%s)->%s()", operand->ann, literal);
        }
    }
}
static void
annotate_for_lambda_expr(expr_ty e){
}
static void
annotate_for_ifexp_expr(expr_ty e){
    expr_ty test = e->ifexp.test;
    expr_ty body = e->ifexp.body;
    expr_ty orelse = e->ifexp.orelse;

    annotate_for_expr(test);
    annotate_for_expr(body);
    annotate_for_expr(orelse);

    sprintf(e->ann, "%s ? %s : %s", test->ann, body->ann, orelse->ann);
}
static void
annotate_for_listcomp_expr(expr_ty e){
    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    char* ann = search_hashtable((get_context())->ht,  scope_name);
    strcpy(e->ann, ann);

    type_ty tp = lookup_variable(scope_name);

    char line[512] = "";
    new_line();
    smart_write_buffer("{");


    output_symtab(tp->scope);

    expr_ty elt = e->listcomp.elt;
    sprintf(line, "%s = make_shared< pylist< %s > >(0);", ann, elt->e_type->name);
    write_bufferln(line);
    int i, n = e->listcomp.n_com;
    comprehension_ty* coms = e->listcomp.generators;

    annotate_for_expr(elt);

    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        annotate_for_expr(target);
        annotate_for_expr(iter);
        if(iter->kind != Name_kind) {
            char* tmp = newTemp();
            sprintf(line, "%s %s = %s;", iter->e_type->name, tmp, iter->ann);
            write_buffer(line);
            strcpy(iter->ann, tmp);
        }
        for(j = 0; j < m; j ++) {
            annotate_for_expr(tests[j]);
        }
    }

    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        new_line();
        sprintf(line, "for(; %s->has_next();) {", iter->ann);
        smart_write_buffer(line);

        sprintf(line, "%s = %s->next();",  target->ann, iter->ann);
        write_bufferln(line);

        for(j = 0; j < m; j++ ) {
            sprintf(line, "if (%s)", tests[j]->ann);
            write_bufferln(line);
            incr_indent();
        }
    }

    sprintf(line, "%s->add(%s);", ann, elt->ann);
    write_bufferln(line);
    for(i = 0;i < n; i ++ ) {
      int j, m = coms[i]->n_test;
      for(j = 0; j < m ; j ++ ) {
        decr_indent();
      }
      smart_write_buffer("}");
    }
    smart_write_buffer("}");

}
static void
annotate_for_dict_expr(expr_ty e){
    int i, n = e->dict.n_key;
    expr_ty* keys = e->dict.keys;
    expr_ty* values = e->dict.values;

    sprintf(e->ann, "make_shared< pydict<%s, %s > >( %d, ",
            e->e_type->kbase->name,e->e_type->vbase->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(values[i]);
        annotate_for_expr(keys[i]);

        strcat(e->ann, keys[i]->ann);
        strcat(e->ann, ", ");
        strcat(e->ann, values[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");

}
static void
annotate_for_set_expr(expr_ty e){
    int i, n = e->set.n_elt;
    expr_ty* elts = e->set.elts;

    sprintf(e->ann, "make_shared< pyset< %s > >( %d, ", e->e_type->base->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
static void
annotate_for_dictcomp_expr(expr_ty e){
  char scope_name[128] = "";
  sprintf(scope_name, "comp_%p", e);
  char* ann = search_hashtable((get_context())->ht, scope_name);
  strcpy(e->ann, ann);
  
  type_ty tp = lookup_variable(scope_name);

  new_line();
  smart_write_buffer("{");

  output_symtab(tp->scope);

  expr_ty key = e->dictcomp.key;
  expr_ty value = e->dictcomp.value;

  char line[512] = "";
  sprintf(line, "%s = make_shared< pydict< %s, %s > > (0);", ann, key->e_type->name, value->e_type->name);
  write_bufferln(line);


  int i, n = e->dictcomp.n_com;
  comprehension_ty * coms = e->dictcomp.generators;

  annotate_for_expr(key);
  annotate_for_expr(value);

  
  for(i = 0; i< n; i ++ ) {
    comprehension_ty com = coms[i];
    expr_ty target = com->target;
    expr_ty iter = com->iter;

    int j, m = com->n_test;

    expr_ty * tests = com->tests;

    annotate_for_expr(target);
    annotate_for_expr(iter);

    if ( iter->kind != Name_kind ) {
      char* tmp = newTemp();
      sprintf(line, "%s %s = %s;", iter->e_type->name, tmp, iter->ann);
      write_bufferln(line);
      strcpy(iter->ann, tmp);
    }

    for(j = 0; j < m; j ++ ) {
      annotate_for_expr(tests[j]);
    }
  }

  for(i = 0; i < n ; i ++ ) {
    comprehension_ty com = coms[i];
    expr_ty target = com->target;
    expr_ty iter = com->iter;
    int j , m = com->n_test;
    expr_ty * tests = com->tests;

    sprintf(line, "for(; %s->has_next(); ) {", iter->ann);
    smart_write_buffer(line);

    sprintf(line, "%s = %s->next();", target->ann, iter->ann);
    write_bufferln(line);

    if ( target->e_type->kind == TUPLE_KIND ) {
      sprintf(line, "%s = %s->first();",target->tuple.elts[0]->name.id, target->ann);
      write_bufferln(line);
      sprintf(line, "%s = %s->second();", target->tuple.elts[1]->name.id, target->ann);
      write_bufferln(line);
    }

    for(j = 0; j < m; j ++ ) {
      sprintf(line, "if ( %s )", tests[j]->ann);
      write_bufferln(line);
      incr_indent();
    }
  }

  sprintf(line, "%s->insert(%s, %s);", ann, key->ann, value->ann);
  write_bufferln(line);

  for(i = 0; i < n; i ++ ) {
    int j, m = coms[i]->n_test;
    for(j = 0; j < m; j ++ ) {
      decr_indent();
    }
    smart_write_buffer("}");
  }
  smart_write_buffer("}");

}
static void
annotate_for_setcomp_expr(expr_ty e){
    char scope_name[128] = "";
    sprintf(scope_name, "comp_%p", e);
    char* ann = search_hashtable((get_context())->ht,  scope_name);
    strcpy(e->ann, ann);

    type_ty tp = lookup_variable(scope_name);

    char line[512] = "";
    new_line();
    smart_write_buffer("{");

    output_symtab(tp->scope);

    expr_ty elt = e->setcomp.elt;
    sprintf(line, "%s = make_shared< pyset< %s > >(0);", ann, elt->e_type->name);
    write_bufferln(line);
    int i, n = e->setcomp.n_com;
    comprehension_ty* coms = e->setcomp.generators;

    annotate_for_expr(elt);


    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        annotate_for_expr(target);
        annotate_for_expr(iter);
        if(iter->kind != Name_kind) {
            char* tmp = newTemp();
            sprintf(line, "%s %s = %s;", iter->e_type->name, tmp, iter->ann);
            write_bufferln(line);
            strcpy(iter->ann, tmp);
        }
        for(j = 0; j < m; j ++) {
            annotate_for_expr(tests[j]);
        }
    }

    for(i = 0; i < n; i ++ ) {
        comprehension_ty com = coms[i];
        expr_ty target = com->target;
        expr_ty iter = com->iter;
        int j, m = com->n_test;
        expr_ty* tests = com->tests;

        sprintf(line, "for(; %s->has_next();) {", iter->ann);
        smart_write_buffer(line);

        sprintf(line, "%s = %s->next();",  target->ann, iter->ann);
        write_bufferln(line);

        for(j = 0; j < m; j++ ) {
            sprintf(line, "if (%s)", tests[j]->ann);
            write_bufferln(line);
            incr_indent();
        }
    }

    sprintf(line, "%s->append(%s);", ann, elt->ann);
    write_bufferln(line);
    for (i = 0; i < n ; i ++ ) {
      int j, m = coms[i]->n_test;
      for(j = 0;j < m; j ++ ) {
        decr_indent();
      }
      smart_write_buffer("}");
    }
    smart_write_buffer("}");

}
static void
annotate_for_generator_expr(expr_ty e){
  char scope_name[128] = "";
  sprintf(scope_name, "comp_%p", e);

  char* ann = search_hashtable((get_context())->ht, scope_name);
  strcpy(e->ann, ann);

  char line[512] = "";

  type_ty tp = lookup_variable(scope_name);
  expr_ty elt =e->generatorexp.elt;
  comprehension_ty* coms = e->listcomp.generators;
  
  annotate_for_expr(elt);

  int i, n = e->listcomp.n_com;
  for(i = 0; i < n; i ++ ) {
    comprehension_ty com = coms[i];
    expr_ty target = com->target;
    expr_ty iter = com->iter;
    annotate_for_expr(target);
    annotate_for_expr(iter);

    if (iter->kind != Name_kind) {
      char* tmp = newTemp();
      sprintf(line, "%s %s = %s;", iter->e_type->name, tmp, iter->ann);
      write_bufferln(line);
      strcpy(iter->ann, tmp);
    }
  }
  sprintf(line, "pygenerator< %s, %s, %s>", coms[0]->iter->e_type->name, e->e_type->base->name, coms[0]->iter->e_type->base->name);
  sprintf(e->ann, "make_shared< %s > ( ([] (%s %s) -> %s { return %s; }) , %s)",  line, coms[0]->target->e_type->name,
                                                coms[0]->target->ann, e->e_type->base->name, elt->ann, coms[0]->iter->ann);
}
static void
annotate_for_yield_expr(expr_ty e){
  annotate_for_expr(e->yield.value);
  char buf[512] = "";

  sprintf(buf, "YIELD(%s)", e->yield.value->ann);
  write_bufferln(buf);
}
static void
annotate_for_compare_expr(expr_ty e){
    expr_ty left = e->compare.left;
    expr_ty* comps = e->compare.comparators;
    int i, n = e->compare.n_comparator;
    compop_ty* ops = e->compare.ops;

    char* op;
    char* prev;
    char* p;
    annotate_for_expr(left);

    for(i = 0; i < n; i ++ ) {
      if(i == 0) {
          prev = left->ann;
          p = comps[i]->ann;
      }else {
          prev = comps[i-1]->ann;
          p = comps[i]->ann;
      }

      annotate_for_expr(comps[i]);
      char tmp[128];
      
      if ( compare_in_exception_list(left, comps[i], ops[i] ) ) {
        op = get_cmpop_true_literal(ops[i]);
        sprintf(tmp, "%s %s %s", prev, op, p);
      }
      else {
        op = get_cmpop_fake_literal(ops[i]);

        if ( ops[i] == In ) {
            sprintf(tmp, "%s->%s(%s)", p, op, prev);
        }
        else if(ops[i] == NotIn) {
            sprintf(tmp, "!%s->%s(%s)", p, op, prev);
        }else
            sprintf(tmp, "%s->%s(%s)", prev, op, p);

      }

      strcat(e->ann, tmp);

      if(i != n-1) {
          strcat(e->ann, " && ");
      }
    }
}



static void
annotate_for_call_expr(expr_ty e){
    expr_ty func = e->call.func;
    Parameter * args = e->call.args;
    int i, n = e->call.n_arg;
    expr_ty varg = e->call.varg;
    expr_ty karg = e->call.karg;

    annotate_for_expr(func);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(args[i].args);
    }

    if(func->e_type->kind ==  CLASS_KIND) {
        sprintf(e->ann, "new %s(", func->ann);
    }else if (func->e_type->kind != FUNCTION_KIND &&
            func->e_type->kind != LAMBDA_KIND) {
        sprintf(e->ann, "DLambda(%s)(", func->ann);
    }else {
        sprintf(e->ann, "%s(", func->ann);
    }

    for(i = 0;i < n; i ++ ) {
        strcat(e->ann, args[i].args->ann);
        if(i != n-1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, ")");
}
static void
annotate_for_repr_expr(expr_ty e){
}
static void
annotate_for_num_expr(expr_ty e){
    if(e->num.kind == INTEGER) {
        sprintf(e->ann, "%d", e->num.ivalue);
    }else {
        sprintf(e->ann, "%f", e->num.fvalue);
    }
}
static void
annotate_for_str_expr(expr_ty e){
    sprintf(e->ann, "\"%s\"", e->str.s);
}
static void
annotate_for_attribute_expr(expr_ty e){
    annotate_for_expr(e->attribute.value);
    sprintf(e->ann, "%s->%s", e->attribute.value->ann, e->attribute.attr);
}
static void
annotate_for_subscript_expr(expr_ty e){
    expr_ty value = e->sub.value;
    int i, n = e->sub.n_slice;
    slice_ty * slices = e->sub.slices;

    if((get_context())->setitem) {
        (get_context())->setitem = 0;
        annotate_for_expr(value);
        /* Some case there are multiple subscript in python
         * but we don't consider it here
         * So what we are gonna do is we're gonna take n equals to 1
         */
        slice_ty slice = slices[0];
        if(slice->kind == Index_kind) {
            annotate_for_expr(slice->index.value);
            sprintf(e->ann, "(%s)->__setitem__( %s , %s)", value->ann,
                    slice->index.value->ann, (get_context())->itemname);
        }else if(slice->kind == Slice_kind) {
            char* lower = "NULL";
            char* upper = "NULL";
            char* step = "NULL";

            expr_ty l = slice->slice.lower;
            expr_ty u = slice->slice.upper;
            expr_ty s = slice->slice.step;

            if(NULL != l ) {
                annotate_for_expr(l);
                lower = l->ann;
            }
            if(NULL != u ) {
                annotate_for_expr(u);
                upper = u->ann;
            }

            sprintf(e->ann, "(%s)->__setslice__( %s , %s, %s)", value->ann,
                    lower, upper, (get_context())->itemname);
        }
    }else {
        annotate_for_expr(value);
        slice_ty slice = slices[0];
        if(slice->kind == Index_kind) {
            annotate_for_expr(slice->index.value);
            sprintf(e->ann, "(%s)->__getitem__( %s )", value->ann,
                    slice->index.value->ann);
        }else if(slice->kind == Slice_kind) {
            char* lower = "NULL";
            char* upper = "NULL";
            char* step = "NULL";

            expr_ty l = slice->slice.lower;
            expr_ty u = slice->slice.upper;
            expr_ty s = slice->slice.step;

            if(NULL != l ) {
                annotate_for_expr(l);
                lower = l->ann;
            }
            if(NULL != u ) {
                annotate_for_expr(u);
                upper = u->ann;
            }

            sprintf(e->ann, "(%s)->__getslice__( %s , %s)", value->ann,
                    lower, upper);
        }
    }
}
static void
annotate_for_name_expr(expr_ty e){
    if((get_context())->inmember == 1 && strcmp(e->name.id, (get_context())->selfname) == 0 ) {
        strcpy(e->ann, "this");
    }
    else {
        strcpy(e->ann, e->name.id);
    }
}
static void
annotate_for_list_expr(expr_ty e){
    int i, n = e->list.n_elt;
    expr_ty* elts = e->list.elts;

    sprintf(e->ann, "make_shared< pylist< %s > >( %d, ", e->e_type->base->name, n);
    for(i = 0; i < n; i ++ ){
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
static void
annotate_for_tuple_expr(expr_ty e){
    int i, n = e->tuple.n_elt;
    expr_ty * elts = e->tuple.elts;

    sprintf(e->ann, "make_shared< pytuple > ( %d, ", n);
    for(i = 0; i < n; i ++ ) {
        annotate_for_expr(elts[i]);
        strcat(e->ann, elts[i]->ann);
        if(i != n -1)
            strcat(e->ann, ", ");
    }
    strcat(e->ann, " )");
}
