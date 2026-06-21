#ifndef PLATFORM_I2C_H
#define PLATFORM_I2C_H

#include "../core/platform_status.h"

#include <stdint.h>

/*
 * I2C is the standard name used by specifications and Linux. A client binds
 * a 7-bit bus address to a provider instance-table index.
 */
typedef uint8_t platform_i2c_bus_t;

#define PLATFORM_I2C_MSG_READ (1U << 0U)

struct platform_i2c_client
{
    platform_i2c_bus_t bus;
    uint16_t address;
};

struct platform_i2c_msg
{
    uint16_t flags;
    uint16_t length;
    uint8_t *data;
};

struct platform_i2c_diagnostics
{
    uint32_t transfer_count;
    uint32_t busy_reject_count;
    uint32_t timeout_count;
    uint32_t io_error_count;
    uint32_t recovery_count;
};

struct platform_i2c_ops
{
    platform_status_t (*transfer)(
        const struct platform_i2c_client *client,
        const struct platform_i2c_msg *messages,
        uint32_t message_count);
    platform_status_t (*get_diagnostics)(
        platform_i2c_bus_t bus,
        struct platform_i2c_diagnostics *diagnostics);
    platform_status_t (*reset_diagnostics)(
        platform_i2c_bus_t bus);
};

/* Registers the one active I2C provider. */
platform_status_t platform_i2c_register(
    const struct platform_i2c_ops *ops);

/*
 * Executes all messages synchronously. Adjacent messages use repeated START
 * and the provider generates STOP after the last message.
 */
platform_status_t platform_i2c_transfer(
    const struct platform_i2c_client *client,
    const struct platform_i2c_msg *messages,
    uint32_t message_count);
platform_status_t platform_i2c_get_diagnostics(
    platform_i2c_bus_t bus,
    struct platform_i2c_diagnostics *diagnostics);
platform_status_t platform_i2c_reset_diagnostics(
    platform_i2c_bus_t bus);

#endif /* PLATFORM_I2C_H */
