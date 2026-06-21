#include "platform_examples.h"

#include <stddef.h>

static void platform_example_gpio_irq(
    platform_gpio_pin_t pin,
    void *context)
{
    struct platform_gpio_example *example = context;

    (void)pin;
    example->edge_count++;
}

platform_status_t platform_example_gpio_start(
    struct platform_gpio_example *example)
{
    platform_status_t rc;

    if (!example) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    example->edge_count = 0U;
    rc = platform_gpio_set_direction(
        example->output_pin, PLATFORM_GPIO_OUTPUT);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_gpio_write(
        example->output_pin, PLATFORM_GPIO_LOW);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_gpio_set_direction(
        example->input_pin, PLATFORM_GPIO_INPUT);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_gpio_irq_attach(
        example->input_pin,
        PLATFORM_GPIO_IRQ_BOTH,
        platform_example_gpio_irq,
        example);
    return rc == PLATFORM_STATUS_OK
               ? platform_gpio_irq_enable(example->input_pin)
               : rc;
}

platform_status_t platform_example_gpio_stop(
    struct platform_gpio_example *example)
{
    if (!example) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return platform_gpio_irq_detach(example->input_pin);
}
