#include "bytecode.h"

int main(int argc, char** argv){
    struct bytecode_chunk chunk;
    bcchunk_init(&chunk);

    bcchunk_write_code(&chunk, OP_RETURN);

    bcchunk_write_code(&chunk, OP_CONSTANT);
    bcchunk_write_code(&chunk, 0);
    bcchunk_write_data(&chunk, 123);

    bcchunk_write_code(&chunk, OP_RETURN);

    bcchunk_write_code(&chunk, OP_CONSTANT);
    bcchunk_write_code(&chunk, 1);
    bcchunk_write_data(&chunk, 98);

    bcchunk_write_code(&chunk, OP_CONSTANT);
    bcchunk_write_code(&chunk, 0);

    bcchunk_write_code(&chunk, OP_RETURN);

    bcchunk_disassemble("MAIN", &chunk);

    return 0;
}