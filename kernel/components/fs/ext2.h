//
// Created by yanayg on 10/25/19.
//

#ifndef MYOS_EXT2_H

#include "../../utils.h"

#define EXT2_state_clean 1
#define EXT2_state_errors 2

#define EXT2_err_handling_ignore 1
#define EXT2_err_handling_remount 2
#define EXT2_err_handling_panic 3

#define EXT2_os_id_linux 0
#define EXT2_os_id_GNU_HURD 1
#define EXT2_os_id_MASIX 2
#define EXT2_os_id_freeBSD 3
#define EXT2_os_id_other_lites 4

#define EXT2_optional_feats_preallocate_dirs 0x1
#define EXT2_optional_feats_AFS_server_inodes 0x2
#define EXT2_optional_feats_fs_has_journal 0x4
#define EXT2_optional_feats_extended_inodes 0x8
#define EXT2_optional_feats_resizable_fs 0x10
#define EXT2_optional_feats_hash_index_firs 0x20

#define EXT2_required_feats_compression 0x1
#define EXT2_required_feats_dir_types 0x2
#define EXT2_required_feats_replay_journal 0x4
#define EXT2_required_feats_journal_device 0x8

#define EXT2_read_only_feats_sparse 0x1
#define EXT2_read_only_feats_64_bit_size 0x2
#define EXT2_read_only_feats_binary_tree_dir 0x4

#define EXT2_inode_type_FIFO 0x1
#define EXT2_inode_type_char_device 0x2
#define EXT2_inode_type_dir 0x4
#define EXT2_inode_type_block_device 0x6
#define EXT2_inode_type_regular 0x8
#define EXT2_inode_type_symlink 0xa
#define EXT2_inode_type_socket 0xc

#define EXT2_dir_type_regular 1
#define EXT2_dir_type_dir 2
#define EXT2_dir_type_block_device 3
#define EXT2_dir_type_char_device 4
#define EXT2_dir_type_FIFO 5
#define EXT2_dir_type_socket 6
#define EXT2_dir_type_symlink 7

typedef struct {
    u32 inode;
    u16 entry_size;
    u8 name_lengthl;
    union {
        u8 name_lengthh;
        u8 type;
    } feature_specific;
    u8 name[1];
} EXT2_dir_entry_t;

typedef struct {
    u16 permissions : 12;
    u16 type : 4;
    u16 user_id;
    u32 sizel;
    u32 last_access_time;
    u32 creation_time;
    u32 last_modification_time;
    u32 deletion_time;
    u16 group_id;
    u16 hard_link_count;
    u32 disk_sectors_count;
    u32 flags;
    u32 os_value1;
    u32 block_ptr[12];
    u32 singly_block_ptr;
    u32 doubly_block_ptr;
    u32 triply_block_ptr;
    u32 generation_number;
    union {
        u32 rsv;
        u32 extended_attr_block;
    } version_specific1;
    union {
        u32 rsv;
        u32 sizeh;
    } version_specific2;
    u32 fragment_baddr;
    u32 os_value2[3];
} EXT2_inode_t;

typedef struct {
    u32 block_usage_bitmap_baddr;
    u32 inode_usage_bitmap_baddr;
    u32 inode_table_baddr;
    u16 unallocated_blocks;
    u16 unallocated_inodes;
    u16 directories;
    u8 rsv[14];
} EXT2_block_group_descriptor_t;

typedef struct {
    u32 first_non_reserved_inode;
    u16 inode_size;
    u16 containing_block_group;
    u32 optional_features_present;
    u32 required_features_present;
    u32 read_only_features_present;
    u8 fs_id[16];
    u8 volume_name[16];
    u8 last_mounted_to_path[64];
    u32 compression_used;
    u8 preallocate_for_files;
    u8 preallocate_for_dirs;
    u16 rsv;
    u8 journal_id[16];
    u32 journal_inode;
    u32 journal_device;
    u32 orphan_inode_head;
} EXT2_superblock_extended_t;

typedef struct {
    u32 total_inodes;
    u32 total_blocks;
    u32 reserved_blocks;
    u32 unallocated_blocks;
    u32 unallocated_inodes;
    u32 containing_block;
    u32 block_size_exp;
    u32 fragment_size_exp;
    u32 blocks_in_block_group;
    u32 fragments_in_block_group;
    u32 inodes_in_block_group;
    u32 last_mount_time;
    u32 last_written_time;
    u16 mounts_since_consistency_check;
    u16 max_mounts_since_consistency_check;
    u16 signature;
    u16 state;
    u16 err_handling;
    u16 version_minor;
    u16 consistency_check_time;
    u16 consistency_check_interval;
    u16 os_id;
    u16 version_major;
    u16 super_user_id;
    u16 super_group_id;
    EXT2_superblock_extended_t extended;
} EXT2_superblock_t;

#define MYOS_EXT2_H

#endif //MYOS_EXT2_H
