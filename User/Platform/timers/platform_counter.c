#include "platform_counter.h"

#include <stddef.h>

static const struct platform_counter_ops *s_counter_ops;

platform_status_t platform_counter_register(
    const struct platform_counter_ops *ops)
{
    if (!ops || !ops->start || !ops->stop ||
        !ops->read || !ops->write) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_counter_ops && s_counter_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_counter_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_counter_start(
    platform_counter_id_t counter)
{
    if (!s_counter_ops || !s_counter_ops->start) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_counter_ops->start(counter);
}

platform_status_t platform_counter_stop(
    platform_counter_id_t counter)
{
    if (!s_counter_ops || !s_counter_ops->stop) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_counter_ops->stop(counter);
}

platform_status_t platform_counter_read(
    platform_counter_id_t counter,
    int64_t *value)
{
    if (!s_counter_ops || !s_counter_ops->read) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!value) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_counter_ops->read(counter, value);
}

platform_status_t platform_counter_write(
    platform_counter_id_t counter,
    int64_t value)
{
    if (!s_counter_ops || !s_counter_ops->write) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_counter_ops->write(counter, value);
}

platform_status_t platform_counter_get_direction(
    platform_counter_id_t counter,
    platform_counter_direction_t *direction)
{
    if (!s_counter_ops) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!direction) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (!s_counter_ops->get_direction) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_counter_ops->get_direction(
        counter, direction);
}
