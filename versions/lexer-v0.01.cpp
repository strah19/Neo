#include "../include/lexer.h"
#include "../include/err.h"

#include <ctype.h>
#include <string>  
#include <sys/stat.h>

struct Keyword {
    const char* name;
    int type;
};

static const Keyword keywords[] = {
    {"int", Tok::T_INT},
    {"if", Tok::T_IF},
    {"float", Tok::T_FLOAT},
    {"::", Tok::T_DOUBLE_COLON},
    {"double", Tok::T_DOUBLE},
    {"while", Tok::T_WHILE},
    {"elif", Tok::T_ELIF},
    {"else", Tok::T_ELSE},
    {"(", Tok::T_LPAR},
    {")", Tok::T_RPAR},
    {"{", Tok::T_LCURLY},
    {"}", Tok::T_RCURLY},
    {"==", Tok::T_COMPARE_EQUAL},
    {"=", Tok::T_EQUAL},
};

enum {
    IDENTIFIER = 1,
    NUMERIC,
    SYMBOL,
    SINGLE_LINE_COMMENT,
    MULTI_LINE_COMMENT
};

uint8_t* load_file(const char* filepath, size_t* filesize) {
    uint8_t* stream;
    FILE* file;
    int size;

    if(file = fopen(filepath, LEXER_FILE_MODE)) {
        struct stat st;

        if (fstat(fileno(file), &st) != -1) {
            stream = (uint8_t *) malloc(st.st_size);
            *filesize = st.st_size;
            fread((void*) stream, 1, st.st_size, file);
        }
        else 
            fatal_error("Failed to load stats of input file");
    }
    else 
        fatal_error("Failed to open input file for compilation");

    return stream;
}

Lexer* init_lexer(uint8_t* stream) {
    Lexer* lexer = new Lexer;

    lexer->stream = stream;
    lexer->tokens.reserve(RESERVE_SIZE);
    lexer->current.reserve(RESERVE_SIZE);
    lexer->current_token = lexer->tokens.get_arr();
    lexer->current_line = lexer->current_pos = 1;

    return lexer;
}

void new_line(Lexer* lexer) {
    if (*lexer->stream == '\n') {
        lexer->current_line++;
        lexer->current_pos = 1;
    }
}

bool is_identifier(char c) {
    return (isalpha(c) || c == '_');
}

void remove_whitespaces(char* s) {
    char* d = s;
    do while (isspace(*s)) s++; while (*d++ = *s++);
}

Token fill_token(int type, uint32_t pos, uint32_t line) {
    Token t;

    t.line = line;
    t.pos = pos;
    t.type = type;

    return t;
}

void check_for_overflow(Lexer* lexer) {
    if (lexer->tokens.top() + 1 == lexer->tokens.size())
        lexer->tokens.reserve(RESERVE_SIZE);
}

void create_token(Lexer* lexer, int type) {
    check_for_overflow(lexer);
    lexer->tokens.push(fill_token(type, lexer->current_pos, lexer->current_line));
}

void create_id_token(Lexer* lexer, int type, const char* name) {
    Token t = fill_token(type, lexer->current_pos, lexer->current_line);

    strcpy(t.identifier, name);
    remove_whitespaces(t.identifier);

    check_for_overflow(lexer);
    lexer->tokens.push(t);
}

void create_numeric_token(Lexer* lexer, int type, const char* int_const) {
    Token t = fill_token(type, lexer->current_pos, lexer->current_line);
    t.int_const = atoi(int_const);

    check_for_overflow(lexer);
    lexer->tokens.push(t);
}

bool is_special_character(char character) {
    return (character == ' ' || character == '\n' || character == '\t');
}

void reset(int* flags, Lexer* lexer) {
    *flags = 0;
    lexer->current.clear();
}

void Lexer::run() {
    int token_flag = 0;

    while(*stream != '`') {
        new_line(this);

        if(*stream == '\n') {
            if (token_flag == SINGLE_LINE_COMMENT)
                token_flag = 0;
        }
        
        bool is_digit = isdigit(*stream);

        if (*stream == '/' && *(stream + 1) == '/') {
            token_flag = SINGLE_LINE_COMMENT;
            stream++;
        }
        if (*stream == '/' && *(stream + 1) == '*') {
            printf("Found\n");
            token_flag = MULTI_LINE_COMMENT;
        }
        if (*stream == '*' && *(stream + 1) == '*') {
            token_flag = MULTI_LINE_COMMENT;
            printf("Found End\n");
        }

        if (token_flag != SINGLE_LINE_COMMENT && token_flag != MULTI_LINE_COMMENT) {
            if (((is_special_character(*stream) || is_digit) && token_flag == IDENTIFIER) || (token_flag == SYMBOL && (is_identifier(*stream) || is_special_character(*stream) || is_digit))) {
                for(int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
                    if (keywords[i].name[0] == current.get_arr()[0]) {
                        if (strcmp(keywords[i].name, current.get_arr()) == 0) {
                            create_token(this, keywords[i].type);
                            reset(&token_flag, this);
                        }
                    }
                }

                if (token_flag == IDENTIFIER && !is_digit) {
                    create_id_token(this, Tok::T_IDENTIFIER, current.get_arr());
                    reset(&token_flag, this);
                }
            }
            else if(!is_digit && token_flag == NUMERIC) {
                create_numeric_token(this, Tok::T_INT_CONST, current.get_arr());
                reset(&token_flag, this);
            }

            if (!is_special_character(*stream)) {
                current.push(*stream);
                if (current.top() == 1) {
                    if(is_digit) 
                        token_flag = NUMERIC;
                    else {
                        if (is_identifier(*stream)) 
                            token_flag = IDENTIFIER;
                        else
                            token_flag = SYMBOL;           
                    }
                }
            }
        }

        stream++;
        current_pos++;
    }
}

Token* Lexer::peek() {
    return current_token;
}

Token* Lexer::next() {
    current_token += sizeof(Token);
    return current_token;
}

Token* Lexer::peek_off(size_t offset) {
    current_token += sizeof(Token) * offset;
    return current_token;
}

void Lexer::log() {
    printf("lexer: Tokenized %d lines of code in '%s'.\n", current_line, file);

    for(int i = 0; i < tokens.top(); i++) {
        Token* token = &tokens.get_arr()[i];
        printf("token: '%s', type: %d, line: %d. pos: %d index: %d.\n", token_to_str(token), token->type, token->line, token->pos, i);
    }
}

const char* token_to_str(Token* token) {
    for(int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (keywords[i].type == token->type) {
            return keywords[i].name;
        }
    }

    switch(token->type) {
        case Tok::T_IDENTIFIER: return token->identifier;
        case Tok::T_INT_CONST:  return std::to_string(token->int_const).c_str();
        default: break;
    }

    return "";
}