#include "bytecode.h"
#include "vm.h"

int main(int argc, char** argv){
    /*if(argc != 2)
        fatal_printf("One input file expected!\n");

    scanner_init(fopen(argv[1], "r"));
    scanner_debug_tokens();
    */

    vm_init();

    struct bytecode_chunk chunk;
    bcchunk_init(&chunk);

    bcchunk_write_constant(&chunk, 20);
    bcchunk_write_constant(&chunk, 5);
    bcchunk_write_simple_op(&chunk, OP_MUL);
    
    vm_execute(&chunk);

    vm_free();
    return 0;
}