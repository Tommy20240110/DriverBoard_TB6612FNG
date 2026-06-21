#include "platform_periodic_timer.h"

#include <stddef.h>

static const struct platform_periodic_timer_ops *s_timer_ops;

platform_status_t platform_periodic_timer_register(
    const struct platform_periodic_timer_ops *ops)
{
    if (!ops || !ops->set_period || !ops->start ||
        !ops->stop || !ops->attach || !ops->detach ||
        !ops->get_tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_timer_ops && s_timer_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_timer_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_periodic_timer_set_period(
    platform_periodic_timer_id_t timer,
    uint32_t period_ticks)
{
    if (!s_timer_ops || !s_timer_ops->set_period) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (period_ticks == 0U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_timer_ops->set_period(timer, period_ticks);
}

platform_status_t platform_periodic_timer_start(
    platform_periodic_timer_id_t timer)
{
    if (!s_timer_ops || !s_timer_ops->start) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_timer_ops->start(timer);
}

platform_status_t platform_periodic_timer_stop(
    platform_periodic_timer_id_t timer)
{
    if (!s_timer_ops || !s_timer_ops->stop) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_timer_ops->stop(timer);
}

platform_status_t platform_periodic_timer_attach(
    platform_periodic_timer_id_t timer,
    platform_periodic_timer_callback_t callback,
    void *context)
{
    if (!s_timer_ops || !s_timer_ops->attach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!callback) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_timer_ops->attach(timer, callback, context);
}

platform_status_t platform_periodic_timer_detach(
    platform_periodic_timer_id_t timer)
{
    if (!s_timer_ops || !s_timer_ops->detach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_timer_ops->detach(timer);
}

platform_status_t platform_periodic_timer_get_tick_hz(
    platform_periodic_timer_id_t timer,
    uint32_t *tick_hz)
{
    if (!s_timer_ops || !s_timer_ops->get_tick_hz) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_timer_ops->get_tick_hz(timer, tick_hz);
}
