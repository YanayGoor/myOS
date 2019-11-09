#ifndef MEMORY_H
#define MEMORY_H

#include "../utils.h"

u32 kmalloc(u32 size, u32 page_aligned, u32 *addr);

u32 kmalloc_page(u32 *addr);


void init_heap(u32 start, u32 size);

void enlarge_heap(u32 size);

u32 malloc(u32 size);

u32 malloc_a(u32 size);

void free(u32 data);

#endif