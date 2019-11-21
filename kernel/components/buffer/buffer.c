#include "../../../utils.h"
#include "buffer.h"

/**
 * adler32 hash algorithem
 */
u32 hash(u8 *buffer, u32 length) {
    u32 s1 = 1;
    u32 s2 = 0;

    u32 i;
    for (i = 0; i < length; i++) {
        s1 = (s1 + buffer[i]) % 65521;
        s1 = (s2 + s1) % 65521;
    }
    return (s2 << 16) | s1;
}

u8 _BFR_is_match(buffer_header_t *a, u32 device_id, u32 buffer_id) {
    return a->device_id == device_id && a->buffer_id == buffer_id;
}

u8 _BFR_hash(u32 device_id, u32 buffer_id) {
    u32 ids[2] = { device_id, buffer_id };
    return hash(&ids, 2 * sizeof(u32));
}

#define buffer_from_node(node) ((buffer_header_t *)(((u8 *)node) - (sizeof(u32) * 2)))

buffer_header_t *_BFR_get_from_hashtable(cache_t *cache, u32 device_id, u32 buffer_id) {
    u32 hash_result = _BFR_hash(device_id, buffer_id);
    node_t *curr = (node_t *)cache->entries[hash_result % cache->size];
    while (curr) {
        if (_BFR_is_match(buffer_from_node(curr), device_id, buffer_id)) {
            return buffer_from_node(curr);
        }
        curr = curr->next;
    }
    return 0;
}

void _BFR_insert_to_hashtable(cache_t *cache, buffer_header_t *buffer_header) {
    u32 hash_result = _BFR_hash(buffer_header->device_id, buffer_header->buffer_id);
    if (!cache->entries[hash_result % cache->size]) {
        cache->entries[hash_result % cache->size] = (u32)&buffer_header->hash_table_node;
        buffer_header->hash_table_node.prev = 0;
        return;
    }
    node_t *curr = (node_t *)cache->entries[hash_result % cache->size];
    while (curr) curr = curr->next;
    curr->next = &buffer_header->hash_table_node;
    buffer_header->hash_table_node.prev = curr;
    buffer_header->hash_table_node.next = 0;
}

void _BFR_remove_from_hashtable(cache_t *cache, buffer_header_t *buffer_header) {
    remove(&buffer_header->hash_table_node);
    if(!buffer_header->hash_table_node.prev) {
        u32 hash_result = _BFR_hash(buffer_header->device_id, buffer_header->buffer_id);
        cache->entries[hash_result % cache->size] = 0;
    }
}

void _BFR_insert_to_freelist(cache_t *cache, buffer_header_t *buffer_header) {
    if (!cache->free_list) {
        cache->free_list = &buffer_header->linked_list_node;
        buffer_header->linked_list_node.next = &buffer_header->linked_list_node;
        buffer_header->linked_list_node.prev = &buffer_header->linked_list_node;
    } else {
        insert_after(cache->free_list, &buffer_header->linked_list_node);
    }

}

void _BFR_free_from_freelist(cache_t *cache, u32 amount) {
    if (!cache->free_list) return;
    node_t *curr = cache->free_list->prev;
    while(curr && amount > 0) {
        if (cache->free_list->next == cache->free_list) {
            cache->free_list = 0;
            return amount;
        }
        remove(curr);
        curr = curr->prev;
        amount--;
    }

}

buffer_header_t *_BFR_get_from_cache(cache_t *cache, u32 device_id, u32 buffer_id) {
    buffer_header_t *buffer_header = _BFR_get_from_hashtable(cache, device_id, buffer_id);
    if (!buffer_header->used) {
        remove(&buffer_header->linked_list_node);
        cache->used_count++;
        cache->free_count--;
    }
    buffer_header->used++;
    return buffer_header;
}

void _BFR_free_from_cache(cache_t *cache, buffer_header_t *buffer_header) {
    buffer_header->used--;
    if (!buffer_header->used) {
        _BFR_insert_to_freelist(cache, buffer_header);
    }
    cache->used_count--;
    cache->free_count++;
}

void _BFR_insert_to_cache(cache_t *cache, buffer_header_t *buffer_header) {
    buffer_header->used++;    
    cache->used_count++;
    _BFR_insert_to_hashtable(cache, buffer_header);
    // free from cache
    if (cache->used_count + cache->free_count > cache->size) {
        _BFR_free_from_freelist(cache, cache->size - cache->used_count - cache->free_count);
    }
}

void _BFR_print_cache_description() {}
