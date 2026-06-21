#ifndef MOTOR_BDC_H
#define MOTOR_BDC_H

#include <stdint.h>
#include "motor_base.h"

typedef enum motor_bdc_mode
{
    MOTOR_BDC_COAST = 0,
    MOTOR_BDC_FORWARD,
    MOTOR_BDC_REVERSE,
    MOTOR_BDC_BRAKE,
} motor_bdc_mode_t;

/*
 * PWM values stay in the timer's quantized domain. The command is valid when
 * period_ticks is nonzero and pulse_ticks is not greater than period_ticks.
 */
struct motor_bdc_output
{
    motor_bdc_mode_t mode;
    uint32_t period_ticks;
    uint32_t pulse_ticks;
};

struct motor_bdc;

struct motor_bdc_ops
{
    int (*apply_output)(struct motor_bdc *me,
                        const struct motor_bdc_output *output);
};

struct motor_bdc
{
    struct motor_base base;
    const struct motor_bdc_ops *ops;
    struct motor_bdc_output output;
};

int motor_bdc_init(struct motor_bdc *me, const char *name,
                   const struct motor_ops *motor_ops,
                   const struct motor_bdc_ops *bdc_ops,
                   const struct motor_bdc_output *initial_output);
int motor_bdc_enable(struct motor_bdc *me);
int motor_bdc_disable(struct motor_bdc *me);
int motor_bdc_apply_output(struct motor_bdc *me,
                           const struct motor_bdc_output *output);
int motor_bdc_get_output(const struct motor_bdc *me,
                         struct motor_bdc_output *output);

#endif /* MOTOR_BDC_H */
