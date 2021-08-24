#include "../include/lexer.h"
#include "../include/err.h"
#include "../include/sym.h"

#include <ctype.h>
#include <string>  
#include <sys/stat.h>

static SymTable keywords;
static SymTable symbols;

#define REALLOC_TOKEN_SIZE 512

enum {
    IDENTIFIER = 1,
    NUMERIC,
    SYMBOL,
    SINGLE_LINE_COMMENT,
    MULTI_LINE_COMMENT
};

uint8_t* backtrack_symbol_position = 0;

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

Lexer* Lexer::init(uint8_t* stream) {
    Lexer* lexer = new Lexer;
    lexer->stream = stream;
    lexer->tokens = (Token*) malloc(sizeof(Token) * REALLOC_TOKEN_SIZE);
    lexer->size = 0;
    lexer->allocated_size = REALLOC_TOKEN_SIZE;

    lexer->current_line = lexer->current_pos = 1;

    keywords.insert("if", Tok::T_IF);
    keywords.insert("else", Tok::T_ELSE);
    keywords.insert("elif", Tok::T_ELIF);
    keywords.insert("while", Tok::T_WHILE);
    keywords.insert("continue", Tok::T_CONTINUE);
    keywords.insert("return", Tok::T_RETURN);
    keywords.insert("break", Tok::T_BREAK);
    keywords.insert("int", Tok::T_INT);
    keywords.insert("boolean", Tok::T_BOOLEAN);
    keywords.insert("byte", Tok::T_BYTE);
    keywords.insert("double", Tok::T_DOUBLE);
    keywords.insert("float", Tok::T_FLOAT);
    keywords.insert("foreign", Tok::T_FOREIGN);
    keywords.insert("return", Tok::T_RETURN);
    keywords.insert("from", Tok::T_FROM);
    keywords.insert("constant", Tok::T_CONST);

    symbols.insert(":=", Tok::T_COLON_ASSIGN);
    symbols.insert("<=", Tok::T_LTE);
    symbols.insert(">=", Tok::T_GTE);
    symbols.insert("!=", Tok::T_NOT_EQUAL);
    symbols.insert("==", Tok::T_COMPARE_EQUAL);
    symbols.insert("++", Tok::T_INC);
    symbols.insert("--", Tok::T_DEC);
    symbols.insert("->", Tok::T_DASH_ARROW);

    backtrack_symbol_position = 0;
    return lexer;
}

