// //
// // Created by yanayg on 10/27/19.
// //

// #ifndef MYOS_SUPERBLOCK_H

// //VFS_read_super_t
// //VFS_create_t
// //VFS_remove_t
// //VFS_read_t
// //VFS_write_t
// //VFS_open_t
// //VFS_close_t
// //VFS_readdir_t
// //VFS_finddir_t

// struct VFS_fs_t{
//     VFS_read_super_t read_super;
//     u8 name[255];
//     VFS_fs_t *next;
// } VFS_fs_t;

// struct {
//     u8 storage_device_id;
//     u16 block_size;
//     VFS_fs_t type;
//     VFS_inode_t *mounted; // the mounted root inode
//     VFS_inode_t *covered; // the inode the superblock is covering
//     VFS_create_t create; // create files
//     VFS_remove_t remove; // remove files
//     void *fs_specific;
// } VFS_superblock_t;

// #define MYOS_SUPERBLOCK_H

// #endif //MYOS_SUPERBLOCK_H
