//
// Created by yanayg on 10/14/19.
//

#include "pci.h"
#include "ahci.h"
#include "../screen.h"
#include "../../kernel/components/paging.h"
#include "../../kernel/components/storage.h"
#include "../../kernel/utils.h"

AHCI_port_type_t get_port_type(AHCI_HBA_port_t *port) {
    u32 status = port->sata_status;
    u8 ipm = (status >> 8) & 0x0f;
    u8 det = status & 0x0f;
    if (ipm != AHCI_HBA_PORT_IPM_ACTIVE || det != AHCI_HBA_PORT_DET_PRESENT) return AHCI_non_present;
    switch (port->signature) {
        case AHCI_SATA_ATA_sign:
            return AHCI_SATA;
        case AHCI_SATA_ATAPI_sign:
            return AHCI_SATAPI;
        case AHCI_SATA_emb_sign:
            return AHCI_EMB;
        case AHCI_SATA_port_multiplier_sign:
            return AHCI_port_multiplier;
        default:
            return AHCI_unrecognized;
    }
}

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

void stop_cmd(AHCI_HBA_port_t *port) {
    port->cmd_n_status &= ~HBA_PxCMD_ST;
    port->cmd_n_status &= ~HBA_PxCMD_FRE;
    while(1) {
        if (port->cmd_n_status & HBA_PxCMD_FR) {
            continue;
        }
        if (port->cmd_n_status & HBA_PxCMD_CR) {
            continue;
        }
        break;
    }
}

void start_cmd(AHCI_HBA_port_t *port) {
    while (port->cmd_n_status & HBA_PxCMD_CR) {};
    port->cmd_n_status |= HBA_PxCMD_FRE;
    port->cmd_n_status |= HBA_PxCMD_ST;
}

void init_port(AHCI_HBA_port_t *port) {
    stop_cmd(port);
    u32 port_addr = malloc_a(1024 + 256 + (256 * 32));

    port->cmd_list_base_addr.addr = port_addr;
    memory_set(port->cmd_list_base_addr.addr, 0, 1024);

    port->FIS_base_addr.addr = port_addr + 1024;
    memory_set(port->FIS_base_addr.addr, 0, 256);

    AHCI_cmd_header_t *cmd_header = (AHCI_cmd_header_t *)port->cmd_list_base_addr.addr;
    int i;
    for (i = 0; i<32; i++) {
        cmd_header[i].prdt_length = 8;
        cmd_header[i].cmd_table_base_addr.addr = port_addr + 1024 + 256 + (256 * i);
        memory_set(cmd_header[i].cmd_table_base_addr.addr, 0, 256);
    }

    kprint("initialized port at memory addr ");
    print_uint(port_addr);
    start_cmd(port);
}

u8 find_free_slot(AHCI_HBA_port_t *port) {
    u8 i;
    for (i = 0; i < 32; i++) {
        if (!(port->cmd_issue & (1 << i)) && !(port->sata_active & (1 << i))) {
            return i;
        }
    }
    return -1;
}

void AHCI_fill_cmd(AHCI_cmd_header_t *cmd_header, u32 cmd, u32 start, u32 length, u32 buffer) {
    cmd_header->direction = 0;
    cmd_header->length = sizeof(ATA_FIS_REG_H2D) / sizeof(u32);
    AHCI_cmd_table_t *cmd_table = (AHCI_cmd_table_t *)cmd_header->cmd_table_base_addr.addr;
    cmd_table->cmd_fis.h2d.fis_type = FIS_TYPE_REG_H2D;
    cmd_table->cmd_fis.h2d.command = cmd;
    cmd_table->cmd_fis.h2d.c = 1;
    cmd_table->cmd_fis.h2d.lba0 = (u8)start;
    cmd_table->cmd_fis.h2d.lba1 = (u8)(start>>8);
    cmd_table->cmd_fis.h2d.lba2 = (u8)(start>>16);
    cmd_table->cmd_fis.h2d.device = 1 << 6; //LBA mode
    cmd_table->cmd_fis.h2d.lba3 = (u8)(start>>24);
    cmd_table->cmd_fis.h2d.lba4 = 0;
    cmd_table->cmd_fis.h2d.lba5 = 0;
    u32 sector_length = length;
    cmd_table->cmd_fis.h2d.countl = sector_length & 0xff;
    cmd_table->cmd_fis.h2d.counth = (sector_length >> 8) & 0xff;
    int i = 0;
    AHCI_prdt_entry_t *prdt = cmd_table->prdt;
    while(length) {
        u32 size = 8*1024;
        if (length < (8*1024)) size = length * 512;
        prdt[i].byte_count = size;
        prdt[i].data_base_addr.addr = buffer;
        prdt[i].interrupt_on_completion = (size == length * 512);
        i++;
        buffer += size;
        length -= size / 512;
    }
    cmd_header->prdt_length = i;
}

u8 AHCI_issue_cmd(AHCI_HBA_port_t *port, u32 slots) {
    u32 spin = 0;
    while((port->task_file_data & ATA_DEV_BUSY || port->task_file_data & ATA_DEV_DRQ) && spin < 10000000) {
        spin++;
    }
    if (spin == 10000000) {
        kprint("ahci port is hung\n");
        return -1;
    }
    port->sata_active = slots;
    port->cmd_issue = slots;
    while(1) {
        if ((port->cmd_issue & slots) == 0) {
            break;
        }
        if (port->interrupt_status & (1 << 30)) {
            kprint("ahci port task file error\n");
            return -1;
        }
    }
    return 0;
}

