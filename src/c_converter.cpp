#include "../include/c_converter.h"
#include "../include/err.h"

#define FILE_NAME_LEN 256
#define C_OUT_FILE_MODE "w"
#define C_OUT_FILE_TYPE ".c"

const char* C_preamble_buffer = 
"#include <stdio.h>\n"
"#include <stdint.h>\n\n"
"typedef uint32_t u32;\n"
"typedef int32_t i32;\n"
"typedef float f32;\n"
"typedef double f64;\n"
"typedef uint64_t u64;\n"
"typedef int64_t i64;\n\n"
"void __code_gen() {\n";

const char* C_postamble_buffer = 
"}\n\n"
"int main(int argc, char *argv[]) {\n"
"\t__code_gen();\n"
"\treturn 0;\n}";

FILE* open_c_file(const char* file_name, char* buf) {    
    memset(buf, 0, FILE_NAME_LEN);
    strcpy(buf, file_name);

    strcat(buf, C_OUT_FILE_TYPE);
    FILE* file = fopen(buf, C_OUT_FILE_MODE);

    if (!file) 
        fatal_error("could not open %s for converting.\n", buf);

    fprintf(file, C_preamble_buffer);

    return file;
}

void C_Converter::end() {
    fprintf(file, ";\n");
}

void C_Converter::convert_unary_expression(Ast_Expression* expr) {
    auto unary = static_cast<Ast_Unary_Expression*>(expr);

    if (expr->type == AST_UNARY_EXPESSION) {
        switch (unary->op) {
        case AST_UNARY_INC:
            fprintf(file, "++");
            break;
        case AST_UNARY_DEC:
            fprintf(file, "--");
            break;
        case AST_UNARY_NESTED:
            fprintf(file, "(");
            convert_expression(unary->nested_expr);
            fprintf(file, ")");
            break;
        }
    }

    if (unary->expr)
        convert_unary_expression(unary->expr);
    else
        convert_postfix_expression(expr);
}

void C_Converter::convert_postfix_expression(Ast_Expression* expr) {
    if (expr->type == AST_PRIMARY_EXPRESSION) {
        auto p = static_cast<Ast_Primary_Expression*>(expr);
        switch (p->v_type) {
        case AST_INT_P:
            fprintf(file, "%d", p->int_const);
            break;
        case AST_ID_P:
            fprintf(file, "%s", p->ident->name);
            break;
        }
    }
}

void C_Converter::convert_binary_expression(Ast_Expression* expr) {
    if (expr->type == AST_BINARY_EXPRESSION) {
        auto bin = static_cast<Ast_Binary_Expression*>(expr);
        convert_expression(bin->left);

        switch (bin->op) {
        case AST_OPERATOR_MULTIPLICATIVE:
            fprintf(file, "*");
            break;
        case AST_OPERATOR_PLUS:
            fprintf(file, "+");
            break;
        case AST_OPERATOR_DIVISION:
            fprintf(file, "/");
            break;
        }

        convert_expression(bin->right);
    }
    else {
        convert_unary_expression(expr);
    }
}

void C_Converter::convert_expression(Ast_Expression* expr) {
    convert_binary_expression(expr);
}

void C_Converter::convert_identifier(Ast_Ident* id) {
    fprintf(file, "%s", id->name);
}

void C_Converter::convert_type(Ast_Type* type) {
    switch (type->atom_type) {
    case AST_TYPE_INT:
        fprintf(file, "int ");
        break;
    }
}

void C_Converter::convert_decleration(Ast_Decleration* decleration) {
    if (decleration->type == AST_DECLERATION) {
        convert_type(decleration->type_info);
        convert_identifier(decleration->id);

        if (decleration->expr) {
            fprintf(file, "=");
            convert_expression(decleration->expr);
        }
        end();
    }
}

void convert_transition_unit(const char* obj_name, Ast_Translation_Unit* root) {
    C_Converter c;
    char buf[FILE_NAME_LEN];
    c.file = open_c_file(obj_name, buf);

    for(int i = 0; i < root->scope.statements.top(); i++) {
        c.convert_decleration(static_cast<Ast_Decleration*>(root->scope.statements.get_arr()[i]));
    }

    fprintf(c.file, C_postamble_buffer);
    fclose(c.file);

    compile_and_link(buf, obj_name);
}

void compile_and_link(const char* file_name, const char* obj_name) {
    char cmd_buf[FILE_NAME_LEN] = "gcc ";
    strcat(cmd_buf, file_name);
    strcat(cmd_buf, " -o ");
    strcat(cmd_buf, obj_name);

    system(cmd_buf);
}