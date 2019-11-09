#include "../utils.h"
#include "linked_list.h"

/** very simply allocation logic used for the initial paging tables **/

u32 memory_addr = 0x10000;

u32 kmalloc(u32 size, u32 page_aligned, u32 *addr) {
    if (page_aligned && (memory_addr & 0xfff)) {
        memory_addr &= 0xfffff000;
        memory_addr += 0x1000;
    }
    u32 curr = memory_addr;
    *addr = memory_addr;
    memory_addr += size;
    return curr;
}

u32 kmalloc_page(u32 *addr) {
    return kmalloc(1024 * 4, 1, addr);
}


/**
 *  heap implementation
 *  ------------
 *  based on Doug Lea's malloc which is used in the GNU C library.
 *  https://web.archive.org/web/20160325150114/http://g.oswego.edu/dl/html/malloc.html
 **/

u32 heap_start = 0;
u32 heap_size = 0;

typedef struct {
    u8 present;
    u32 size;
    node_t node;
} __attribute__((packed)) chunk_header_t;

typedef struct {
    chunk_header_t *header;
} chunk_footer_t;

u32 log(u32 num) {
    u32 res = 0;
    u32 i;
    for(i = 0; i < 32; i++) {
        if (num & 0x1) {
            res = i;
        }
        num = num >> 1;
    }
    return res;
}

#define BINS_AMOUNT 80
#define MIN_CHUNK_SIZE (sizeof(node_t))

u32 get_bin_index(u32 size) {
    // up to 512 we have y = 8x
    if (size <= 512) {
        return size / 8;
    }
    // between 512 and 576 we need to keep pointing to bin 64
    if (size < 576) {
        return 64;
    }
    // more then 576 we have y = 512 + 2^(x-59)
    u32 res = log(size - 512) + 59;
    if (res > BINS_AMOUNT - 1) res = BINS_AMOUNT - 1;
    return res;
}

#define PRESENT_HEADER_SIZE (sizeof(chunk_header_t) - sizeof(node_t))

u32 bytes_until_alignment(u32 addr) {
    if (addr & 0xfff) return 0x1000 - (addr & 0xfff);
    return 0;
}

node_t *pop_chunk_from_bin(u32 size, node_t **bin, u8 aligned) {
    if (!*bin) return 0;
    node_t *chunk = *bin;
    u32 curr_size = *((u32 *)chunk - 1);
    u32 offset = 0;
    if (aligned) offset = bytes_until_alignment((u32)chunk + PRESENT_HEADER_SIZE);
    while (chunk && curr_size < (size + offset) && (!offset || offset > MIN_CHUNK_SIZE)) {
        chunk = chunk->next;
        if (chunk) {
            curr_size = *((u32 *)chunk - 1);
            offset = 0;
            if (aligned) offset = bytes_until_alignment((u32)chunk + PRESENT_HEADER_SIZE);
        }
    }
    return chunk;
}

chunk_header_t *pop_chunk(u32 size, u8 aligned) {
    u32 index = get_bin_index(size);
    node_t **bin = (node_t **)(heap_start + (index * sizeof(void *)));
    node_t *chunk = pop_chunk_from_bin(size, bin, aligned);
    while (!chunk && index < BINS_AMOUNT - 1) {
        index++;
        bin = (node_t **)(heap_start + (index * sizeof(void *)));
        chunk = pop_chunk_from_bin(size, bin, aligned);
    }
    if (!chunk) {
        return 0;
    }

    if (chunk == *bin) (*bin) = chunk->next;
    remove(chunk);
    return (chunk_header_t *)((u32)chunk - sizeof(chunk_header_t) + sizeof(node_t));
}

void append_chunk(chunk_header_t *chunk) {
    u32 index = get_bin_index(chunk->size);
    node_t **bin = (node_t **)(heap_start + (index * sizeof(void *)));
    node_t *first_chunk = *bin;

    if (first_chunk) insert_before(&chunk->node, first_chunk);
    (*bin) = &(chunk->node);
}

#define FIRST_CHUNK_ADDR (heap_start + BINS_AMOUNT * sizeof(u32))

chunk_header_t *split_chunk(chunk_header_t *chunk, u32 size) {
    u32 prev_size = chunk->size;
    chunk->present = 1;
    chunk->size = size;

    chunk_footer_t *chunk_footer = (chunk_footer_t *)((u32)chunk + size + PRESENT_HEADER_SIZE);
    chunk_footer->header = chunk;


    chunk_header_t *new_chunk_header = (chunk_header_t *)((u32)chunk + size + PRESENT_HEADER_SIZE + sizeof(chunk_footer_t));
    new_chunk_header->present = 0;
    new_chunk_header->size = prev_size - size - sizeof(chunk_footer_t) - PRESENT_HEADER_SIZE;
    new_chunk_header->node.next = 0;
    new_chunk_header->node.prev = 0;

    chunk_footer_t *old_chunk_footer = (chunk_footer_t *)((u32)chunk + prev_size + PRESENT_HEADER_SIZE);
    old_chunk_footer->header = new_chunk_header;

    return (chunk_header_t *)((u32)chunk + size + PRESENT_HEADER_SIZE + sizeof(chunk_footer_t));
}


chunk_header_t *split_chunk_at(chunk_header_t *chunk, u32 size, u32 offset, chunk_header_t **leftover) {
    chunk_header_t *second_chunk = split_chunk(chunk, offset - PRESENT_HEADER_SIZE - sizeof(chunk_footer_t));
    *leftover = split_chunk(second_chunk, size);
    chunk->present = 0;
    second_chunk->present = 1;
    (*leftover)->present = 0;
    return second_chunk;
}

