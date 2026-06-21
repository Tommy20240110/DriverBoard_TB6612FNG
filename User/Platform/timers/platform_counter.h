#ifndef PLATFORM_COUNTER_H
#define PLATFORM_COUNTER_H

#include "../core/platform_status.h"

#include <stdint.h>

typedef uint8_t platform_counter_id_t;

typedef enum platform_counter_direction
{
    PLATFORM_COUNTER_DIRECTION_UNKNOWN = 0,
    PLATFORM_COUNTER_DIRECTION_FORWARD,
    PLATFORM_COUNTER_DIRECTION_REVERSE,
} platform_counter_direction_t;

struct platform_counter_ops
{
    platform_status_t (*start)(
        platform_counter_id_t counter);
    platform_status_t (*stop)(
        platform_counter_id_t counter);
    platform_status_t (*read)(
        platform_counter_id_t counter,
        int64_t *value);
    platform_status_t (*write)(
        platform_counter_id_t counter,
        int64_t value);
    platform_status_t (*get_direction)(
        platform_counter_id_t counter,
        platform_counter_direction_t *direction);
};

platform_status_t platform_counter_register(
    const struct platform_counter_ops *ops);
platform_status_t platform_counter_start(
    platform_counter_id_t counter);
platform_status_t platform_counter_stop(
    platform_counter_id_t counter);
platform_status_t platform_counter_read(
    platform_counter_id_t counter,
    int64_t *value);
platform_status_t platform_counter_write(
    platform_counter_id_t counter,
    int64_t value);
platform_status_t platform_counter_get_direction(
    platform_counter_id_t counter,
    platform_counter_direction_t *direction);

#endif /* PLATFORM_COUNTER_H */
