#include "vfs.h"
#include "../../utils.h"
#include "../../../drivers/screen.h"

VFS_fs_t filesystems[255];
u8 filesystems_count = 0;

VFS_mounted_fs_t mounted_filesystems[255];
u8 mounted_filesystems_count = 0;

void add_file_system(char *name, read_super_t read_super, confirm_partition_t confirm_partition) {
    filesystems[filesystems_count].name = name;
    filesystems[filesystems_count].read_super = read_super;
    filesystems[filesystems_count].confirm_partition = confirm_partition;
    kprint("\nadded filesystem at ");
    print_uint(filesystems_count);
    kprint("fs name: ");
    kprint(name);
    kprint("\n\n");
    filesystems_count++;
}

VFS_fs_t *get_file_system_at(u8 fs_id) {
    if (fs_id > filesystems_count - 1) return 0;
    return &filesystems[fs_id];
}

void mount_file_system(u8 fs_id, u8 storage_id, VFS_inode_t *covered) {
    mounted_filesystems[mounted_filesystems_count].fs = fs_id;
    mounted_filesystems[mounted_filesystems_count].storage_id = storage_id;
    VFS_fs_t *fs = get_file_system_at(fs_id);
    VFS_superblock_t *super = fs->read_super(storage_id);
    mounted_filesystems[mounted_filesystems_count].superblock = super;
    if (covered) {
        super->covered = covered;
        covered->ptr = super->mounted;
    }
    kprint("\nmounted filesystem at ");
    print_uint(mounted_filesystems_count);
    kprint("fs name: ");
    kprint(get_file_system_at(fs_id)->name);
    kprint("\n");
    mounted_filesystems_count++;
}

VFS_mounted_fs_t *get_mounted_file_system_at(u8 mounted_fs_id) {
    if (mounted_fs_id > (s16)mounted_filesystems_count - 1) return 0;
    return &mounted_filesystems[mounted_fs_id];
}