#ifndef BUFFER_H
#define BUFFER_H

#include "../../libc/linked_list.h"
#include "../../utils.h"

// buffer header
typedef struct {
    u32 device_id;
    u32 buffer_id;
    node_t hash_table_node;
    node_t linked_list_node;
    u8 dirty;
    u8 used;
    u32 *data;
} buffer_header_t;

typedef struct {
    u32 size;
    u32 used_count;
    u32 free_count;
    node_t *free_list;
    node_t *entries[];
} cache_t;

// keep a hash table of device_id+block_id
// keep a linked list of all free blocks in the cache
// append to the linked list from the start so rarely used blocks are pushed to the back
// keep count of the total amount of buffers in cahce
// when total amount larger then [const] free from the end of the free list


// read from buffer cache
// write to buffer cache

#endif