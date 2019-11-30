#include "../ports.h"
#include "../screen.h"
#include "../../kernel/utils.h"
#include "../../cpu/isr.h"
#include "../../kernel/libc/linked_list.h"
#include "../../kernel/libc/hashtable.h"
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

    timer_callback_node_t *curr = get_from_hashtable(callback_hashtable, timer_tick);
    while(curr) {
        _execute_callback(curr);
        curr = get_from_hashtable(callback_hashtable, timer_tick);
    }
}

hashtable_t *callback_hashtable = 0;
u32 latest_callback_id = 1;

timer_callback_node_t *set_callback(timer_callback_t *callback, u32 msec, u8 repeat) {
    timer_callback_node_t *node = malloc(sizeof(timer_callback_node_t));
    memory_set((char *)&node, 0, sizeof(timer_callback_node_t));
    node->execute_time = timer_tick + msec;
    node->time_interval = msec;
    node->repeat = repeat;
    node->callback = callback;
    insert_to_hashtable(callback_hashtable, node->execute_time, node);
    return node;
}

void clear_callback(timer_callback_node_t *callback_node) {
    // since we can't remove from the hashtable by value, only by key. and there might be duplicates of the key
    callback_node->cleared = 1;
}

void _execute_callback(timer_callback_node_t *callback_node) {
    // we can remove by key here since the code calling this is will always get the first match for the key
    if (!callback_node->cleared) {
        callback_node->callback();
        if (callback_node->repeat) {
            remove_from_hashtable(callback_hashtable, callback_node->execute_time);
            callback_node->execute_time += callback_node->time_interval;
            insert_to_hashtable(callback_hashtable, callback_node->execute_time, callback_node);
        } else {
            remove_from_hashtable(callback_hashtable, callback_node->execute_time); 
        }
    } else {
        remove_from_hashtable(callback_hashtable, callback_node->execute_time);
    }
        
}

u8 callback_is_match(void *key, void *entry) {
    return (u32)key == ((timer_callback_node_t *)entry)->execute_time;
}

u8 callback_hash_key(void *key) {
    return hash(key, 4);
}

void init_timer() {
    callback_hashtable = create_hashtable(100, callback_is_match, callback_hash_key);

    register_interrupt_handler(IRQ0, timer_interrupt_handler);

    u16 divider = (u16)(TIMER_HZ / freq);

    port_byte_out(PIT_CMD, PIT_CMD_CHANNEL(0) | PIT_CMD_ACCESS_ALL | PIT_CMD_OPERATION(3));

    port_byte_out(PIT_CHANNEL(0), low_8(divider));
    port_byte_out(PIT_CHANNEL(0), high_8(divider));
}
