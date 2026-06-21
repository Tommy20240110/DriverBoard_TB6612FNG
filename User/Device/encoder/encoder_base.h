/* encoder_base.h */
#ifndef ENCODER_BASE_H
#define ENCODER_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include "Utils/fixedpoint.h"

/* 编码器运行模式 */
typedef enum encoder_mode
{
    ENCODER_STOP = 0,
    ENCODER_FORWARD = 1,
    ENCODER_REVERSE = 2,
} encoder_mode_t;

struct encoder_base;

/* 操作表 */
struct encoder_ops
{
    int (*enable)(struct encoder_base *me);     /* 必填 */
    int (*disable)(struct encoder_base *me);    /* 必填 */
    int (*set_counter)(struct encoder_base *me, /* 必填 */
                       int64_t counter);
    int (*get_counter)(struct encoder_base *me, /* 必填 */
                       int64_t *counter);
    int (*get_mode)(struct encoder_base *me, /* 选填 */
                    encoder_mode_t *mode);
    int (*get_rev)(struct encoder_base *me, /* 选填 */
                   q16_16_t *rev);
};

struct encoder_base
{
    const struct encoder_ops *ops;
    const char *name;
    bool is_enabled;
    int64_t counter;
};

int encoder_base_init(struct encoder_base *me, const char *name,
                      const struct encoder_ops *ops);

/* 基类统一接口 */
int encoder_enable(struct encoder_base *me);
int encoder_disable(struct encoder_base *me);
int encoder_set_counter(struct encoder_base *me,
                        int64_t counter);
int encoder_get_counter(struct encoder_base *me,
                        int64_t *counter);
int encoder_get_mode(struct encoder_base *me,
                     encoder_mode_t *mode);
int encoder_get_rev(struct encoder_base *me,
                    q16_16_t *rev);

#endif /* ENCODER_BASE_H */
