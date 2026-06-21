/* motor_tb6612fng.c */
#include "motor_tb6612fng.h"
#include "Utils/container_of.h"

static struct motor_tb6612fng *motor_tb6612fng_from_base(
    struct motor_base *base)
{
    struct motor_bdc *bdc;

    bdc = container_of(base, struct motor_bdc, base);
    return container_of(bdc, struct motor_tb6612fng, bdc);
}

static int motor_tb6612fng_enable(struct motor_base *me)
{
    struct motor_tb6612fng *self;

    self = motor_tb6612fng_from_base(me);
    return tb6612fng_channel_enable(self->driver, self->channel);
}

static int motor_tb6612fng_disable(struct motor_base *me)
{
    struct motor_tb6612fng *self;

    self = motor_tb6612fng_from_base(me);
    return tb6612fng_channel_disable(self->driver, self->channel);
}

static int motor_tb6612fng_apply_output(
    struct motor_bdc *me,
    const struct motor_bdc_output *output)
{
    struct motor_tb6612fng *self;
    tb6612fng_output_mode_t mode;

    self = container_of(me, struct motor_tb6612fng, bdc);
    switch (output->mode) {
    case MOTOR_BDC_COAST:
        mode = TB6612FNG_OUTPUT_COAST;
        break;
    case MOTOR_BDC_FORWARD:
        mode = TB6612FNG_OUTPUT_FORWARD;
        break;
    case MOTOR_BDC_REVERSE:
        mode = TB6612FNG_OUTPUT_REVERSE;
        break;
    case MOTOR_BDC_BRAKE:
        mode = TB6612FNG_OUTPUT_BRAKE;
        break;
    default:
        return -2;
    }

    return tb6612fng_apply_output(
        self->driver,
        self->channel,
        mode,
        output->period_ticks,
        output->pulse_ticks);
}

static const struct motor_ops motor_tb6612fng_ops = {
    .enable = motor_tb6612fng_enable,
    .disable = motor_tb6612fng_disable,
};

static const struct motor_bdc_ops motor_tb6612fng_bdc_ops = {
    .apply_output = motor_tb6612fng_apply_output,
};

int motor_tb6612fng_init(struct motor_tb6612fng *me, const char *name,
                         const struct motor_tb6612fng_config *config)
{
    struct motor_bdc_output initial_output;
    int rc;

    if (!me || !name || !config || !config->driver ||
        config->initial_period_ticks == 0U) {
        return -1;
    }

    if (config->channel >= TB6612FNG_CHANNEL_COUNT) {
        return -2;
    }

    me->driver = config->driver;
    me->channel = config->channel;
    initial_output = (struct motor_bdc_output){
        .mode = MOTOR_BDC_COAST,
        .period_ticks = config->initial_period_ticks,
        .pulse_ticks = 0U,
    };

    rc = motor_bdc_init(
        &me->bdc,
        name,
        &motor_tb6612fng_ops,
        &motor_tb6612fng_bdc_ops,
        &initial_output);
    if (rc != 0) {
        return rc;
    }

    return motor_tb6612fng_apply_output(&me->bdc, &initial_output);
}
