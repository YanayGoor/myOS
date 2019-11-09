#include "ext2.h"
#include "../../libc/memory.h"
#include "../../libc/linked_list.h"
#include "../storage.h"

/**
 * returns a buffer of size 1024 bytes containing the superblock
 * the returned buffer should be freed if storage device disconnects 
 */
EXT2_superblock_t *EXT2_read_superblock(u32 device_id) {
    EXT2_superblock_t *buff = (EXT2_superblock_t *)malloc(1024);
    storage_read(device_id, 2, 2, (u32)buff);
    return buff;
}

EXT2_superblock_t *EXT2_write_superblock(u32 device_id, EXT2_superblock_t *buff) {
    return storage_write(device_id, 2, 2, (u32)buff);
}

u32 EXT2_get_block_size(EXT2_superblock_t *superblock) {
    // should be between sector size and page size
    return 1024 << superblock->block_size_exp;
}

u32 EXT2_read_blocks(u32 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 count) {
    u32 block_size = EXT2_get_block_size(superblock) / 512;
    u32 *buff = (EXT2_block_group_descriptor_t *)malloc(block_size);
    storage_read(device_id, 2 + (baddr * block_size), block_size * count, buff);
    return buff;
}

u8 EXT2_write_blocks(u32 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 count, u32 buff) {
    u32 block_size = EXT2_get_block_size(superblock) / 512;
    return storage_read(device_id, 2 + (baddr * block_size), block_size * count, buff);
}

u32 EXT2_read_block(u32 device_id, EXT2_superblock_t *superblock, u32 baddr) {
    return EXT2_read_blocks(device_id, superblock, baddr, 1);
}

u32 EXT2_write_block(u32 device_id, EXT2_superblock_t *superblock, u32 baddr, u32 buffer) {
    return EXT2_write_blocks(device_id, superblock, baddr, 1, buffer);
}

u32 EXT2_get_block_group_count(EXT2_superblock_t *superblock) {
    // should be between sector size and page size
    return superblock->total_blocks / superblock->blocks_in_block_group + !!(superblock->total_blocks % superblock->blocks_in_block_group);
}

EXT2_block_group_descriptor_t *EXT2_read_group_descriptors(u32 device_id, EXT2_superblock_t *superblock) {
    return EXT2_read_block(device_id, superblock, 1);
}

EXT2_block_group_descriptor_t *EXT2_write_group_descriptors(u32 device_id, EXT2_superblock_t *superblock, u32 buff) {
    return EXT2_write_block(device_id, superblock, 1, buff);
}

EXT2_inode_t *EXT2_read_inode(u32 device_id, EXT2_superblock_t *superblock, u32 inode) {
    u32 inodes_per_group = superblock->inodes_in_block_group;
    u32 block_group = (inode - 1) / inodes_per_group;
    u32 index_in_group = (inode - 1) % inodes_per_group;
    u32 inode_size = sizeof(EXT2_inode_t);
    if (superblock->version_major >= 1) inode_size = superblock->extended.inode_size;
    u32 inodes_per_block = EXT2_get_block_size(superblock) / inode_size;

    // get the group descriptor
    EXT2_block_group_descriptor_t *groups = EXT2_read_group_descriptors(device_id, superblock);
    // read the block on the inode table
    u32 *inode_table_block = EXT2_read_blocks(device_id, superblock, groups[block_group].inode_table_baddr + inode / inodes_per_group, 1);
    // copy the desired inode
    EXT2_inode_t *result = malloc(inode_size);
    memory_copy(inode_table_block + inode_size * inode % inodes_per_group, result, inode_size);
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
    if (a > b) return a;
    return b;
}

u32 pow(u32 a, u32 b) {
    u32 result = a;
    int i;
    for (i = 1; i < b; i++) {
        a *= a;
    }
    return result;
}

typedef struct {
    node_t node;
    u32 block;
} EXT2_gather_node_t;

