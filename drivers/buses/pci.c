#include "pci.h"
#include "../ports.h"
#include "../../kernel/utils.h"

pci_conf_header_descriptor_t pci_deviceID = {2, 2};
pci_conf_header_descriptor_t pci_vendorID = {0, 2};
pci_conf_header_descriptor_t pci_class_code = {11, 1};
pci_conf_header_descriptor_t pci_subclass = {10, 1};
pci_conf_header_descriptor_t pci_header_type = {0x0e, 1};
pci_conf_header_descriptor_t pci_progIF = {0x09, 1};
pci_conf_header_descriptor_t pci_BAR0 = {0x10, 4};
pci_conf_header_descriptor_t pci_BAR1 = {0x14, 4};
pci_conf_header_descriptor_t pci_BAR2 = {0x18, 4};
pci_conf_header_descriptor_t pci_BAR3 = {0x1c, 4};
pci_conf_header_descriptor_t pci_BAR4 = {0x20, 4};
pci_conf_header_descriptor_t pci_BAR5 = {0x24, 4};
pci_conf_header_descriptor_t pci_interrupt_pin = {0x3d, 1};
pci_conf_header_descriptor_t pci_interrupt_line = {0x3c, 1};

char *pci_class_names[] = { "(Unclassified)", "(Mass Storage Controller)"};
#define PCI_NAMED_CLASSES_COUNT 2

//char *pci_Unclassified_subclasses[] = { "(Non-VGA-Compatible devices)", "(VGA-Compatible Device)" };
//char *pci_Mass_Storage_Controller_subclasses[] = { "(SCSI Bus Controller)", "(IDE Controller)", "(Floppy Disk Controller)", "(IPI Bus Controller)", "(RAID Controller)", "(ATA Controller)", "(Serial ATA)", "(Serial Attached SCSI)", "(Non-Volatile Memory Controller)" };


pci_driver_t pci_drivers[128] = { 0 };
u8 pci_driver_count = 0;

u32 pci_config_read_dword(u8 bus, u8 slot, u8 function, u8 offset) {
    // you can only read dwords from pci config space, so we mask the lower 2 bits, then shift the
    // value in either 0 or 16 depending on whether the offset is odd or even.
    pci_config_reg_t reg = { .fields={.offset=offset & 0xfc, .function=function, .device=slot, .bus=bus, .reserved=0, .enable=1 }};
    port_dword_out(PCI_CONFIG_ADDRESS, reg.data);
    return port_dword_in(PCI_CONFIG_DATA);
}

void pci_config_write_dword(u8 bus, u8 slot, u8 function, u8 offset, u32 value) {
    // you can only read dwords from pci config space, so we mask the lower 2 bits, then shift the
    // value in either 0 or 16 depending on whether the offset is odd or even.
    pci_config_reg_t reg = { .fields={.offset=offset & 0xfc, .function=function, .device=slot, .bus=bus, .reserved=0, .enable=1 }};
    port_dword_out(PCI_CONFIG_ADDRESS, reg.data);
    port_dword_out(PCI_CONFIG_DATA, value);
}

u16 pci_config_read_word(u8 bus, u8 slot, u8 function, u8 offset) {
    return pci_config_read_dword(bus, slot, function, offset) >> ((offset & 2) * 8) & 0xffff;
}

void pci_config_write_word(u8 bus, u8 slot, u8 function, u8 offset, u32 value) {
    u32 dword = pci_config_read_dword(bus, slot, function, offset);
    if (offset & 2) {
        pci_config_write_dword(bus, slot, function, offset, (dword & 0xffff0000) | (value & 0xffff));
    } else {
        pci_config_write_dword(bus, slot, function, offset, (dword & 0xffff) | (value & 0xffff0000));
    }
}

