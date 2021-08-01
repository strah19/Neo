#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#include "arr.h"

#define LEXER_FILE_MODE "r"
#define MAX_TOKEN_SIZE 512

namespace Tok {
        enum {
            T_COLON = ':',
            T_PLUS = '+',
            T_SLASH = '/',
            T_STAR = '*',
            T_MINUS = '-',
            T_EQUAL = '=',
            T_CARET = '^',
            T_REF = '&',
            
            T_LPAR = '(',
            T_RPAR = ')',
            T_LBRACKET = '[',
            T_RBRACKET = ']',
            T_LCURLY = '{',
            T_RCURLY = '}',
            T_SEMI = ';',
            T_PERCENT = '%',
            T_POUND = '#',
            T_EXCLAMATION = '!',
            T_LARROW = '<',
            T_RARROW = '>',
            T_COMMA = ',', 

            T_EOF = 255,


            T_COLON_ASSIGN,
            T_AND,
            T_OR,
            T_IF,
            T_ELIF,
            T_ELSE,
            T_WHILE,
            T_BREAK,
            T_RETURN,
            T_CONTINUE,
            T_COMPARE_EQUAL,
            T_LTE,
            T_GTE,
            T_NOT_EQUAL,
            
            T_CONST,
            T_INT,
            T_FLOAT,
            T_DOUBLE,
            T_BYTE,
            T_BOOLEAN,

            T_IDENTIFIER,
            T_INT_CONST,
            T_FLOAT_CONST,

            T_INC,
            T_DEC,
        };
    }

struct Token {
    int type;
    uint32_t line;
    uint32_t pos;

    union {
        int int_const;
        float float_const;
        char identifier[MAX_TOKEN_SIZE];
    };
};

const char* token_to_str(Token* token);

const char* type_to_str(int type);

void log_token(Token* token);

struct Lexer {
    uint32_t size;
    uint32_t allocated_size;
    uint32_t current_index = 0;
    
    Token* tokens;

    char current[MAX_TOKEN_SIZE];
    size_t current_len = 0;

    uint8_t* stream;
    char* file;

    uint32_t current_line;
    uint32_t current_pos;

    void run();
    void log();

    static Lexer* init(uint8_t* stream);
};

uint8_t* load_file(const char* filepath, size_t* filesize);

#endif //!LEXER_H