u32 remove_from_bin(node_t *node) {
    int i;
    for (i = 0; i < BINS_AMOUNT; i++) {
        node_t **temp = (node_t **)(heap_start + (i * sizeof(void *)));
        if (*temp == node) {
            *temp = node->next;
        }
    }
    remove(node);
}

chunk_header_t *merge_with_neighbours(chunk_header_t *chunk) {
    chunk_header_t *next_chunk = ((u32)chunk + PRESENT_HEADER_SIZE + chunk->size + sizeof(chunk_footer_t));
    int blocked = 1;
    if (next_chunk && !next_chunk->present && (u32)next_chunk < heap_start + heap_size) {
        chunk->size += next_chunk->size + PRESENT_HEADER_SIZE + sizeof(chunk_footer_t);
        chunk_footer_t *next_chunk_footer = (chunk_footer_t *)((u32)next_chunk + next_chunk->size + PRESENT_HEADER_SIZE);
        next_chunk_footer->header = chunk;
        remove_from_bin(&next_chunk->node);
        blocked = 0;
    }

    chunk_header_t *prev_chunk = ((chunk_footer_t *)((u32)chunk - sizeof(chunk_footer_t)))->header;
    if (prev_chunk && !prev_chunk->present && (u32)prev_chunk >= heap_start) {
        remove_from_bin(&prev_chunk->node);
        prev_chunk->size += chunk->size + PRESENT_HEADER_SIZE + sizeof(chunk_footer_t);
        chunk_footer_t *chunk_footer = (chunk_footer_t *)((u32)chunk + chunk->size + PRESENT_HEADER_SIZE);
        chunk_footer->header = prev_chunk;
        chunk = prev_chunk;
        blocked = 0;
    }
    if (!blocked) chunk = merge_with_neighbours(chunk);
    return chunk;
}

u32 malloc_core(u32 size, u8 aligned) {
//    print_all_chunks();
    if (size < MIN_CHUNK_SIZE) size = MIN_CHUNK_SIZE;
    chunk_header_t *chunk = pop_chunk(size, aligned);
    if (!chunk) return 0;

    int offset = 0;
    if (aligned) offset = bytes_until_alignment((u32)chunk + PRESENT_HEADER_SIZE);
    if (chunk->size - size > MIN_CHUNK_SIZE) {
        if (offset) {
            chunk_header_t *before_leftover = chunk;
            chunk_header_t *after_leftover;
            chunk = split_chunk_at(chunk, size, offset, &after_leftover);
            append_chunk(merge_with_neighbours(before_leftover));
            append_chunk(merge_with_neighbours(after_leftover));
        } else {
            chunk_header_t *leftover = split_chunk(chunk, size);
            append_chunk(merge_with_neighbours(leftover));
        }
    }
    chunk->present = 1;
    return (u32)chunk + PRESENT_HEADER_SIZE;
}

u32 malloc(u32 size) {
    return malloc_core(size, 0);
}

u32 malloc_a(u32 size) {
    return malloc_core(size, 1);
}

void memprint_int(int i) {
    char str[255];
    int_to_ascii(i, str);
    kprint(str);
    kprint("-");
}

void free(u32 data) {
    chunk_header_t *chunk = (chunk_header_t *)(data - PRESENT_HEADER_SIZE);
    chunk->present = 0;
    chunk->node.next = 0;
    chunk->node.prev = 0;
    append_chunk(merge_with_neighbours(chunk));
}

void init_chunk(u32 start, u32 size) {
    chunk_header_t *chunk = (chunk_header_t *)start;
    chunk->present = 0;
    chunk->size = size - PRESENT_HEADER_SIZE - sizeof(chunk_footer_t);
    chunk->node.prev = 0;
    chunk->node.next = 0;
    chunk_footer_t *chunk_footer = (chunk_footer_t *)((u32)chunk + chunk->size + PRESENT_HEADER_SIZE);
    chunk_footer->header = chunk;
    append_chunk(merge_with_neighbours(chunk));
}


void init_heap(u32 start, u32 size) {
    heap_start = start;
    heap_size = size;
    u32 i;
    memory_set((char *)start, 0, BINS_AMOUNT * sizeof(u32));
    init_chunk(FIRST_CHUNK_ADDR, start + size - FIRST_CHUNK_ADDR);
}

void print_all_chunks() {
    kprint("\n");
    chunk_header_t *chunk = (chunk_header_t *)FIRST_CHUNK_ADDR;
    u32 heap_max = heap_start + heap_size;
    while(chunk->size && (u32)chunk < (heap_start + heap_size)) {
        char num[256];
        kprint("[chunk at ");
        uint_to_ascii((u32)chunk, num);
        kprint(num);
        kprint(", size ");
        uint_to_ascii((u32)chunk->size, num);
        kprint(num);
        kprint(", present ");
        uint_to_ascii((u32)chunk->present, num);
        kprint(num);
        chunk_footer_t *chunk_footer = (chunk_footer_t *)((u32)chunk + chunk->size + PRESENT_HEADER_SIZE);
        kprint(", footer_header ");
        uint_to_ascii((u32)chunk_footer->header, num);
        kprint(num);
        kprint(", end ");
        uint_to_ascii((u32)chunk_footer + 1, num);
        kprint(num);
        kprint("]\n");
        chunk = (chunk_header_t *)(chunk_footer + 1);
    }
}

void enlarge_heap(u32 size) {
    u32 prev_heap_size = heap_size;
    heap_size += size;
    init_chunk(heap_start + prev_heap_size, size);
}