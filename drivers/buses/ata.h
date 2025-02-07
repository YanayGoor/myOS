//
// Created by yanayg on 10/15/19.
//

#ifndef ATA_H

typedef enum
{
    a = 0x1,
    ATA_cmd_read_sectors_retry = 0x20,
    ATA_cmd_read_sectors = 0x21,
    ATA_cmd_read_long_retry = 0x22,
    ATA_cmd_read_long = 0x23,
    ATA_cmd_read_sectors_DMA = 0x25,
    ATA_cmd_write_sectors_DMA = 0x35,
    ATA_cmd_write_sectors_retry = 0x30,
    ATA_cmd_write_sectors = 0x31,
    ATA_cmd_identify_device = 0xec
} ATA_cmd_t;

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

typedef struct tagATA_FIS_REG_H2D
{
    // DWORD 0
    u8  fis_type;	// FIS_TYPE_REG_H2D

    u8  pmport:4;	// Port multiplier
    u8  rsv0:3;		// Reserved
    u8  c:1;		// 1: Command, 0: Control

    u8  command;	// Command register
    u8  featurel;	// Feature register, 7:0

    // DWORD 1
    u8  lba0;		// LBA low register, 7:0
    u8  lba1;		// LBA mid register, 15:8
    u8  lba2;		// LBA high register, 23:16
    u8  device;		// Device register

    // DWORD 2
    u8  lba3;		// LBA register, 31:24
    u8  lba4;		// LBA register, 39:32
    u8  lba5;		// LBA register, 47:40
    u8  featureh;	// Feature register, 15:8

    // DWORD 3
    u8  countl;		// Count register, 7:0
    u8  counth;		// Count register, 15:8
    u8  icc;		// Isochronous command completion
    u8  control;	// Control register

    // DWORD 4
    u8  rsv1[4];	// Reserved
} ATA_FIS_REG_H2D;

typedef struct tagATA_FIS_REG_D2H
{
    // DWORD 0
    u8  fis_type;    // FIS_TYPE_REG_D2H

    u8  pmport:4;    // Port multiplier
    u8  rsv0:2;      // Reserved
    u8  i:1;         // Interrupt bit
    u8  rsv1:1;      // Reserved

    u8  status;      // Status register
    u8  error;       // Error register

    // DWORD 1
    u8  lba0;        // LBA low register, 7:0
    u8  lba1;        // LBA mid register, 15:8
    u8  lba2;        // LBA high register, 23:16
    u8  device;      // Device register

    // DWORD 2
    u8  lba3;        // LBA register, 31:24
    u8  lba4;        // LBA register, 39:32
    u8  lba5;        // LBA register, 47:40
    u8  rsv2;        // Reserved

    // DWORD 3
    u8  countl;      // Count register, 7:0
    u8  counth;      // Count register, 15:8
    u8  rsv3[2];     // Reserved

    // DWORD 4
    u8  rsv4[4];     // Reserved
} ATA_FIS_REG_D2H;

typedef struct tagATA_FIS_DATA
{
    // DWORD 0
    u8  fis_type;	// FIS_TYPE_DATA

    u8  pmport:4;	// Port multiplier
    u8  rsv0:4;		// Reserved

    u8  rsv1[2];	// Reserved

    // DWORD 1 ~ N
    u32 data[1];	// Payload
} ATA_FIS_DATA;


typedef struct tagATA_FIS_DMA_SETUP
{
    // DWORD 0
    u8  fis_type;	// FIS_TYPE_DMA_SETUP

    u8  pmport:4;	// Port multiplier
    u8  rsv0:1;		// Reserved
    u8  d:1;		// Data transfer direction, 1 - device to host
    u8  i:1;		// Interrupt bit
    u8  a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed

    u8  rsved[2];       // Reserved

    //DWORD 1&2

    u64 DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

    //DWORD 3
    u32 rsvd;           //More reserved

    //DWORD 4
    u32 DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0

    //DWORD 5
    u32 TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

    //DWORD 6
    u32 resvd;          //Reserved

} ATA_FIS_DMA_SETUP;

#define ATA_H

#endif //MYOS_ATA_H
