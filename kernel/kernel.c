#include "../drivers/screen.h"
#include "../drivers/ps2/keyboard.h"
#include "../drivers/chips/pit.h"
#include "../drivers/buses/pci.h"
#include "../drivers/buses/ahci.h"
#include "libc/memory.h"
#include "utils.h"
#include "components/buffer/buffer.h"
#include "components/paging.h"
#include "components/storage.h"

/* This will force us to create a kernel entry function instead of jumping to kernel.c:0x00 */
void dummy_test_entrypoint() {
}

void main() {
    char* video_memory = (char*) 0xb8000;
    *video_memory = 'X';

    clear_screen();

    isr_install();
    kprint("initialized isrs\n");


    init_timer();
    kprint("initialized timer\n");


    init_keyboard();
    kprint("initialized keyboard\n");

    init_paging();
    kprint("initialized paging\n");


    init_heap(0x30000, 0x10000);
    kprint("initialized heap\n");

    kprint("...\n\n");

    kprint("malloc(400) = ");
    int address = malloc(400);
    print_int(address);

    kprint("malloc(400) = ");
    int address2 = malloc(400);
    print_int(address2);

    kprint("malloc(400) = ");
    int address3 = malloc(400);
    print_int(address3);

    kprint("free(address)\n");
    kprint("free(address3)\n");
    kprint("free(address2)\n");
    free(address);
    free(address3);
    free(address2);


    kprint("malloc(1200) = ");
    print_int(malloc(1200));

    kprint("(uses the same space as freed chunks)\n");

    kprint("\naligned to page:\n");

    kprint("malloc_a(400) = ");
    address = malloc_a(400);
    print_int(address);

    kprint("malloc_a(400) = ");
    address2 = malloc_a(400);
    print_int(address2);

    kprint("malloc_a(400) = ");
    address3 = malloc_a(400);
    print_int(address3);

    kprint("free(address)\n");
    kprint("free(address3)\n");
    kprint("free(address2)\n");
    free(address);
    free(address3);
    free(address2);

    kprint("malloc_a(1200) = ");
    print_int(malloc_a(1200));

    kprint("(uses the same space as freed chunks)");

    init_ahci_driver();

    pci_detect_devices();

    u32 *buff = (u32 *)malloc(512);
    storage_read(0, 0, 1, (u32)buff);

    print_uint(*buff);

    cache_t *cache = (cache_t *)malloc(sizeof(cache_t) + (sizeof(u32) * 100));
    memory_set((char *)cache, 0, sizeof(cache_t) + (sizeof(u32) * 100));
    cache->size = 100;
    
    _BFR_print_cache_description(cache);

    buffer_header_t buffer_header;
    memory_set(&buffer_header, 0, sizeof(buffer_header_t));
    buffer_header.buffer_id = 5;

    _BFR_insert_to_cache(cache, &buffer_header);

    kprint("\n");
    kprint("\n");

    _BFR_print_cache_description(cache);

    kprint("\n");

    _BFR_print_buffer_header(_BFR_get_from_cache(cache, 0, 5));

    kprint("\n");
    kprint("\n");

    _BFR_print_cache_description(cache);

    kprint("\n");
    kprint("\n");

    _BFR_free_from_cache(cache, &buffer_header);

    _BFR_print_cache_description(cache);

    while(1) {}

    //kprint("testing page fault\n");
    //kprint("testing page fault\n");
//    asm("int $6");


//   int *ptr = (int*)0xcccccccc;
//   int do_page_fault = *ptr;
//   print_int(do_page_fault);
//    kprint_at("aaa\nbbb\nccc", 2, 2);
}