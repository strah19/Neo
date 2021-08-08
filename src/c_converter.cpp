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
"typedef int64_t i64;\n\n";

const char* C_postamble_buffer = 
"\n"
"int main(int argc, char *argv[]) {\n";

Ast_Function_Call* run_directives[64];
size_t run_directives_size = 0;

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
        case AST_CHAR_P:
            fprintf(file, "'%c'", p->char_const);
            break;
        }

        if (p->expr) {
            auto postfix = static_cast<Ast_Postfix_Expression*>(p->expr);

            switch (postfix->op) {
            case AST_UNARY_INC:
                fprintf(file, "++");
                break;
            case AST_UNARY_DEC:
                fprintf(file, "--");
                break;
            }
        }
    }
}

void C_Converter::convert_unary_expression(Ast_Expression* expr) {
    auto unary = static_cast<Ast_Unary_Expression*>(expr);

    if (expr->type == AST_UNARY_EXPESSION) {
        switch (unary->op) {
        case AST_UNARY_INC:
            fprintf(file, " ++");
            break;
        case AST_UNARY_DEC:
            fprintf(file, " --");
            break;
        case AST_UNARY_DEREF:
            fprintf(file, "*");
            break;
        case AST_UNARY_REF:
            fprintf(file, "&");
            break;
        case AST_UNARY_NESTED:
            fprintf(file, "(");
            convert_expression(unary->nested_expr);
            fprintf(file, ")");
            break;
        }

        if (unary->expr) {
            convert_unary_expression(unary->expr);
        }   
    }
    else if (expr->type == AST_PRIMARY_EXPRESSION) {
        convert_postfix_expression(expr);

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
         case AST_OPERATOR_MINUS:
            fprintf(file, "-");
            break;
        case AST_OPERATOR_COMPARITIVE_EQUAL:
            fprintf(file, "==");
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
        fprintf(file, "i32 ");
        break;
    case AST_TYPE_BYTE:
        fprintf(file, "char ");
        break;
    }
}

void C_Converter::convert_statement(Ast* ast) {
    switch (ast->type) {
    case AST_STATEMENT: {
        auto stmt = static_cast<Ast_Statement*>(ast);

        switch (stmt->flags) {
        case AST_RETURN: {
            fprintf(file, "return ");
            convert_expression(stmt->expr);
            fprintf(file, ";\n");
            break;
        }
        }

        break;
    }
    case AST_CONDITION: {
        auto condition = static_cast<Ast_Conditional*>(ast);

        fprintf(file, "if(");
        convert_expression(condition->condition);
        fprintf(file, "){\n");

        for(int i = 0; i < condition->scope.size; i++) {
            convert_statement(condition->scope.statements[i]);
        }

        fprintf(file, "}\n");

        auto current = condition->next;
        while (current) {
            if (current->flag == AST_CONDITION_ELIF) {
                fprintf(file, "else if(");
                convert_expression(current->condition);
                fprintf(file, "){\n");
            }
            else if(current->flag == AST_CONDITION_ELSE) 
                fprintf(file, "else{\n");

            for(int i = 0; i < current->scope.size; i++) {
                convert_statement(condition->scope.statements[i]);
            }

            fprintf(file, "}\n");

            current = current->next;
        }

        break;
    }
    default: {
        convert_decleration(static_cast<Ast_Decleration*>(ast));
    }
    }
}

void C_Converter::convert_function_definition(Ast_Function_Definition* func) {
    if (!func->type_info)
        fprintf(file, "void ");
    else {
        convert_type(func->type_info);
    }
    
    convert_identifier(func->id);

    fprintf(file, "(");
    for(int i = 0; i < func->arg_count; i++) {
        convert_type(func->args[i]->type_info);
        convert_identifier(func->args[i]->id);

        if (i < func->arg_count - 1)
            fprintf(file, ",");
    }
    fprintf(file, ") ");
    fprintf(file, "{\n");

    for(int i = 0; i < func->scope.size; i++) {
        convert_statement(func->scope.statements[i]);
    }

    fprintf(file, "}\n");
}

void C_Converter::convert_function_call(Ast_Function_Call* call) {
    fprintf(file, "%s(", call->id->name);
        for (int j = 0; j < call->arg_count; j++) {       
            convert_expression(call->args[j]);
            if (j < call->arg_count - 1)
                fprintf(file, ",");
        }
    fprintf(file, ");\n");
}

void C_Converter::convert_decleration(Ast_Decleration* decleration) {
    if (decleration->type == AST_DECLERATION || decleration->type == AST_ASSIGNMENT) {
        if (decleration->type_info)
            convert_type(decleration->type_info);
        convert_identifier(decleration->id);

        if (decleration->expr) {
            fprintf(file, "=");
            convert_expression(decleration->expr);
        }
        end();
    }
    else if (decleration->type == AST_FUNCTION_DEFINITION) 
        convert_function_definition(static_cast<Ast_Function_Definition*>(decleration));
    else if(decleration->type == AST_FUNCTION_CALL) {
        auto call = static_cast<Ast_Function_Call*>(decleration);
        if (call->run_in_directive)
            run_directives[run_directives_size++] = call;
        else
            convert_function_call(call);
    }
}

void convert_transition_unit(const char* obj_name, Ast_Translation_Unit* root) {
    C_Converter c;
    char buf[FILE_NAME_LEN];
    c.file = open_c_file(obj_name, buf);

    for(int i = 0; i < root->scope.size; i++) {
        c.convert_decleration(static_cast<Ast_Decleration*>(root->scope.statements[i]));
    }

    fprintf(c.file, C_postamble_buffer);

    for (int i = 0; i < run_directives_size; i++) {
        c.convert_function_call(run_directives[i]);
    }

    fprintf(c.file, "\treturn 0;\n}");
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