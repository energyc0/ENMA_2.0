#include "garbage_collector.h"
#include "scanner.h"
#include "symtable.h"
#include "utils.h"
#include "vm.h"
#include "scope.h"
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

extern int return_code;

int main(int argc, char** argv){
    if(argc != 2)
        user_error_printf("Usage: %s [input file]\n", argv[0]);
    if(strcmp("--help", argv[1]) == 0 ||
        strcmp("-help", argv[1]) == 0 || 
        strcmp("-h", argv[1]) == 0){
        printf("Usage: %s [input file]\n", argv[0]);
        return 0;
    }

    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL)
        user_error_printf("Failed to open %s: %s\n", argv[1], strerror(errno));

    symtable_init();
    scope_init();
    scanner_init(fp);
#ifdef DEBUG
    scanner_debug_tokens();
    //reinitialize the scanner
    fseek(fp, 0, SEEK_SET);
    scanner_init(fp);
#endif

    vm_interpret();

    symtable_cleanup();
    gc_cleanup();
    
    fclose(fp);
    return return_code;
}