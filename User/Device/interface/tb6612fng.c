/* tb6612fng.c */
#include "tb6612fng.h"

static bool tb6612fng_channel_is_valid(tb6612fng_channel_t channel)
{
    return channel >= TB6612FNG_CHANNEL_A &&
           channel < TB6612FNG_CHANNEL_COUNT;
}

static uint8_t tb6612fng_channel_mask(tb6612fng_channel_t channel)
{
    return (uint8_t)(1U << (uint8_t)channel);
}

int tb6612fng_set_stby(struct tb6612fng *me, bool en)
{
    if (!me) {
        return -1;
    }

    return platform_gpio_write(
        me->stby_pin,
        en ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW);
}

int tb6612fng_set_in(struct tb6612fng *me,
                     tb6612fng_channel_t channel,
                     bool in1, bool in2)
{
    int rc;

    if (!me || !tb6612fng_channel_is_valid(channel)) {
        return -1;
    }

    rc = platform_gpio_write(
        me->channel[channel].in1_pin,
        in1 ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW);
    if (rc != 0) {
        return rc;
    }

    return platform_gpio_write(
        me->channel[channel].in2_pin,
        in2 ? PLATFORM_GPIO_HIGH : PLATFORM_GPIO_LOW);
}

int tb6612fng_set_pwm(struct tb6612fng *me,
                      tb6612fng_channel_t channel,
                      uint32_t period_ticks,
                      uint32_t pulse_ticks)
{
    struct platform_pwm_state state;
    platform_pwm_channel_t pwm_channel;
    int rc;

    if (!me || !tb6612fng_channel_is_valid(channel) ||
        period_ticks == 0U || pulse_ticks > period_ticks) {
        return -1;
    }

    pwm_channel = me->channel[channel].pwm_channel;
    rc = platform_pwm_get_state(pwm_channel, &state);
    if (rc != 0) {
        return rc;
    }

    state.period_ticks = period_ticks;
    state.pulse_ticks = pulse_ticks;
    state.is_enabled =
        (me->enabled_channels &
         tb6612fng_channel_mask(channel)) != 0U;
    return platform_pwm_apply(pwm_channel, &state);
}

int tb6612fng_channel_enable(struct tb6612fng *me,
                             tb6612fng_channel_t channel)
{
    struct platform_pwm_state state;
    uint8_t mask;
    int rc;

    if (!me || !tb6612fng_channel_is_valid(channel)) {
        return -1;
    }

    mask = tb6612fng_channel_mask(channel);
    if ((me->enabled_channels & mask) != 0U) {
        return 0;
    }

    if (me->enabled_channels == 0U) {
        rc = tb6612fng_set_stby(me, true);
        if (rc != 0) {
            return rc;
        }
    }

    rc = platform_pwm_get_state(
        me->channel[channel].pwm_channel, &state);
    if (rc != 0) {
        goto restore_standby;
    }

    state.is_enabled = true;
    rc = platform_pwm_apply(
        me->channel[channel].pwm_channel, &state);
    if (rc != 0) {
        goto restore_standby;
    }

    me->enabled_channels |= mask;
    return 0;

restore_standby:
    if (me->enabled_channels == 0U) {
        (void)tb6612fng_set_stby(me, false);
    }
    return rc;
}

int tb6612fng_channel_disable(struct tb6612fng *me,
                              tb6612fng_channel_t channel)
{
    struct platform_pwm_state state;
    struct platform_pwm_state previous_state;
    uint8_t mask;
    uint8_t remaining_channels;
    int rc;

    if (!me || !tb6612fng_channel_is_valid(channel)) {
        return -1;
    }

    mask = tb6612fng_channel_mask(channel);
    if ((me->enabled_channels & mask) == 0U) {
        return 0;
    }

    rc = platform_pwm_get_state(
        me->channel[channel].pwm_channel, &state);
    if (rc != 0) {
        return rc;
    }
    previous_state = state;

    rc = tb6612fng_set_in(me, channel, false, false);
    if (rc != 0) {
        return rc;
    }

    state.pulse_ticks = 0U;
    state.is_enabled = false;
    rc = platform_pwm_apply(
        me->channel[channel].pwm_channel, &state);
    if (rc != 0) {
        return rc;
    }

    remaining_channels =
        me->enabled_channels & (uint8_t)~mask;
    if (remaining_channels == 0U) {
        rc = tb6612fng_set_stby(me, false);
        if (rc != 0) {
            (void)platform_pwm_apply(
                me->channel[channel].pwm_channel,
                &previous_state);
            return rc;
        }
    }

    me->enabled_channels = remaining_channels;
    return 0;
}

int tb6612fng_apply_output(struct tb6612fng *me,
                           tb6612fng_channel_t channel,
                           tb6612fng_output_mode_t mode,
                           uint32_t period_ticks,
                           uint32_t pulse_ticks)
{
    int rc;

    if (!me || !tb6612fng_channel_is_valid(channel)) {
        return -1;
    }

    if (mode < TB6612FNG_OUTPUT_COAST ||
        mode > TB6612FNG_OUTPUT_BRAKE) {
        return -2;
    }

    /*
     * Enter coast before changing PWM or direction to avoid a transient
     * shoot-through command when reversing a running motor.
     */
    rc = tb6612fng_set_in(me, channel, false, false);
    if (rc != 0) {
        return rc;
    }

    rc = tb6612fng_set_pwm(
        me, channel, period_ticks, pulse_ticks);
    if (rc != 0) {
        return rc;
    }

    switch (mode) {
    case TB6612FNG_OUTPUT_COAST:
        return 0;
    case TB6612FNG_OUTPUT_FORWARD:
        return tb6612fng_set_in(me, channel, true, false);
    case TB6612FNG_OUTPUT_REVERSE:
        return tb6612fng_set_in(me, channel, false, true);
    case TB6612FNG_OUTPUT_BRAKE:
        return tb6612fng_set_in(me, channel, true, true);
    default:
        return -2;
    }
}