u32 pci_config_read_header(u8 bus, u8 slot, u8 function, pci_conf_header_descriptor_t header) {
    if (header.size == 1) {
        return pci_config_read_word(bus, slot, function, header.offset & 0xfe) >> ((header.offset & 1) * 8) & 0xff;
    } else if (header.size == 2) {
        return pci_config_read_word(bus, slot, function, header.offset);
    }
    else if (header.size == 4) {
        return pci_config_read_dword(bus, slot, function, header.offset);
    }
    return 0;
}
void pci_config_write_header(u8 bus, u8 slot, u8 function, pci_conf_header_descriptor_t header, u32 value) {
    if (header.size == 1) {
        u32 word = pci_config_read_word(bus, slot, function, header.offset & 0xfe);
        if (header.offset & 1) {
            pci_config_write_word(bus, slot, function, header.offset & 0xfe, (word & 0xff00) | (value & 0xff));
        } else {
            pci_config_write_dword(bus, slot, function, header.offset & 0xfe, (word & 0xff) | (value & 0xff00));
        }
    } else if (header.size == 2) {
        pci_config_write_word(bus, slot, function, header.offset, value);
    }
    else if (header.size == 4) {
        pci_config_write_dword(bus, slot, function, header.offset, value);
    }
}

u8 pci_check_function(u8 bus, u8 device, u8 function) {
    return pci_config_read_word(bus, device, function, 0) != 0xffff;
}

u8 pci_check_device(u8 bus, u8 device) {
    // returns the number of supported functions
    if (pci_config_read_word(bus, device, 0, 0) == 0xffff) return 0;
    u8 function;
    if ((pci_config_read_header(bus, device, 0, pci_header_type) & 80) != 0) {
        for (function=1; function < 8; function++) {
            if (pci_check_function(bus, device, function) == 0xffff) {
                return function - 1;
            }
        }
        return 8;
    }
    return 1;
}

void print_details(u8 bus, u8 device, u8 function) {
    u32 vendor = pci_config_read_header(bus, device, function, pci_vendorID);
    u32 device_id = pci_config_read_header(bus, device, function, pci_deviceID);
    u32 class = pci_config_read_header(bus, device, function, pci_class_code);
    u32 subclass = pci_config_read_header(bus, device, function, pci_subclass);
    char *name;
    if (class < PCI_NAMED_CLASSES_COUNT) name = pci_class_names[class];
//    char **names;
    kprint("vendor id: ");
    print_int(vendor);
    kprint("device id: ");
    print_int(device_id);
    kprint("class: ");
    if (name) kprint(name);
    kprint(" ");
    print_int(class);
    if (subclass != 0x80) {
        kprint("subclass: ");
//        if (name) kprint(names[subclass]);
        kprint(" ");
        print_int(subclass);
    }
    kprint("\n");
}

void pci_detect_devices() {
    u8 bus;
    u8 device;
    kprint("\n");
    for(bus = 0; bus < 255; bus++) {
        for(device = 0; device < 32; device++) {
            u32 wo = pci_config_read_word(bus, device, 0, 0);
            u8 functions = pci_check_device(bus, device);
            u8 function;
            for (function = 0; function < functions; function++) {
                print_details(bus, device, function);
                u32 class = pci_config_read_header(bus, device, function, pci_class_code);
                u32 subclass = pci_config_read_header(bus, device, function, pci_subclass);
                u32 interface = pci_config_read_header(bus, device, function, pci_progIF);
                u8 i;
                for (i = 0; i < pci_driver_count; i++) {
                    if (pci_drivers[i].class == class &&
                    pci_drivers[i].subclass == subclass &&
                    pci_drivers[i].interface == interface) {
                        kprint("found matching driver \n");
                        pci_drivers[i].driver_init(bus, device, function);
                    }
                }
            }
        }
    }
}

void pci_add_driver(u8 class, u8 subclass, u8 interface, pci_driver_init_t init) {
    pci_driver_t driver = {.class=class, .subclass=subclass, .interface=interface, .driver_init=init};
    pci_drivers[pci_driver_count++] = driver;
}



