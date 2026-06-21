/* motor_base.c */
#include "motor_base.h"
#include <assert.h>
#include <stddef.h>

int motor_base_init(struct motor_base *me, const char *name,
                    const struct motor_ops *ops)
{
    if (!me || !name || !ops || !ops->enable || !ops->disable) {
        return -1;
    }

    me->ops = ops;
    me->name = name;
    me->is_enabled = false;

    return 0;
}

/* 父类统一接口 */
int motor_enable(struct motor_base *me)
{
    int rc;

    if (!me || !me->ops || !me->ops->enable) {
        return -1;
    }

    assert(me->ops->enable);
    if (me->is_enabled) {
        return 0;
    }

    rc = me->ops->enable(me);
    if (rc == 0) {
        me->is_enabled = true;
    }
    return rc;
}

int motor_disable(struct motor_base *me)
{
    int rc;

    if (!me || !me->ops || !me->ops->disable) {
        return -1;
    }

    assert(me->ops->disable);
    if (!me->is_enabled) {
        return 0;
    }

    rc = me->ops->disable(me);
    if (rc == 0) {
        me->is_enabled = false;
    }
    return rc;
}

const char *motor_get_name(const struct motor_base *me)
{
    if (!me) {
        return NULL;
    }
    return me->name;
}

bool motor_is_enabled(const struct motor_base *me)
{
    return me && me->is_enabled;
}
