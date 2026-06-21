#ifndef MOTOR_SERVO_PWM_H
#define MOTOR_SERVO_PWM_H

#include "motor_base.h"
#include "Platform/timers/platform_pwm.h"
#include "Utils/fixedpoint.h"

struct motor_servo_pwm_config
{
    const q16_16_t MAX_ANGLE;
    const q16_16_t MAX_ANGLE_INV;
    const q16_16_t SERVO_PULSE_MAX_US;
    const q16_16_t SERVO_PULSE_MIN_US;
    const q16_16_t SERVO_PERIOD_US;
};

struct motor_servo_pwm
{
    struct motor_base base;
    platform_pwm_channel_t channel;
    struct platform_pwm_state pwm_state;
    q16_16_t angle;
    const struct motor_servo_pwm_config *config;
};

int motor_servo_pwm_init(
    struct motor_servo_pwm *me,
    const char *name,
    platform_pwm_channel_t channel,
    const struct motor_servo_pwm_config *config);
int motor_servo_pwm_enable(struct motor_servo_pwm *me);
int motor_servo_pwm_disable(struct motor_servo_pwm *me);
int motor_servo_pwm_set_angle(struct motor_servo_pwm *me,
                              q16_16_t angle);

#endif /* MOTOR_SERVO_PWM_H */
