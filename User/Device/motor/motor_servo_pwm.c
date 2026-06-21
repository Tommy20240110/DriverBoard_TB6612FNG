#include "motor_servo_pwm.h"

#include "Utils/container_of.h"

static q16_16_t motor_servo_pwm_angle_to_pulse_us(
    const struct motor_servo_pwm *me,
    q16_16_t angle)
{
    q16_16_t pulse_range = Q16_16_SUB(
        me->config->SERVO_PULSE_MAX_US,
        me->config->SERVO_PULSE_MIN_US);
    q16_16_t angle_ratio =
        Q16_16_MUL(angle, me->config->MAX_ANGLE_INV);

    return Q16_16_ADD(
        me->config->SERVO_PULSE_MIN_US,
        Q16_16_MUL(pulse_range, angle_ratio));
}

static int motor_servo_pwm_us_to_ticks(
    q16_16_t time_us,
    uint32_t tick_hz,
    uint32_t *ticks)
{
    const uint64_t denominator =
        (uint64_t)1000000U << Q16_16_FRAC_BITS;
    uint64_t numerator;
    uint64_t result;

    if (time_us <= 0 || tick_hz == 0U || !ticks) {
        return -1;
    }

    numerator = (uint64_t)tick_hz * (uint32_t)time_us;
    result = (numerator + (denominator / 2U)) / denominator;
    if (result == 0U || result > UINT32_MAX) {
        return -2;
    }

    *ticks = (uint32_t)result;
    return 0;
}

static int motor_servo_pwm_enable_op(struct motor_base *me)
{
    struct motor_servo_pwm *self;
    struct platform_pwm_state state;
    int rc;

    self = container_of(me, struct motor_servo_pwm, base);
    state = self->pwm_state;
    state.is_enabled = true;
    rc = platform_pwm_apply(self->channel, &state);
    if (rc != 0) {
        return rc;
    }

    self->pwm_state = state;
    return 0;
}

static int motor_servo_pwm_disable_op(struct motor_base *me)
{
    struct motor_servo_pwm *self;
    struct platform_pwm_state state;
    int rc;

    self = container_of(me, struct motor_servo_pwm, base);
    state = self->pwm_state;
    state.is_enabled = false;
    rc = platform_pwm_apply(self->channel, &state);
    if (rc != 0) {
        return rc;
    }

    self->pwm_state = state;
    return 0;
}

static int motor_servo_pwm_set_angle_op(
    struct motor_servo_pwm *self,
    q16_16_t angle)
{
    struct platform_pwm_state state;
    q16_16_t pulse_us;
    uint32_t tick_hz;
    uint32_t pulse_ticks;
    int rc;

    if (angle < 0 || angle > self->config->MAX_ANGLE) {
        return -2;
    }

    rc = platform_pwm_get_tick_hz(self->channel, &tick_hz);
    if (rc != 0) {
        return rc;
    }

    pulse_us = motor_servo_pwm_angle_to_pulse_us(self, angle);
    rc = motor_servo_pwm_us_to_ticks(
        pulse_us, tick_hz, &pulse_ticks);
    if (rc != 0) {
        return rc;
    }

    state = self->pwm_state;
    state.pulse_ticks = pulse_ticks;
    rc = platform_pwm_apply(self->channel, &state);
    if (rc != 0) {
        return rc;
    }

    self->pwm_state = state;
    self->angle = angle;
    return 0;
}

static const struct motor_ops motor_servo_pwm_ops = {
    .enable = motor_servo_pwm_enable_op,
    .disable = motor_servo_pwm_disable_op,
};

int motor_servo_pwm_init(
    struct motor_servo_pwm *me,
    const char *name,
    platform_pwm_channel_t channel,
    const struct motor_servo_pwm_config *config)
{
    q16_16_t initial_angle;
    q16_16_t pulse_us;
    uint32_t tick_hz;
    int rc;

    if (!me || !name || !config ||
        config->MAX_ANGLE <= 0 ||
        config->MAX_ANGLE_INV <= 0 ||
        config->SERVO_PULSE_MIN_US <= 0 ||
        config->SERVO_PULSE_MAX_US <
            config->SERVO_PULSE_MIN_US ||
        config->SERVO_PERIOD_US <
            config->SERVO_PULSE_MAX_US) {
        return -1;
    }

    rc = motor_base_init(&me->base, name, &motor_servo_pwm_ops);
    if (rc != 0) {
        return rc;
    }

    me->channel = channel;
    me->config = config;
    initial_angle = config->MAX_ANGLE / 2;
    me->angle = initial_angle;

    rc = platform_pwm_get_tick_hz(channel, &tick_hz);
    if (rc != 0) {
        return rc;
    }

    rc = motor_servo_pwm_us_to_ticks(
        config->SERVO_PERIOD_US,
        tick_hz,
        &me->pwm_state.period_ticks);
    if (rc != 0) {
        return rc;
    }

    pulse_us = motor_servo_pwm_angle_to_pulse_us(
        me, initial_angle);
    rc = motor_servo_pwm_us_to_ticks(
        pulse_us, tick_hz, &me->pwm_state.pulse_ticks);
    if (rc != 0) {
        return rc;
    }

    me->pwm_state.is_enabled = false;
    return platform_pwm_apply(channel, &me->pwm_state);
}

int motor_servo_pwm_enable(struct motor_servo_pwm *me)
{
    if (!me) {
        return -1;
    }
    return motor_enable(&me->base);
}

int motor_servo_pwm_disable(struct motor_servo_pwm *me)
{
    if (!me) {
        return -1;
    }
    return motor_disable(&me->base);
}

int motor_servo_pwm_set_angle(struct motor_servo_pwm *me,
                              q16_16_t angle)
{
    if (!me) {
        return -1;
    }
    return motor_servo_pwm_set_angle_op(me, angle);
}
