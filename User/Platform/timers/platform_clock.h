#ifndef PLATFORM_CLOCK_H
#define PLATFORM_CLOCK_H

#include "../core/platform_status.h"

#include <stdint.h>

typedef uint8_t platform_clock_id_t;

struct platform_clock_ops
{
    platform_status_t (*read)(
        platform_clock_id_t clock,
        uint32_t *ticks);
    platform_status_t (*get_frequency)(
        platform_clock_id_t clock,
        uint32_t *frequency_hz);
};

platform_status_t platform_clock_register(
    const struct platform_clock_ops *ops);
platform_status_t platform_clock_read(
    platform_clock_id_t clock,
    uint32_t *ticks);
platform_status_t platform_clock_get_frequency(
    platform_clock_id_t clock,
    uint32_t *frequency_hz);

#endif /* PLATFORM_CLOCK_H */
