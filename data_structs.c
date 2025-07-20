#include "data_structs.h"
#include "utils.h"
#include <stdlib.h>

static inline int tr_hash_char(int c);

struct trie_node* tr_alloc(){
    struct trie_node* ptr = malloc(sizeof(struct trie_node));
    if(ptr == NULL)
        MALLOC_ERROR();
    return ptr;
}
void tr_init(struct trie_node* node, int data, int is_final){
    node->data = data;
    node->is_final = is_final;
    for(int i = 0; i < TRIE_MAX_NODES_SIZE; i++)
        node->children[i] = NULL;
}

int tr_add(struct trie_node* root, char* s, int data){
    for(; *s != '\0'; s++){
        int idx = tr_hash_char(*s);
#ifdef DEBUG
        if(idx == -1)
            fatal_printf("Undefined character in tr_add()!\n");
#endif
        if(!root->children[idx]){
            root->children[idx] = tr_alloc();
            tr_init(root->children[idx], 0, 0);
        }
        root = root->children[idx];

    }
    root->is_final = 1;
    root->data = data;
    return 1;
}

struct trie_node* tr_find(struct trie_node* root, char* s){
    for(;*s != '\0';s++){
        int idx = tr_hash_char(*s);
#ifdef DEBUG
        if(idx == -1)
            fatal_printf("Undefined character in tr_add()!\n");
#endif   
        if(!root->children[idx])
            return NULL;
        root = root->children[idx];
    }
    if(root->is_final)
        return root;
    return NULL;
}

static inline int tr_hash_char(int c){
    #define OFFSET_CAPITAL (0)
    #define OFFSET_LOWER (26)
    #define OFFSET_DIGIT (52)
    #define OFFSET_UNDERLINE (62)

    if('A' <= c && c <= 'Z'){
        return c - 'A';
    }
    if('a' <= c && c <= 'z'){
        return c - 'a' + 26;
    }
    if('0' <= c && c <= '9'){
        return c - '0' + 52;
    }
    if(c == '_'){
        return OFFSET_UNDERLINE;
    }
    return -1;
}