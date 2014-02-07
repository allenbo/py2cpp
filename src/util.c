#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define  HASHMOD 15

hashtable_ty
create_hashtable() {
    hashtable_ty ht = (hashtable_ty) malloc( sizeof(hashtable));
    memset(ht, 0, sizeof(hashtable));
    
    ht->cap = HASHMOD;
    ht->mod = HASHMOD;
    int n = ht->cap;
    ht->keys = (char**) malloc(sizeof(char*) * n);
    ht->values = (char** ) malloc( sizeof(char*) * n);
    ht->hashes = (int*) malloc(sizeof(int) * n);

    return ht;
}

void
insert_hashtable(hashtable_ty ht, char* key , char* value) {
    if(NULL == ht || NULL == key|| NULL == value){
        return ;
    }

    if(ht->size == ht->cap) {
        ht->cap += HASHMOD;
        ht->keys = (char**) realloc(ht->keys,  ht->cap * sizeof(char*) );
        ht->values = (char**) realloc(ht->values, ht->cap * sizeof(char*) );
    }
    ht->keys[ht->size] = strdup(key);
    ht->values[ht->size] = strdup(value);
    ht->size ++;
}


char*
search_hashtable(hashtable_ty ht, char* key) {
    if(NULL == ht || NULL == key) {
        return ;
    }

    int i, n = ht->size;
    for(i = 0; i < n ; i ++ ) {
        if(strcmp(ht->keys[i] , key) == 0) {
            return ht->values[i];
        }
    }
    return NULL;
}


char* newTemp() {
    static int i = 0;
    char * tmp = (char* ) malloc( sizeof(char) * 10);
    sprintf(tmp, "_t%d", i++);
    return tmp;
}


/*
int main() {
    hashtable_ty ht = create_hashtable();
    insert_hashtable(ht, "one", "1");
    insert_hashtable(ht, "two", "2");

    char* value = search_hashtable(ht, "one");
    printf("%s\n", value);
}*/
