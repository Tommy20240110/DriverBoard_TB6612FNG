#include "../../communications/platform_i2c.h"
#include "../../core/platform_irq.h"

#include "mspm0_internal.h"
#include "mspm0_providers.h"
#include "ti_msp_dl_config.h"

#include <stdbool.h>
#include <stddef.h>

#define MSPM0_I2C_MAX_TRANSFER_LENGTH (4095U)
#define MSPM0_I2C_TIMER_PERIOD_400KHZ (9U)
#define MSPM0_I2C_TIMEOUT_TICKS (8000000U)
#define MSPM0_I2C_START_DELAY_CYCLES (6U)

struct mspm0_i2c_bus
{
    I2C_Regs *i2c;
    bool is_busy;
    struct platform_i2c_diagnostics diagnostics;
};

static struct mspm0_i2c_bus s_i2c_buses[] = {
    {
        .i2c = I2C_DRIVERBOARD_INST,
        .is_busy = false,
    },
};

static struct mspm0_i2c_bus *mspm0_i2c_get_bus(
    platform_i2c_bus_t bus)
{
    if (bus >= MSPM0_ARRAY_COUNT(s_i2c_buses)) {
        return NULL;
    }
    return &s_i2c_buses[bus];
}

static bool mspm0_i2c_timeout_expired(uint32_t start)
{
    uint32_t now = DL_Timer_getTimerCount(TIMG12);

    return (uint32_t)(now - start) >=
           MSPM0_I2C_TIMEOUT_TICKS;
}

static platform_status_t mspm0_i2c_acquire(
    struct mspm0_i2c_bus *bus)
{
    platform_irq_state_t state;

    state = platform_irq_save();
    if (bus->is_busy) {
        bus->diagnostics.busy_reject_count++;
        platform_irq_restore(state);
        return PLATFORM_STATUS_BUSY;
    }
    bus->is_busy = true;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static void mspm0_i2c_release(
    struct mspm0_i2c_bus *bus)
{
    platform_irq_state_t state;

    state = platform_irq_save();
    bus->is_busy = false;
    platform_irq_restore(state);
}

static platform_status_t mspm0_i2c_wait_idle(
    struct mspm0_i2c_bus *bus)
{
    uint32_t start = DL_Timer_getTimerCount(TIMG12);

