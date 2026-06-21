#include "../../communications/platform_uart.h"
#include "../../core/platform_irq.h"

#include "mspm0_internal.h"
#include "mspm0_providers.h"
#include "ti_msp_dl_config.h"

#include <stddef.h>

#define MSPM0_UART_OVERSAMPLING (16U)
#define MSPM0_UART_FRACTION_SCALE (64U)
#define MSPM0_UART_ISR_EVENT_LIMIT (32U)
#define MSPM0_UART_ISR_RX_LIMIT (16U)

#define MSPM0_UART_ERROR_INTERRUPTS \
    (DL_UART_INTERRUPT_OVERRUN_ERROR | \
     DL_UART_INTERRUPT_BREAK_ERROR | \
     DL_UART_INTERRUPT_PARITY_ERROR | \
     DL_UART_INTERRUPT_FRAMING_ERROR | \
     DL_UART_INTERRUPT_NOISE_ERROR)

#define MSPM0_UART_INTERRUPTS \
    (DL_UART_INTERRUPT_RX | MSPM0_UART_ERROR_INTERRUPTS)

struct mspm0_uart_port
{
    UART_Regs *uart;
    IRQn_Type irq;
    platform_uart_port_t id;
    uint32_t clock_hz;
    platform_uart_callback_t volatile callback;
    void *volatile context;
    struct platform_uart_diagnostics diagnostics;
};

static struct mspm0_uart_port s_uart_ports[] = {
    {
        .uart = UART_DRIVERBOARD_INST,
        .irq = UART_DRIVERBOARD_INST_INT_IRQN,
        .id = 0U,
        .clock_hz = UART_DRIVERBOARD_INST_FREQUENCY,
        .callback = NULL,
        .context = NULL,
    },
};

static struct mspm0_uart_port *mspm0_uart_get_port(
    platform_uart_port_t port)
{
    if (port >= MSPM0_ARRAY_COUNT(s_uart_ports)) {
        return NULL;
    }
    return &s_uart_ports[port];
}

