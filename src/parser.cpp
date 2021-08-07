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

void Parser::match(int type) {
    if (peek()->type != type) 
        report_error("Expected '%s' on line %d.\n", type_to_str(type), peek()->line);

    next();
}

Ast_Expression* Parser::parse_primary_expression() {
    auto prime = AST_NEW(Ast_Primary_Expression);

    switch(peek()->type) {
    case Tok::T_INT_CONST:
        prime->v_type = AST_INT_P;
        prime->int_const = peek()->int_const;
        match(Tok::T_INT_CONST);     
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
        switch (peek()->type) {
        case Tok::T_INC:
            match(Tok::T_INC);
            postfix->op = AST_UNARY_INC;
            prime->expr = postfix;
            break;
        case Tok::T_DEC:
            match(Tok::T_DEC);
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
        match(Tok::T_INC);
        unary->op = AST_UNARY_INC;
        break;
    case Tok::T_DEC:
        match(Tok::T_DEC);
        unary->op = AST_UNARY_DEC;
        break;
    case Tok::T_LPAR:
        match(Tok::T_LPAR);
        unary->op = AST_UNARY_NESTED;
        unary->nested_expr = parse_expression();
        match(Tok::T_RPAR);
        break;
    case Tok::T_STAR:
        match(Tok::T_STAR);
        unary->op = AST_UNARY_DEREF;
        break;
    case Tok::T_AMBERSAND:
        match(Tok::T_AMBERSAND);
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

    match(peek()->type);
    expr->left = lexpr;
    expr->right = parse_expression();

    return expr;
}

Ast_Type* Parser::parse_type() {
    auto type_info = AST_NEW(Ast_Type);

    switch (peek()->type) {
    case Tok::T_INT:
        type_info->atom_type = AST_TYPE_INT;
        match(peek()->type);
        return type_info;
        break;
    default:
        report_error("'%s' is not a valid type on line %d.\n", token_to_str(peek()), peek()->line);
        match(peek()->type);
        break;
    }

    AST_DELETE(type_info);
    return nullptr;
}

Ast_Ident* Parser::parse_identity() {
    auto id = AST_NEW(Ast_Ident);

    size_t id_len = strlen(peek()->identifier);
    char* name = (char *)calloc(1, id_len + 1);
    memcpy(name, peek()->identifier, id_len);
    id->name = name;

    match(Tok::T_IDENTIFIER);
    return id;
}

Ast* Parser::parse_statement() {
    return parse_decleration();
}

Ast_Function_Definition* Parser::parse_function_definition() {
    auto func = AST_NEW(Ast_Function_Definition);
    func->id = parse_identity();
    add_identifier_to_scope(func);

    match(Tok::T_COLON);
    match(Tok::T_LPAR);
    while(peek()->type != Tok::T_RPAR) {
        auto dec = AST_NEW(Ast_Decleration);

        dec->id = parse_identity();
        printf("%s\n", dec->id->name);
        match(Tok::T_COLON);
        dec->type_info = parse_type();

        func->args[func->arg_count++] = dec;

        if (peek()->type == Tok::T_RPAR)
            break;
        match(Tok::T_COMMA);
    }
    match(Tok::T_RPAR);

    if (peek()->type != Tok::T_DASH_ARROW) {
        func->type_info = nullptr;
    }

    if (peek()->type == Tok::T_LCURLY) {
        match(Tok::T_LCURLY);

        func->scope.parent = current_scope;
        current_scope = &func->scope;

        while (peek()->type != Tok::T_RCURLY) {
            auto stmt = parse_statement();
            func->scope.statements[func->scope.size++] = stmt;
        }
        match(Tok::T_RCURLY);
        current_scope = func->scope.parent;
    }

    return func;
}

Ast_Function_Call* Parser::parse_function_call() {
    auto call = AST_NEW(Ast_Function_Call);
    call->id = parse_identity();

    match(Tok::T_LPAR);

    while(peek()->type != Tok::T_RPAR) {
        call->args[call->arg_count++] = parse_expression();

        if (peek()->type == Tok::T_RPAR)
            break;
        match(Tok::T_COMMA);
    }

    match(Tok::T_RPAR);
    match(Tok::T_SEMI);

    return call;
}

Ast_Decleration* Parser::parse_decleration() {
    if ((peek_off(1)->type == Tok::T_COLON && peek_off(2)->type == Tok::T_LPAR)) {
        return parse_function_definition();
    }
    else if (peek_off(1)->type == Tok::T_LPAR) {
        return parse_function_call();
    }

    auto dec = AST_NEW(Ast_Decleration);

    dec->id = parse_identity();
    dec->type_info = nullptr;

    if (peek()->type == Tok::T_COLON) {
        match(Tok::T_COLON);

        dec->type_info = parse_type();
    }

    if (peek()->type == Tok::T_EQUAL) {
        if (!dec->type_info) 
            dec->type = AST_ASSIGNMENT;
        match(Tok::T_EQUAL);
        dec->expr = parse_expression();
    }

    match(Tok::T_SEMI);

    add_identifier_to_scope(dec);
    return dec;
}

void Parser::add_identifier_to_scope(Ast_Decleration* dec) {
    Entry* e = nullptr;
    auto c = current_scope;
    while (c) {
        e = c->table.look_up(dec->id->name);

        if (e)
            break;

        c = c->parent;
    }

    if (e && dec->type == AST_DECLERATION)  {
        report_error("redecleration of identifier '%s' on line %d.\n", dec->id->name, dec->id->line);
        error_count++;
    }

    if (!e) {
        if (dec->type == AST_DECLERATION || dec->type == AST_FUNCTION_DEFINITION) 
            current_scope->table.insert(dec->id->name, Tok::T_IDENTIFIER);
        else {
            report_error("undeclared identifier '%s' on line %d.\n", dec->id->name, dec->id->line);
            error_count++;
        }
    }    
}

void Parser::run() {
    root = AST_NEW(Ast_Translation_Unit);
    current_scope = &root->scope;
  
    while (peek()->type != Tok::T_EOF) {
        auto dec = parse_decleration();

        root->scope.statements[root->scope.size++] = dec;
    }
}

void free_translation_unit(Ast_Translation_Unit* root) {
    delete root;
}