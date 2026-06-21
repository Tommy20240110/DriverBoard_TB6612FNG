#include "platform_can.h"

#include <stddef.h>

static const struct platform_can_ops *s_can_ops;

static bool platform_can_frame_is_valid(
    const struct platform_can_frame *frame)
{
    const uint32_t supported_flags =
        PLATFORM_CAN_FRAME_EXTENDED |
        PLATFORM_CAN_FRAME_REMOTE |
        PLATFORM_CAN_FRAME_FD |
        PLATFORM_CAN_FRAME_BRS |
        PLATFORM_CAN_FRAME_ESI;
    bool is_extended;
    bool is_fd;

    if (!frame ||
        frame->length > PLATFORM_CAN_MAX_DATA_LENGTH ||
        (frame->flags & ~supported_flags) != 0U) {
        return false;
    }

    is_extended =
        (frame->flags & PLATFORM_CAN_FRAME_EXTENDED) != 0U;
    is_fd = (frame->flags & PLATFORM_CAN_FRAME_FD) != 0U;
    if ((!is_extended && frame->id > 0x7FFU) ||
        (is_extended && frame->id > 0x1FFFFFFFU) ||
        (!is_fd && frame->length > 8U) ||
        (is_fd &&
         (frame->flags & PLATFORM_CAN_FRAME_REMOTE) != 0U) ||
        (!is_fd &&
         (frame->flags &
          (PLATFORM_CAN_FRAME_BRS |
           PLATFORM_CAN_FRAME_ESI)) != 0U)) {
        return false;
    }
    return true;
}

platform_status_t platform_can_register(
    const struct platform_can_ops *ops)
{
    if (!ops || !ops->start || !ops->stop ||
        !ops->send || !ops->receive) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if ((ops->get_diagnostics == NULL) !=
        (ops->reset_diagnostics == NULL)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_can_ops && s_can_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_can_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_can_start(
    platform_can_controller_t controller)
{
    if (!s_can_ops || !s_can_ops->start) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_can_ops->start(controller);
}

platform_status_t platform_can_stop(
    platform_can_controller_t controller)
{
    if (!s_can_ops || !s_can_ops->stop) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_can_ops->stop(controller);
}

platform_status_t platform_can_send(
    platform_can_controller_t controller,
    const struct platform_can_frame *frame)
{
    if (!s_can_ops || !s_can_ops->send) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!platform_can_frame_is_valid(frame)) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_can_ops->send(controller, frame);
}

platform_status_t platform_can_receive(
    platform_can_controller_t controller,
    struct platform_can_frame *frame)
{
    if (!s_can_ops || !s_can_ops->receive) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!frame) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_can_ops->receive(controller, frame);
}

platform_status_t platform_can_get_status(
    platform_can_controller_t controller,
    struct platform_can_status *status)
{
    if (!s_can_ops || !s_can_ops->get_status) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!status) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_can_ops->get_status(controller, status);
}

platform_status_t platform_can_get_diagnostics(
    platform_can_controller_t controller,
    struct platform_can_diagnostics *diagnostics)
{
    if (!s_can_ops || !s_can_ops->get_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!diagnostics) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return s_can_ops->get_diagnostics(
        controller, diagnostics);
}

platform_status_t platform_can_reset_diagnostics(
    platform_can_controller_t controller)
{
    if (!s_can_ops || !s_can_ops->reset_diagnostics) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    return s_can_ops->reset_diagnostics(controller);
}