void new_line(int* type, Lexer* lexer) {
    if (*lexer->stream == '\n') {
        lexer->current_line++;
        lexer->current_pos = 1;

        if (*type == SINGLE_LINE_COMMENT)
            *type = 0;
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
    if (lexer->size + 1 > lexer->allocated_size) {
        uint32_t new_size = (sizeof(lexer->tokens) * 2);
        lexer->tokens = (Token*) realloc(lexer->tokens, (sizeof(struct Token) * lexer->size) * 2);
        if (!lexer->tokens) 
            fatal_error("could not resize token memory.\n");
        lexer->allocated_size = lexer->size * 2;
    }
}

void create_token(Lexer* lexer, int type) {
    check_for_overflow(lexer);
    lexer->tokens[lexer->size] = fill_token(type, lexer->current_pos, lexer->current_line);
    lexer->size++;
}

void create_id_token(Lexer* lexer, int type, const char* name) {
    Token t = fill_token(type, lexer->current_pos, lexer->current_line);

    strcpy(t.identifier, name);

    check_for_overflow(lexer);
    lexer->tokens[lexer->size] = t;
    lexer->size++;
}

void create_numeric_token(Lexer* lexer, int type, const char* int_const) {
    Token t = fill_token(type, lexer->current_pos, lexer->current_line);
    t.int_const = atoi(int_const);

    check_for_overflow(lexer);
    lexer->tokens[lexer->size] = t;
    lexer->size++;
}

void create_char_const_token(Lexer* lexer) {
    Token t = fill_token(Tok::T_CHAR_CONST, lexer->current_pos, lexer->current_line);
    t.char_const = *lexer->stream;

    check_for_overflow(lexer);
    lexer->tokens[lexer->size] = t;
    lexer->size++;    
}

bool is_special_character(char character) {
    return (character == ' ' || character == '\n' || character == '\t');
}

void reset(int* type, Lexer* lexer) {
    *type = 0;
    memset(lexer->current, 0, MAX_TOKEN_SIZE);
    lexer->current_len = 0;
}

int get_type_of_token(char character) {
    if (isdigit(character)) 
        return NUMERIC;
    else if (is_identifier(character))
        return IDENTIFIER;

    return (!is_special_character(character)) ? SYMBOL : 0;
}

void create_symbol(Lexer* lexer, int* type) {
    for (int i = lexer->current_len - 1; i >= 0; i--) {
        lexer->current[i + 1] = '\0';

        Entry* e = symbols.look_up(lexer->current);

        if (e) {
            create_token(lexer, e->type);   
            reset(type, lexer);
            lexer->stream = backtrack_symbol_position + i + 1;
            return;
        }
    }

    create_token(lexer, lexer->current[0]);
    lexer->stream = backtrack_symbol_position + 1;
    reset(type, lexer);
}

void single_line_comment(Lexer* lexer, int* type) {
    if (*lexer->stream == '/' && *(lexer->stream + 1) == '/') {
        if (*type == 0)
            *type = SINGLE_LINE_COMMENT;
        lexer->stream++;
    }
}

void multi_line_comment(Lexer* lexer, int* type, int* nested) {
    if(*lexer->stream == '{' && *(lexer->stream + 1) == '-') {
        (*nested)++;
        *type = MULTI_LINE_COMMENT;
        lexer->stream++;
    }
    else if(*lexer->stream == '-' && *(lexer->stream + 1) == '}') {
        (*nested)--;
        if (*nested == 0) {
            *type = 0;  
        }
        lexer->stream++;
    }
}

void Lexer::run() {
    int type = 0;
    int nested = 0;

    while(*stream != '`') {   
        new_line(&type, this);

        multi_line_comment(this, &type, &nested);
        
        if (type != SINGLE_LINE_COMMENT && type != MULTI_LINE_COMMENT) {
            single_line_comment(this, &type);
            if (type == IDENTIFIER && !is_identifier(*stream)) {
                Entry* e = keywords.look_up(current);
                if (e) {
                    create_token(this, e->type);
                    reset(&type, this);
                }
                else if (!isdigit(*stream)) {
                    create_id_token(this, Tok::T_IDENTIFIER, current);
                    reset(&type, this);
                }
            }
            else if (!isdigit(*stream) && type == NUMERIC) {
                create_numeric_token(this, Tok::T_INT_CONST, current);
                reset(&type, this);
            }
            else if (type == SYMBOL && get_type_of_token(*stream) != SYMBOL) {             
                create_symbol(this, &type);
            }

            if (!is_special_character(*stream)) {
                current[current_len++] = *stream;
                if (current_len == 1) {
                    type = get_type_of_token(*stream);

                    if (*stream == '\'') {
                        stream++;
                        create_char_const_token(this);
                        stream++;
                        current_pos += 2;
                        reset(&type, this);
                    }
                    else if (type == SYMBOL) {
                        backtrack_symbol_position = stream;
                    }
                }
            }
        }

        stream++;
        current_pos++;
    }

    create_token(this, Tok::T_EOF);
}

void Lexer::log() {
    printf("lexer: Tokenized %d lines of code in '%s'.\n", current_line, file);

    for(int i = 0; i < size; i++) 
        log_token(&tokens[i]);
}

const char* token_to_str(Token* token) {
    Entry* e = keywords.look_up_by_type(token->type);
    if (e) 
        return e->name;

    e = symbols.look_up_by_type(token->type);
    if (e) 
        return e->name;

    static char single_char_token[2] = { '\0' };

    switch(token->type) {
        case Tok::T_IDENTIFIER: return token->identifier;
        case Tok::T_INT_CONST:  return std::to_string(token->int_const).c_str();
        case Tok::T_CHAR_CONST:  single_char_token[0] = token->char_const; return single_char_token;
        case Tok::T_EOF:        return "End of file";
        default: break;
    }

    single_char_token[0] = token->type;
    return single_char_token;
}

const char* type_to_str(int type) {
    Entry* e = keywords.look_up_by_type(type);
    if (e) 
        return e->name;

    e = symbols.look_up_by_type(type);
    if (e) 
        return e->name;

    switch(type) {
        case Tok::T_IDENTIFIER: return "Identifier";
        case Tok::T_INT_CONST:  return "Int Const";
        case Tok::T_EOF:        return "End of file";
        default: break;
    }

    static char single_char_token[2] = { '\0' };
    single_char_token[0] = type;

    return single_char_token;
}

void log_token(Token* token) {
    printf("token: '%s', type: %d, line: %d. pos: %d.\n", token_to_str(token), token->type, token->line, token->pos);
}