#ifndef _UTIL_H_
#define _UTIL_H_

typedef struct strstrhashtable  hashtable;
typedef hashtable* hashtable_ty;

/* I should implement a hashtable to hold the map from comprehension
 * address to comprehension variable name
 * For now I just implement in a way that it's gonna insert and find
 * the data from index 0 up to size
 */

struct strstrhashtable {
    char** keys;
    char** values;
    int* hashes;

    int mod;
    int cap;
    int size;
};

hashtable_ty create_hashtable();
void insert_hashtable(hashtable_ty ht, char* , char*);
char* search_hashtable(hashtable_ty ht, char*);

#endif
