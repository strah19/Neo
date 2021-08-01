#include "../include/parser.h"
#include "../include/err.h"

#include <stdio.h>

Ast* Parser::default_ast(Ast* ast) {
    ast->file = lexer->file;
    ast->pos = peek()->pos;
    ast->line = peek()->line;

    return ast;
}

#define AST_NEW(type) \
    static_cast<type*>(default_ast(new type))

#define AST_DELETE(type) delete type

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

Token* Parser::peek_off(int off) {
    return (index + off <= lexer->size) ? &lexer->tokens[index + off] : nullptr;  
}

void Parser::match(int type, int line) {
    if (peek()->type != type) 
        report_error("Expected '%s' on line %d [In 'parser.cpp' on line %d].", type_to_str(type), peek()->line, line);

    next();
}

Ast_Expression* Parser::parse_primary_expression() {
    auto prime = AST_NEW(Ast_Primary_Expression);

    switch(peek()->type) {
    case Tok::T_INT_CONST:
        prime->v_type = AST_INT_P;
        prime->int_const = peek()->int_const;
        match(Tok::T_INT_CONST, __LINE__);     
        break;
    case Tok::T_IDENTIFIER:
        prime->v_type = AST_ID_P;
        prime->ident = parse_identity();
        break;
    default:
        return nullptr;
    }

    return prime;
}

Ast_Expression* Parser::parse_posfix_expression() {
    auto prime = static_cast<Ast_Primary_Expression*>(parse_primary_expression());
    if (prime) {
        auto postfix = AST_NEW(Ast_Postfix_Expression);
        log_token(peek());
        switch (peek()->type) {
        case Tok::T_INC:
        printf("POSTFIX!\n");
            match(Tok::T_INC, __LINE__);
            postfix->op = AST_UNARY_INC;
            prime->expr = postfix;
            break;
        case Tok::T_DEC:
            match(Tok::T_DEC, __LINE__);
            postfix->op = AST_UNARY_DEC;
            prime->expr = postfix;
            break;
        default:
            prime->expr = nullptr;
            AST_DELETE(postfix);
            break;
        }

    }
    return prime;
}

Ast_Expression* Parser::parse_unary_expression() {
    auto unary = AST_NEW(Ast_Unary_Expression);
    switch(peek()->type) {
    case Tok::T_INC:
        match(Tok::T_INC, __LINE__);
        unary->op = AST_UNARY_INC;
        break;
    case Tok::T_DEC:
        match(Tok::T_DEC, __LINE__);
        unary->op = AST_UNARY_DEC;
        break;
    case Tok::T_LPAR:
        match(Tok::T_LPAR, __LINE__);
        unary->op = AST_UNARY_NESTED;
        unary->nested_expr = parse_expression();
        match(Tok::T_RPAR, __LINE__);
        break;
    case Tok::T_STAR:
        match(Tok::T_STAR, __LINE__);
        unary->op = AST_UNARY_DEREF;
        break;
    case Tok::T_AMBERSAND:
        match(Tok::T_AMBERSAND, __LINE__);
        unary->op = AST_UNARY_REF;
        break;
    default:
        AST_DELETE(unary); 
        return parse_posfix_expression();
    }

    unary->expr = parse_unary_expression();
    return unary;
}

Ast_Expression* Parser::parse_expression() {
    auto lexpr = parse_unary_expression();
    auto expr = AST_NEW(Ast_Binary_Expression);

    switch(peek()->type) {
    case Tok::T_STAR:
        expr->op = AST_OPERATOR_MULTIPLICATIVE;
        break;
    case Tok::T_SLASH:
        expr->op = AST_OPERATOR_DIVISION;
        break;
    case Tok::T_PERCENT:
        expr->op = AST_OPERATOR_MODULO;
        break;
    case Tok::T_PLUS:
        expr->op = AST_OPERATOR_PLUS;
        break;
    case Tok::T_MINUS:
        expr->op = AST_OPERATOR_MINUS;
        break;
    default:
        AST_DELETE(expr);
        return lexpr;
    }

    match(peek()->type, __LINE__);
    expr->left = lexpr;
    expr->right = parse_expression();

    return expr;
}

Ast_Type* Parser::parse_type() {
    auto type_info = AST_NEW(Ast_Type);

    switch (peek()->type) {
    case Tok::T_INT:
        type_info->atom_type = AST_TYPE_INT;
        break;
    default:
        report_error("'%s' is not a valid type on line %d.\n", token_to_str(peek()), peek()->line);
        break;
    }

    match(peek()->type, __LINE__);
    return type_info;
}

Ast_Ident* Parser::parse_identity() {
    auto id = AST_NEW(Ast_Ident);

    size_t id_len = strlen(peek()->identifier);
    char* name = (char *)calloc(1, id_len + 1);
    memcpy(name, peek()->identifier, id_len);
    id->name = name;

    match(Tok::T_IDENTIFIER, __LINE__);
    return id;
}

Ast_Statement* Parser::parse_statement() {
    return nullptr;
}

Ast_Decleration* Parser::parse_decleration() {
    auto dec = AST_NEW(Ast_Decleration);

    dec->id = parse_identity();

    if (peek()->type == Tok::T_COLON) {
        match(Tok::T_COLON, __LINE__);
        dec->type_info = parse_type();
    }


    if (peek()->type == Tok::T_EQUAL) {
        match(Tok::T_EQUAL, __LINE__);
        dec->expr = parse_expression();
    }

    return dec;
}

Ast_Decleration* Parser::parse_extern_decleration() {
    if (peek_off(1)->type == Tok::T_LPAR) {
        //Function goes here
        return nullptr;
    }

    auto dec = parse_decleration();
    match(Tok::T_SEMI, __LINE__);

    return dec;
}

void Parser::run() {
    root = AST_NEW(Ast_Translation_Unit);
    root->scope.statements.reserve(256);
  
    while (peek()->type != Tok::T_EOF) {
        auto dec = parse_extern_decleration();

        root->scope.statements.push(dec);

        Entry* e = root->scope.table.look_up(dec->id->name);
        if (!e)
            root->scope.table.insert(dec->id->name, Tok::T_IDENTIFIER);
    }
}

void free_translation_unit(Ast_Translation_Unit* root) {
    delete root;
}