#ifndef PARSER_H
#define PARSER_H

#include "../include/lexer.h"
#include "arr.h"
#include "sym.h"

enum {
    AST_EXPRESSION,
    AST_BINARY_EXPRESSION,
    AST_PRIMARY_EXPRESSION,
    AST_UNARY_EXPESSION,
    AST_FUNCTION_DEFINITION,
    AST_FUNCTION_CALL,
    AST_STATEMENT,
    AST_IDENTIFIER,
    AST_ASSIGNMENT,
    AST_DECLERATION,
    AST_CONDITION,
    AST_TYPE,
    AST_SCOPE
};

struct Ast {
    uint32_t line, pos;
    const char* file;
    int type;
};

struct Ast_Expression;
struct Ast_Ident;
struct Ast_Decleration;
struct Ast_Function_Definition;
struct Ast_Function_Call;

struct Ast_Scope : public Ast {
    Ast_Scope() { type = AST_SCOPE; }

    SymTable table;
    Ast* statements[64];
    int size = 0;

    Ast_Scope* parent = nullptr;

    Ast_Decleration* get_decleration(Ast_Ident* iden);
};

enum {
    AST_TYPE_INT,
    AST_TYPE_BYTE
};

struct Ast_Ident : public Ast {
    Ast_Ident() { type = AST_IDENTIFIER; }

    char* name;
};

struct Ast_Type : public Ast {
    Ast_Type() { type = AST_TYPE; }

    int atom_type;
    bool constant = false;
};

struct Ast_Decleration : public Ast {
    Ast_Decleration() { type = AST_DECLERATION; }

    Ast_Type* type_info = nullptr;
    Ast_Ident* id = nullptr;
    Ast_Expression* expr = nullptr;
};

struct Ast_Expression : public Ast {
    Ast_Expression() { type = AST_EXPRESSION; }

    Ast_Expression* next = nullptr;
};

enum {
    AST_OPERATOR_PLUS,
    AST_OPERATOR_MINUS,
    AST_OPERATOR_MULTIPLICATIVE,
    AST_OPERATOR_DIVISION,
    AST_OPERATOR_MODULO,
    AST_OPERATOR_COMPARITIVE_EQUAL,
    AST_OPERATOR_COMPARITIVE_NOT_EQUAL,
    AST_OPERATOR_LTE,
    AST_OPERATOR_GTE,
    AST_OPERATOR_LT,
    AST_OPERATOR_GT
};

struct Ast_Binary_Expression : public Ast_Expression {
    Ast_Binary_Expression() { type = AST_BINARY_EXPRESSION; }
    int op;

    Ast_Expression* left = nullptr;
    Ast_Expression* right = nullptr;
};

enum {
    AST_INT_P,
    AST_FLOAT_P,
    AST_STR_P,
    AST_CHAR_P,
    AST_ID_P,
    AST_CALL_P
};

struct Ast_Primary_Expression : public Ast_Expression {
    Ast_Primary_Expression() { type = AST_PRIMARY_EXPRESSION; }
    int v_type;
    union 
    {
        int64_t int_const;
        double float_const;
        char* string_literal;
        char char_const;
        Ast_Ident *ident;
        Ast_Function_Call* call;
    };

    Ast_Expression* expr = nullptr;
};

enum {
    AST_UNARY_INC,
    AST_UNARY_DEC,
    AST_UNARY_NESTED,
    AST_UNARY_DEREF,
    AST_UNARY_REF,
    AST_UNARY_NONE,
};

struct Ast_Unary_Expression : public Ast_Expression {
    Ast_Unary_Expression() { type = AST_UNARY_EXPESSION; }

    int op = AST_UNARY_NONE;
    Ast_Expression* expr = nullptr;
    Ast_Expression* nested_expr = nullptr;
};

struct Ast_Postfix_Expression : public Ast_Expression {
    int op = AST_UNARY_NONE;

    Ast_Ident* id = nullptr;
    Ast_Expression* expr = nullptr;    
};

enum {
    AST_FUNCTION_NONE = 0x00,
    AST_FUNCTION_GLOBAL = 0x01,
    AST_FUNCTION_LOCAL = 0x02,
};

struct Ast_Function_Definition : public Ast_Decleration {
    Ast_Function_Definition() { type = AST_FUNCTION_DEFINITION; }
    
    Ast_Scope scope;
    Ast_Decleration* args[64];
    size_t arg_count = 0;
    int flags = AST_FUNCTION_GLOBAL;
    Ast_Ident* from = nullptr;
};

struct Ast_Function_Call : public Ast_Decleration {
    Ast_Function_Call() { type = AST_FUNCTION_CALL; }

    Ast_Expression* args[64];
    size_t arg_count = 0;
    bool run_in_directive = false;
};

enum {
    AST_ATTRIB_NONE = 0x00,
    AST_RETURN = 0x01,
    AST_BREAK = 0x02
};

struct Ast_Statement : public Ast {
    Ast_Statement() { type = AST_STATEMENT; }

    int flags = AST_ATTRIB_NONE;

    Ast_Expression* expr = nullptr;
};

enum {
    AST_CONTROL_NONE,
    AST_CONTROL_IF,
    AST_CONTROL_ELSE,
    AST_CONTROL_ELIF,
    AST_CONTROL_WHILE
};

struct Ast_ControlFlow : public Ast {
    Ast_ControlFlow() { type = AST_CONDITION; }

    Ast_Expression* condition;
    Ast_Scope scope;
    int flag = AST_CONTROL_NONE;
    Ast_ControlFlow* next;
};

struct Ast_Translation_Unit : public Ast {
    Ast_Scope scope;
};

struct Parser {
    static Parser* init(Lexer* lexer);
    void run();

    SymTable extra_headers;

    Ast_Translation_Unit* root;
    Ast_Scope* current_scope;

    Lexer* lexer;
    uint32_t index = 0;

    Token* peek();
    Token* peek_off(int off);
    Token* next();
    void match(int type);

    Ast* default_ast(Ast* ast);

    Ast_Type* parse_type();
    Ast_Ident* parse_identity();
    Ast_Decleration* parse_decleration();
    Ast* parse_statement();
    Ast_Expression* parse_expression();
    Ast_Expression* parse_unary_expression();
    Ast_Expression* parse_posfix_expression();
    Ast_Expression* parse_primary_expression();
    Ast_Function_Definition* parse_function_definition();
    Ast_Function_Call* parse_function_call();

    Ast_Function_Definition* parse_function_decleration();

    Ast_Expression* parse_postfix_symbol();

    void parse_scope(Ast_Scope* scope);

    void add_identifier_to_scope(Ast_Decleration* dec);

    int error_count = 0;
};

void free_translation_unit(Ast_Translation_Unit* root);

#endif //!PARSER_H