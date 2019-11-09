#ifndef PAGING_H
#define PAGING_H

typedef struct {
    u8 present : 1;
    u8 read_write : 1;
    u8 user_supervisor : 1;
    u8 write_through : 1;
    u8 cache_disabled : 1;
    u8 accessed : 1;
    u8 always_0 : 1;
    u8 page_size : 1;
    u8 ignored : 1;
    u8 avail : 3;
    u32 address : 20;
} page_dir_entry_t;

typedef struct {
    u8 present : 1;
    u8 read_write : 1;
    u8 user_supervisor : 1;
    u8 write_through : 1;
    u8 cache_disabled : 1;
    u8 accessed : 1;
    u8 dirty : 1;
    u8 always_0 : 1;
    u8 global : 1;
    u8 avail : 3;
    u32 address : 20;
} page_table_entry_t;

void init_paging();

u8 unmap(u32 pd, u32 vaddr);

u32 map_page(u32 pd, u32 paddr, u32 vaddr, u32 flags);

u32 identity_map_page(u32 pd, u32 addr, u32 flags);

#endif