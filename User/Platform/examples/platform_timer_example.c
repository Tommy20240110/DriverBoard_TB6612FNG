#include "platform_examples.h"

platform_status_t platform_example_timer_configure(
    platform_clock_id_t clock,
    platform_periodic_timer_id_t timer,
    platform_pwm_channel_t pwm,
    uint32_t period_ticks,
    uint32_t pulse_ticks,
    platform_periodic_timer_callback_t callback,
    void *context)
{
    struct platform_pwm_state state = {
        .period_ticks = period_ticks,
        .pulse_ticks = pulse_ticks,
        .is_enabled = true,
    };
    uint32_t ticks;
    platform_status_t rc;

    rc = platform_clock_read(clock, &ticks);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    (void)ticks;
    rc = platform_periodic_timer_set_period(
        timer, period_ticks);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_periodic_timer_attach(
        timer, callback, context);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_pwm_apply(pwm, &state);
    return rc == PLATFORM_STATUS_OK
               ? platform_periodic_timer_start(timer)
               : rc;
}
