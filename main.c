#include "ast.h"
#include "bytecode.h"
#include "garbage_collector.h"
#include "lang_types.h"
#include "scanner.h"
#include "parser.h"
#include "symtable.h"
#include "utils.h"
#include "vm.h"
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "hash_table.h"
int main(int argc, char** argv){
    if(argc != 2)
        user_error_printf("Usage: %s [input file]\n", argv[0]);
    
    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL)
        user_error_printf("Failed to open %s: %s\n", argv[1], strerror(errno));

    symtable_init();
    vm_init();
    scanner_init(fp);
#ifdef DEBUG
    scanner_debug_tokens();
    //reinitialize the scanner
    fseek(fp, 0, SEEK_SET);
    scanner_init(fp);
#endif

    ast_node* node;
    struct bytecode_chunk chunk;
    while((node = ast_generate()) != NULL){
#ifdef DEBUG
        printf("Parsed statement:\n");
        ast_debug_tree(node);
        putchar('\n');
#endif
        bcchunk_init(&chunk);
        bcchunk_generate(node, &chunk);
        vm_execute(&chunk);
        bcchunk_free(&chunk);
        ast_freenode(node);
    }

    vm_free();
    symtable_cleanup();
    gc_cleanup();
    /*/
    struct hash_table t;

    table_init(&t);
    const char* s1 = "a1";
    const char* s2 = "a2";
    const char* s3 = "a3";
    obj_id_t* id1 = mk_objid(s1, strlen(s1), hash_string(s1, strlen(s1)));
    obj_id_t* id2 = mk_objid(s2, strlen(s2), hash_string(s2, strlen(s2)));
    obj_id_t* id3 = mk_objid(s3, strlen(s3), hash_string(s3, strlen(s3)));

    table_set(&t, id1, VALUE_NUMBER(1));
    table_set(&t, id2, VALUE_BOOLEAN(true));
    table_set(&t, id3, VALUE_NUMBER(123));

    value_t val;
    table_check(&t, id1, &val);
    table_check(&t, id2, &val);
    table_check(&t, id3, &val);

    table_set(&t, id1, VALUE_BOOLEAN(true));
    table_set(&t, id2, VALUE_BOOLEAN(false));
    table_set(&t, id3, VALUE_NUMBER(11123));

     table_check(&t, id1, &val);
    table_check(&t, id2, &val);
    table_check(&t, id3, &val);

    bool ret;
    ret=table_unset(&t, id1);

    ret=table_check(&t, id1, &val);
    ret=table_check(&t, id2, &val);
    ret=table_check(&t, id3, &val);
    */
    return 0;

}