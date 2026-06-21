/* fixedpoint.h */
#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdint.h>
#include <limits.h>

/* Q16.16 格式 */
typedef int32_t q16_16_t;

#define Q16_16_FRAC_BITS 16
#define Q16_16_SCALE 65536.0f

/* 浮点 <-> 定点 转换（仅初始化用，不在控制循环里用）*/
#define FLOAT_TO_Q16_16(x) ((q16_16_t)((x) * Q16_16_SCALE))
#define Q16_16_TO_FLOAT(x) ((float)(x) / Q16_16_SCALE)

/* 整数 <-> 定点 转换 */
#define INT_TO_Q16_16(x) ((q16_16_t)(x) << Q16_16_FRAC_BITS)
#define Q16_16_TO_INT(x) ((x) >> Q16_16_FRAC_BITS)

/* 饱和保护 */
#define Q16_16_SAT(x)                                               \
    (((int64_t)(x) > (int64_t)INT32_MAX) ? (q16_16_t)INT32_MAX :    \
     ((int64_t)(x) < (int64_t)INT32_MIN) ? (q16_16_t)INT32_MIN :    \
     (q16_16_t)(x))

/* 带饱和保护的定点数运算 */
#define Q16_16_ADD(a, b) \
    Q16_16_SAT((int64_t)(a) + (int64_t)(b))

#define Q16_16_SUB(a, b) \
    Q16_16_SAT((int64_t)(a) - (int64_t)(b))

#define Q16_16_MUL(a, b) \
    Q16_16_SAT(((int64_t)(a) * (b)) >> 16)

#define Q16_16_DIV(a, b) \
    Q16_16_SAT(((int64_t)(a) << 16) / (b))

#define Q16_16_INV(a)    \
    Q16_16_DIV(INT_TO_Q16_16(1), a)

#endif /* fixedpoint.h */
