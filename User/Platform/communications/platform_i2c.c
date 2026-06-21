#include "platform_i2c.h"

#include <stddef.h>

static const struct platform_i2c_ops *s_i2c_ops;

platform_status_t platform_i2c_register(
    const struct platform_i2c_ops *ops)
{
    if (!ops || !ops->transfer) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if ((ops->get_diagnostics == NULL) !=
        (ops->reset_diagnostics == NULL)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_i2c_ops && s_i2c_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_i2c_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_i2c_transfer(
    const struct platform_i2c_client *client,
    const struct platform_i2c_msg *messages,
    uint32_t message_count)
{
    uint32_t index;

    if (!s_i2c_ops || !s_i2c_ops->transfer) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!client || !messages || message_count == 0U ||
        client->address > 0x7FU) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    for (index = 0U; index < message_count; index++) {
        if (!messages[index].data ||
            messages[index].length == 0U ||
            (messages[index].flags &
             (uint16_t)~PLATFORM_I2C_MSG_READ) != 0U) {
            return PLATFORM_STATUS_INVALID_ARGUMENT;
        }
    }

    return s_i2c_ops->transfer(
        client, messages, message_count);
}

platform_status_t platform_i2c_get_diagnostics(
    platform_i2c_bus_t bus,
    struct platform_i2c_diagnostics *diagnostics)
{
    if (!s_i2c_ops || !s_i2c_ops->get_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!diagnostics) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_i2c_ops->get_diagnostics(bus, diagnostics);
}

platform_status_t platform_i2c_reset_diagnostics(
    platform_i2c_bus_t bus)
{
    if (!s_i2c_ops || !s_i2c_ops->reset_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_i2c_ops->reset_diagnostics(bus);
}
