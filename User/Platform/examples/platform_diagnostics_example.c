#include "platform_examples.h"

#include <stddef.h>

platform_status_t platform_example_get_diagnostics(
    platform_gpio_pin_t gpio_pin,
    platform_uart_port_t uart,
    platform_i2c_bus_t i2c,
    platform_can_controller_t can,
    struct platform_diagnostics_snapshot *snapshot)
{
    platform_status_t rc;

    if (!snapshot) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    rc = platform_gpio_get_diagnostics(
        gpio_pin, &snapshot->gpio);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_uart_get_diagnostics(
        uart, &snapshot->uart);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_i2c_get_diagnostics(
        i2c, &snapshot->i2c);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    return platform_can_get_diagnostics(
        can, &snapshot->can);
}
