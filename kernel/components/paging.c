#include "../libc/memory.h"
#include "paging.h"
#include "../utils.h"

//void register_interrupt_handler(u8 n, isr_t handler);
u32 curr_pd = 0;

u32 map_page(u32 pd, u32 paddr, u32 vaddr, u32 flags) {
    if (!pd) pd = curr_pd;
    page_dir_entry_t *pd_e = (page_dir_entry_t *)(pd + ((vaddr >> 22) * 4));
    if (!pd_e->present) {
        //create new page table

        u32 t_paddr;
        u32 t_vaddr = kmalloc_page(&t_paddr);
        pd_e->present = 1;
        pd_e->address = t_paddr >> 12;  // we dont care about the offset within the page (it's aligned)
        //TODO: handle flags
    }
    page_table_entry_t *pt_e = (page_table_entry_t *)((pd_e->address << 12) + ((vaddr >> 12 & 0x000003ff) * 4));
    if (pt_e->present) {
        //error
        return -1;
    }
    pt_e->present = 1;
    pt_e->address = paddr >> 12; // we dont care about the offset within the page
    //TODO: handle flags
    return 0;
}

u8 unmap(u32 pd, u32 vaddr) {
    if (!pd) pd = curr_pd;
    page_dir_entry_t *pd_e = (page_dir_entry_t *)(pd + ((vaddr >> 22) * 4));
    if (!pd_e->present) {
        return 0;
    }
    page_table_entry_t *pt_e = (page_table_entry_t *)((pd_e->address << 12) + ((vaddr >> 12 & 0x000003ff) * 4));
    if (pt_e->present) {
        pt_e->present = 0;
        pt_e->address = 0; // we dont care about the offset within the page
        return 0;
    }
    return -1;
}

u32 identity_map_page(u32 pd, u32 addr, u32 flags) {
    return map_page(pd, addr, addr, flags);
}

void switch_page_directory(u32 pd) {
    __asm__ __volatile__("movl %0, %%cr3" :: "r" (pd));
}

void enable_paging() {
   u32 cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void init_paging() {
    u32 paddr;
    u32 vaddr = kmalloc_page(&paddr);
    curr_pd = paddr;
    u32 i;
    for (i = 0; i < 50000; i++) {
        identity_map_page(paddr, 0x1000 * i, 0);
    }
    switch_page_directory(paddr);
    enable_paging();
}