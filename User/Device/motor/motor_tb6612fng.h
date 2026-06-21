/* motor_tb6612fng.h */
#ifndef MOTOR_TB6612FNG_H
#define MOTOR_TB6612FNG_H

#include "motor_bdc.h"
#include "Device/interface/tb6612fng.h"

struct motor_tb6612fng_config
{
    struct tb6612fng *driver;
    tb6612fng_channel_t channel;
    uint32_t initial_period_ticks;
};

struct motor_tb6612fng
{
    struct motor_bdc bdc;
    struct tb6612fng *driver;
    tb6612fng_channel_t channel;
};

int motor_tb6612fng_init(struct motor_tb6612fng *me, const char *name,
                         const struct motor_tb6612fng_config *config);

#endif /* MOTOR_TB6612FNG_H */
