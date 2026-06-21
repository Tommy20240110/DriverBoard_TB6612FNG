#include "motor_bdc.h"

static int motor_bdc_validate_output(
    const struct motor_bdc_output *output)
{
    if (!output || output->period_ticks == 0U ||
        output->pulse_ticks > output->period_ticks) {
        return -1;
    }

    if (output->mode < MOTOR_BDC_COAST ||
        output->mode > MOTOR_BDC_BRAKE) {
        return -2;
    }

    return 0;
}

int motor_bdc_init(struct motor_bdc *me, const char *name,
                   const struct motor_ops *motor_ops,
                   const struct motor_bdc_ops *bdc_ops,
                   const struct motor_bdc_output *initial_output)
{
    int rc;

    if (!me || !bdc_ops || !bdc_ops->apply_output) {
        return -1;
    }

    rc = motor_bdc_validate_output(initial_output);
    if (rc != 0) {
        return rc;
    }

    rc = motor_base_init(&me->base, name, motor_ops);
    if (rc != 0) {
        return rc;
    }

    me->ops = bdc_ops;
    me->output = *initial_output;
    return 0;
}

int motor_bdc_enable(struct motor_bdc *me)
{
    if (!me) {
        return -1;
    }
    return motor_enable(&me->base);
}

int motor_bdc_disable(struct motor_bdc *me)
{
    if (!me) {
        return -1;
    }
    return motor_disable(&me->base);
}

int motor_bdc_apply_output(struct motor_bdc *me,
                           const struct motor_bdc_output *output)
{
    int rc;

    if (!me || !me->ops || !me->ops->apply_output) {
        return -1;
    }

    rc = motor_bdc_validate_output(output);
    if (rc != 0) {
        return rc;
    }

    rc = me->ops->apply_output(me, output);
    if (rc == 0) {
        me->output = *output;
    }
    return rc;
}

int motor_bdc_get_output(const struct motor_bdc *me,
                         struct motor_bdc_output *output)
{
    if (!me || !output) {
        return -1;
    }

    *output = me->output;
    return 0;
}
