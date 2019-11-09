#ifndef PCI_H
#define PCI_H

#include "../../kernel/utils.h"

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

typedef union {
    struct {
        u8 offset : 8;
        u8 function : 3;
        u8 device : 5;
        u8 bus : 8;
        u8 reserved : 7;
        u8 enable : 1;
    } fields;
    u32 data;
} pci_config_reg_t;

typedef struct {
    u8 offset;
    u8 size;
} pci_conf_header_descriptor_t;

typedef void (*pci_driver_init_t)(u8 bus, u8 device, u8 function);

typedef struct {
    u8 class;
    u8 subclass;
    u8 interface;
    pci_driver_init_t driver_init;
} pci_driver_t;

pci_conf_header_descriptor_t pci_deviceID;
pci_conf_header_descriptor_t pci_vendorID;
pci_conf_header_descriptor_t pci_class_code;
pci_conf_header_descriptor_t pci_subclass;
pci_conf_header_descriptor_t pci_header_type;
pci_conf_header_descriptor_t pci_progIF;
pci_conf_header_descriptor_t pci_BAR0;
pci_conf_header_descriptor_t pci_BAR1;
pci_conf_header_descriptor_t pci_BAR2;
pci_conf_header_descriptor_t pci_BAR3;
pci_conf_header_descriptor_t pci_BAR4;
pci_conf_header_descriptor_t pci_BAR5;
pci_conf_header_descriptor_t pci_interrupt_pin;
pci_conf_header_descriptor_t pci_interrupt_line;

void pci_detect_devices();
void pci_add_driver(u8 class, u8 subclass, u8 interface, pci_driver_init_t init);

#endif