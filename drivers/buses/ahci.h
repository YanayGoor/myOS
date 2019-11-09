//
// Created by yanayg on 10/14/19.
//

#ifndef AHCI_H

#include "../../kernel/utils.h"
#include "ata.h"

void init_ahci_driver_for_device(u8 bus, u8 device, u8 function);
void init_ahci_driver();

#define AHCI_HBA_PORT_IPM_ACTIVE 1
#define AHCI_HBA_PORT_DET_PRESENT 3

#define AHCI_HBA_PORT_IPM_NO_COMMUNICATION 0
#define AHCI_HBA_PORT_DET_NO_COMMUNICATION 1

typedef union {
    u32 addr;
    u64 addr64;
} AHCI_addr;

typedef enum
{
    FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
    FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
    FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
    FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
    FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

typedef volatile struct tagHBA_PORT {
    AHCI_addr cmd_list_base_addr;
    AHCI_addr FIS_base_addr;
    u32 interrupt_status;
    u32 interrupt_enable;
    u32 cmd_n_status;
    u32 rsv0;
    u32 task_file_data;
    u32 signature;
    u32 sata_status;
    u32 sata_control;
    u32 sata_error;
    u32 sata_active;
    u32 cmd_issue;
    u32 sata_notification;
    u32 FIS_based_switch_control;
    u32 rsv1[11];
    u32 vendor_specific[4];

} AHCI_HBA_port_t;


typedef volatile struct tagHBA_MEM {
    // 0x00 - 0x2B, Generic Host Control
    u32 host_capability;
    u32 global_host_control;
    u32 interrupt_status;
    u32 port_implemented;
    u32 version;
    u32 ccc_control;
    u32 ccc_ports;
    u32 enclosure_managment_location;
    u32 enclosure_managment_control;
    u32 host_capability_extended;
    u32 bios_os_control;

    // 0x2C - 0x9F, Reserved
    u8  rsv[0x74];

    // 0xA0 - 0xFF, Vendor specific registers
    u8  vendor[0x60];

    // 0x100 - 0x10FF, Port control registers
    AHCI_HBA_port_t	ports[1];	// 1 ~ 32
} AHCI_HBA_memory_t;

typedef volatile struct {
    // 0x00
    ATA_FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
    u8         pad0[4];

    // 0x20
//    ATA_FIS_PIO_SETUP	psfis;		// PIO Setup FIS
    u8         pad1[32];

    // 0x40
    ATA_FIS_REG_D2H	rfis;		// Register – Device to Host FIS
    u8         pad2[4];

    // 0x58
//    ATA_FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS
    u8         sdbfis[2];		// Set Device Bit FIS

    // 0x60
    u8         ufis[64];

    // 0xA0
    u8   	rsv[0x100-0xA0];
} AHCI_HBA_recived_FIS_t;

typedef volatile struct {
    u8 length : 5; // in dwords 2-16
    u8 atapi : 1;
    u8 direction : 1; // Write, 1: H2D, 0: D2H
    u8 prefetchable : 1;
    u8 reset : 1;
    u8 BIST : 1;
    u8 clear_busy : 1;
    u8 rsv0 : 1;
    u8 port_multiplier_port : 4;
    u16 prdt_length; //in entries
    volatile u32 prd_bytes_transferred;
    AHCI_addr cmd_table_base_addr;
    u32 rsv2[4];
} AHCI_cmd_header_t;


typedef struct {
    AHCI_addr data_base_addr;
    u32 rsv;
    u32 byte_count : 22;
    u16 rsv1 : 9;
    u8 interrupt_on_completion : 1;
} AHCI_prdt_entry_t;

typedef struct {
    union {
        ATA_FIS_DATA data;
        ATA_FIS_DMA_SETUP dma;
        ATA_FIS_REG_D2H d2h;
        ATA_FIS_REG_H2D h2d;
        u8 bytes[64];
    } cmd_fis;
    u8 ATAPI_cmd[16];
    u8 rsv[48];
    AHCI_prdt_entry_t prdt[1];
} AHCI_cmd_table_t;

typedef enum {
    AHCI_SATA_ATA_sign = 0x00000101,
    AHCI_SATA_ATAPI_sign = 0xEB140101,
    AHCI_SATA_emb_sign = 0xC33C0101,
    AHCI_SATA_port_multiplier_sign = 0x96690101,
} AHCI_signature_t;

typedef enum {
    AHCI_non_present,
    AHCI_unrecognized,
    AHCI_SATA,
    AHCI_SATAPI,
    AHCI_EMB,
    AHCI_port_multiplier
} AHCI_port_type_t;

typedef AHCI_cmd_header_t AHCI_cmd_list_t[32];

#define AHCI_H

#endif //AHCI_H
