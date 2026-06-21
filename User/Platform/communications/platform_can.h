#ifndef PLATFORM_CAN_H
#define PLATFORM_CAN_H

#include "../core/platform_status.h"

#include <stdbool.h>
#include <stdint.h>

/* CAN controller IDs are provider instance-table indices. */
typedef uint8_t platform_can_controller_t;

#define PLATFORM_CAN_FRAME_EXTENDED (1U << 0U)
#define PLATFORM_CAN_FRAME_REMOTE (1U << 1U)
#define PLATFORM_CAN_FRAME_FD (1U << 2U)
#define PLATFORM_CAN_FRAME_BRS (1U << 3U)
#define PLATFORM_CAN_FRAME_ESI (1U << 4U)
#define PLATFORM_CAN_MAX_DATA_LENGTH (64U)

struct platform_can_frame
{
    uint32_t id;
    uint32_t flags;
    uint32_t timestamp;
    uint8_t length;
    uint8_t data[PLATFORM_CAN_MAX_DATA_LENGTH];
};

struct platform_can_status
{
    uint32_t tx_error_count;
    uint32_t rx_error_count;
    uint32_t last_error_code;
    bool is_error_passive;
    bool is_warning;
    bool is_bus_off;
    bool has_rx_overflow;
};

struct platform_can_diagnostics
{
    uint32_t send_count;
    uint32_t receive_count;
    uint32_t no_frame_count;
    uint32_t tx_busy_count;
    uint32_t rx_overflow_count;
    uint32_t io_error_count;
    uint32_t bus_off_observation_count;
};

struct platform_can_ops
{
    platform_status_t (*start)(
        platform_can_controller_t controller);
    platform_status_t (*stop)(
        platform_can_controller_t controller);
    platform_status_t (*send)(
        platform_can_controller_t controller,
        const struct platform_can_frame *frame);
    platform_status_t (*receive)(
        platform_can_controller_t controller,
        struct platform_can_frame *frame);
    platform_status_t (*get_status)(
        platform_can_controller_t controller,
        struct platform_can_status *status);
    platform_status_t (*get_diagnostics)(
        platform_can_controller_t controller,
        struct platform_can_diagnostics *diagnostics);
    platform_status_t (*reset_diagnostics)(
        platform_can_controller_t controller);
};

/* Registers the one active CAN provider. */
platform_status_t platform_can_register(
    const struct platform_can_ops *ops);

/* Enters normal bus operation. */
platform_status_t platform_can_start(
    platform_can_controller_t controller);

/* Enters software-initialization mode. */
platform_status_t platform_can_stop(
    platform_can_controller_t controller);

/* Queues one validated classic CAN or CAN FD frame. */
platform_status_t platform_can_send(
    platform_can_controller_t controller,
    const struct platform_can_frame *frame);

/* Returns PLATFORM_STATUS_TRY_AGAIN when no frame is available. */
platform_status_t platform_can_receive(
    platform_can_controller_t controller,
    struct platform_can_frame *frame);

/* Reads controller error and receive-overflow state. */
platform_status_t platform_can_get_status(
    platform_can_controller_t controller,
    struct platform_can_status *status);
platform_status_t platform_can_get_diagnostics(
    platform_can_controller_t controller,
    struct platform_can_diagnostics *diagnostics);
platform_status_t platform_can_reset_diagnostics(
    platform_can_controller_t controller);

#endif /* PLATFORM_CAN_H */
