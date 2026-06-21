#ifndef PLATFORM_SPI_H
#define PLATFORM_SPI_H

#include "../core/platform_status.h"

#include <stdint.h>

/*
 * SPI is contract-only until a hardware provider is registered. Calls return
 * PLATFORM_STATUS_NOT_SUPPORTED while no provider exists.
 */
typedef uint8_t platform_spi_bus_t;

typedef enum platform_spi_mode
{
    PLATFORM_SPI_MODE_0 = 0,
    PLATFORM_SPI_MODE_1,
    PLATFORM_SPI_MODE_2,
    PLATFORM_SPI_MODE_3,
} platform_spi_mode_t;

typedef enum platform_spi_bit_order
{
    PLATFORM_SPI_MSB_FIRST = 0,
    PLATFORM_SPI_LSB_FIRST,
} platform_spi_bit_order_t;

struct platform_spi_device
{
    platform_spi_bus_t bus;
    uint8_t chip_select;
    uint8_t bits_per_word;
    platform_spi_mode_t mode;
    platform_spi_bit_order_t bit_order;
    uint32_t max_frequency_hz;
};

struct platform_spi_transfer
{
    const uint8_t *tx_data;
    uint8_t *rx_data;
    uint32_t length;
};

struct platform_spi_ops
{
    platform_status_t (*transfer)(
        const struct platform_spi_device *device,
        const struct platform_spi_transfer *transfers,
        uint32_t transfer_count);
};

/* Registers the one active SPI provider. */
platform_status_t platform_spi_register(
    const struct platform_spi_ops *ops);

/* Executes the transfer list while keeping the device selected. */
platform_status_t platform_spi_transfer(
    const struct platform_spi_device *device,
    const struct platform_spi_transfer *transfers,
    uint32_t transfer_count);

#endif /* PLATFORM_SPI_H */
