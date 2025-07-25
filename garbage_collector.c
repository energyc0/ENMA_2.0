#include "garbage_collector.h"
#include <string.h>

static obj_t* root = NULL;

void gc_add(obj_t* obj){
    obj->next = root;
    root = obj;
}

void gc_cleanup(){
    for (; root != NULL;) {
        obj_t* temp = root->next;
        obj_free(root);
        root = temp;
    }
}