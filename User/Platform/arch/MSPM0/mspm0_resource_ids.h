#ifndef MSPM0_RESOURCE_IDS_H
#define MSPM0_RESOURCE_IDS_H

#include "../../gpio/platform_gpio.h"

#define MSPM0_PLATFORM_GPIO_PORT_A_INDEX (0U)
#define MSPM0_PLATFORM_GPIO_PORT_B_INDEX (1U)

#define MSPM0_PLATFORM_GPIO_ID(port_index, pin_index) \
    ((platform_gpio_pin_t)(                            \
        ((uint8_t)(port_index) << 5U) |               \
        ((uint8_t)(pin_index) & 0x1FU)))

#endif /* MSPM0_RESOURCE_IDS_H */
