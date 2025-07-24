#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#define TRIE_MAX_NODES_SIZE (26 + 26 + 10 + 1)

//for keywords and identifiers
struct trie_node{
    struct trie_node* children[TRIE_MAX_NODES_SIZE];
    int is_final;
    int data;
};

struct trie_node* tr_alloc();
void tr_init(struct trie_node* node, int data, int is_final);
void tr_free(struct trie_node* node); //frees 'guts' of the struct and pointer to struct trie_node*

//return 1 if successfully added
//word must only contain characters 'a'-'z', 'A'-'Z', '0'-'9' or '_'
//puts data in the final node
int tr_add(struct trie_node* root, char* s, int data);

//return final node if found 
//return NULL otherwise
struct trie_node* tr_find(struct trie_node* root, char* s);

#endif

