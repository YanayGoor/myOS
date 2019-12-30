#include "ext2.h"
#include "vfs.h"
#include "../../libc/memory.h"
#include "../../libc/linked_list.h"
#include "../storage.h"

VFS_inode_t *_EXT2_to_VFS_inode(u8 device_id, EXT2_superblock_t *ext2_super, EXT2_inode_t *ext2_inode);
EXT2_superblock_t *_EXT2_read_superblock(u8 device_id);
u32 _EXT2_get_block_size(EXT2_superblock_t *superblock);
EXT2_inode_t *EXT2_read_inode(u8 device_id, EXT2_superblock_t *superblock, u32 inode);
u64 _EXT2_get_inode_content_size(EXT2_superblock_t *superblock, EXT2_inode_t *inode);
EXT2_dir_entry_t *EXT2_readdir(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 index);

VFS_dentry_t *_EXT2_vfs_readdir(VFS_inode_t *inode, u32 index) {
    EXT2_superblock_t *ext2_super = _EXT2_read_superblock(inode->storage_device_id);
    EXT2_dir_entry_t *ext2_dentry = EXT2_readdir(inode->storage_device_id, ext2_super, inode->id, index);
    EXT2_inode_t *ext2_inode = EXT2_read_inode(inode->storage_device_id, ext2_super, ext2_dentry->inode);
    
    VFS_dentry_t *vfs_dentry = (VFS_dentry_t *)malloc(sizeof(VFS_dentry_t));
    vfs_dentry->inode = _EXT2_to_VFS_inode(inode->storage_device_id, ext2_super, ext2_inode);

    u32 name_length = ext2_dentry->name_lengthl;
    if (!ext2_super->version_major) name_length += ext2_dentry->feature_specific.name_lengthh;

    u8 *name = (u8 *)malloc(name_length);
    memory_copy(ext2_dentry->name, name, name_length);

    vfs_dentry->name = name;
    vfs_dentry->parent = inode;
    return vfs_dentry;
}

VFS_dentry_t *_EXT2_vfs_lookup(VFS_inode_t *inode, u8 *name) {

}

u8 _EXT2_vfs_confirm(u8 storage_id) {
    EXT2_superblock_t *superblock = _EXT2_read_superblock(storage_id);
    u8 result = superblock->signature == 0xef53;
    free(superblock);
    return result;
}

VFS_inode_t *_EXT2_to_VFS_inode(u8 device_id, EXT2_superblock_t *ext2_super, EXT2_inode_t *ext2_inode) {
    VFS_inode_t *vfs_inode = (VFS_inode_t *)malloc(sizeof(VFS_inode_t));
    memory_set((char *)vfs_inode, 0, sizeof(VFS_inode_t));
    vfs_inode->storage_device_id = device_id;
    vfs_inode->id = 2;
    vfs_inode->permissions = ext2_inode->permissions;
    vfs_inode->type = ext2_inode->type;
    vfs_inode->size = _EXT2_get_inode_content_size(ext2_super, ext2_inode);
    vfs_inode->last_access_time = ext2_inode->last_access_time;
    vfs_inode->last_modification_time = ext2_inode->last_modification_time;
    vfs_inode->deletion_time = ext2_inode->deletion_time;
    vfs_inode->block_size = _EXT2_get_block_size(ext2_super);
    vfs_inode->readdir = _EXT2_vfs_readdir;
    return vfs_inode;
}

void _EXT2_read_vfs_superblock(u8 device_id) {
    EXT2_superblock_t *ext2_super = _EXT2_read_superblock(device_id);
    EXT2_inode_t *ext2_inode = EXT2_read_inode(device_id, ext2_super, 2);
    
    u32 block_size = _EXT2_get_block_size(ext2_super);
    
    VFS_inode_t *vfs_inode = _EXT2_to_VFS_inode(device_id, ext2_super, ext2_inode);

    VFS_superblock_t *vfs_super = (VFS_superblock_t *)malloc(sizeof(VFS_superblock_t));
    memory_set((char *)vfs_super, 0, sizeof(VFS_superblock_t));
    vfs_super->storage_id = device_id;
    vfs_super->block_size = block_size;
    vfs_super->mounted = vfs_inode;
    //TODO: fs_id
}

