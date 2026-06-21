#include "platform_uart.h"

#include <stddef.h>

static const struct platform_uart_ops *s_uart_ops;

platform_status_t platform_uart_register(
    const struct platform_uart_ops *ops)
{
    if (!ops || !ops->configure ||
        !ops->write || !ops->read) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if ((ops->attach == NULL) != (ops->detach == NULL)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if ((ops->get_diagnostics == NULL) !=
        (ops->reset_diagnostics == NULL)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_uart_ops && s_uart_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_uart_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_uart_configure(
    platform_uart_port_t port,
    const struct platform_uart_config *config)
{
    if (!s_uart_ops || !s_uart_ops->configure) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!config) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (config->baud_rate == 0U ||
        config->data_bits < 5U ||
        config->data_bits > 8U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_uart_ops->configure(port, config);
}

platform_status_t platform_uart_write(
    platform_uart_port_t port,
    const uint8_t *data,
    uint32_t length,
    uint32_t *written)
{
    if (!s_uart_ops || !s_uart_ops->write) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!data || !written || length == 0U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    *written = 0U;
    return s_uart_ops->write(
        port, data, length, written);
}

platform_status_t platform_uart_read(
    platform_uart_port_t port,
    uint8_t *data,
    uint32_t capacity,
    uint32_t *received)
{
    if (!s_uart_ops || !s_uart_ops->read) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!data || !received || capacity == 0U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    *received = 0U;
    return s_uart_ops->read(
        port, data, capacity, received);
}

platform_status_t platform_uart_attach(
    platform_uart_port_t port,
    platform_uart_callback_t callback,
    void *context)
{
    if (!s_uart_ops || !s_uart_ops->attach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!callback) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_uart_ops->attach(port, callback, context);
}

platform_status_t platform_uart_detach(
    platform_uart_port_t port)
{
    if (!s_uart_ops || !s_uart_ops->detach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_uart_ops->detach(port);
}

platform_status_t platform_uart_get_diagnostics(
    platform_uart_port_t port,
    struct platform_uart_diagnostics *diagnostics)
{
    if (!s_uart_ops || !s_uart_ops->get_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!diagnostics) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_uart_ops->get_diagnostics(port, diagnostics);
}

platform_status_t platform_uart_reset_diagnostics(
    platform_uart_port_t port)
{
    if (!s_uart_ops || !s_uart_ops->reset_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_uart_ops->reset_diagnostics(port);
}
