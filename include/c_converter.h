#ifndef C_CONVERT_H
#define C_CONVERT_H

#include "parser.h"

#include <stdio.h>

FILE* open_c_file(const char* file_name, char* buf, SymTable* extra_headers);

void convert_transition_unit(const char* obj_name, Ast_Translation_Unit* root, SymTable* extra_headers);

void compile_and_link(const char* file_name, const char* obj_name);

struct C_Converter {
    FILE* file;
    void convert_decleration(Ast_Decleration* decleration);
    void convert_type(Ast_Type* type);
    void convert_identifier(Ast_Ident* id);
    void convert_expression(Ast_Expression* expr);
    void convert_binary_expression(Ast_Expression* expr);
    void convert_unary_expression(Ast_Expression* expr);
    void convert_postfix_expression(Ast_Expression* expr);
    void convert_function_definition(Ast_Function_Definition* func);
    void convert_function_call(Ast_Function_Call* call);
    void convert_statement(Ast* ast);

    void end();
};

#endif //!C_CONVERT_H