void init_ext2() {
    add_file_system("ext2", _EXT2_read_vfs_superblock, _EXT2_vfs_confirm);
}

/**
 * returns a buffer of size 1024 bytes containing the superblock
 * the returned buffer should be freed if storage device disconnects 
 */
EXT2_superblock_t *_EXT2_read_superblock(u8 device_id) {
    EXT2_superblock_t *buff = (EXT2_superblock_t *)malloc(1024);
    storage_read(device_id, 2, 2, (u32)buff);
    return buff;
}

u8 _EXT2_write_superblock(u8 device_id, EXT2_superblock_t *buff) {
    return storage_write(device_id, 2, 2, (u32)buff);
}

u32 _EXT2_get_block_size(EXT2_superblock_t *superblock) {
    // should be between sector size and page size
    return 1024 << superblock->block_size_exp;
}

u32 *_EXT2_read_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 count) {
    u32 block_size = _EXT2_get_block_size(superblock) / 512;
    u32 buff = malloc(block_size * count * 512);
    storage_read(device_id, baddr * block_size, block_size * count, buff);
    return (u32 *)buff;
}

u8 _EXT2_write_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 count, u32 buff) {
    u32 block_size = _EXT2_get_block_size(superblock) / 512;
    return storage_read(device_id, baddr * block_size, block_size * count, buff);
}

u32 *_EXT2_read_block(u8 device_id, EXT2_superblock_t *superblock, u32 baddr) {
    return _EXT2_read_blocks(device_id, superblock, baddr, 1);
}

u8 _EXT2_write_block(u8 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 buffer) {
    return _EXT2_write_blocks(device_id, superblock, baddr, 1, buffer);
}

u32 _EXT2_get_block_group_count(EXT2_superblock_t *superblock) {
    // should be between sector size and page size
    return superblock->total_blocks / superblock->blocks_in_block_group + !!(superblock->total_blocks % superblock->blocks_in_block_group);
}

EXT2_block_group_descriptor_t *_EXT2_read_group_descriptors(u8 device_id, EXT2_superblock_t *superblock) {
    return (EXT2_block_group_descriptor_t *)_EXT2_read_block(device_id, superblock, 2);
}

u32 _EXT2_write_group_descriptors(u8 device_id, EXT2_superblock_t *superblock, u32 buff) {
    return _EXT2_write_block(device_id, superblock, 1, buff);
}

EXT2_inode_t *EXT2_read_inode(u8 device_id, EXT2_superblock_t *superblock, u32 inode) {
    u32 inodes_per_group = superblock->inodes_in_block_group;
    u32 block_group = (inode - 1) / inodes_per_group;
    u32 index_in_group = (inode - 1) % inodes_per_group;
    u32 inode_size = sizeof(EXT2_inode_t);
    if (superblock->version_major >= 1) inode_size = superblock->extended.inode_size;
    u32 inodes_per_block = _EXT2_get_block_size(superblock) / inode_size;

    // get the group descriptor
    EXT2_block_group_descriptor_t *groups = _EXT2_read_group_descriptors(device_id, superblock);
    // read the block on the inode table
    u32 *inode_table_block = _EXT2_read_blocks(device_id, superblock, groups[block_group].inode_table_baddr + (index_in_group / inodes_per_block), 1);
    // copy the desired inode
    EXT2_inode_t *result = (EXT2_inode_t *)malloc(inode_size);
    memory_copy((char *)inode_table_block + (inode_size * (index_in_group % inodes_per_block)), (char *)result, inode_size);
    // release used resources
    //TODO: implement buffer caching for less malloc and free and device reads
    free(inode_table_block);
    free(groups);
    return result;
}

u32 divide_ceil(u32 a, u32 b) {
    return a / b + !!(a % b);
}


u32 min(u32 a, u32 b) {
    if (a < b) return a;
    return b;
}

u32 pow(u32 a, u32 b) {
    if (!b) return 1;
    u32 result = a;
    int i;
    for (i = 1; i < b; i++) {
        a *= a;
    }
    return result;
}


