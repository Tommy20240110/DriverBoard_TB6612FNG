#ifndef PLATFORM_GPIO_H
#define PLATFORM_GPIO_H

#include "../core/platform_status.h"

#include <stdint.h>

typedef uint8_t platform_gpio_pin_t;

typedef enum platform_gpio_level
{
    PLATFORM_GPIO_LOW = 0,
    PLATFORM_GPIO_HIGH,
} platform_gpio_level_t;

typedef enum platform_gpio_direction
{
    PLATFORM_GPIO_INPUT = 0,
    PLATFORM_GPIO_OUTPUT,
} platform_gpio_direction_t;

typedef enum platform_gpio_irq_trigger
{
    PLATFORM_GPIO_IRQ_RISING = 0,
    PLATFORM_GPIO_IRQ_FALLING,
    PLATFORM_GPIO_IRQ_BOTH,
} platform_gpio_irq_trigger_t;

typedef void (*platform_gpio_irq_callback_t)(
    platform_gpio_pin_t pin,
    void *context);

struct platform_gpio_diagnostics
{
    uint32_t irq_dispatch_count;
    uint32_t irq_unhandled_count;
    uint32_t isr_limit_hit_count;
};

struct platform_gpio_ops
{
    platform_status_t (*set_direction)(
        platform_gpio_pin_t pin,
        platform_gpio_direction_t direction);
    platform_status_t (*write)(
        platform_gpio_pin_t pin,
        platform_gpio_level_t level);
    platform_status_t (*read)(
        platform_gpio_pin_t pin,
        platform_gpio_level_t *level);
    platform_status_t (*irq_attach)(
        platform_gpio_pin_t pin,
        platform_gpio_irq_trigger_t trigger,
        platform_gpio_irq_callback_t callback,
        void *context);
    platform_status_t (*irq_enable)(
        platform_gpio_pin_t pin);
    platform_status_t (*irq_disable)(
        platform_gpio_pin_t pin);
    platform_status_t (*irq_detach)(
        platform_gpio_pin_t pin);
    platform_status_t (*get_diagnostics)(
        platform_gpio_pin_t pin,
        struct platform_gpio_diagnostics *diagnostics);
    platform_status_t (*reset_diagnostics)(
        platform_gpio_pin_t pin);
};

platform_status_t platform_gpio_register(
    const struct platform_gpio_ops *ops);
platform_status_t platform_gpio_set_direction(
    platform_gpio_pin_t pin,
    platform_gpio_direction_t direction);
platform_status_t platform_gpio_write(
    platform_gpio_pin_t pin,
    platform_gpio_level_t level);
platform_status_t platform_gpio_read(
    platform_gpio_pin_t pin,
    platform_gpio_level_t *level);
platform_status_t platform_gpio_irq_attach(
    platform_gpio_pin_t pin,
    platform_gpio_irq_trigger_t trigger,
    platform_gpio_irq_callback_t callback,
    void *context);
platform_status_t platform_gpio_irq_enable(
    platform_gpio_pin_t pin);
platform_status_t platform_gpio_irq_disable(
    platform_gpio_pin_t pin);
platform_status_t platform_gpio_irq_detach(
    platform_gpio_pin_t pin);
platform_status_t platform_gpio_get_diagnostics(
    platform_gpio_pin_t pin,
    struct platform_gpio_diagnostics *diagnostics);
platform_status_t platform_gpio_reset_diagnostics(
    platform_gpio_pin_t pin);

#endif /* PLATFORM_GPIO_H */
