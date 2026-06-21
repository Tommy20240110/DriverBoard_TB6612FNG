#include "platform_spi.h"

#include <stddef.h>

static const struct platform_spi_ops *s_spi_ops;

platform_status_t platform_spi_register(
    const struct platform_spi_ops *ops)
{
    if (!ops || !ops->transfer) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (s_spi_ops && s_spi_ops != ops) {
        return PLATFORM_STATUS_ALREADY_REGISTERED;
    }

    s_spi_ops = ops;
    return PLATFORM_STATUS_OK;
}

platform_status_t platform_spi_transfer(
    const struct platform_spi_device *device,
    const struct platform_spi_transfer *transfers,
    uint32_t transfer_count)
{
    uint32_t index;

    if (!s_spi_ops || !s_spi_ops->transfer) {
        return PLATFORM_STATUS_NOT_SUPPORTED;
    }
    if (!device || !transfers || transfer_count == 0U ||
        device->bits_per_word == 0U ||
        device->mode > PLATFORM_SPI_MODE_3 ||
        device->bit_order > PLATFORM_SPI_LSB_FIRST ||
        device->max_frequency_hz == 0U) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    for (index = 0U; index < transfer_count; index++) {
        if (transfers[index].length == 0U ||
            (!transfers[index].tx_data &&
             !transfers[index].rx_data)) {
            return PLATFORM_STATUS_INVALID_ARGUMENT;
        }
    }

    return s_spi_ops->transfer(
        device, transfers, transfer_count);
}