u32 EXT2_transform_gather_nodes(EXT2_superblock_t *superblock, EXT2_gather_node_t *gather) {
    u32 block_size = EXT2_get_block_size(superblock);
    EXT2_gather_node_t *curr = gather;
    storage_vector_node_t *storage_curr;
    while(curr->node.next) {
        if (!storage_curr->node.prev) {
            // if list is empty
            storage_curr->offset = curr->block;
            storage_curr->length = block_size;
            storage_curr->node.prev = 0;
            storage_curr->node.next = 0;
        } else if (((EXT2_gather_node_t *)curr->node.prev)->block == curr->block - 1) {
            // check if contegious - extend last node
           ((storage_vector_node_t *)storage_curr->node.prev)->length += block_size / 512;
        } else {
            // else create new node
            storage_vector_node_t *new_node = (storage_vector_node_t *)malloc(sizeof(storage_vector_node_t));
            storage_curr->node.next = new_node;
            new_node->node.prev = storage_curr;
            new_node->node.next = 0;
            new_node->offset = curr->block * (block_size / 512);
            new_node->length = block_size / 512;
        }

        curr = (EXT2_gather_node_t *)curr->node.next;
        if (curr->node.prev) free(curr->node.prev);
    }
    return storage_curr;
}

void EXT2_read_inode_singly_ptr_vectors(u32 device_id, EXT2_superblock_t *superblock, u32 block, u32 offset, u32 length, EXT2_gather_node_t *gather) {
    u32 block_size = EXT2_get_block_size(superblock);
    u32 block_proportion = block_size / 512;
    u32 *singly_block = (u32 *)EXT2_read_block(device_id, superblock, block);
    int i;
    for (i = offset / block_proportion; i < min(divide_ceil(length, block_proportion), block_size / sizeof(u32)); i++) {
        EXT2_gather_node_t *new_node = (EXT2_gather_node_t *)malloc(sizeof(EXT2_gather_node_t));
        new_node->node.next = 0;
        new_node->node.prev = 0;
        new_node->block = singly_block[i];
        append(gather, new_node);
    }
    free(singly_block);
}

void EXT2_read_inode_double_ptr_vectors(u32 device_id, EXT2_superblock_t *superblock, u32 block, u32 offset, u32 length, EXT2_gather_node_t *gather) {
    u32 block_size = EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    u32 *doubly_block = (u32 *)EXT2_read_block(device_id, superblock, block);
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(length, sectors_in_block), pow(block_size / sizeof(u32), 2)); i+= block_size / sizeof(u32)) {
        EXT2_read_inode_singly_ptr_vectors(device_id, superblock, doubly_block[i], i, length - i, gather);
    }
    free(doubly_block);
}

//TODO: make this recursive
void EXT2_read_inode_triple_ptr_vectors(u32 device_id, EXT2_superblock_t *superblock, u32 block, u32 offset, u32 length, EXT2_gather_node_t *gather) {
    u32 block_size = EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    u32 *triply_block = (u32 *)EXT2_read_block(device_id, superblock, block);
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(length, sectors_in_block), pow(block_size / sizeof(u32), 2)); i+= pow(block_size / sizeof(u32), 2)) {
        EXT2_read_inode_double_ptr_vectors(device_id, superblock, triply_block[i], i, length - i, gather);
    }
    free(triply_block);
}

void EXT2_get_inode_vectors(u32 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length, EXT2_gather_node_t *gather) {
    // offset, length ==> in 512 byte units
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u32 block_size = EXT2_get_block_size(superblock);
    u32 sectors_in_block = block_size / 512;
    int i;
    for (i = offset / sectors_in_block; i < min(divide_ceil(length, sectors_in_block), 12); i++) {
        EXT2_gather_node_t *new_node = (EXT2_gather_node_t *)malloc(sizeof(EXT2_gather_node_t));
        new_node->node.next = 0;
        new_node->node.prev = 0;
        new_node->block = inode->block_ptr[i];
        append(&gather, new_node);
    }
    safe_subtract(&offset, 12 * sectors_in_block);
    safe_subtract(&length,  12 * sectors_in_block);

    // handle block_size / sizeof(pointer)
    EXT2_read_inode_singly_ptr_vectors(device_id, superblock, inode->singly_block_ptr, offset, length, &gather);
    
    safe_subtract(&offset, block_size / sizeof(u32) * sectors_in_block);
    safe_subtract(&length, block_size / sizeof(u32) * sectors_in_block);

    // handle (block_size / sizeof(pointer))^2
    EXT2_read_inode_data_doubly_ptr(device_id, superblock, inode->doubly_block_ptr, offset, length, &gather);
    
    safe_subtract(&offset, block_size / sizeof(u32) * sectors_in_block);
    safe_subtract(&length, block_size / sizeof(u32) * sectors_in_block);

    // handle (block_size / sizeof(pointer))^3
    EXT2_read_inode_data_doubly_ptr(device_id, superblock, inode->doubly_block_ptr, offset, length, &gather);
}