storage_vector_node_t *_EXT2_transform_blocks_to_vectors(EXT2_superblock_t *superblock, u32 *buffer, u32 buffer_size) {
    u32 block_size = _EXT2_get_block_size(superblock);
    storage_vector_node_t *storage_curr = (storage_vector_node_t *)malloc(sizeof(storage_vector_node_t));
    int i;
    for (i = 0; i < buffer_size; i++) {
        if (!storage_curr->node.prev) {
            // if list is empty
            storage_curr->offset = buffer[i];
            storage_curr->length = block_size;
            storage_curr->node.prev = 0;
            storage_curr->node.next = 0;
        } else if (buffer[i] == buffer[i - 1] + 1) {
            // check if contegious - extend last node
            ((storage_vector_node_t *)storage_curr->node.prev)->length += block_size / 512;
        } else {
            storage_vector_node_t *new_node = (storage_vector_node_t *)malloc(sizeof(storage_vector_node_t));
            storage_curr->node.next = (node_t *)new_node;
            new_node->node.prev = (node_t *)storage_curr;
            new_node->node.next = 0;
            new_node->offset = buffer[i] * (block_size / 512);
            new_node->length = block_size / 512;
        }
    }
    return storage_curr;
}

void _EXT2_read_inode_direct_blocks(u8 device_id, EXT2_superblock_t *superblock, EXT2_inode_t *inode, u32 offset, u32 length, u32 *buffer) {
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(offset + length, sectors_in_block), 12); i++) {
        buffer[i] = inode->block_ptr[i];
    }
}

void _EXT2_write_inode_direct_blocks(u8 device_id, EXT2_superblock_t *superblock, EXT2_inode_t *inode, u32 offset, u32 length, u32 *buffer) {
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(offset + length, sectors_in_block), 12); i++) {
        inode->block_ptr[i] = buffer[i];
    }
}

void _EXT2_read_inode_ptr_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 block, u32 depth, u32 offset, u32 length, u32 *buffer) {
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    u32 *ptr_block = (u32 *)_EXT2_read_block(device_id, superblock, block);
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(offset + length, sectors_in_block), pow(block_size / sizeof(u32), depth)); i+= pow(block_size / sizeof(u32), depth - 1)) {
        if (depth == 1) {
            buffer[i] = ptr_block[i];
        } else {
            _EXT2_read_inode_ptr_blocks(device_id, superblock, ptr_block[i], depth - 1, i * sectors_in_block, length, buffer);
        }
    }
    free(ptr_block);
}

void _EXT2_write_inode_ptr_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 block, u32 depth, u32 offset, u32 length, u32 *buffer) {
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    u32 *ptr_block = (u32 *)_EXT2_read_block(device_id, superblock, block);
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(offset + length, sectors_in_block), pow(block_size / sizeof(u32), depth)); i+= pow(block_size / sizeof(u32), depth - 1)) {
        if (depth == 1) {
            ptr_block[i] = buffer[i];
        } else {
            _EXT2_write_inode_ptr_blocks(device_id, superblock, ptr_block[i], depth - 1, i * sectors_in_block, length, buffer);
        }
    }
    free(ptr_block);
}

void safe_subtract(u32 *a, u32 b) {
    if (b >= *a) *a = 0;
    if (*a > b) *a -= b; 
}

void _EXT2_read_inode_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length, u32 *buffer) {
    // offset, length ==> in 512 byte units
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;

    _EXT2_read_inode_direct_blocks(device_id, superblock, inode, offset, length, buffer);
    safe_subtract(&offset, 12 * sectors_in_block);
    safe_subtract(&length,  12 * sectors_in_block);

    // handle block_size / sizeof(pointer)
    _EXT2_read_inode_ptr_blocks(device_id, superblock, inode->singly_block_ptr, 1, offset, length, buffer + 12);
    
    safe_subtract(&offset, block_size / sizeof(u32) * sectors_in_block);
    safe_subtract(&length, block_size / sizeof(u32) * sectors_in_block);
    if(!offset || !length) return;

    // handle (block_size / sizeof(pointer))^2
    _EXT2_read_inode_ptr_blocks(device_id, superblock, inode->doubly_block_ptr, 2, offset, length, buffer + block_size / sizeof(u32));
    
    safe_subtract(&offset, pow(block_size / sizeof(u32) * sectors_in_block, 2));
    safe_subtract(&length, pow(block_size / sizeof(u32) * sectors_in_block, 2));
    if(!offset || !length) return;

    // handle (block_size / sizeof(pointer))^3
    _EXT2_read_inode_ptr_blocks(device_id, superblock, inode->doubly_block_ptr, 3, offset, length, buffer + pow(block_size / sizeof(u32), 2));
}

