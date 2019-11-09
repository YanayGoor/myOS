#include "../ports.h"
#include "../screen.h"
#include "../../kernel/utils.h"
#include "../../cpu/isr.h"
#include "pit.h"

u32 timer_tick = 0;

static void timer_interrupt_handler(registers_t r) {
    timer_tick++;
    u32 minutes = timer_tick / 100 / 60;
    u32 seconds = (timer_tick / 100) - (minutes * 60);

    if (timer_tick % 100 == 0) {
        int offset = get_offset();
        char tick_ascii[256];
        int_to_ascii(minutes, tick_ascii);
        kprint_at("     ", MAX_COLS-6, MAX_ROWS-1);
        kprint_at(tick_ascii, MAX_COLS-6, MAX_ROWS-1);
        kprint(":");
        int_to_ascii(seconds, tick_ascii);
        kprint(tick_ascii);
        set_offset(offset);
    }
}

void init_timer(u32 freq) {
    register_interrupt_handler(IRQ0, timer_interrupt_handler);

    u16 divider = (u16)(TIMER_HZ / freq);

    port_byte_out(PIT_CMD, PIT_CMD_CHANNEL(0) | PIT_CMD_ACCESS_ALL | PIT_CMD_OPERATION(3));

    port_byte_out(PIT_CHANNEL(0), low_8(divider));
    port_byte_out(PIT_CHANNEL(0), high_8(divider));
}
