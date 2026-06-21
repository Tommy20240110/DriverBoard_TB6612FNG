#include "platform_input_capture.h"

#include <stddef.h>

static const struct platform_input_capture_ops *s_capture_ops;

platform_status_t platform_input_capture_register(
    const struct platform_input_capture_ops *ops)
{
    if (!ops || !ops->configure || !ops->start ||
        !ops->stop || !ops->read || !ops->get_tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if ((ops->attach == NULL) != (ops->detach == NULL)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_capture_ops && s_capture_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_capture_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_input_capture_configure(
    platform_input_capture_channel_t channel,
    platform_input_capture_edge_t edge)
{
    if (!s_capture_ops || !s_capture_ops->configure) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (edge > PLATFORM_INPUT_CAPTURE_BOTH) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_capture_ops->configure(channel, edge);
}

platform_status_t platform_input_capture_start(
    platform_input_capture_channel_t channel)
{
    if (!s_capture_ops || !s_capture_ops->start) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_capture_ops->start(channel);
}

platform_status_t platform_input_capture_stop(
    platform_input_capture_channel_t channel)
{
    if (!s_capture_ops || !s_capture_ops->stop) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_capture_ops->stop(channel);
}

platform_status_t platform_input_capture_read(
    platform_input_capture_channel_t channel,
    struct platform_input_capture_sample *sample)
{
    if (!s_capture_ops || !s_capture_ops->read) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!sample) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_capture_ops->read(channel, sample);
}

platform_status_t platform_input_capture_attach(
    platform_input_capture_channel_t channel,
    platform_input_capture_callback_t callback,
    void *context)
{
    if (!s_capture_ops || !s_capture_ops->attach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!callback) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_capture_ops->attach(
        channel, callback, context);
}

platform_status_t platform_input_capture_detach(
    platform_input_capture_channel_t channel)
{
    if (!s_capture_ops || !s_capture_ops->detach) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_capture_ops->detach(channel);
}

platform_status_t platform_input_capture_get_tick_hz(
    platform_input_capture_channel_t channel,
    uint32_t *tick_hz)
{
    if (!s_capture_ops || !s_capture_ops->get_tick_hz) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!tick_hz) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_capture_ops->get_tick_hz(channel, tick_hz);
}