void _EXT2_write_inode_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length, u32 *buffer) {
    // offset, length ==> in 512 byte units
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;

    _EXT2_write_inode_direct_blocks(device_id, superblock, inode, offset, length, buffer);
    safe_subtract(&offset, 12 * sectors_in_block);
    safe_subtract(&length,  12 * sectors_in_block);

    // handle block_size / sizeof(pointer)
    _EXT2_write_inode_ptr_blocks(device_id, superblock, inode->singly_block_ptr, 1, offset, length, buffer + 12);
    
    safe_subtract(&offset, block_size / sizeof(u32) * sectors_in_block);
    safe_subtract(&length, block_size / sizeof(u32) * sectors_in_block);
    if(!offset || !length) return;

    // handle (block_size / sizeof(pointer))^2
    _EXT2_write_inode_ptr_blocks(device_id, superblock, inode->doubly_block_ptr, 2, offset, length, buffer + block_size / sizeof(u32));
    
    safe_subtract(&offset, pow(block_size / sizeof(u32) * sectors_in_block, 2));
    safe_subtract(&length, pow(block_size / sizeof(u32) * sectors_in_block, 2));
    if(!offset || !length) return;

    // handle (block_size / sizeof(pointer))^3
    _EXT2_write_inode_ptr_blocks(device_id, superblock, inode->doubly_block_ptr, 3, offset, length, buffer + pow(block_size / sizeof(u32), 2));
}

u32 _EXT2_read_inode_block(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset) {
    u32 buff;
    u32 block_size = _EXT2_get_block_size(superblock);
    _EXT2_read_inode_blocks(device_id, superblock, inode_i, offset, block_size, &buff);
    return buff;
}

u32 _EXT2_read_inode_last_block(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i) {
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u64 curr_size = inode->sizel;
    if (superblock->version_major >= 1) curr_size += ((u64)inode->version_specific2.sizeh) << 32;
    u32 block_size = _EXT2_get_block_size(superblock);
    return _EXT2_read_inode_block(device_id, superblock, inode_i, curr_size-block_size);
}

u64 _EXT2_get_inode_content_size(EXT2_superblock_t *superblock, EXT2_inode_t *inode) {
    u64 curr_size = inode->sizel;
    if (superblock->version_major >= 1) curr_size += ((u64)inode->version_specific2.sizeh) << 32;
    return curr_size;
}

u32 _EXT2_append_contagious_inode_blocks(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 starting_block, u32 size) {
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 content_size_in_sectors = _EXT2_get_inode_content_size(superblock, inode) / 512;
    u32 *buffer = (u32 *)malloc(size * sizeof(u32)); 
    u32 i;
    for (i = 0; i < size; i++) {
        buffer[i] = starting_block + i;
    }
    _EXT2_write_inode_blocks(device_id, superblock, inode_i, content_size_in_sectors, size, buffer);
}

/**
 *  return an u32 integer with the bits between start (inclusive) and end (exclusive) on
 *  (5, 10) => ‭0000‬ 0001 1111 0000‬
 */
u64 bits_between(u64 start, u64 end) {
    return ~((1 << (start - 1)) - 1) ^ ~((1 << (end - 1)) - 1);
}

u32 _EXT2_test_bitmap(u32 *bitmap, u32 offset, u32 length) {
    u32 bitmap_index = offset / 32;
    u32 bitmap_offset = offset & 32;
    u64 bitmap_chunk = (u64)bitmap[bitmap_index] + (u64)bitmap[bitmap_index + 1] << 32;
    return !(bitmap_chunk & bits_between(bitmap_offset, bitmap_offset + length));
}

u32 _EXT2_alloc_in_bitmap(u32 *bitmap, u32 offset, u32 length) {
    u32 bitmap_index = offset / 32;
    u32 bitmap_offset = offset & 32;
    bitmap[bitmap_index] |= bits_between(bitmap_offset, bitmap_offset + length);
    bitmap[bitmap_index + 1] |= bits_between(bitmap_offset, bitmap_offset + length) << 32;
}

