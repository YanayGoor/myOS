#include "pic.h"
#include "../../cpu/types.h"
#include "../ports.h"

void PIC_remap(u32 offset1, u32 offset2) {
    port_byte_out(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    port_byte_out(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);

    port_byte_out(PIC1_DATA, offset1);
    port_byte_out(PIC2_DATA, offset2);

    port_byte_out(PIC1_DATA, 1 << 2); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    port_byte_out(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)

    port_byte_out(PIC1_DATA, PIC_ICW4_8086);
    port_byte_out(PIC2_DATA, PIC_ICW4_8086);

    port_byte_out(PIC1_DATA, 0);  //no mask
    port_byte_out(PIC2_DATA, 0);
}
