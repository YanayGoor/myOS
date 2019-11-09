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

void init_timer(u32 freq);

#endif