void safe_subtract(u32 *a, u32 b) {
    if (b >= *a) *a = 0;
    if (*a > b) *a -= b; 
}

u32 EXT2_read_inode_data(u32 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length) {
    EXT2_gather_node_t gather;
    EXT2_get_inode_vectors(device_id, superblock, inode_i, offset, length, &gather);
    
    u32 buff = malloc(length);

    // gather should now be a linked list of all blocks to read
    u32 storage_nodes = EXT2_transform_gather_nodes(superblock, &gather);
    storage_read_gather(device_id, storage_nodes, buff);
    return buff;
}

u32 EXT2_get_last_inode_data_block(u32 device_id, EXT2_superblock_t *superblock, u32 inode_i) {
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u64 curr_size = inode->sizel;
    if (superblock->version_major >= 1) curr_size += inode->version_specific2.sizeh << 32;
    u32 block_size = EXT2_get_block_size(superblock);
    curr_size /= block_size; // turn the size to block units
    u32 last_block = 0;

    if (curr_size <= 12) {
        return inode->block_ptr[curr_size];
    } 
    curr_size -= 12;

    // singly ptr
    if (curr_size <= block_size / sizeof(u32)) {
        u32 *buff = EXT2_read_block(device_id, superblock, inode->singly_block_ptr);
        return buff[curr_size];
    }
    curr_size -= block_size / sizeof(u32);

    // doubly ptr
    if (curr_size <= pow(block_size / sizeof(u32), 2)) {
        u32 *doubly = EXT2_read_block(device_id, superblock, inode->doubly_block_ptr);
        u32 *singly = EXT2_read_block(device_id, superblock, doubly[curr_size / (block_size / sizeof(u32))]);
        return singly[curr_size % (block_size / sizeof(u32))];
    }
    curr_size -= pow(block_size / sizeof(u32), 2);

    // tripl y ptr
    if (curr_size <= pow(block_size / sizeof(u32), 3)) {
        u32 *triply = EXT2_read_block(device_id, superblock, inode->doubly_block_ptr);
        u32 *doubly = EXT2_read_block(device_id, superblock, triply[curr_size / pow(block_size / sizeof(u32), 2)]);
        u32 *singly = EXT2_read_block(device_id, superblock, doubly[curr_size % pow(block_size / sizeof(u32), 2) / (block_size / sizeof(u32))]);
        return singly[curr_size % (block_size / sizeof(u32))];
    }
    //illegal size
    return 0;
}

/**
 *  return an u32 integer with the bits between start (inclusive) and end (exclusive) on
 *  (5, 10) => ‭0000‬ 0001 1111 0000‬
 */
u64 bits_between(u64 start, u64 end) {
    return ~((1 << (start - 1)) - 1) ^ ~((1 << (end - 1)) - 1);
}

u32 EXT2_test_bitmap(u32 *bitmap, u32 offset, u32 length) {
    u32 bitmap_index = offset / 32;
    u32 bitmap_offset = offset & 32;
    u64 bitmap_chunk = bitmap[bitmap_index] + bitmap[bitmap_index + 1] << 32;
    return !(bitmap_chunk & bits_between(bitmap_offset, bitmap_offset + length));
}

