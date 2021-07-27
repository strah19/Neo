#include "../include/parser.h"

#include <stdio.h>

Ast* Parser::default_ast(Ast* ast) {
    ast->file = lexer->file;
    ast->pos = peek()->pos;
    ast->line = peek()->line;

    return ast;
}

#define AST_NEW(type) \
    static_cast<type*>(default_ast(new type))

Parser* Parser::init(Lexer* lexer) {
    Parser* parser = new Parser;

    parser->lexer = lexer;

    return parser;
}


Token* Parser::peek() {
    return (index <= lexer->size) ? &lexer->tokens[index] : nullptr;
}

Token* Parser::next() {
    return (index <= lexer->size) ? &lexer->tokens[index++] : nullptr;
}

void Parser::run() {
    root = AST_NEW(Ast_Translation_Unit);

    while (peek()->type != Tok::T_EOF) {
        printf("%d\n", peek()->type);
        next();
    }
}

void free_translation_unit(Ast_Translation_Unit* root) {
    delete root;
}