#ifndef PLATFORM_INPUT_CAPTURE_H
#define PLATFORM_INPUT_CAPTURE_H

#include "../core/platform_status.h"

#include <stdint.h>

/*
 * Input capture reports raw timer ticks and edges. Calls return
 * PLATFORM_STATUS_NOT_SUPPORTED while no provider is registered.
 */
typedef uint8_t platform_input_capture_channel_t;

typedef enum platform_input_capture_edge
{
    PLATFORM_INPUT_CAPTURE_RISING = 0,
    PLATFORM_INPUT_CAPTURE_FALLING,
    PLATFORM_INPUT_CAPTURE_BOTH,
} platform_input_capture_edge_t;

struct platform_input_capture_sample
{
    uint32_t ticks;
    platform_input_capture_edge_t edge;
};

typedef void (*platform_input_capture_callback_t)(
    platform_input_capture_channel_t channel,
    const struct platform_input_capture_sample *sample,
    void *context);

struct platform_input_capture_ops
{
    platform_status_t (*configure)(
        platform_input_capture_channel_t channel,
        platform_input_capture_edge_t edge);
    platform_status_t (*start)(
        platform_input_capture_channel_t channel);
    platform_status_t (*stop)(
        platform_input_capture_channel_t channel);
    platform_status_t (*read)(
        platform_input_capture_channel_t channel,
        struct platform_input_capture_sample *sample);
    platform_status_t (*attach)(
        platform_input_capture_channel_t channel,
        platform_input_capture_callback_t callback,
        void *context);
    platform_status_t (*detach)(
        platform_input_capture_channel_t channel);
    platform_status_t (*get_tick_hz)(
        platform_input_capture_channel_t channel,
        uint32_t *tick_hz);
};

/* Registers the one active input-capture provider. */
platform_status_t platform_input_capture_register(
    const struct platform_input_capture_ops *ops);

/* Selects the edge or edges captured by a channel. */
platform_status_t platform_input_capture_configure(
    platform_input_capture_channel_t channel,
    platform_input_capture_edge_t edge);

/* Enables capture on a channel. */
platform_status_t platform_input_capture_start(
    platform_input_capture_channel_t channel);

/* Disables capture on a channel. */
platform_status_t platform_input_capture_stop(
    platform_input_capture_channel_t channel);

/* Polls the latest provider-defined capture sample. */
platform_status_t platform_input_capture_read(
    platform_input_capture_channel_t channel,
    struct platform_input_capture_sample *sample);

/* Attaches one callback which executes in interrupt context. */
platform_status_t platform_input_capture_attach(
    platform_input_capture_channel_t channel,
    platform_input_capture_callback_t callback,
    void *context);

/* Disables events and removes the callback. */
platform_status_t platform_input_capture_detach(
    platform_input_capture_channel_t channel);

/* Returns the raw capture timer frequency in ticks per second. */
platform_status_t platform_input_capture_get_tick_hz(
    platform_input_capture_channel_t channel,
    uint32_t *tick_hz);

#endif /* PLATFORM_INPUT_CAPTURE_H */
