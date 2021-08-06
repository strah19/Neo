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

struct Ast_Scope : public Ast {
    Ast_Scope() { type = AST_SCOPE; }

    SymTable table;
    Array<Ast*> statements;
    Ast_Scope* parent = nullptr;

    Ast_Decleration* get_decleration(Ast_Ident* iden);
};

enum {
    AST_TYPE_INT
};

struct Ast_Ident : public Ast {
    Ast_Ident() { type = AST_IDENTIFIER; }

    char* name;
};

struct Ast_Type : public Ast {
    Ast_Type() { type = AST_TYPE; }

    int atom_type;
};

struct Ast_Decleration : public Ast {
    Ast_Decleration() { type = AST_DECLERATION; }

    Ast_Type* type_info;
    Ast_Ident* id;
    Ast_Expression* expr;
};

struct Ast_Expression : public Ast {
    Ast_Expression() { type = AST_EXPRESSION; }
};

enum {
    AST_OPERATOR_PLUS,
    AST_OPERATOR_MINUS,
    AST_OPERATOR_MULTIPLICATIVE,
    AST_OPERATOR_DIVISION,
    AST_OPERATOR_MODULO,
};

struct Ast_Binary_Expression : public Ast_Expression {
    Ast_Binary_Expression() { type = AST_BINARY_EXPRESSION; }
    int op;

    Ast_Expression* left;
    Ast_Expression* right;
};

enum {
    AST_INT_P,
    AST_FLOAT_P,
    AST_STR_P,
    AST_ID_P
};

struct Ast_Primary_Expression : public Ast_Expression {
    Ast_Primary_Expression() { type = AST_PRIMARY_EXPRESSION; }
    int v_type;
    union 
    {
        int64_t int_const;
        double float_const;
        char* string_literal;
        Ast_Ident *ident;
    };

    Ast_Expression* expr;
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
    Ast_Expression* expr;
    Ast_Expression* nested_expr;
};

struct Ast_Postfix_Expression : public Ast_Expression {
    int op = AST_UNARY_NONE;

    Ast_Ident* id;
    Ast_Expression* expr;    
};

struct Ast_Function_Definition : public Ast_Decleration {
    Ast_Function_Definition() { type = AST_FUNCTION_DEFINITION; }
    
    Ast_Scope scope;
    Array<Ast_Decleration*> args;
};

struct Ast_Function_Call : public Ast_Postfix_Expression {
    Ast_Function_Call() { type = AST_FUNCTION_CALL; }

    Array<Ast_Expression*> args;
};

struct Ast_Translation_Unit : public Ast {
    Ast_Scope scope;
};

struct Parser {
    static Parser* init(Lexer* lexer);
    void run();

    Ast_Translation_Unit* root;
    Ast_Scope* current_scope;

    Lexer* lexer;
    uint32_t index = 0;

    Token* peek();
    Token* peek_off(int off);
    Token* next();
    void match(int type, int parser_line);

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

    void add_identifier_to_scope(Ast_Decleration* dec);

    int error_count = 0;
};

void free_translation_unit(Ast_Translation_Unit* root);

#endif //!PARSER_H