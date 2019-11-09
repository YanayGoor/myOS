#ifndef PIC_H
#define PIC_H

#include "../../cpu/types.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20		/* IO base address for master PIC */

#define PIC_ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define PIC_ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define PIC_ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define PIC_ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define PIC_ICW1_INIT	0x10		/* Initialization - required! */

#define PIC_ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define PIC_ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define PIC_ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define PIC_ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define PIC_ICW4_SFNM	0x10		/* Special fully nested (not) */

void PIC_remap(u32 offset1, u32 offset2);

#endif