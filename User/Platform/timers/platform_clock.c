#include "platform_clock.h"

#include <stddef.h>

static const struct platform_clock_ops *s_clock_ops;

platform_status_t platform_clock_register(
    const struct platform_clock_ops *ops)
{
    if (!ops || !ops->read || !ops->get_frequency) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_clock_ops && s_clock_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_clock_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_clock_read(
    platform_clock_id_t clock,
    uint32_t *ticks)
{
    if (!s_clock_ops || !s_clock_ops->read) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!ticks) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_clock_ops->read(clock, ticks);
}

platform_status_t platform_clock_get_frequency(
    platform_clock_id_t clock,
    uint32_t *frequency_hz)
{
    if (!s_clock_ops || !s_clock_ops->get_frequency) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!frequency_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_clock_ops->get_frequency(clock, frequency_hz);
}
