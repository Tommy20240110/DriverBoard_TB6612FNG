#ifndef PLATFORM_PWM_H
#define PLATFORM_PWM_H

#include "../core/platform_status.h"

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t platform_pwm_channel_t;

struct platform_pwm_state
{
    uint32_t period_ticks;
    uint32_t pulse_ticks;
    bool is_enabled;
};

struct platform_pwm_ops
{
    platform_status_t (*apply)(
        platform_pwm_channel_t channel,
        const struct platform_pwm_state *state);
    platform_status_t (*get_state)(
        platform_pwm_channel_t channel,
        struct platform_pwm_state *state);
    platform_status_t (*get_tick_hz)(
        platform_pwm_channel_t channel,
        uint32_t *tick_hz);
};

platform_status_t platform_pwm_register(
    const struct platform_pwm_ops *ops);
platform_status_t platform_pwm_apply(
    platform_pwm_channel_t channel,
    const struct platform_pwm_state *state);
platform_status_t platform_pwm_get_state(
    platform_pwm_channel_t channel,
    struct platform_pwm_state *state);
platform_status_t platform_pwm_get_tick_hz(
    platform_pwm_channel_t channel,
    uint32_t *tick_hz);

#endif /* PLATFORM_PWM_H */
