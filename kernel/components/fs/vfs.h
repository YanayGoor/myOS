#ifndef VFS_H
#define VFS_H

#include "../../utils.h"

typedef struct inode_t VFS_inode_t;
typedef struct VFS_fs_t VFS_fs_t;

typedef struct {
    u8 *name;
    VFS_inode_t *inode;
    VFS_inode_t *parent;
} VFS_dentry_t;

typedef VFS_dentry_t *(VFS_readdir_t)(VFS_inode_t *, u32);
typedef VFS_dentry_t *(VFS_lookup_t)(VFS_inode_t *, u8 *);

typedef struct inode_t {
    // device
    u8 storage_device_id;
    // inode number
    u32 id;
    // mode
    u16 permissions : 12;
    u16 type : 4;
    // user ids
    u32 user_id;
    u32 group_id;
    // size in bytes
    u64 size;
    // times
    u32 last_access_time;
    u32 creation_time;
    u32 last_modification_time;
    u32 deletion_time;
    //block size
    u16 block_size;
    // Used by mountpoints and symlinks.
    struct inode_t *ptr;
    // inode operations
    // VFS_read_t read;
    // VFS_write_t write;
    // VFS_open_t open;
    // VFS_close_t close;
    // VFS_create_t create // create child of directory
    VFS_readdir_t *readdir; // Returns the n'th child of a directory.
    VFS_lookup_t *lookup; // Try to find a child in a directory by name.
    // status
    u32 count;
    u32 lock;
    u8 dirty;
    // fs_specific
    void *fs_specific;
} VFS_inode_t;

typedef struct {
    u8 storage_id;
    u16 block_size;
    VFS_fs_t *type;
    VFS_inode_t *mounted; // the mounted root inode
    VFS_inode_t *covered; // the inode the superblock is covering
//     VFS_create_t create; // create inode
//     VFS_remove_t remove; // remove inode
    void *fs_specific;
} VFS_superblock_t;

typedef void (read_super_t)(u8 storage_id);

typedef struct VFS_fs_t {
    read_super_t *read_super;
    char *name;
} VFS_fs_t;

void add_file_system(char *name, read_super_t read_super);


#endif