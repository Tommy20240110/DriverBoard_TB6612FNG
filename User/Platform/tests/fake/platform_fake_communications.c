#include "platform_fake_communications.h"

#include "Platform/communications/platform_can.h"
#include "Platform/communications/platform_i2c.h"
#include "Platform/communications/platform_uart.h"

#include <stddef.h>

static struct platform_uart_diagnostics s_uart_diagnostics;
static struct platform_i2c_diagnostics s_i2c_diagnostics;
static struct platform_can_diagnostics s_can_diagnostics;

static platform_status_t fake_uart_configure(
    platform_uart_port_t port,
    const struct platform_uart_config *config)
{
    (void)config;
    return port == 0U ? PLATFORM_STATUS_OK
                      : PLATFORM_STATUS_NO_DEVICE;
}

static platform_status_t fake_uart_write(
    platform_uart_port_t port,
    const uint8_t *data,
    uint32_t length,
    uint32_t *written)
{
    (void)data;
    if (port != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    *written = length;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_uart_read(
    platform_uart_port_t port,
    uint8_t *data,
    uint32_t capacity,
    uint32_t *received)
{
    (void)data;
    (void)capacity;
    if (port != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    *received = 0U;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_uart_get_diagnostics(
    platform_uart_port_t port,
    struct platform_uart_diagnostics *diagnostics)
{
    if (port != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    *diagnostics = s_uart_diagnostics;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_uart_reset_diagnostics(
    platform_uart_port_t port)
{
    if (port != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_uart_diagnostics =
        (struct platform_uart_diagnostics){0};
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_i2c_transfer(
    const struct platform_i2c_client *client,
    const struct platform_i2c_msg *messages,
    uint32_t message_count)
{
    (void)messages;
    (void)message_count;
    if (client->bus != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_i2c_diagnostics.transfer_count++;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_i2c_get_diagnostics(
    platform_i2c_bus_t bus,
    struct platform_i2c_diagnostics *diagnostics)
{
    if (bus != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    *diagnostics = s_i2c_diagnostics;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_i2c_reset_diagnostics(
    platform_i2c_bus_t bus)
{
    if (bus != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_i2c_diagnostics =
        (struct platform_i2c_diagnostics){0};
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_can_start(
    platform_can_controller_t controller)
{
    return controller == 0U ? PLATFORM_STATUS_OK
                            : PLATFORM_STATUS_NO_DEVICE;
}

static platform_status_t fake_can_stop(
    platform_can_controller_t controller)
{
    return fake_can_start(controller);
}

static platform_status_t fake_can_send(
    platform_can_controller_t controller,
    const struct platform_can_frame *frame)
{
    (void)frame;
    if (controller != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_can_diagnostics.send_count++;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_can_receive(
    platform_can_controller_t controller,
    struct platform_can_frame *frame)
{
    (void)frame;
    if (controller != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_can_diagnostics.no_frame_count++;
    return PLATFORM_STATUS_TRY_AGAIN;
}

static platform_status_t fake_can_get_diagnostics(
    platform_can_controller_t controller,
    struct platform_can_diagnostics *diagnostics)
{
    if (controller != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    *diagnostics = s_can_diagnostics;
    return PLATFORM_STATUS_OK;
}

static platform_status_t fake_can_reset_diagnostics(
    platform_can_controller_t controller)
{
    if (controller != 0U) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    s_can_diagnostics =
        (struct platform_can_diagnostics){0};
    return PLATFORM_STATUS_OK;
}

static const struct platform_uart_ops s_uart_ops = {
    .configure = fake_uart_configure,
    .write = fake_uart_write,
    .read = fake_uart_read,
    .get_diagnostics = fake_uart_get_diagnostics,
    .reset_diagnostics = fake_uart_reset_diagnostics,
};

static const struct platform_i2c_ops s_i2c_ops = {
    .transfer = fake_i2c_transfer,
    .get_diagnostics = fake_i2c_get_diagnostics,
    .reset_diagnostics = fake_i2c_reset_diagnostics,
};

static const struct platform_can_ops s_can_ops = {
    .start = fake_can_start,
    .stop = fake_can_stop,
    .send = fake_can_send,
    .receive = fake_can_receive,
    .get_diagnostics = fake_can_get_diagnostics,
    .reset_diagnostics = fake_can_reset_diagnostics,
};

platform_status_t platform_fake_communications_install(void)
{
    platform_status_t rc;

    rc = platform_uart_register(&s_uart_ops);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    rc = platform_i2c_register(&s_i2c_ops);
    if (rc != PLATFORM_STATUS_OK) {
        return rc;
    }
    return platform_can_register(&s_can_ops);
}

void platform_fake_uart_record_rx(void)
{
    s_uart_diagnostics.rx_byte_count++;
}
