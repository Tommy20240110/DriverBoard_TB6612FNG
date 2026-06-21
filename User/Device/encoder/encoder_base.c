/* encoder_base.c */
#include "encoder_base.h"
#include <assert.h>

int encoder_base_init(struct encoder_base *me, const char *name,
                      const struct encoder_ops *ops)
{
    if (!me || !name || !ops || !ops->enable ||
        !ops->disable || !ops->set_counter ||
        !ops->get_counter) {
        return -1;
    }

    me->ops = ops;
    me->name = name;
    me->is_enabled = false;
    me->counter = 0;

    return 0;
}

int encoder_enable(struct encoder_base *me)
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

int encoder_disable(struct encoder_base *me)
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

int encoder_set_counter(struct encoder_base *me,
                        int64_t counter)
{
    if (!me || !me->ops || !me->ops->set_counter) {
        return -1;
    }

    assert(me->ops->set_counter);

    return me->ops->set_counter(me, counter);
}

int encoder_get_counter(struct encoder_base *me,
                        int64_t *counter)
{
    if (!me || !counter || !me->ops ||
        !me->ops->get_counter) {
        return -1;
    }

    assert(me->ops->get_counter);

    return me->ops->get_counter(me, counter);
}

int encoder_get_mode(struct encoder_base *me,
                     encoder_mode_t *mode)
{
    if (!me || !mode || !me->ops) {
        return -1;
    }

    if (!me->ops->get_mode) {
        return -2;
    }

    return me->ops->get_mode(me, mode);
}

int encoder_get_rev(struct encoder_base *me,
                    q16_16_t *rev)
{
    if (!me || !rev || !me->ops) {
        return -1;
    }

    if (!me->ops->get_rev) {
        return -2;
    }

    return me->ops->get_rev(me, rev);
}
