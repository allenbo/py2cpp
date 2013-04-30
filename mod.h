#ifndef _MOD_H_
#define _MOD_H_

#define NAME_MAX_SIZE 128

typedef struct type* type_ty;

typedef struct _stmt * stmt_ty;

typedef struct _expr* expr_ty;

typedef struct Variable Variable;

typedef Variable Parameter;

typedef struct stmt_seq stmt_seq;

typedef struct arguments * arguments_ty;

typedef struct exception_handler * exception_handler_ty;

typedef struct slice * slice_ty;

typedef enum _boolop { And = 1, Or = 2 } boolop_ty;

typedef enum _expr_context { Load = 1, Store = 2, Del = 3, AugLoad = 4, AugStore = 5,
    Param = 6} expr_context_ty;

typedef enum _operator { Add = 1, Sub = 2, Mult = 3, Div = 4, Mod = 5, Pow = 6, LShift =7,
    RShift =8, BitOr = 9, BitXor = 10, BitAnd= 11, FloorDiv = 12} operator_ty;

typedef enum _unaryop { Invert = 1, Not = 2, UAdd = 3, USub = 4 } unaryop_ty;

typedef enum _compop { Eq = 1, NotEq = 2 ,Lt = 3, LtE = 4, Gt = 5, GtE = 6, Is = 7, IsNot = 8,
    In = 9, NotIn = 10 } compop_ty;

typedef struct comprehension * comprehension_ty;

enum stmt_kind { FuncDef_kind = 1, ClassDef_kind = 2, Return_kind = 3, Delete_kind = 4,
                Assign_kind = 5, AugAssign_kind = 6, Print_kind = 7, For_kind = 8, While_kind = 9,
                If_kind = 10, With_kind = 11, Raise_kind = 12, Try_kind = 13, 
                Assert_kind = 15, Import_kind = 16, ImportFrom_kind = 18, 
                Exec_kind = 18, Global_kind = 19, Expr_kind = 20, Pass_kind = 21, 
                Break_kind = 22, Continue_kind = 23};
enum Accessibility { Private = 1, Public = 2, Protected = 3 };

struct Variable {
    expr_ty args;
    int type; /* index to type table */
    expr_ty value;
};


struct stmt_seq {
    int size;
    stmt_ty*  seqs;
};


struct arguments {
    int n_param;
    Parameter** params;
    int n_default;
    Parameter** default_params;
    expr_ty vargs;
    expr_ty kargs;
};


typedef struct FuncDef {
    char name[NAME_MAX_SIZE];
    arguments_ty args; 
    stmt_seq * body;
    
    int  is_class_member;
    enum Accessibility acc;
    char fullname[NAME_MAX_SIZE];
}FuncDef;


typedef struct ClassDef {
    char name[NAME_MAX_SIZE];
    int base; /* Only support a single base class */
    expr_ty super;
    stmt_seq * body;
}ClassDef;


typedef struct Return {
    expr_ty value;
}Return;


typedef struct Delete {
    int n_target;
    expr_ty* targets;
}Delete;

typedef struct Assign {
    int n_target;
    expr_ty* targets;
    expr_ty value;
}Assign;


typedef struct AugAssign {
    expr_ty target;
    operator_ty op;
    expr_ty value;
}AugAssign;

typedef struct Print{
    expr_ty dest;
    int  n_value;
    expr_ty * values;
    int newline_mark;
}Print;


typedef struct For {
    expr_ty target;
    expr_ty iter;
    stmt_seq * body;
    stmt_seq * orelse;
}For;


typedef struct While {
    expr_ty test;
    stmt_seq* body;
    stmt_seq* orelse;
}While;


typedef struct If {
    expr_ty test;
    stmt_seq* body;
    stmt_seq* orelse;
}If;

typedef struct With {
    expr_ty context_expr;
    expr_ty optional_vars;
    stmt_seq* body;
}With;


typedef struct Raise {
    expr_ty type;
    expr_ty inst;
    expr_ty tback;
}Raise;


typedef struct {
    stmt_seq* body;
    int n_handler;
    exception_handler_ty* handlers;
    stmt_seq* orelse;
    stmt_seq* final;
}Try;


typedef struct {
    expr_ty test;
    expr_ty msg;
}Assert;


typedef struct {
    int n_module;
    char** modules;
    char** names;
}Import;

typedef struct {
    char* from;
    int all;
    int n_module;
    char** modules;
    char** names;
}ImportFrom;


typedef struct {
    int n_name;
    expr_ty * names;
}Global;

typedef struct {
    expr_ty value;
}Expr;



