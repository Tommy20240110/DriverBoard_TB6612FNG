#include "platform_gpio.h"

#include <stdbool.h>
#include <stddef.h>

static const struct platform_gpio_ops *s_gpio_ops;

static bool platform_gpio_ops_are_valid(
    const struct platform_gpio_ops *ops)
{
    bool has_any_irq;
    bool has_all_irq;

    if (!ops || !ops->set_direction ||
        !ops->write || !ops->read) {
        return false;
    }

    has_any_irq = ops->irq_attach || ops->irq_enable ||
                  ops->irq_disable || ops->irq_detach;
    has_all_irq = ops->irq_attach && ops->irq_enable &&
                  ops->irq_disable && ops->irq_detach;
    if (has_any_irq && !has_all_irq) {
        return false;
    }
    if ((ops->get_diagnostics == NULL) !=
        (ops->reset_diagnostics == NULL)) {
        return false;
    }
    return true;
}

platform_status_t platform_gpio_register(
    const struct platform_gpio_ops *ops)
{
    if (!platform_gpio_ops_are_valid(ops)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_gpio_ops && s_gpio_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_gpio_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_gpio_set_direction(
    platform_gpio_pin_t pin,
    platform_gpio_direction_t direction)
{
    if (!s_gpio_ops || !s_gpio_ops->set_direction) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (direction > PLATFORM_GPIO_OUTPUT) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_gpio_ops->set_direction(pin, direction);
}

platform_status_t platform_gpio_write(
    platform_gpio_pin_t pin,
    platform_gpio_level_t level)
{
    if (!s_gpio_ops || !s_gpio_ops->write) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (level > PLATFORM_GPIO_HIGH) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_gpio_ops->write(pin, level);
}

platform_status_t platform_gpio_read(
    platform_gpio_pin_t pin,
    platform_gpio_level_t *level)
{
    if (!s_gpio_ops || !s_gpio_ops->read) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!level) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_gpio_ops->read(pin, level);
}

platform_status_t platform_gpio_irq_attach(
    platform_gpio_pin_t pin,
    platform_gpio_irq_trigger_t trigger,
    platform_gpio_irq_callback_t callback,
    void *context)
{
    if (!s_gpio_ops || !s_gpio_ops->irq_attach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!callback || trigger > PLATFORM_GPIO_IRQ_BOTH) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_gpio_ops->irq_attach(
        pin, trigger, callback, context);
}

platform_status_t platform_gpio_irq_enable(
    platform_gpio_pin_t pin)
{
    if (!s_gpio_ops || !s_gpio_ops->irq_enable) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_gpio_ops->irq_enable(pin);
}

platform_status_t platform_gpio_irq_disable(
    platform_gpio_pin_t pin)
{
    if (!s_gpio_ops || !s_gpio_ops->irq_disable) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_gpio_ops->irq_disable(pin);
}

platform_status_t platform_gpio_irq_detach(
    platform_gpio_pin_t pin)
{
    if (!s_gpio_ops || !s_gpio_ops->irq_detach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_gpio_ops->irq_detach(pin);
}

platform_status_t platform_gpio_get_diagnostics(
    platform_gpio_pin_t pin,
    struct platform_gpio_diagnostics *diagnostics)
{
    if (!s_gpio_ops || !s_gpio_ops->get_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!diagnostics) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_gpio_ops->get_diagnostics(pin, diagnostics);
}

platform_status_t platform_gpio_reset_diagnostics(
    platform_gpio_pin_t pin)
{
    if (!s_gpio_ops || !s_gpio_ops->reset_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_gpio_ops->reset_diagnostics(pin);
}
