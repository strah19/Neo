#include <stdio.h>

#include "../include/err.h"
#include "../include/arr.h"
#include "../include/lexer.h"
#include "../include/benc.h"
#include "../include/parser.h"

#define INPUT_FILE_INDEX 1

bool no_input_file(char* argv[]) {
    return (argv[INPUT_FILE_INDEX] == nullptr);
}

int main(int argc, char* argv[]) {
    if (no_input_file(argv)) 
        fatal_error("No input files");
    
    size_t filesize;
    Lexer* lexer = Lexer::init(load_file(argv[INPUT_FILE_INDEX], &filesize));
    lexer->file = argv[INPUT_FILE_INDEX];

    begin_debug_benchmark();
    lexer->run();
    end_debug_benchmark("lexer");
    printf("Tokenized %d token!\n", lexer->size);

    Parser* parser = Parser::init(lexer);
    
    parser->run();

    delete lexer;

    free_translation_unit(parser->root);
    delete parser;

    return 0;
}