#include "encoder_incremental.h"

#include "Platform/core/platform_irq.h"
#include "Utils/container_of.h"

#include <limits.h>
#include <stddef.h>

#define ENCODER_INCREMENTAL_MAX_INSTANCES (2U)

static struct encoder_incremental
    *s_encoders[ENCODER_INCREMENTAL_MAX_INSTANCES];
static uint8_t s_encoder_count;
static uint8_t s_enabled_count;
static bool s_sample_timer_is_configured;
static platform_periodic_timer_id_t s_sample_timer;
static uint32_t s_sample_period_ticks;

static int encoder_incremental_get_rev(
    struct encoder_base *me,
    q16_16_t *rev);

static void encoder_incremental_pulse_isr(
    platform_gpio_pin_t pin,
    void *context)
{
    struct encoder_incremental *self = context;
    platform_gpio_level_t direction;
    uint32_t timestamp;

    (void)pin;
    if (!self) {
        return;
    }

    if (platform_gpio_read(self->dir_pin, &direction) == 0) {
        self->direction =
            direction == PLATFORM_GPIO_HIGH;
    }

    if (self->direction) {
        self->base.counter++;
    } else {
        self->base.counter--;
    }

    if (platform_clock_read(
            self->timestamp_clock, &timestamp) != 0) {
        return;
    }

    if (!self->pulse_ready) {
        self->window_start = timestamp;
        self->window_pulses = 1;
        self->pulse_ready = true;
    } else if (self->window_pulses < INT32_MAX) {
        self->window_pulses++;
    }
    self->window_end = timestamp;
}

static q16_16_t encoder_incremental_calculate_rpm(
    const struct encoder_incremental *self,
    int32_t pulse_count,
    uint32_t elapsed_ticks)
{
    uint32_t ppr_effective =
        (uint32_t)self->config->PPR *
        (uint32_t)self->config->MULTIPLIER;
    uint64_t numerator =
        60ULL * self->timestamp_freq *
        (uint32_t)pulse_count;
    uint64_t denominator =
        (uint64_t)ppr_effective * elapsed_ticks;
    uint64_t scaled;

    if (denominator == 0U) {
        return 0;
    }
    if (numerator > (UINT64_MAX >> Q16_16_FRAC_BITS)) {
        return INT32_MAX;
    }

    scaled =
        (numerator << Q16_16_FRAC_BITS) / denominator;
    if (scaled > INT32_MAX) {
        return INT32_MAX;
    }
    return (q16_16_t)scaled;
}

static void encoder_incremental_update_distance(
    struct encoder_incremental *self,
    int64_t counter)
{
    int64_t delta = counter - self->last_counter;
    int64_t distance_delta;
    int64_t distance;

    self->last_counter = counter;
    distance_delta = delta * self->dist_per_pulse;
    distance = (int64_t)self->distance_m + distance_delta;
    if (distance > INT32_MAX) {
        self->distance_m = INT32_MAX;
    } else if (distance < INT32_MIN) {
        self->distance_m = INT32_MIN;
    } else {
        self->distance_m = (q16_16_t)distance;
    }
}

static void encoder_incremental_snapshot_window(
    struct encoder_incremental *self)
{
    uint32_t primask;
    uint32_t window_start;
    uint32_t window_end;
    int32_t pulse_count;
    bool ready;

    primask = platform_irq_save();
    ready = self->pulse_ready;
    pulse_count = self->window_pulses;
    window_start = self->window_start;
    window_end = self->window_end;
    self->sample_pulses = ready ? pulse_count : 0;
    self->sample_elapsed_ticks =
        window_end - window_start;
    self->sample_direction = self->direction;
    self->sample_counter = self->base.counter;
    self->sample_ready = true;
    self->pulse_ready = false;
    self->window_pulses = 0;
    platform_irq_restore(primask);
}

