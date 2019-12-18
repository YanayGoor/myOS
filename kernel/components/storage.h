//
// Created by yanayg on 10/19/19.
//

#ifndef STORAGE_H

#include "../utils.h"
#include "../libc/linked_list.h"

typedef struct {
    node_t node;
    u32 offset;
    u32 length;
} storage_vector_node_t;

typedef u8 (storage_operation_t)(volatile void *device, u32 start, u32 length, u32 buffer);
typedef u8 (storage_vectored_operation_t)(volatile void *device, storage_vector_node_t *nodes, u32 buffer);
typedef u8 (storage_release_t)(volatile void *device);

typedef struct {
    storage_operation_t *read;
    storage_vectored_operation_t *read_gather;
    storage_operation_t *write;
    storage_vectored_operation_t *write_scatter;
    storage_release_t *release;
    volatile void *driver_info;
} storage_device_t;

typedef struct {
    u8 active;
    u8 start_head;
    u8 start_sector : 6;
    u16 start_cylinder : 10;
    u8 system_id;
    u8 end_head;
    u8 end_sector : 6;
    u16 end_cylinder : 10;
    u32 sector_offset;
    u32 sector_count;
} partition_table_entry_t;

typedef struct {
    u8 index;
    partition_table_entry_t entry;
    volatile storage_device_t *device_driver_info;
} partition_driver_info_t;

typedef struct {
    u8 bootloader[0x01BE];
    partition_table_entry_t partition_table_entry[4];
    u16 signature;
} __attribute__((packed)) mbr_t;

void add_storage_device(volatile void *driver_info, storage_operation_t read, storage_operation_t write, storage_vectored_operation_t read_gather, storage_vectored_operation_t write_scatter);

u8 storage_read(u32 storage_index, u32 start, u32 length, u32 buffer);
u8 storage_read_gather(u32 storage_index,storage_vector_node_t *nodes, u32 buffer);
u8 storage_write(u32 storage_index, u32 start, u32 length, u32 buffer);
u8 storage_write_scatter(u32 storage_index, storage_vector_node_t *nodes, u32 buffer);

#define STORAGE_H

#endif //MYOS_STORAGE_H