struct _stmt {
    enum stmt_kind kind;
    union {
        FuncDef funcdef;
        ClassDef classdef;
        Return ret;
        Delete del;
        Assign assign;
        AugAssign augassignstmt;
        Print print;
        For forstmt;
        While whilestmt;
        If ifstmt;
        With with;
        Raise raise;
        Try trystmt;
        Assert assert;
        Import import;
        ImportFrom importfrom;
        Global global;
        Expr exprstmt;
    };
    int lineno;
    int col_offset;
};

enum expr_kind { BoolOp_kind = 1,BinOp_kind = 2,UnaryOp_kind = 3,Lambda_kind = 4,IfExp_kind = 5,
    Dict_kind = 6,Set_kind = 7,ListComp_kind = 8,SetComp_kind = 9, DictComp_kind = 10, 
    GeneratorExp_kind = 11, Yield_kind = 12, Compare_kind= 13, Call_kind = 14, Repr_kind = 15,
    Num_kind = 16, Str_kind = 17, Attribute_kind = 18, Subscript_kind =19, Name_kind = 20, 
    List_kind = 21, Tuple_kind = 22 };



typedef struct {
    boolop_ty op;
    int n_value;
    expr_ty* values;
}BoolOp;

typedef struct {
    expr_ty left;
    operator_ty op;
    expr_ty right;
}BinOp;


typedef struct {
    unaryop_ty op;
    expr_ty operand;
}UnaryOp;


typedef struct {
    arguments_ty args;
    expr_ty body;
}Lambda;

typedef struct {
    expr_ty test;
    expr_ty body;
    expr_ty orelse;
}IfExp;

typedef struct {
    int n_key;
    expr_ty* keys;
    expr_ty* values;
}Dict;


typedef struct {
    int n_elt;
    expr_ty * elts;
}Set;

typedef struct {
    expr_ty elt;
    int n_com;
    comprehension_ty *generators;
}ListComp;


typedef struct {
    expr_ty elt;
    int n_com;
    comprehension_ty* generators;
}SetComp;

typedef struct {
    expr_ty key;
    expr_ty value;
    int n_com;
    comprehension_ty* generators;
}DictComp;


typedef struct {
    expr_ty elt;
    int n_com;
    expr_ty* generators;
}GeneratorExp;


typedef struct{
    expr_ty value;
}Yield;


typedef struct {
    expr_ty left;
    int n_comparator;
    compop_ty* ops;
    expr_ty* comparators;
}Compare;



typedef struct {
    expr_ty value;
}Repr;


enum num_kind { INTEGER = 1, DECIMAL = 2 };

typedef struct {
    enum num_kind kind;
    union {
        int ivalue;
        double fvalue;
    };
}Num;


typedef struct {
    char* s;
}Str;


typedef struct {
    expr_ty value;
    char attr[NAME_MAX_SIZE];
    expr_context_ty ctx;
}Attribute;

typedef struct {
    expr_ty value;
    int n_slice;
    slice_ty* slices;
    expr_context_ty ctx;
}Subscript;

typedef struct {
    expr_ty func;
    int n_arg;
    Parameter* args;
    expr_ty varg;
    expr_ty karg;
    char fullname[NAME_MAX_SIZE];
}Call;


typedef struct {
    char id [NAME_MAX_SIZE];
    expr_context_ty ctx;
}Name;


typedef struct {
    int n_elt;
    expr_ty* elts;
    expr_context_ty ctx;
}List;

typedef struct {
    int n_elt;
    expr_ty* elts;
    expr_context_ty ctx;
}Tuple;

struct _expr {
    enum expr_kind kind;
    union {
        BoolOp boolop;
        BinOp binop;
        UnaryOp unaryop;
        Lambda lambda;
        IfExp ifexp;
        Dict dict;
        Set set;
        ListComp listcomp;
        SetComp setcomp;
        DictComp dictcomp;
        GeneratorExp generatorexp;
        Yield yield;
        Compare compare;
        Call call;
        Repr repr;
        Num num;
        Str str;
        Attribute attribute;
        Subscript sub;
        Name name;
        List list;
        Tuple tuple;
    };
    type_ty e_type;
    char addr[100];
    int isplain;
    int lineno;
    int col_offset;
};


enum slice_kind { Ellipsis_kind = 1, Slice_kind = 2, ExtSlice_kind = 3,  Index_kind = 4 };

typedef struct  {
    expr_ty lower;
    expr_ty upper;
    expr_ty step;
}Slice;

typedef struct {
    stmt_seq * dims;
}ExtSlice;

typedef struct {
    expr_ty value;
}Index;

struct slice {
    enum slice_kind kind;
    union {
        Slice slice;
        ExtSlice extslice;
        Index index;
    };
};


struct comprehension {
    expr_ty target;
    expr_ty iter;
    int n_test;
    expr_ty* tests;
};


struct exception_handler {
    expr_ty type;
    expr_ty value;
    stmt_seq* body;
};

#endif