static void encoder_incremental_sample_isr(
    platform_periodic_timer_id_t timer,
    void *context)
{
    uint8_t index;

    (void)context;
    if (timer != s_sample_timer) {
        return;
    }

    for (index = 0U; index < s_encoder_count; index++) {
        if (s_encoders[index] &&
            s_encoders[index]->base.is_enabled) {
            encoder_incremental_snapshot_window(
                s_encoders[index]);
        }
    }
}

static int encoder_incremental_enable_op(
    struct encoder_base *me)
{
    struct encoder_incremental *self;
    int rc;

    self = container_of(
        me, struct encoder_incremental, base);
    rc = platform_gpio_irq_enable(self->pulse_pin);
    if (rc != 0) {
        return rc;
    }

    if (s_enabled_count == 0U) {
        rc = platform_periodic_timer_start(
            self->sample_timer);
        if (rc != 0) {
            (void)platform_gpio_irq_disable(
                self->pulse_pin);
            return rc;
        }
    }

    s_enabled_count++;
    return 0;
}

static int encoder_incremental_disable_op(
    struct encoder_base *me)
{
    struct encoder_incremental *self;
    int rc;

    self = container_of(
        me, struct encoder_incremental, base);
    rc = platform_gpio_irq_disable(self->pulse_pin);
    if (rc != 0) {
        return rc;
    }

    if (s_enabled_count == 1U) {
        rc = platform_periodic_timer_stop(
            self->sample_timer);
        if (rc != 0) {
            (void)platform_gpio_irq_enable(
                self->pulse_pin);
            return rc;
        }
    }

    if (s_enabled_count > 0U) {
        s_enabled_count--;
    }
    return 0;
}

static int encoder_incremental_get_counter(
    struct encoder_base *me,
    int64_t *counter)
{
    uint32_t primask;

    if (!counter) {
        return -1;
    }

    primask = platform_irq_save();
    *counter = me->counter;
    platform_irq_restore(primask);
    return 0;
}

static int encoder_incremental_set_counter(
    struct encoder_base *me,
    int64_t counter)
{
    struct encoder_incremental *self;
    uint32_t primask;

    self = container_of(
        me, struct encoder_incremental, base);
    primask = platform_irq_save();
    self->base.counter = counter;
    self->last_counter = counter;
    self->distance_m = 0;
    self->sample_counter = counter;
    self->sample_ready = false;
    platform_irq_restore(primask);
    return 0;
}

static int encoder_incremental_get_mode(
    struct encoder_base *me,
    encoder_mode_t *mode)
{
    q16_16_t speed;
    int rc;

    if (!mode) {
        return -1;
    }

    rc = encoder_incremental_get_rev(me, &speed);
    if (rc != 0) {
        return rc;
    }
    if (speed == 0) {
        *mode = ENCODER_STOP;
    } else if (speed > 0) {
        *mode = ENCODER_FORWARD;
    } else {
        *mode = ENCODER_REVERSE;
    }
    return 0;
}

static int encoder_incremental_get_rev(
    struct encoder_base *me,
    q16_16_t *rev)
{
    struct encoder_incremental *self;
    uint32_t elapsed_ticks;
    uint32_t primask;
    int32_t pulse_count;
    int64_t counter;
    bool direction;
    bool ready;

    if (!rev) {
        return -1;
    }

    self = container_of(
        me, struct encoder_incremental, base);
    primask = platform_irq_save();
    ready = self->sample_ready;
    pulse_count = self->sample_pulses;
    elapsed_ticks = self->sample_elapsed_ticks;
    direction = self->sample_direction;
    counter = self->sample_counter;
    platform_irq_restore(primask);

    if (ready) {
        if (pulse_count > 1 && elapsed_ticks > 0U) {
            self->speed_rpm =
                encoder_incremental_calculate_rpm(
                    self,
                    pulse_count - 1,
                    elapsed_ticks);
            if (!direction) {
                self->speed_rpm = -self->speed_rpm;
            }
        } else {
            self->speed_rpm = 0;
        }
        encoder_incremental_update_distance(
            self, counter);
    }

    *rev = self->speed_rpm;
    return 0;
}

