#include "vfs.h"
#include "../../utils.h"
#include "../drivers/screen.h"

VFS_fs_t filesystems[255];
u32 filesystems_count = 0;

void add_file_system(char *name, read_super_t read_super) {
    filesystems[filesystems_count].name = name;
    filesystems[filesystems_count].read_super = read_super;
    filesystems_count++;
    kprint("added filesystem at ");
    print_uint(filesystems_count);
    kprint(name);
}