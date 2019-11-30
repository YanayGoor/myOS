#include "../ports.h"
#include "../screen.h"
#include "../../kernel/utils.h"
#include "../../cpu/isr.h"
#include "../../kernel/libc/linked_list.h"
#include "pit.h"

u32 timer_tick = 0;

u32 freq = 1000;

static void timer_interrupt_handler(registers_t r) {
    timer_tick++;
    u32 minutes = timer_tick / freq / 60;
    u32 seconds = (timer_tick / freq) - (minutes * 60);

    if (timer_tick % freq == 0) {
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

// TODO: this solution is for small amounts of callbacks at a time, 
// make all this based on a hashtable based on the next tick to execute on to reduce the logic complexity in the isr to O(1)

typedef void (timer_callback_t)();

typedef struct {
    node_t node;
    u32 id;
    u32 time_offset;
    u32 time_interval;
    u32 repeat;
    timer_callback_t *callback;  
} timer_callback_node_t;

timer_callback_node_t *callback_hashtable = 0;
u32 latest_callback_id = 1;

timer_callback_node_t *set_callback(timer_callback_t *callback, u32 msec, u8 repeat) {
}

void clear_callback(timer_callback_node_t *callback_node) {

}

void execute_callback(timer_callback_node_t *callback_node) {

}

void init_timer() {
    register_interrupt_handler(IRQ0, timer_interrupt_handler);

    u16 divider = (u16)(TIMER_HZ / freq);

    port_byte_out(PIT_CMD, PIT_CMD_CHANNEL(0) | PIT_CMD_ACCESS_ALL | PIT_CMD_OPERATION(3));

    port_byte_out(PIT_CHANNEL(0), low_8(divider));
    port_byte_out(PIT_CHANNEL(0), high_8(divider));
}
