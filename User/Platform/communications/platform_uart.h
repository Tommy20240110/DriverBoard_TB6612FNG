#ifndef PLATFORM_UART_H
#define PLATFORM_UART_H

#include "../core/platform_status.h"

#include <stdint.h>

/*
 * UART IDs are provider instance-table indices, not MCU register addresses.
 * FIFO I/O is non-blocking; callbacks execute in interrupt context.
 */
typedef uint8_t platform_uart_port_t;

typedef enum platform_uart_parity
{
    PLATFORM_UART_PARITY_NONE = 0,
    PLATFORM_UART_PARITY_EVEN,
    PLATFORM_UART_PARITY_ODD,
} platform_uart_parity_t;

typedef enum platform_uart_stop_bits
{
    PLATFORM_UART_STOP_BITS_ONE = 0,
    PLATFORM_UART_STOP_BITS_TWO,
} platform_uart_stop_bits_t;

typedef enum platform_uart_event
{
    PLATFORM_UART_EVENT_RX_DATA = 0,
    PLATFORM_UART_EVENT_OVERRUN,
    PLATFORM_UART_EVENT_BREAK,
    PLATFORM_UART_EVENT_PARITY,
    PLATFORM_UART_EVENT_FRAMING,
    PLATFORM_UART_EVENT_NOISE,
} platform_uart_event_t;

struct platform_uart_config
{
    uint32_t baud_rate;
    uint8_t data_bits;
    platform_uart_parity_t parity;
    platform_uart_stop_bits_t stop_bits;
};

typedef void (*platform_uart_callback_t)(
    platform_uart_port_t port,
    platform_uart_event_t event,
    uint8_t data,
    void *context);

struct platform_uart_diagnostics
{
    uint32_t rx_byte_count;
    uint32_t overrun_error_count;
    uint32_t break_error_count;
    uint32_t parity_error_count;
    uint32_t framing_error_count;
    uint32_t noise_error_count;
    uint32_t isr_limit_hit_count;
};

struct platform_uart_ops
{
    platform_status_t (*configure)(
        platform_uart_port_t port,
        const struct platform_uart_config *config);
    platform_status_t (*write)(
        platform_uart_port_t port,
        const uint8_t *data,
        uint32_t length,
        uint32_t *written);
    platform_status_t (*read)(
        platform_uart_port_t port,
        uint8_t *data,
        uint32_t capacity,
        uint32_t *received);
    platform_status_t (*attach)(
        platform_uart_port_t port,
        platform_uart_callback_t callback,
        void *context);
    platform_status_t (*detach)(
        platform_uart_port_t port);
    platform_status_t (*get_diagnostics)(
        platform_uart_port_t port,
        struct platform_uart_diagnostics *diagnostics);
    platform_status_t (*reset_diagnostics)(
        platform_uart_port_t port);
};

/* Registers the one active UART provider. */
platform_status_t platform_uart_register(
    const struct platform_uart_ops *ops);

/* Applies baud rate and frame format to a port. */
platform_status_t platform_uart_configure(
    platform_uart_port_t port,
    const struct platform_uart_config *config);

/* Writes as many bytes as fit and stores the count in written. */
platform_status_t platform_uart_write(
    platform_uart_port_t port,
    const uint8_t *data,
    uint32_t length,
    uint32_t *written);

/* Reads available FIFO bytes and stores the count in received. */
platform_status_t platform_uart_read(
    platform_uart_port_t port,
    uint8_t *data,
    uint32_t capacity,
    uint32_t *received);

/* Attaches one ISR callback to a port; returns nonzero on failure. */
platform_status_t platform_uart_attach(
    platform_uart_port_t port,
    platform_uart_callback_t callback,
    void *context);

/* Disables UART events and removes the callback. */
platform_status_t platform_uart_detach(
    platform_uart_port_t port);
platform_status_t platform_uart_get_diagnostics(
    platform_uart_port_t port,
    struct platform_uart_diagnostics *diagnostics);
platform_status_t platform_uart_reset_diagnostics(
    platform_uart_port_t port);

#endif /* PLATFORM_UART_H */
