#include "platform_pwm.h"

#include <stddef.h>

static const struct platform_pwm_ops *s_pwm_ops;

platform_status_t platform_pwm_register(
    const struct platform_pwm_ops *ops)
{
    if (!ops || !ops->apply || !ops->get_state ||
        !ops->get_tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_pwm_ops && s_pwm_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_pwm_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_pwm_apply(
    platform_pwm_channel_t channel,
    const struct platform_pwm_state *state)
{
    if (!s_pwm_ops || !s_pwm_ops->apply) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!state) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (state->period_ticks == 0U ||
        state->pulse_ticks > state->period_ticks) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_pwm_ops->apply(channel, state);
}

platform_status_t platform_pwm_get_state(
    platform_pwm_channel_t channel,
    struct platform_pwm_state *state)
{
    if (!s_pwm_ops || !s_pwm_ops->get_state) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!state) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_pwm_ops->get_state(channel, state);
}

platform_status_t platform_pwm_get_tick_hz(
    platform_pwm_channel_t channel,
    uint32_t *tick_hz)
{
    if (!s_pwm_ops || !s_pwm_ops->get_tick_hz) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_pwm_ops->get_tick_hz(channel, tick_hz);
}
