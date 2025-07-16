#include "scanner.h"
#include "parser.h"
#include "utils.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char** argv){
    if(argc != 2)
        user_error_printf("Usage: %s [input file]\n", argv[0]);
    
    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL)
        user_error_printf("Failed to open %s: %s\n", argv[1], strerror(errno));

    scanner_init(fp);
#ifdef DEBUG
    scanner_debug_tokens();
    //reinitialize the scanner
    fseek(fp, 0, SEEK_SET);
    scanner_init(fp);
#endif

    printf("Parsed expression:\n");
    ast_node* node = ast_generate();
    ast_debug_tree(node);
    /*
    vm_init();

    struct bytecode_chunk chunk;
    bcchunk_init(&chunk);

    bcchunk_write_constant(&chunk, 20);
    bcchunk_write_constant(&chunk, 5);
    bcchunk_write_simple_op(&chunk, OP_MUL);
    
    vm_execute(&chunk);

    vm_free();
    */
    return 0;
}