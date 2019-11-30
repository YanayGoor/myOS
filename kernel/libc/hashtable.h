#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "../utils.h"
#include "linked_list.h"

typedef u8 (is_match_t)(void *key, void *entry);
typedef u8 (hash_t)(void *key);

typedef struct {
    u32 size;
    is_match_t *is_match;
    hash_t *hash;
    node_t *entries[];
} hashtable_t;

void insert_to_hashtable(hashtable_t *hashtable, void *key, void *entry);
void *get_from_hashtable(hashtable_t *hashtable, void *key);
void remove_from_hashtable(hashtable_t *hashtable, void *key);

hashtable_t *create_hashtable(u32 size, is_match_t *is_match, hash_t *hash);

#endif

