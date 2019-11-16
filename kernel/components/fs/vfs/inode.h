// //
// // Created by yanayg on 10/27/19.
// //

// #ifndef MYOS_INODE_H

// #include "../../../utils.h"

// struct inode_t {
//     // device
//     u8 storage_device_id;
//     // inode number
//     u32 id;
//     // mode
//     u16 permissions : 12;
//     u16 type : 4;
//     // user ids
//     u32 user_id;
//     u32 group_id;
//     // size in bytes
//     u64 size;
//     // times
//     u32 last_access_time;
//     u32 creation_time;
//     u32 last_modification_time;
//     u32 deletion_time;
//     //block size
//     u16 block_size;
//     // Used by mountpoints and symlinks.
//     struct inode_t *ptr;
//     // inode operations
//     VFS_read_t read;
//     VFS_write_t write;
//     VFS_open_t open;
//     VFS_close_t close;
//     VFS_create_t create // create child of directory
//     VFS_readdir_t readdir; // Returns the n'th child of a directory.
//     VFS_lookup_t lookup; // Try to find a child in a directory by name.
//     // status
//     u32 count;
//     u32 lock;
//     u8 dirty;
//     // fs_specific
//     void *fs_specific;
// } VFS_inode_t;

// struct {

// } VFS_inode_cache_node;

// #define MYOS_INODE_H

// #endif //MYOS_INODE_H
