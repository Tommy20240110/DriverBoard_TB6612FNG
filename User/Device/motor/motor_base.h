/* motor_base.h */
#ifndef MOTOR_BASE_H
#define MOTOR_BASE_H

#include <stdbool.h>

struct motor_base;

/*
 * motor_base only models the lifecycle shared by every motor-like actuator.
 * Speed, position and PWM belong to narrower, type-specific interfaces.
 */
struct motor_ops
{
    int (*enable)(struct motor_base *me);
    int (*disable)(struct motor_base *me);
};

struct motor_base
{
    const struct motor_ops *ops;
    const char *name;
    bool is_enabled;
};

int motor_base_init(struct motor_base *me, const char *name,
                    const struct motor_ops *ops);

int motor_enable(struct motor_base *me);
int motor_disable(struct motor_base *me);
const char *motor_get_name(const struct motor_base *me);
bool motor_is_enabled(const struct motor_base *me);

#endif /* MOTOR_BASE_H */
