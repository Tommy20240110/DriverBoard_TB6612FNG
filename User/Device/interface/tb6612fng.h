/* tb6612fng.h */
#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <stdbool.h>
#include <stdint.h>

#include "Platform/gpio/platform_gpio.h"
#include "Platform/timers/platform_pwm.h"

typedef enum tb6612fng_channel
{
    TB6612FNG_CHANNEL_A = 0,
    TB6612FNG_CHANNEL_B,
    TB6612FNG_CHANNEL_COUNT,
} tb6612fng_channel_t;

typedef enum tb6612fng_output_mode
{
    TB6612FNG_OUTPUT_COAST = 0,
    TB6612FNG_OUTPUT_FORWARD,
    TB6612FNG_OUTPUT_REVERSE,
    TB6612FNG_OUTPUT_BRAKE,
} tb6612fng_output_mode_t;

struct tb6612fng_channel_config
{
    platform_gpio_pin_t in1_pin;
    platform_gpio_pin_t in2_pin;
    platform_pwm_channel_t pwm_channel;
};

struct tb6612fng
{
    struct tb6612fng_channel_config
        channel[TB6612FNG_CHANNEL_COUNT];
    platform_gpio_pin_t stby_pin;
    uint8_t enabled_channels;
};

int tb6612fng_set_stby(struct tb6612fng *me, bool en);
int tb6612fng_set_in(struct tb6612fng *me,
                     tb6612fng_channel_t channel,
                     bool in1, bool in2);
int tb6612fng_set_pwm(struct tb6612fng *me,
                      tb6612fng_channel_t channel,
                      uint32_t period_ticks,
                      uint32_t pulse_ticks);
int tb6612fng_channel_enable(struct tb6612fng *me,
                             tb6612fng_channel_t channel);
int tb6612fng_channel_disable(struct tb6612fng *me,
                              tb6612fng_channel_t channel);
int tb6612fng_apply_output(struct tb6612fng *me,
                           tb6612fng_channel_t channel,
                           tb6612fng_output_mode_t mode,
                           uint32_t period_ticks,
                           uint32_t pulse_ticks);

#endif /* TB6612FNG_H */
