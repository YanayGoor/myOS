#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../cpu/isr.h"
#include "../../kernel/utils.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD 0x64

#define PS2_STATUS_OUTPUT(u8) ((u8 & 1) > 0)
#define PS2_STATUS_INPUT(u8) ((u8 & 1 << 1) > 0)
#define PS2_STATUS_SYS_FLAG(u8) ((u8 & 1 << 2) > 0)
#define PS2_STATUS_CMD_OR_DATA(u8) ((u8 & 1 << 3) > 0)
//#define PS2_STATUS_OUTPUT(u8) ((u8 & 1 << 4) > 0)
//#define PS2_STATUS_OUTPUT(u8) ((u8 & 1 << 5) > 0)
#define PS2_STATUS_TIMEOUT_ERR(u8) ((u8 & 1 << 6) > 0)
#define PS2_STATUS_PARITY_ERR(u8) ((u8 & 1 << 7) > 0)

#define PS2_CMD_CONFIG_READ 0x20
#define PS2_CMD_CONFIG_WRITE 0x60

#define PS2_CMD_ENABLE_1 0xae
#define PS2_CMD_DISABLE_1 0xad

#define PS2_CMD_ENABLE_2 0xa8
#define PS2_CMD_DISABLE_2 0xa7

#define PS2_CONFIG_INTERRUPT_1(u8) ((u8 & 1) > 0)
#define PS2_CONFIG_INTERRUPT_2(u8) ((u8 & (1 << 1)) > 0)


#define PS2_IRQ IRQ1
#define PS2_IRQ2 IRQ12

void init_keyboard();

#endif