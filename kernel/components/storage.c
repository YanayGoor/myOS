//
// Created by yanayg on 10/19/19.
//

#include "../utils.h"
#include "../libc/memory.h"
#include "../libc/linked_list.h"
#include "storage.h"

storage_device_t storage_devices[256];
u8 storage_devices_len = 0;

#define PAR_INFO ((partition_driver_info_t *)info)

u8 partition_read(volatile void *info, u32 start, u32 length, u32 buffer) {
    u32 offset = PAR_INFO->entry.sector_offset;
    return PAR_INFO->device_driver_info->read(PAR_INFO->device_driver_info->driver_info, start + offset, length, buffer);
}

u8 partition_write(volatile void *info, u32 start, u32 length, u32 buffer) {
    u32 offset = PAR_INFO->entry.sector_offset;
    return PAR_INFO->device_driver_info->write(PAR_INFO->device_driver_info->driver_info, start + offset, length, buffer);
}

u8 partition_read_gather(volatile void *info, storage_vector_node_t *nodes, u32 buffer) {
    u32 offset = PAR_INFO->entry.sector_offset;
    return PAR_INFO->device_driver_info->read_gather(PAR_INFO->device_driver_info->driver_info, nodes, buffer);
}

u8 partition_write_scatter(volatile void *info, storage_vector_node_t *nodes, u32 buffer) {
    u32 offset = PAR_INFO->entry.sector_offset;
    return PAR_INFO->device_driver_info->write_scatter(PAR_INFO->device_driver_info->driver_info, nodes, buffer);
}

u8 storage_read(u32 storage_index, u32 start, u32 length, u32 buffer) {
    if (storage_index > storage_devices_len) return 0;
    storage_device_t device = storage_devices[storage_index];
    if (!device.driver_info || !device.read) return 0;
    return device.read(device.driver_info, start, length, buffer);
}
u8 storage_read_gather(u32 storage_index, storage_vector_node_t *nodes, u32 buffer) {
    if (storage_index > storage_devices_len) return 0;
    storage_device_t device = storage_devices[storage_index];
    if (!device.driver_info || !device.read_gather) return 0;
    return device.read_gather(device.driver_info, nodes, buffer);
}
u8 storage_write_scatter(u32 storage_index, storage_vector_node_t *nodes, u32 buffer) {
    if (storage_index > storage_devices_len) return 0;
    storage_device_t device = storage_devices[storage_index];
    if (!device.driver_info || !device.read_gather) return 0;
    return device.read_gather(device.driver_info, nodes, buffer);
}
u8 storage_write(u32 storage_index, u32 start, u32 length, u32 buffer) {
    if (storage_index > storage_devices_len) return 0;
    storage_device_t device = storage_devices[storage_index];
    if (!device.driver_info || !device.write) return 0;
    return device.write(device.driver_info, start, length, buffer);
}

void add_storage_device(volatile void *driver_info, storage_operation_t read, storage_operation_t write, storage_vectored_operation_t read_gather, storage_vectored_operation_t write_scatter) {
    mbr_t *buff = (mbr_t *)malloc(512);
    read(driver_info, 0, 1, (u32)buff);
    int i;
    for (i = 0; i < 4; i++) {
        if (buff->partition_table_entry[i].system_id != 0) {
            if (buff->partition_table_entry[i].active == 0x80) {
                kprint("found os owned partition at ");
                print_int(i);
            } else {
                kprint("found usable partition at ");
                print_int(i);

                storage_device_t *info = (storage_device_t *)malloc(sizeof(storage_device_t));
                info->read = read;
                info->write = write;
                info->read_gather = read_gather;
                info->write_scatter = write_scatter;
                info->driver_info = driver_info;

                partition_driver_info_t *partition_info = (partition_driver_info_t *)malloc(sizeof(partition_driver_info_t));
                partition_info->index;
                partition_info->entry = buff->partition_table_entry[i];
                partition_info->device_driver_info = info;

                storage_devices[storage_devices_len].driver_info = partition_info;
                storage_devices[storage_devices_len].read = partition_read;
                storage_devices[storage_devices_len].write = partition_write;
                storage_devices[storage_devices_len].read = partition_read;
                storage_devices[storage_devices_len].write = partition_write;
                storage_devices_len += 1;
            }
        }
    }
    free((u32)buff);
}

storage_device_t get_storage_device(u8 index) {
    return storage_devices[index];
}


