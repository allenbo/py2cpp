#include <stdio.h>
#include <string.h>
#include "operator.h"

int precedence[] = {
  0, // Or
  1, // And
  2, // BitOr
  3, // BitXor
  4, // BitAnd
  8, 8, // Add, Sub
  9,  9,  9, 9, // Mult, Div, Mod, FloorDiv
  10000, // Pow
  7, 7, // LShift, RShift
  10, 10, 10, 10, // unaryop
  5, 5, // Eq, NotEq
  6, 6, 6, 6, // Lt, LtE, Gt, GtE
  10000, 10000, 10000, 10000
};

char* get_binop_fake_literal(operator_ty op) {
    switch(op) {
        case Add: return "__add__";
        case Sub: return "__sub__";
        case Mult: return "__mul__";
        case Div:  return "__div__";
        case Mod:  return "__mod__";
        case Pow: return "__pow__";
        case LShift: return "__lshift__";
        case RShift: return "__rshift__";
        case BitOr:  return "__or__";
        case BitXor:return "__xor__";
        case BitAnd:  return "__and__";
    }
    return NULL;
}


char* get_binop_true_literal(operator_ty op ) {
    switch(op) {
      case Add: return "+";
      case Sub: return "-";
      case Mult: return "*";
      case Div:  return "/";
      case Mod:  return "%";
      case LShift: return "<<";
      case RShift: return ">>";
      case BitOr:  return "|";
      case BitXor:return "^";
      case BitAnd:  return "&";
  }
  return NULL;
}


char* get_augop_true_literal(operator_ty op) {
    switch(op) {
        case Add: return "+=";
        case Sub: return "-=";
        case Mult: return "*=";
        case Div:  return "/=";
        case Mod:  return "%=";
        case LShift: return "<<=";
        case RShift: return ">>=";
        case BitOr:  return "|=";
        case BitXor:return "^=";
        case BitAnd:  return "&=";
    }
    return NULL;
}

char* get_augop_fake_literal(operator_ty op) {
    switch(op) {
        case Add: return "__iadd__";
        case Sub: return "__isub__";
        case Mult: return "__imul__";
        case Div:  return "__idiv__";
        case Mod:  return "__imod__";
        case Pow: return "__ipow__";
        case LShift: return "__ilshift__";
        case RShift: return "__irshift__";
        case BitOr:  return "__ior__";
        case BitXor:return "__ixor__";
        case BitAnd:  return "__iand__";
    }
    return NULL;
}

char* get_cmpop_true_literal(compop_ty op) {
    switch(op) {
        case Eq: return "==";
        case NotEq: return "!=";
        case Lt: return "<";
        case LtE: return "<=";
        case Gt: return ">";
        case GtE: return ">=";
        case Is: return "==";
        case IsNot: return "!=";
    }
    return NULL;
}
char* get_cmpop_fake_literal(compop_ty op) {
    switch(op) {
        case Eq: return "__eq__";
        case NotEq: return "__ne__";
        case Lt: return "__lt__";
        case LtE: return "__le__";
        case Gt: return "__gt__";
        case GtE: return "__ge__";
        case Is: return "==";
        case IsNot: return "!=";
        case In: return "__contains__";
        case NotIn: return "__contains__";
    }
    return NULL;
}

char* get_unaryop_true_literal(unaryop_ty op) {
    switch(op) {
        case Invert: return "~";
        case UAdd: return "+";
        case USub: return "-";
        case Not: return "!";
    }
    return NULL;
}

char* get_unaryop_fake_literal(unaryop_ty op) {
    switch(op) {
        case Invert: return "__invert__";
        case UAdd: return "__pos__";
        case USub: return "__neg__";
        case Not: return "!";
    }
}

char* get_boolop_literal(boolop_ty op) {
    switch(op) {
        case And: return "&&";
        case Or: return "||";
    }
}

int is_precedent(operator_ty f, operator_ty s) {
  return ! precedence[f] < precedence[s];  
}