u8 AHCI_issue_dma_scatter_gather_cmd(u32 cmd, AHCI_HBA_port_t *port, storage_vector_node_t *nodes, u32 buffer) {
    // TODO: use virtual to physical converter function
    port->interrupt_status = -1;
    storage_vector_node_t *curr;
    curr->node.next = nodes;
    u32 buffer_offset = 0;
    u32 slots = 0;
    while (curr->node.next) {
        curr = curr->node.next;
        buffer_offset += curr->length * 512;
        u8 free_slot = find_free_slot(port);
        if (free_slot == (u8)-1) return -1;
        AHCI_cmd_header_t *cmd_header = (AHCI_cmd_header_t *)port->cmd_list_base_addr.addr + free_slot;
        AHCI_fill_cmd(cmd_header, cmd, curr->offset, curr->length, buffer + buffer_offset);
        slots |= 1 << free_slot;
    }
    return AHCI_issue_cmd(port, slots);
}

u8 AHCI_read_gather(volatile void *port, storage_vector_node_t *nodes, u32 buffer) {
    return AHCI_issue_dma_scatter_gather_cmd(ATA_cmd_read_sectors_DMA, (AHCI_HBA_port_t *)port, nodes, buffer);
}

u8 AHCI_write_scatter(volatile void *port, storage_vector_node_t *nodes, u32 buffer) {
    return AHCI_issue_dma_scatter_gather_cmd(ATA_cmd_write_sectors_DMA, (AHCI_HBA_port_t *)port, nodes, buffer);
}

u8 AHCI_issue_dma_cmd(u32 cmd, AHCI_HBA_port_t *port, u32 start, u32 length, u32 buffer) {
    // TODO: many-to-many (cmd headers), one-to-many (prdt), many-to-one (split one to cmd headers)
    // TODO: use virtual to physical converter function
    port->interrupt_status = -1;
    u8 free_slot = find_free_slot(port);
    if (free_slot == (u8)-1) return -1;
    AHCI_cmd_header_t *cmd_header = (AHCI_cmd_header_t *)port->cmd_list_base_addr.addr + free_slot;
    AHCI_fill_cmd(cmd_header, cmd, start, length, buffer);
    return AHCI_issue_cmd(port, 1 << free_slot);
}


u8 AHCI_read(volatile void *port, u32 start, u32 length, u32 buffer) {
    return AHCI_issue_dma_cmd(ATA_cmd_read_sectors_DMA, (AHCI_HBA_port_t *)port, start, length, buffer);
}

u8 AHCI_write(volatile void *port, u32 start, u32 length, u32 buffer) {
    return AHCI_issue_dma_cmd(ATA_cmd_write_sectors_DMA, (AHCI_HBA_port_t *)port, start, length, buffer);
}

void AHCI_release_port(AHCI_HBA_port_t *port) {
    free(port->cmd_list_base_addr.addr);
}

void init_ports(AHCI_HBA_memory_t *memory) {
     memory->global_host_control = 1 << 31;
     u32 ports_implemented = memory->port_implemented;
     u8 i;
     for (i = 0; i < 32; i++) {
         if (ports_implemented & 1) {
             AHCI_port_type_t port_type = get_port_type(&memory->ports[i]);
             switch (port_type) {
                 case AHCI_SATA:
                     init_port(&memory->ports[i]);
                     kprint("detected SATA device at port ");
                     print_uint(i);
                     add_storage_device(&memory->ports[i], &AHCI_read, &AHCI_write, &AHCI_read_gather, &AHCI_write_scatter);
                     break;
                 case AHCI_SATAPI:
                     kprint("detected SATAPI device at port ");
                     print_uint(i);
                     break;
                 case AHCI_EMB:
                     kprint("detected EMB device at port ");
                     print_uint(i);
                     break;
                 case AHCI_port_multiplier:
                     kprint("detected port multiplier device at port ");
                     print_uint(i);
                     break;
                 case AHCI_unrecognized:
                     kprint("detected unrecognized device at port ");
                     print_uint(i);
                     break;
             }
         }
         ports_implemented >>= 1;
     }

}

//#define abar_virtual ((u32)(50000 * 0x1000))

void init_ahci_driver_for_device(u8 bus, u8 device, u8 function) {
    kprint("I'm an ahci driver for device on bus num ");
    print_int(bus);
    u32 abar = pci_config_read_header(bus, device, function, pci_BAR5);
//    kprint("using abar at ");
//    print_uint(abar);
//    u32 addr = malloc_a(0x2000);
//    pci_config_write_header(bus, device, function, pci_BAR5, addr);

//    unmap(0, addr);
//    unmap(0, addr + 0x1000);
//    map_page(0, abar, addr, 0);
//    map_page(0, abar + 0x1000, addr + 0x1000, 0);
    identity_map_page(0, abar, 0);
    identity_map_page(0, abar + 0x1000, 0 + 0x1000);
    init_ports((AHCI_HBA_memory_t *)abar);
}

void init_ahci_driver() {
    pci_add_driver(1, 6, 1, &init_ahci_driver_for_device);
}
