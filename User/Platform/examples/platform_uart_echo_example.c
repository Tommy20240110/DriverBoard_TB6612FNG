#include "platform_examples.h"

#include <stddef.h>

static void platform_example_uart_callback(
    platform_uart_port_t port,
    platform_uart_event_t event,
    uint8_t data,
    void *context)
{
    struct platform_uart_echo_example *example = context;

    (void)port;
    if (event == PLATFORM_UART_EVENT_RX_DATA &&
        !example->byte_pending) {
        example->byte = data;
        example->byte_pending = true;
    }
}

platform_status_t platform_example_uart_echo_start(
    struct platform_uart_echo_example *example,
    const struct platform_uart_config *config)
{
    platform_status_t rc;

    if (!example || !config) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    example->byte_pending = false;
    rc = platform_uart_configure(example->port, config);
    return rc == PLATFORM_STATUS_OK
               ? platform_uart_attach(
                     example->port,
                     platform_example_uart_callback,
                     example)
               : rc;
}

platform_status_t platform_example_uart_echo_poll(
    struct platform_uart_echo_example *example)
{
    uint32_t written;
    uint8_t byte;

    if (!example) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (!example->byte_pending) {
        return PLATFORM_STATUS_TRY_AGAIN;
    }
    byte = example->byte;
    example->byte_pending = false;
    return platform_uart_write(
        example->port, &byte, 1U, &written);
}