    while ((DL_I2C_getControllerStatus(bus->i2c) &
            DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (mspm0_i2c_timeout_expired(start)) {
            return PLATFORM_STATUS_TIMEOUT;
        }
    }
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_i2c_check_status(
    struct mspm0_i2c_bus *bus)
{
    uint32_t status =
        DL_I2C_getControllerStatus(bus->i2c);

    if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U ||
        (status &
         DL_I2C_CONTROLLER_STATUS_ARBITRATION_LOST) != 0U) {
        return PLATFORM_STATUS_IO_ERROR;
    }
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_i2c_flush_tx_fifo(
    struct mspm0_i2c_bus *bus)
{
    uint32_t start = DL_Timer_getTimerCount(TIMG12);

    DL_I2C_startFlushControllerTXFIFO(bus->i2c);
    while (!DL_I2C_isControllerTXFIFOEmpty(bus->i2c)) {
        if (mspm0_i2c_timeout_expired(start)) {
            DL_I2C_stopFlushControllerTXFIFO(bus->i2c);
            return PLATFORM_STATUS_TIMEOUT;
        }
    }
    DL_I2C_stopFlushControllerTXFIFO(bus->i2c);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_i2c_flush_rx_fifo(
    struct mspm0_i2c_bus *bus)
{
    uint32_t start = DL_Timer_getTimerCount(TIMG12);

    DL_I2C_startFlushControllerRXFIFO(bus->i2c);
    while (!DL_I2C_isControllerRXFIFOEmpty(bus->i2c)) {
        if (mspm0_i2c_timeout_expired(start)) {
            DL_I2C_stopFlushControllerRXFIFO(bus->i2c);
            return PLATFORM_STATUS_TIMEOUT;
        }
    }
    DL_I2C_stopFlushControllerRXFIFO(bus->i2c);
    return PLATFORM_STATUS_OK;
}

static void mspm0_i2c_recover(
    struct mspm0_i2c_bus *bus)
{
    bus->diagnostics.recovery_count++;
    DL_I2C_resetControllerTransfer(bus->i2c);
    (void)mspm0_i2c_flush_tx_fifo(bus);
    (void)mspm0_i2c_flush_rx_fifo(bus);
}

static platform_status_t mspm0_i2c_wait_message_done(
    struct mspm0_i2c_bus *bus,
    uint32_t start)
{
    platform_status_t rc;

    while ((DL_I2C_getControllerStatus(bus->i2c) &
            DL_I2C_CONTROLLER_STATUS_BUSY) != 0U) {
        rc = mspm0_i2c_check_status(bus);
        if (rc != 0) {
            return rc;
        }
        if (mspm0_i2c_timeout_expired(start)) {
            return PLATFORM_STATUS_TIMEOUT;
        }
    }
    return mspm0_i2c_check_status(bus);
}

static platform_status_t mspm0_i2c_write_message(
    struct mspm0_i2c_bus *bus,
    uint16_t address,
    const struct platform_i2c_msg *message,
    DL_I2C_CONTROLLER_STOP stop)
{
    uint32_t start = DL_Timer_getTimerCount(TIMG12);
    uint16_t offset;
    platform_status_t rc;

    rc = mspm0_i2c_flush_tx_fifo(bus);
    if (rc != 0) {
        return rc;
    }
    offset = DL_I2C_fillControllerTXFIFO(
        bus->i2c, message->data, message->length);
    DL_I2C_startControllerTransferAdvanced(
        bus->i2c,
        address,
        DL_I2C_CONTROLLER_DIRECTION_TX,
        message->length,
        DL_I2C_CONTROLLER_START_ENABLE,
        stop,
        DL_I2C_CONTROLLER_ACK_DISABLE);
    delay_cycles(MSPM0_I2C_START_DELAY_CYCLES);

    while (offset < message->length) {
        while (offset < message->length &&
               !DL_I2C_isControllerTXFIFOFull(bus->i2c)) {
            DL_I2C_transmitControllerData(
                bus->i2c, message->data[offset]);
            offset++;
        }
        rc = mspm0_i2c_check_status(bus);
        if (rc != 0) {
            return rc;
        }
        if (mspm0_i2c_timeout_expired(start)) {
            return PLATFORM_STATUS_TIMEOUT;
        }
    }

    return mspm0_i2c_wait_message_done(bus, start);
}

static platform_status_t mspm0_i2c_read_message(
    struct mspm0_i2c_bus *bus,
    uint16_t address,
    const struct platform_i2c_msg *message,
    DL_I2C_CONTROLLER_STOP stop)
{
    uint32_t start = DL_Timer_getTimerCount(TIMG12);
    uint16_t offset = 0U;
    platform_status_t rc;

    rc = mspm0_i2c_flush_rx_fifo(bus);
    if (rc != 0) {
        return rc;
    }
    DL_I2C_startControllerTransferAdvanced(
        bus->i2c,
        address,
        DL_I2C_CONTROLLER_DIRECTION_RX,
        message->length,
        DL_I2C_CONTROLLER_START_ENABLE,
        stop,
        DL_I2C_CONTROLLER_ACK_DISABLE);
    delay_cycles(MSPM0_I2C_START_DELAY_CYCLES);

    while (offset < message->length) {
        while (offset < message->length &&
               !DL_I2C_isControllerRXFIFOEmpty(bus->i2c)) {
            message->data[offset] =
                DL_I2C_receiveControllerData(bus->i2c);
            offset++;
        }
        rc = mspm0_i2c_check_status(bus);
        if (rc != 0) {
            return rc;
        }
        if (mspm0_i2c_timeout_expired(start)) {
            return PLATFORM_STATUS_TIMEOUT;
        }
    }

    return mspm0_i2c_wait_message_done(bus, start);
}

static platform_status_t mspm0_i2c_transfer(
    const struct platform_i2c_client *client,
    const struct platform_i2c_msg *messages,
    uint32_t message_count)
{
    struct mspm0_i2c_bus *bus;
    uint32_t index;
    platform_status_t rc;

    bus = mspm0_i2c_get_bus(client->bus);
    if (!bus) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    rc = mspm0_i2c_acquire(bus);
    if (rc != 0) {
        return rc;
    }
    bus->diagnostics.transfer_count++;
    rc = mspm0_i2c_wait_idle(bus);

    for (index = 0U;
         rc == 0 && index < message_count;
         index++) {
        DL_I2C_CONTROLLER_STOP stop =
            index == (message_count - 1U)
                ? DL_I2C_CONTROLLER_STOP_ENABLE
                : DL_I2C_CONTROLLER_STOP_DISABLE;

        if (messages[index].length >
            MSPM0_I2C_MAX_TRANSFER_LENGTH) {
            rc = PLATFORM_STATUS_INVALID_ARGUMENT;
        } else if ((messages[index].flags &
                    PLATFORM_I2C_MSG_READ) != 0U) {
            rc = mspm0_i2c_read_message(
                bus,
                client->address,
                &messages[index],
                stop);
        } else {
            rc = mspm0_i2c_write_message(
                bus,
                client->address,
                &messages[index],
                stop);
        }
    }

    if (rc != 0) {
        if (rc == PLATFORM_STATUS_TIMEOUT) {
            bus->diagnostics.timeout_count++;
        } else if (rc == PLATFORM_STATUS_IO_ERROR) {
            bus->diagnostics.io_error_count++;
        }
        mspm0_i2c_recover(bus);
    }
    mspm0_i2c_release(bus);
    return rc;
}

static platform_status_t mspm0_i2c_get_diagnostics(
    platform_i2c_bus_t bus_id,
    struct platform_i2c_diagnostics *diagnostics)
{
    struct mspm0_i2c_bus *bus =
        mspm0_i2c_get_bus(bus_id);
    platform_irq_state_t state;

    if (!bus || !diagnostics) {
        return !bus
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    state = platform_irq_save();
    *diagnostics = bus->diagnostics;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_i2c_reset_diagnostics(
    platform_i2c_bus_t bus_id)
{
    struct mspm0_i2c_bus *bus =
        mspm0_i2c_get_bus(bus_id);
    platform_irq_state_t state;

    if (!bus) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    state = platform_irq_save();
    bus->diagnostics =
        (struct platform_i2c_diagnostics){0};
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static const struct platform_i2c_ops s_mspm0_i2c_ops = {
    .transfer = mspm0_i2c_transfer,
    .get_diagnostics = mspm0_i2c_get_diagnostics,
    .reset_diagnostics = mspm0_i2c_reset_diagnostics,
};

platform_status_t mspm0_i2c_provider_init(void)
{
    struct mspm0_i2c_bus *bus = &s_i2c_buses[0];
    platform_status_t rc;

    NVIC_DisableIRQ(I2C_DRIVERBOARD_INST_INT_IRQN);
    DL_I2C_disableInterrupt(bus->i2c, UINT32_MAX);
    DL_I2C_clearInterruptStatus(bus->i2c, UINT32_MAX);
    DL_I2C_disableController(bus->i2c);
    DL_I2C_resetControllerTransfer(bus->i2c);
    DL_I2C_setTimerPeriod(
        bus->i2c, MSPM0_I2C_TIMER_PERIOD_400KHZ);
    DL_I2C_setControllerTXFIFOThreshold(
        bus->i2c, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(
        bus->i2c, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(bus->i2c);
    rc = mspm0_i2c_flush_tx_fifo(bus);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_i2c_flush_rx_fifo(bus);
    if (rc != 0) {
        return rc;
    }
    DL_I2C_enableController(bus->i2c);
    NVIC_ClearPendingIRQ(I2C_DRIVERBOARD_INST_INT_IRQN);

    return platform_i2c_register(&s_mspm0_i2c_ops);
}
