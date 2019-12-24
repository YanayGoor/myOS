#include "../drivers/screen.h"
#include "../drivers/ps2/keyboard.h"
#include "../drivers/chips/pit.h"
#include "../drivers/buses/pci.h"
#include "../drivers/buses/ahci.h"
#include "libc/memory.h"
#include "utils.h"
#include "components/paging.h"
#include "components/storage.h"
#include "components/fs/ext2.h"
#include "components/fs/vfs.h"

/* This will force us to create a kernel entry function instead of jumping to kernel.c:0x00 */
void dummy_test_entrypoint() {
}

void main() {
    char* video_memory = (char*) 0xb8000;
    *video_memory = 'X';

    clear_screen();

    isr_install();
    kprint("initialized isrs\n");


    init_timer(100);
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
    free((void *)address);
    free((void *)address3);
    free((void *)address2);


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
    free((void *)address);
    free((void *)address3);
    free((void *)address2);

    kprint("malloc_a(1200) = ");
    print_int(malloc_a(1200));

    kprint("(uses the same space as freed chunks)\n");

    init_ahci_driver();

    init_ext2();

    pci_detect_devices();

    u32 *buff = (u32 *)malloc(512);
    storage_read(0, 0, 1, (u32)buff);

    u32 *buff2 = (u32 *)malloc(2048);
    storage_read(0, 0, 4, (u32)buff2);

    EXT2_superblock_t *super = (EXT2_superblock_t *)(buff2 + 256);

    print_uint(*buff);
    print_uint((u32)super);

    VFS_mounted_fs_t *fs = get_mounted_file_system_at(0);

    EXT2_inode_t *inode0 = EXT2_read_inode(0, _EXT2_read_superblock(0), 0);
    EXT2_inode_t *inode1 = EXT2_read_inode(0, _EXT2_read_superblock(0), 1);
    EXT2_inode_t *inode2 = EXT2_read_inode(0, _EXT2_read_superblock(0), 2);

    while(1) {}

    //kprint("testing page fault\n");
    //kprint("testing page fault\n");
//    asm("int $6");


//   int *ptr = (int*)0xcccccccc;
//   int do_page_fault = *ptr;
//   print_int(do_page_fault);
//    kprint_at("aaa\nbbb\nccc", 2, 2);
}