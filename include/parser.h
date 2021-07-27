#ifndef PARSER_H
#define PARSER_H

#include "../include/lexer.h"
#include "arr.h"
#include "sym.h"

enum {
    AST_EXPRESSION,
    AST_STATEMENT,
    AST_IDENTIFIER,
    AST_DECLERATION,
    AST_TYPE
};

struct Ast {
    uint32_t line, pos;
    const char* file;
    int type;
};

struct Ast_Expression;
struct Ast_Statement;
struct Ast_Ident;
struct Ast_Decleration;
struct Ast_Function_Definition;

struct Ast_Scope : public Ast {
    SymTable table;
    Array<Ast_Statement> statements;
    Ast_Scope* parent;
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
    Ast_Ident *ident;
};

struct Ast_Statement : public Ast {
    Ast_Statement() { type = AST_STATEMENT; }

    Ast_Scope scope;
};

struct Ast_Decleration : public Ast_Statement {
    Ast_Decleration() { type = AST_DECLERATION; }

    Ast_Type* id;
    Ast_Expression* expr;
};

struct Ast_Expression : public Ast {
    Ast_Expression() { type = AST_EXPRESSION; }
};

struct Ast_Translation_Unit : public Ast {
    Ast_Statement scope;
};

struct Parser {
    static Parser* init(Lexer* lexer);
    void run();

    Ast_Translation_Unit* root;
    Lexer* lexer;
    uint32_t index = 0;

    Token* peek();
    Token* next();

    Ast* default_ast(Ast* ast);
};

void free_translation_unit(Ast_Translation_Unit* root);

#endif //!PARSER_H