u64 div_64(u64 a, u32 b) {

}

 u32 _EXT2_alloc_inode_data_block(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 *size) {
     //TODO: make function with size and make this function get last_block as param
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u64 curr_size = inode->sizel;
    if (superblock->version_major >= 1) curr_size += (u64)inode->version_specific2.sizeh << 32;
    curr_size = curr_size >> (superblock->block_size_exp + 10);
    u32 last_block = _EXT2_read_inode_last_block(device_id, superblock, inode_i);

    u32 inodes_per_group = superblock->inodes_in_block_group;
    u32 blocks_per_group = superblock->blocks_in_block_group;
    u32 containing_group = (inode_i - 1) / inodes_per_group;
    // get the group descriptor
    EXT2_block_group_descriptor_t *groups = _EXT2_read_group_descriptors(device_id, superblock);
    u32 group_count = _EXT2_get_block_group_count(superblock);


    if (!last_block) return 0; // err

    u32 *block_bitmap = _EXT2_read_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr);

    // search contegious block (8 blocks)
    if  (_EXT2_test_bitmap(block_bitmap, last_block + 1, 8)) {
        _EXT2_alloc_in_bitmap(block_bitmap, last_block + 1, 8);
        _EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, (u32)block_bitmap);
        groups[containing_group].unallocated_blocks -= 8;
        _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
        *size = 8;
        return last_block + 1;
    }

    // search 64th block (8 blocks) if it is in the same group
    if  (blocks_per_group - (last_block % blocks_per_group) > 64 && _EXT2_test_bitmap(block_bitmap, last_block + 64, 8)) {
        _EXT2_alloc_in_bitmap(block_bitmap, last_block + 64, 8);
        _EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, (u32)block_bitmap);
        groups[containing_group].unallocated_blocks -= 8;
        _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
        *size = 8;
        return last_block + 64;
    }

    // search block groups (8 blocks)
    int i;
    for (i = 1; i < group_count; i <<= 1) {
        if (groups[(containing_group + i) % group_count].unallocated_inodes >= 8) {
            u32 curr_group = (containing_group + i) % group_count;
            u8 *curr_block_bitmap = (u8 *)_EXT2_read_block(device_id, superblock, groups[curr_group].block_usage_bitmap_baddr);
            int o;
            for (o = 0; o < _EXT2_get_block_size(superblock); o++) {
                if (!curr_block_bitmap[o]) {
                    curr_block_bitmap[o] = (u8)-1;
                    _EXT2_write_block(device_id, superblock, groups[curr_group].block_usage_bitmap_baddr, (u32)curr_block_bitmap);
                    groups[curr_group].unallocated_blocks -= 8;
                    _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
                    *size = 8;
                    return o * 8 + (curr_group * blocks_per_group);
                }
            }
        }
    }

    // search contegious block (1 block)
    if  (_EXT2_test_bitmap(block_bitmap, last_block + 1, 1)) {
        _EXT2_alloc_in_bitmap(block_bitmap, last_block + 1, 1);
        _EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, (u32)block_bitmap);
        groups[containing_group].unallocated_blocks -= 1;
        _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
        *size = 1;
        return last_block + 1;
    }

    // search 64th block (1 block) if it is in the same group
    if  (blocks_per_group - (last_block % blocks_per_group) > 64 && _EXT2_test_bitmap(block_bitmap, last_block + 64, 1)) {
        _EXT2_alloc_in_bitmap(block_bitmap, last_block + 64, 1);
        _EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, (u32)block_bitmap);
        groups[containing_group].unallocated_blocks -= 1;
        _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
        *size = 1;
        return last_block + 64;
    }


    // search block groups (1 block)
    for (i = 1; i < group_count; i <<= 1) {
        if (groups[(containing_group + i) % group_count].unallocated_inodes) {
            u32 curr_group = (containing_group + i) % group_count;
            u8 *curr_block_bitmap = (u8 *)_EXT2_read_block(device_id, superblock, groups[curr_group].block_usage_bitmap_baddr);
            int o;
            for (o = 0; o < _EXT2_get_block_size(superblock); o++) {
                if (!curr_block_bitmap[o]) {
                    u8 current = curr_block_bitmap[o];
                    int j;
                    for (j = 0; j < 8; j++) {
                        if (~current & 1) {
                            //mark the bit as taken
                            curr_block_bitmap[o] |= 1 << j;
                            _EXT2_write_block(device_id, superblock, groups[curr_group].inode_usage_bitmap_baddr, (u32)curr_block_bitmap);
                            groups[curr_group].unallocated_blocks -= 1;
                            _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
                            *size = 1;
                            return j * 8 + o + (curr_group * blocks_per_group);
                        }
                        current >> 1;
                    }
                }
            }
        }
    }

    return 0;
}

