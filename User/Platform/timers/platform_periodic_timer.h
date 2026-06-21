#ifndef PLATFORM_PERIODIC_TIMER_H
#define PLATFORM_PERIODIC_TIMER_H

#include "../core/platform_status.h"

#include <stdint.h>

typedef uint8_t platform_periodic_timer_id_t;

typedef void (*platform_periodic_timer_callback_t)(
    platform_periodic_timer_id_t timer,
    void *context);

struct platform_periodic_timer_ops
{
    platform_status_t (*set_period)(
        platform_periodic_timer_id_t timer,
        uint32_t period_ticks);
    platform_status_t (*start)(
        platform_periodic_timer_id_t timer);
    platform_status_t (*stop)(
        platform_periodic_timer_id_t timer);
    platform_status_t (*attach)(
        platform_periodic_timer_id_t timer,
        platform_periodic_timer_callback_t callback,
        void *context);
    platform_status_t (*detach)(
        platform_periodic_timer_id_t timer);
    platform_status_t (*get_tick_hz)(
        platform_periodic_timer_id_t timer,
        uint32_t *tick_hz);
};

platform_status_t platform_periodic_timer_register(
    const struct platform_periodic_timer_ops *ops);
platform_status_t platform_periodic_timer_set_period(
    platform_periodic_timer_id_t timer,
    uint32_t period_ticks);
platform_status_t platform_periodic_timer_start(
    platform_periodic_timer_id_t timer);
platform_status_t platform_periodic_timer_stop(
    platform_periodic_timer_id_t timer);
platform_status_t platform_periodic_timer_attach(
    platform_periodic_timer_id_t timer,
    platform_periodic_timer_callback_t callback,
    void *context);
platform_status_t platform_periodic_timer_detach(
    platform_periodic_timer_id_t timer);
platform_status_t platform_periodic_timer_get_tick_hz(
    platform_periodic_timer_id_t timer,
    uint32_t *tick_hz);

#endif /* PLATFORM_PERIODIC_TIMER_H */
