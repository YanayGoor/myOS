#ifndef TIMER_H
#define TIMER_H

#include "../../kernel/utils.h"

#define PIT_CHANNEL(channel) (channel + 0x40)
#define PIT_CMD 0x43


#define PIT_CMD_CHANNEL(channel) (channel << 5)
#define PIT_CMD_READ_BACK_CMD 96

#define PIT_CMD_ACCESS_0 0
#define PIT_CMD_ACCESS_LOBYTE 16
#define PIT_CMD_ACCESS_HIBYTE 32
#define PIT_CMD_ACCESS_ALL 48

#define PIT_CMD_OPERATION(mode) (mode << 1)

#define TIMER_HZ 1193180

void init_timer();

typedef void (timer_callback_t)();

typedef struct {
    u32 execute_time;
    u32 time_interval;
    u32 repeat;
    u32 cleared;
    timer_callback_t *callback;  
} timer_callback_node_t;

timer_callback_node_t *set_callback(timer_callback_t *callback, u32 msec, u8 repeat) {
void clear_callback(timer_callback_node_t *callback_node) {


#endif