static const struct encoder_ops
    s_encoder_incremental_ops = {
        .enable = encoder_incremental_enable_op,
        .disable = encoder_incremental_disable_op,
        .get_counter = encoder_incremental_get_counter,
        .set_counter = encoder_incremental_set_counter,
        .get_mode = encoder_incremental_get_mode,
        .get_rev = encoder_incremental_get_rev,
    };

static int encoder_incremental_configure_sample_timer(
    platform_periodic_timer_id_t timer,
    uint32_t period_ticks)
{
    int rc;

    if (s_sample_timer_is_configured) {
        if (timer != s_sample_timer ||
            period_ticks != s_sample_period_ticks) {
            return -3;
        }
        return 0;
    }

    rc = platform_periodic_timer_set_period(
        timer, period_ticks);
    if (rc != 0) {
        return rc;
    }

    rc = platform_periodic_timer_attach(
        timer, encoder_incremental_sample_isr, NULL);
    if (rc != 0) {
        return rc;
    }

    s_sample_timer = timer;
    s_sample_period_ticks = period_ticks;
    s_sample_timer_is_configured = true;
    return 0;
}

int encoder_incremental_init(
    struct encoder_incremental *me,
    const char *name,
    platform_gpio_pin_t pulse_pin,
    platform_gpio_pin_t dir_pin,
    platform_clock_id_t timestamp_clock,
    platform_periodic_timer_id_t sample_timer,
    const struct encoder_incremental_config *config)
{
    uint32_t ppr_effective;
    int rc;

    if (!me || !name || !config ||
        config->PPR == 0U ||
        config->MULTIPLIER == 0U ||
        config->SAMPLE_PERIOD_TICKS == 0U) {
        return -1;
    }
    if (s_encoder_count >=
        ENCODER_INCREMENTAL_MAX_INSTANCES) {
        return -2;
    }

    rc = encoder_base_init(
        &me->base, name, &s_encoder_incremental_ops);
    if (rc != 0) {
        return rc;
    }

    me->pulse_pin = pulse_pin;
    me->dir_pin = dir_pin;
    me->timestamp_clock = timestamp_clock;
    me->sample_timer = sample_timer;
    me->config = config;
    me->pulse_ready = false;
    me->window_pulses = 0;
    me->window_start = 0U;
    me->window_end = 0U;
    me->direction = true;
    me->sample_ready = false;
    me->sample_pulses = 0;
    me->sample_elapsed_ticks = 0U;
    me->sample_direction = true;
    me->sample_counter = 0;
    me->speed_rpm = 0;
    me->distance_m = 0;
    me->last_counter = 0;

    rc = platform_clock_get_frequency(
        timestamp_clock, &me->timestamp_freq);
    if (rc != 0 || me->timestamp_freq == 0U) {
        return -4;
    }

    ppr_effective =
        (uint32_t)config->PPR *
        (uint32_t)config->MULTIPLIER;
    me->dist_per_pulse =
        config->WHEEL_CIRC / (q16_16_t)ppr_effective;

    rc = encoder_incremental_configure_sample_timer(
        sample_timer, config->SAMPLE_PERIOD_TICKS);
    if (rc != 0) {
        return rc;
    }

    rc = platform_gpio_set_direction(
        pulse_pin, PLATFORM_GPIO_INPUT);
    if (rc != 0) {
        return rc;
    }
    rc = platform_gpio_set_direction(
        dir_pin, PLATFORM_GPIO_INPUT);
    if (rc != 0) {
        return rc;
    }
    rc = platform_gpio_irq_disable(pulse_pin);
    if (rc != 0) {
        return rc;
    }
    rc = platform_gpio_irq_attach(
        pulse_pin,
        PLATFORM_GPIO_IRQ_RISING,
        encoder_incremental_pulse_isr,
        me);
    if (rc != 0) {
        return rc;
    }

    s_encoders[s_encoder_count] = me;
    s_encoder_count++;
    return 0;
}
