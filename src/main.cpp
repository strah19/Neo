#include <stdio.h>

#include "../include/err.h"
#include "../include/arr.h"
#include "../include/lexer.h"
#include "../include/benc.h"
#include "../include/parser.h"
#include "../include/c_converter.h"

#define INPUT_FILE_INDEX 1
#define OBJ_NAME_INDEX   2

bool no_input_file(char* argv[]) {
    return (argv[INPUT_FILE_INDEX] == nullptr);
}

bool no_obj_name(char* argv[]) {
    return (argv[OBJ_NAME_INDEX] == nullptr);
}

int main(int argc, char* argv[]) {
    if (no_input_file(argv)) 
        fatal_error("No input files");
    else if (no_obj_name(argv)) 
        fatal_error("No object name");
    
    size_t filesize;
    Lexer* lexer = Lexer::init(load_file(argv[INPUT_FILE_INDEX], &filesize));
    lexer->file = argv[INPUT_FILE_INDEX];

    begin_debug_benchmark();
    lexer->run();
    end_debug_benchmark("lexer");
    printf("Tokenized %d token!\n", lexer->size);

    lexer->log();

    Parser* parser = Parser::init(lexer);
    
    begin_debug_benchmark();
    parser->run();
    end_debug_benchmark("parser");

    convert_transition_unit(argv[2], parser->root);

    delete lexer;

    free_translation_unit(parser->root);
    delete parser;

    return 0;
}