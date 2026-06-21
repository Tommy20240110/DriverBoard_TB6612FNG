#include "platform_examples.h"

platform_status_t platform_example_can_send(
    platform_can_controller_t controller,
    const struct platform_can_frame *frame)
{
    platform_status_t rc = platform_can_start(controller);

    return rc == PLATFORM_STATUS_OK
               ? platform_can_send(controller, frame)
               : rc;
}

platform_status_t platform_example_can_poll(
    platform_can_controller_t controller,
    struct platform_can_frame *frame)
{
    return platform_can_receive(controller, frame);
}
