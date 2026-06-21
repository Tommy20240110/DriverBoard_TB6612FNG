#include "platform_examples.h"

#include <stddef.h>

platform_status_t platform_example_i2c_read_register(
    platform_i2c_bus_t bus,
    uint16_t address,
    uint8_t register_address,
    uint8_t *data,
    uint16_t length)
{
    struct platform_i2c_client client = {
        .bus = bus,
        .address = address,
    };
    struct platform_i2c_msg messages[2] = {
        {
            .flags = 0U,
            .length = 1U,
            .data = &register_address,
        },
        {
            .flags = PLATFORM_I2C_MSG_READ,
            .length = length,
            .data = data,
        },
    };

    if (!data || length == 0U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return platform_i2c_transfer(&client, messages, 2U);
}
