#include <stdio.h>
#include "scanner.h"
#include "token.h"
#include "utils.h"

int main(int argc, char** argv){
    if(argc != 2)
        fatal_printf("One input file expected!\n");

    scanner_init(fopen(argv[1], "r"));
    scanner_debug_tokens();
    
    return 0;
}