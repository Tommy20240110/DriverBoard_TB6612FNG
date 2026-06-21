#ifndef ENCODER_INCREMENTAL_H
#define ENCODER_INCREMENTAL_H

#include "encoder_base.h"
#include "Platform/gpio/platform_gpio.h"
#include "Platform/timers/platform_clock.h"
#include "Platform/timers/platform_periodic_timer.h"

struct encoder_incremental_config
{
    const uint16_t PPR;
    const uint8_t MULTIPLIER;
    const q16_16_t WHEEL_CIRC;
    const uint32_t SAMPLE_PERIOD_TICKS;
};

struct encoder_incremental
{
    struct encoder_base base;

    platform_gpio_pin_t pulse_pin;
    platform_gpio_pin_t dir_pin;
    platform_clock_id_t timestamp_clock;
    platform_periodic_timer_id_t sample_timer;
    const struct encoder_incremental_config *config;

    volatile bool pulse_ready;
    volatile int32_t window_pulses;
    volatile uint32_t window_start;
    volatile uint32_t window_end;
    volatile bool direction;

    volatile bool sample_ready;
    volatile int32_t sample_pulses;
    volatile uint32_t sample_elapsed_ticks;
    volatile bool sample_direction;
    volatile int64_t sample_counter;

    uint32_t timestamp_freq;
    q16_16_t dist_per_pulse;
    q16_16_t speed_rpm;
    q16_16_t distance_m;
    int64_t last_counter;
};

int encoder_incremental_init(
    struct encoder_incremental *me,
    const char *name,
    platform_gpio_pin_t pulse_pin,
    platform_gpio_pin_t dir_pin,
    platform_clock_id_t timestamp_clock,
    platform_periodic_timer_id_t sample_timer,
    const struct encoder_incremental_config *config);

#endif /* ENCODER_INCREMENTAL_H */
