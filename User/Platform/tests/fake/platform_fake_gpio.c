#include "platform_fake_gpio.h"

#include <stdbool.h>
#include <stddef.h>

struct platform_fake_gpio_pin
{
    platform_gpio_direction_t direction;
    platform_gpio_level_t level;
    platform_gpio_irq_callback_t callback;
    void *context;
    bool irq_enabled;
};

static struct platform_fake_gpio_pin
    s_pins[PLATFORM_FAKE_GPIO_PIN_COUNT];
static struct platform_gpio_diagnostics s_diagnostics[2];

static bool platform_fake_gpio_pin_is_valid(
    platform_gpio_pin_t pin)
{
    return pin < PLATFORM_FAKE_GPIO_PIN_COUNT;
}

static platform_status_t platform_fake_gpio_set_direction(
    platform_gpio_pin_t pin,
    platform_gpio_direction_t direction)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_pins[pin].direction = direction;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_write(
    platform_gpio_pin_t pin,
    platform_gpio_level_t level)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    if (s_pins[pin].direction != PLATFORM_GPIO_OUTPUT) {
        return PLATFORM_STATUS_INVALID_STATE;
    }
    s_pins[pin].level = level;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_read(
    platform_gpio_pin_t pin,
    platform_gpio_level_t *level)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    if (!level) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    *level = s_pins[pin].level;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_irq_attach(
    platform_gpio_pin_t pin,
    platform_gpio_irq_trigger_t trigger,
    platform_gpio_irq_callback_t callback,
    void *context)
{
    (void)trigger;
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    if (!callback) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    s_pins[pin].callback = callback;
    s_pins[pin].context = context;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_irq_enable(
    platform_gpio_pin_t pin)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    if (!s_pins[pin].callback) {
        return PLATFORM_STATUS_INVALID_STATE;
    }
    s_pins[pin].irq_enabled = true;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_irq_disable(
    platform_gpio_pin_t pin)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_pins[pin].irq_enabled = false;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_irq_detach(
    platform_gpio_pin_t pin)
{
    platform_status_t rc =
        platform_fake_gpio_irq_disable(pin);

    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    s_pins[pin].callback = NULL;
    s_pins[pin].context = NULL;
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_get_diagnostics(
    platform_gpio_pin_t pin,
    struct platform_gpio_diagnostics *diagnostics)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    if (!diagnostics) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    *diagnostics = s_diagnostics[pin >> 5U];
    return PLATFORM_STATUS_OK;
}

static platform_status_t platform_fake_gpio_reset_diagnostics(
    platform_gpio_pin_t pin)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_diagnostics[pin >> 5U] =
        (struct platform_gpio_diagnostics){0};
    return PLATFORM_STATUS_OK;
}

static const struct platform_gpio_ops s_fake_gpio_ops = {
    .set_direction = platform_fake_gpio_set_direction,
    .write = platform_fake_gpio_write,
    .read = platform_fake_gpio_read,
    .irq_attach = platform_fake_gpio_irq_attach,
    .irq_enable = platform_fake_gpio_irq_enable,
    .irq_disable = platform_fake_gpio_irq_disable,
    .irq_detach = platform_fake_gpio_irq_detach,
    .get_diagnostics =
        platform_fake_gpio_get_diagnostics,
    .reset_diagnostics =
        platform_fake_gpio_reset_diagnostics,
};

platform_status_t platform_fake_gpio_install(void)
{
    platform_fake_gpio_reset();
    return platform_gpio_register(&s_fake_gpio_ops);
}

void platform_fake_gpio_reset(void)
{
    uint32_t pin;

    for (pin = 0U;
         pin < PLATFORM_FAKE_GPIO_PIN_COUNT;
         pin++) {
        s_pins[pin].direction = PLATFORM_GPIO_INPUT;
        s_pins[pin].level = PLATFORM_GPIO_LOW;
        s_pins[pin].callback = NULL;
        s_pins[pin].context = NULL;
        s_pins[pin].irq_enabled = false;
    }
    s_diagnostics[0] =
        (struct platform_gpio_diagnostics){0};
    s_diagnostics[1] =
        (struct platform_gpio_diagnostics){0};
}

platform_status_t platform_fake_gpio_trigger(
    platform_gpio_pin_t pin)
{
    struct platform_gpio_diagnostics *diagnostics;

    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    diagnostics = &s_diagnostics[pin >> 5U];
    diagnostics->irq_dispatch_count++;
    if (!s_pins[pin].irq_enabled ||
        !s_pins[pin].callback) {
        diagnostics->irq_unhandled_count++;
        return PLATFORM_STATUS_OK;
    }
    s_pins[pin].callback(pin, s_pins[pin].context);
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_fake_gpio_force_isr_limit(
    platform_gpio_pin_t pin)
{
    if (!platform_fake_gpio_pin_is_valid(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_diagnostics[pin >> 5U].isr_limit_hit_count++;
    return PLATFORM_STATUS_OK;
}