u32 EXT2_alloc_in_bitmap(u32 *bitmap, u32 offset, u32 length) {
    u32 bitmap_index = offset / 32;
    u32 bitmap_offset = offset & 32;
    bitmap[bitmap_index] |= bits_between(bitmap_offset, bitmap_offset + length);
    bitmap[bitmap_index + 1] |= bits_between(bitmap_offset, bitmap_offset + length) << 32;
}

 u32 EXT2_alloc_inode_data(u32 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 size) {
    EXT2_inode_t *inode = EXT2_read_inode(device_id, superblock, inode_i);
    u64 curr_size = inode->sizel;
    if (superblock->version_major >= 1) curr_size += inode->version_specific2.sizeh << 32;
    curr_size /= EXT2_get_block_size(superblock);
    u32 last_block = EXT2_get_last_inode_data_block(device_id, superblock, inode_i);

    u32 inodes_per_group = superblock->inodes_in_block_group;
    u32 blocks_per_group = superblock->blocks_in_block_group;
    u32 containing_group = (inode_i - 1) / inodes_per_group;
    // get the group descriptor
    EXT2_block_group_descriptor_t *groups = EXT2_read_group_descriptors(device_id, superblock);


    if (!last_block) return 0; // err

    u32 *block_bitmap = EXT2_read_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr);

    // search contegious block (8 blocks)
    if  (EXT2_test_bitmap(block_bitmap, last_block + 1, 8)) {
        EXT2_alloc_in_bitmap(block_bitmap, last_block + 1, 8);
        EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, block_bitmap);
        return 1;
    }

    // search 64th block (8 blocks) if it is in the same group
    if  (blocks_per_group - (last_block % blocks_per_group) > 64 && EXT2_test_bitmap(block_bitmap, last_block + 64, 8)) {
        EXT2_alloc_in_bitmap(block_bitmap, last_block + 64, 8);
        EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, block_bitmap);
        return 1;
    }

    // search block groups (8 blocks)

    // search contegious block (1 block)
    if  (EXT2_test_bitmap(block_bitmap, last_block + 1, 1)) {
        EXT2_alloc_in_bitmap(block_bitmap, last_block + 1, 1);
        EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, block_bitmap);
        return 1;
    }

    // search 64th block (1 block) if it is in the same group
    if  (blocks_per_group - (last_block % blocks_per_group) > 64 && EXT2_test_bitmap(block_bitmap, last_block + 64, 1)) {
        EXT2_alloc_in_bitmap(block_bitmap, last_block + 64, 1);
        EXT2_write_block(device_id, superblock, groups[containing_group].block_usage_bitmap_baddr, block_bitmap);
        return 1;
    }


    // search block groups (1 block)

    // panic
}

u32 EXT2_write_inode_data(u32 device_id, EXT2_superblock_t *superblock, u32 inode_i, u32 offset, u32 length, u32 buff) {
    EXT2_gather_node_t gather;
    EXT2_get_inode_vectors(device_id, superblock, inode_i, offset, length, &gather);
    
    // gather should now be a linked list of all blocks to read
    u32 storage_nodes = EXT2_transform_gather_nodes(superblock, &gather);
    storage_write_scatter(device_id, storage_nodes, buff);
}

u32 EXT2_alloc_inode_in_group(u32 device_id, EXT2_superblock_t *superblock, u32 group) {
    EXT2_block_group_descriptor_t *groups = EXT2_read_group_descriptors(device_id, superblock);
    u8 *bitmap = (u32 *)EXT2_read_block(device_id, superblock, groups[group].inode_usage_bitmap_baddr);
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
                EXT2_write_block(device_id, superblock, groups[group].inode_usage_bitmap_baddr, bitmap);
                EXT2_write_group_descriptors(device_id, superblock, groups);
                EXT2_write_superblock(device_id, superblock);
                return inode_starting_index + i * 8 + o;
            }
            current >> 1;
        }
    }
    free(bitmap);
    free(groups);
    return 0;
}

u32 EXT2_alloc_inode(u32 device_id, EXT2_superblock_t *superblock, u32 dir_group, u8 type) {
    EXT2_block_group_descriptor_t *groups = EXT2_read_group_descriptors(device_id, superblock);
    u32 count = EXT2_get_block_group_count(superblock);
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
                return EXT2_alloc_inode_in_group(device_id, superblock, i);
            } else if (min_directories < groups[dir_group].directories) {
                min_directories = groups[dir_group].unallocated_blocks;
                best_group = i;
            }
        }

        // find the smallest
        free(groups);
        return EXT2_alloc_inode_in_group(device_id, superblock, best_group);

    } else {
        // get the group descriptor
        if (!groups[dir_group].unallocated_inodes) {
            // find free inode
            free(groups);
            return EXT2_alloc_inode_in_group(device_id, superblock, dir_group);
        } else {
            // quadratic search
            // add powers of two
            u32 i;
            for (i = 1; i < count - dir_group; i <<= 1) {
                if (groups[dir_group + i].unallocated_inodes) {
                    free(groups);
                    return EXT2_alloc_inode_in_group(device_id, superblock, (dir_group + i) % count);
                }
            }

            for (i = 1; i < count - dir_group; i += 1) {
                if (groups[dir_group + i].unallocated_inodes) {
                    free(groups);
                    return EXT2_alloc_inode_in_group(device_id, superblock, (dir_group + i) % count);
                }
            }

            return 0;
        }
    }
}