u32 EXT2_resize_inode_data(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 size) {
    while (size > 0) {
        u32 allocated;
        u32 block = _EXT2_alloc_inode_data_block(device_id, superblock, inode_i, &allocated);
        if (!block) {
            return size;
        }
        _EXT2_append_contagious_inode_blocks(device_id, superblock, inode_i, block, allocated);
        if (size <= allocated) {
            break;
        }
    }
    return 0;
}

u32 EXT2_read_inode_data(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length) {
    u32 block_size = _EXT2_get_block_size(superblock);
    u32 blocks_count = length * 512 / block_size;
    u32 *block_buffer = (u32 *)malloc(blocks_count * sizeof(u32));
    _EXT2_read_inode_blocks(device_id, superblock, inode_i, offset, length, block_buffer);

    u32 buff = malloc(length * 512);

    storage_vector_node_t *storage_nodes = _EXT2_transform_blocks_to_vectors(superblock, block_buffer, blocks_count);
    storage_read_gather(device_id, storage_nodes, buff);
    return buff;
}

u32 EXT2_write_inode_data(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length, u32 *buffer) {
    u32 data_size_in_sectors = _EXT2_get_inode_content_size(superblock, EXT2_read_inode(device_id, superblock, inode_i)) / 512;
    if (offset + length > data_size_in_sectors) {
        EXT2_resize_inode_data(device_id, superblock, inode_i, offset + length - data_size_in_sectors);
    }

    u32 block_size = _EXT2_get_block_size(superblock);
    u32 buffer_size = length * 512 / block_size;
    u32 *block_buffer = (u32 *)malloc(buffer_size);
    _EXT2_read_inode_blocks(device_id, superblock, inode_i, offset, length, block_buffer);

    storage_vector_node_t *storage_nodes = _EXT2_transform_blocks_to_vectors(superblock, block_buffer, buffer_size);
    return storage_write_scatter(device_id, storage_nodes, (u32)buffer);
}

EXT2_dir_entry_t *EXT2_lookup(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, char* name) {
    // TODO: dont read the whole inode data at the same time 
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 inode_data_size_in_sectors = _EXT2_get_inode_content_size(superblock, inode) / 512;
    u8 *dir_entries = (u8 *)EXT2_read_inode_data(device_id, superblock, inode_i, 0, inode_data_size_in_sectors);
    EXT2_dir_entry_t *curr = (EXT2_dir_entry_t *)dir_entries;
    while ((u32)curr < (u32)dir_entries + (inode_data_size_in_sectors * 512)) {
        u32 name_length = curr->name_lengthl;
        if (!superblock->version_major) name_length += curr->feature_specific.name_lengthh;
        u32 i;
        u32 query_length = 0;
        while (!name[query_length]) query_length++;
        
        for (i = 0; i < name_length; i++) {
            if (curr->name[i] == name[i]) {
                break;
            }
        }
        if (i == name_length - 1 && name_length == query_length) {
            EXT2_dir_entry_t *result = (EXT2_dir_entry_t *)malloc(sizeof(EXT2_dir_entry_t) + name_length - 1);
            memory_copy((char *)curr, (char *)result, sizeof(EXT2_dir_entry_t) + name_length - 1);
            free(inode);
            free(dir_entries);
            return curr;
        }
        if(!curr->inode && !curr->name_lengthl && curr->feature_specific.type == EXT2_dir_type_unknown) return 0;
        
        curr = (EXT2_dir_entry_t *)(((u8 *)curr) + curr->entry_size);
    }
    return 0;
}