static platform_status_t mspm0_uart_get_word_length(
    uint8_t data_bits,
    DL_UART_WORD_LENGTH *word_length)
{
    if (!word_length) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    switch (data_bits) {
    case 5U:
        *word_length = DL_UART_WORD_LENGTH_5_BITS;
        return PLATFORM_STATUS_OK;
    case 6U:
        *word_length = DL_UART_WORD_LENGTH_6_BITS;
        return PLATFORM_STATUS_OK;
    case 7U:
        *word_length = DL_UART_WORD_LENGTH_7_BITS;
        return PLATFORM_STATUS_OK;
    case 8U:
        *word_length = DL_UART_WORD_LENGTH_8_BITS;
        return PLATFORM_STATUS_OK;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
}

static platform_status_t mspm0_uart_get_parity(
    platform_uart_parity_t parity,
    DL_UART_PARITY *hardware_parity)
{
    if (!hardware_parity) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    switch (parity) {
    case PLATFORM_UART_PARITY_NONE:
        *hardware_parity = DL_UART_PARITY_NONE;
        return PLATFORM_STATUS_OK;
    case PLATFORM_UART_PARITY_EVEN:
        *hardware_parity = DL_UART_PARITY_EVEN;
        return PLATFORM_STATUS_OK;
    case PLATFORM_UART_PARITY_ODD:
        *hardware_parity = DL_UART_PARITY_ODD;
        return PLATFORM_STATUS_OK;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
}

static platform_status_t mspm0_uart_get_stop_bits(
    platform_uart_stop_bits_t stop_bits,
    DL_UART_STOP_BITS *hardware_stop_bits)
{
    if (!hardware_stop_bits) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    switch (stop_bits) {
    case PLATFORM_UART_STOP_BITS_ONE:
        *hardware_stop_bits = DL_UART_STOP_BITS_ONE;
        return PLATFORM_STATUS_OK;
    case PLATFORM_UART_STOP_BITS_TWO:
        *hardware_stop_bits = DL_UART_STOP_BITS_TWO;
        return PLATFORM_STATUS_OK;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
}

static platform_status_t mspm0_uart_calculate_baud_rate(
    const struct mspm0_uart_port *port,
    uint32_t baud_rate,
    uint32_t *integer_divisor,
    uint32_t *fractional_divisor)
{
    uint64_t denominator;
    uint64_t scaled_divisor;

    denominator =
        (uint64_t)MSPM0_UART_OVERSAMPLING * baud_rate;
    scaled_divisor =
        ((uint64_t)port->clock_hz *
         MSPM0_UART_FRACTION_SCALE +
         (denominator / 2U)) /
        denominator;
    *integer_divisor =
        (uint32_t)(scaled_divisor /
                   MSPM0_UART_FRACTION_SCALE);
    *fractional_divisor =
        (uint32_t)(scaled_divisor %
                   MSPM0_UART_FRACTION_SCALE);
    if (*integer_divisor == 0U ||
        *integer_divisor > UINT16_MAX) {
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_configure(
    platform_uart_port_t port_id,
    const struct platform_uart_config *config)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);
    DL_UART_WORD_LENGTH word_length;
    DL_UART_STOP_BITS stop_bits;
    DL_UART_PARITY parity;
    uint32_t integer_divisor;
    uint32_t fractional_divisor;
    platform_status_t rc;

    if (!port || !config) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    rc = mspm0_uart_get_word_length(
        config->data_bits, &word_length);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_uart_get_parity(
        config->parity, &parity);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_uart_get_stop_bits(
        config->stop_bits, &stop_bits);
    if (rc != 0) {
        return rc;
    }
    rc = mspm0_uart_calculate_baud_rate(
        port,
        config->baud_rate,
        &integer_divisor,
        &fractional_divisor);
    if (rc != 0) {
        return rc;
    }

    NVIC_DisableIRQ(port->irq);
    DL_UART_Main_disable(port->uart);
    DL_UART_Main_setWordLength(port->uart, word_length);
    DL_UART_Main_setParityMode(port->uart, parity);
    DL_UART_Main_setStopBits(port->uart, stop_bits);
    DL_UART_Main_setOversampling(
        port->uart, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(
        port->uart,
        integer_divisor,
        fractional_divisor);
    DL_UART_Main_enable(port->uart);
    if (port->callback) {
        NVIC_ClearPendingIRQ(port->irq);
        NVIC_EnableIRQ(port->irq);
    }
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_write(
    platform_uart_port_t port_id,
    const uint8_t *data,
    uint32_t length,
    uint32_t *written)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);

    if (!port || !data || !written) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    *written = DL_UART_Main_fillTXFIFO(
        port->uart, data, length);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_read(
    platform_uart_port_t port_id,
    uint8_t *data,
    uint32_t capacity,
    uint32_t *received)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);

    if (!port || !data || !received) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    *received = DL_UART_Main_drainRXFIFO(
        port->uart, data, capacity);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_attach(
    platform_uart_port_t port_id,
    platform_uart_callback_t callback,
    void *context)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);

    if (!port || !callback) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    if (port->callback && port->callback != callback) {
        return PLATFORM_STATUS_BUSY;
    }

    NVIC_DisableIRQ(port->irq);
    port->callback = callback;
    port->context = context;
    DL_UART_Main_clearInterruptStatus(
        port->uart, UINT32_MAX);
    DL_UART_Main_enableInterrupt(
        port->uart, MSPM0_UART_INTERRUPTS);
    NVIC_ClearPendingIRQ(port->irq);
    NVIC_EnableIRQ(port->irq);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_detach(
    platform_uart_port_t port_id)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    NVIC_DisableIRQ(port->irq);
    DL_UART_Main_disableInterrupt(
        port->uart, MSPM0_UART_INTERRUPTS);
    DL_UART_Main_clearInterruptStatus(
        port->uart, UINT32_MAX);
    NVIC_ClearPendingIRQ(port->irq);
    port->callback = NULL;
    port->context = NULL;
    return PLATFORM_STATUS_OK;
}

static void mspm0_uart_notify(
    struct mspm0_uart_port *port,
    platform_uart_event_t event,
    uint8_t data)
{
    platform_uart_callback_t callback = port->callback;
    void *context = port->context;

    if (callback) {
        callback(port->id, event, data, context);
    }
}

static platform_status_t mspm0_uart_get_diagnostics(
    platform_uart_port_t port_id,
    struct platform_uart_diagnostics *diagnostics)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);
    platform_irq_state_t state;

    if (!port || !diagnostics) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }
    state = platform_irq_save();
    *diagnostics = port->diagnostics;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_uart_reset_diagnostics(
    platform_uart_port_t port_id)
{
    struct mspm0_uart_port *port =
        mspm0_uart_get_port(port_id);
    platform_irq_state_t state;

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    state = platform_irq_save();
    port->diagnostics =
        (struct platform_uart_diagnostics){0};
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

void UART1_IRQHandler(void)
{
    struct mspm0_uart_port *port = &s_uart_ports[0];
    DL_UART_IIDX index;
    uint32_t event_count = 0U;
    uint8_t data;

    while (event_count < MSPM0_UART_ISR_EVENT_LIMIT &&
           (index = DL_UART_Main_getPendingInterrupt(
                port->uart)) != DL_UART_IIDX_NO_INTERRUPT) {
        event_count++;
        switch (index) {
        case DL_UART_IIDX_RX: {
            uint32_t received = 0U;

            while (received < MSPM0_UART_ISR_RX_LIMIT &&
                   DL_UART_Main_receiveDataCheck(
                       port->uart, &data)) {
                mspm0_uart_notify(
                    port,
                    PLATFORM_UART_EVENT_RX_DATA,
                    data);
                port->diagnostics.rx_byte_count++;
                received++;
            }
            break;
        }
        case DL_UART_IIDX_OVERRUN_ERROR:
            port->diagnostics.overrun_error_count++;
            mspm0_uart_notify(
                port, PLATFORM_UART_EVENT_OVERRUN, 0U);
            break;
        case DL_UART_IIDX_BREAK_ERROR:
            port->diagnostics.break_error_count++;
            mspm0_uart_notify(
                port, PLATFORM_UART_EVENT_BREAK, 0U);
            break;
        case DL_UART_IIDX_PARITY_ERROR:
            port->diagnostics.parity_error_count++;
            mspm0_uart_notify(
                port, PLATFORM_UART_EVENT_PARITY, 0U);
            break;
        case DL_UART_IIDX_FRAMING_ERROR:
            port->diagnostics.framing_error_count++;
            mspm0_uart_notify(
                port, PLATFORM_UART_EVENT_FRAMING, 0U);
            break;
        case DL_UART_IIDX_NOISE_ERROR:
            port->diagnostics.noise_error_count++;
            mspm0_uart_notify(
                port, PLATFORM_UART_EVENT_NOISE, 0U);
            break;
        default:
            break;
        }
    }
    if (event_count == MSPM0_UART_ISR_EVENT_LIMIT) {
        port->diagnostics.isr_limit_hit_count++;
    }
}

static const struct platform_uart_ops s_mspm0_uart_ops = {
    .configure = mspm0_uart_configure,
    .write = mspm0_uart_write,
    .read = mspm0_uart_read,
    .attach = mspm0_uart_attach,
    .detach = mspm0_uart_detach,
    .get_diagnostics = mspm0_uart_get_diagnostics,
    .reset_diagnostics = mspm0_uart_reset_diagnostics,
};

platform_status_t mspm0_uart_provider_init(void)
{
    struct mspm0_uart_port *port = &s_uart_ports[0];

    NVIC_DisableIRQ(port->irq);
    DL_UART_Main_disableInterrupt(
        port->uart, UINT32_MAX);
    DL_UART_Main_clearInterruptStatus(
        port->uart, UINT32_MAX);
    DL_UART_Main_enableFIFOs(port->uart);
    DL_UART_Main_setRXFIFOThreshold(
        port->uart, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(
        port->uart, DL_UART_TX_FIFO_LEVEL_ONE_ENTRY);
    NVIC_ClearPendingIRQ(port->irq);

    return platform_uart_register(&s_mspm0_uart_ops);
}
