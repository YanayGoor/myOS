#include "../utils.h"
#include "linked_list.h"
#include "hashtable.h"
#include "memory.h"

typedef struct {
    node_t node;
    void *key;
    void *value;
} hashtable_enrty_t;

void insert_to_hashtable(hashtable_t *hashtable, void *key, void *entry) {
    hashtable_enrty_t *table_entry = malloc(sizeof(hashtable_enrty_t));
    memory_set((char *)table_entry, 0, sizeof(hashtable_enrty_t));
    table_entry->key = key;
    table_entry->value = entry;
    u32 hash = hashtable->hash(key);
    if (!hashtable->entries[hash % hashtable->size]) {
        hashtable->entries[hash % hashtable->size] = &table_entry->node;
        table_entry->node.prev = 0;
    } else {
        node_t *list = hashtable->entries[hash % hashtable->size];
        append(list, table_entry);
    }
}

hashtable_enrty_t *_get_from_hashtable(hashtable_t *hashtable, void *key) {
    u32 hash = hashtable->hash(key);
    node_t *curr = (node_t *)hashtable->entries[hash % hashtable->size];
    while (curr) {
        if (hashtable->is_match(key, curr)) {
            return ((hashtable_enrty_t *)curr);
        }
        curr = curr->next;
    }
    return 0;
}

void *get_from_hashtable(hashtable_t *hashtable, void *key) {
    hashtable_enrty_t *result = _get_from_hashtable(hashtable, key);
    if (result) return result->value;
    return 0;
}

void remove_from_hashtable(hashtable_t *hashtable, void *key) {
    hashtable_enrty_t *result = _get_from_hashtable(hashtable, key);
    remove((node_t *)result);
    if(!result->node.prev) {
        u32 hash_result = hashtable->hash(key);
        hashtable->entries[hash_result % hashtable->size] = 0;
    }
}

hashtable_t *create_hashtable(u32 size, is_match_t *is_match, hash_t *hash) {
    hashtable_t *hashtable = malloc(sizeof(hashtable_t) + (sizeof(u32) * size));
    memory_set((char *)hashtable, 0, sizeof(hashtable_t) + (sizeof(u32) * size));
    hashtable->size = size;
    hashtable->is_match = is_match;
    hashtable->hash = hash;
    return hashtable;
}