EXT2_dir_entry_t *EXT2_readdir(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 index) {
    // TODO: dont read the whole inode data at the same time 
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 inode_data_size_in_sectors = _EXT2_get_inode_content_size(superblock, inode) / 512;
    u8 *dir_entries = (u8 *)EXT2_read_inode_data(device_id, superblock, inode_i, 0, inode_data_size_in_sectors);
    EXT2_dir_entry_t *curr = (EXT2_dir_entry_t *)dir_entries;
    int i = 0;
    while ((u32)curr < (u32)dir_entries + (inode_data_size_in_sectors * 512)) {
        u32 name_length = curr->name_lengthl;
        if (!superblock->version_major) name_length += curr->feature_specific.name_lengthh;
        if (i == index) {
            EXT2_dir_entry_t *result = (EXT2_dir_entry_t *)malloc(sizeof(EXT2_dir_entry_t) + name_length - 1);
            memory_copy((char *)curr, (char *)result, sizeof(EXT2_dir_entry_t) + name_length - 1);
            free(inode);
            free(dir_entries);
            return curr;
        }
        if(!curr->inode && !curr->name_lengthl && curr->feature_specific.type == EXT2_dir_type_unknown) return 0;

        curr = (EXT2_dir_entry_t *)(((u8 *)curr) + curr->entry_size);
        i++;
    }
    return 0;
}

EXT2_inode_t *EXT2_create(u8 device_id, EXT2_superblock_t *superblock, u32 inode_i, char* name) {   
}

u32 _EXT2_alloc_inode_in_group(u8 device_id, EXT2_superblock_t *superblock, u32 group) {
    EXT2_block_group_descriptor_t *groups = _EXT2_read_group_descriptors(device_id, superblock);
    u8 *bitmap = (u8 *)_EXT2_read_block(device_id, superblock, groups[group].inode_usage_bitmap_baddr);
    u32 inode_starting_index = superblock->inodes_in_block_group * group;
    // iterate bytes
    u32 i;
    u8 current;
    for (i = 0; i < superblock->inodes_in_block_group; i++) {
        u8 o;
        current = bitmap[i];
        for (o = 0; o < 8; o++) {
            if (~current & 1) {
                //mark the bit as taken
                bitmap[i] |= 1 << o;
                superblock->unallocated_inodes--;
                groups[group].unallocated_inodes--;
                _EXT2_write_block(device_id, superblock, groups[group].inode_usage_bitmap_baddr, (u32)bitmap);
                _EXT2_write_group_descriptors(device_id, superblock, (u32)groups);
                _EXT2_write_superblock(device_id, superblock);
                return inode_starting_index + i * 8 + o;
            }
            current >> 1;
        }
    }
    free(bitmap);
    free(groups);
    return 0;
}

u32 EXT2_alloc_inode(u8 device_id, EXT2_superblock_t *superblock, u32 dir_group, u8 type) {
    EXT2_block_group_descriptor_t *groups = _EXT2_read_group_descriptors(device_id, superblock);
    u32 count = _EXT2_get_block_group_count(superblock);
    if (type == EXT2_inode_type_dir) {
        // find average inodes and blocks
        u32 average_inodes = superblock->unallocated_inodes / count;
        u32 average_blocks = superblock->unallocated_blocks / count;
        
        // use first above below average
        u32 min_directories = groups[0].directories;
        u32 best_group = 0;
        int i;
        for (i = 0; i < count; i++) {
            if (groups[dir_group].unallocated_blocks > average_blocks && groups[dir_group].unallocated_inodes > average_inodes) {
                free(groups);
                return _EXT2_alloc_inode_in_group(device_id, superblock, i);
            } else if (min_directories < groups[dir_group].directories) {
                min_directories = groups[dir_group].unallocated_blocks;
                best_group = i;
            }
        }

        // find the smallest
        free(groups);
        return _EXT2_alloc_inode_in_group(device_id, superblock, best_group);

    } 
    // get the group descriptor
    if (!groups[dir_group].unallocated_inodes) {
        // find free inode
        free(groups);
        return _EXT2_alloc_inode_in_group(device_id, superblock, dir_group);
    }
    // quadratic search
    // add powers of two
    u32 i;
    for (i = 1; i < count; i <<= 1) {
        if (groups[(dir_group + i) % count].unallocated_inodes) {
            free(groups);
            return _EXT2_alloc_inode_in_group(device_id, superblock, (dir_group + i) % count);
        }
    }

    for (i = 1; i < count; i += 1) {
        if (groups[(dir_group + i) % count].unallocated_inodes) {
            free(groups);
            return _EXT2_alloc_inode_in_group(device_id, superblock, (dir_group + i) % count);
        }
    }

    return 0;
}
