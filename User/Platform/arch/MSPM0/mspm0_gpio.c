#include "../../core/platform_irq.h"
#include "../../gpio/platform_gpio.h"

#include "ti_msp_dl_config.h"
#include "mspm0_providers.h"

#include <stddef.h>

#define GPIO_PORT_INDEX(pin) ((uint8_t)((pin) >> 5U))
#define GPIO_PIN_INDEX(pin) ((uint8_t)((pin) & 0x1FU))
#define GPIO_PIN_MASK(pin) (1UL << GPIO_PIN_INDEX(pin))
#define GPIO_PIN_COUNT (32U)
#define GPIO_POLARITY_BITS_PER_PIN (2U)
#define GPIO_POLARITY_FIELD_MASK (3UL)
#define MSPM0_GPIO_ISR_EVENT_LIMIT (32U)

struct mspm0_gpio_irq_entry
{
    platform_gpio_irq_callback_t volatile callback;
    void *volatile context;
};

struct mspm0_gpio_port
{
    GPIO_Regs *gpio;
    IRQn_Type irq;
    uint8_t index;
    struct platform_gpio_diagnostics diagnostics;
};

static struct mspm0_gpio_port s_gpio_ports[] = {
    {
        .gpio = GPIOA,
        .irq = GPIOA_INT_IRQn,
        .index = 0U,
    },
    {
        .gpio = GPIOB,
        .irq = GPIOB_INT_IRQn,
        .index = 1U,
    },
};

#define GPIO_PORT_COUNT \
    ((uint8_t)(sizeof(s_gpio_ports) / sizeof(s_gpio_ports[0])))

static struct mspm0_gpio_irq_entry
    s_gpio_irq[GPIO_PORT_COUNT][GPIO_PIN_COUNT];

static struct mspm0_gpio_port *mspm0_gpio_get_port(
    platform_gpio_pin_t pin)
{
    uint8_t port_index = GPIO_PORT_INDEX(pin);

    if (port_index >= GPIO_PORT_COUNT) {
        return NULL;
    }
    return &s_gpio_ports[port_index];
}

static platform_status_t mspm0_gpio_set_direction(
    platform_gpio_pin_t pin,
    platform_gpio_direction_t direction)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    uint32_t mask = GPIO_PIN_MASK(pin);

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    switch (direction) {
    case PLATFORM_GPIO_INPUT:
        DL_GPIO_disableOutput(port->gpio, mask);
        return PLATFORM_STATUS_OK;
    case PLATFORM_GPIO_OUTPUT:
        DL_GPIO_enableOutput(port->gpio, mask);
        return PLATFORM_STATUS_OK;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
}

static platform_status_t mspm0_gpio_write(
    platform_gpio_pin_t pin,
    platform_gpio_level_t level)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    uint32_t mask = GPIO_PIN_MASK(pin);

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    switch (level) {
    case PLATFORM_GPIO_LOW:
        DL_GPIO_clearPins(port->gpio, mask);
        return PLATFORM_STATUS_OK;
    case PLATFORM_GPIO_HIGH:
        DL_GPIO_setPins(port->gpio, mask);
        return PLATFORM_STATUS_OK;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }
}

static platform_status_t mspm0_gpio_read(
    platform_gpio_pin_t pin,
    platform_gpio_level_t *level)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    uint32_t mask = GPIO_PIN_MASK(pin);

    if (!port || !level) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    *level = (DL_GPIO_readPins(port->gpio, mask) != 0U)
                 ? PLATFORM_GPIO_HIGH
                 : PLATFORM_GPIO_LOW;
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_gpio_set_irq_trigger(
    GPIO_Regs *port,
    uint8_t pin_index,
    platform_gpio_irq_trigger_t trigger)
{
    uint32_t trigger_value;
    uint32_t shift =
        (uint32_t)(pin_index % 16U) *
        GPIO_POLARITY_BITS_PER_PIN;
    uint32_t mask = GPIO_POLARITY_FIELD_MASK << shift;
    volatile uint32_t *polarity;

    switch (trigger) {
    case PLATFORM_GPIO_IRQ_RISING:
        trigger_value = 1U;
        break;
    case PLATFORM_GPIO_IRQ_FALLING:
        trigger_value = 2U;
        break;
    case PLATFORM_GPIO_IRQ_BOTH:
        trigger_value = 3U;
        break;
    default:
        return PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    polarity = (pin_index < 16U)
                   ? &port->POLARITY15_0
                   : &port->POLARITY31_16;
    *polarity = (*polarity & ~mask) |
                (trigger_value << shift);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_gpio_irq_attach(
    platform_gpio_pin_t pin,
    platform_gpio_irq_trigger_t trigger,
    platform_gpio_irq_callback_t callback,
    void *context)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    uint8_t port_index = GPIO_PORT_INDEX(pin);
    uint8_t pin_index = GPIO_PIN_INDEX(pin);
    platform_irq_state_t state;
    platform_status_t rc;

    if (!port || !callback) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_ARGUMENT;
    }

    DL_GPIO_disableInterrupt(port->gpio, GPIO_PIN_MASK(pin));
    rc = mspm0_gpio_set_irq_trigger(
        port->gpio, pin_index, trigger);
    if (rc != 0) {
        return rc;
    }

    state = platform_irq_save();
    s_gpio_irq[port_index][pin_index].callback =
        callback;
    s_gpio_irq[port_index][pin_index].context =
        context;
    platform_irq_restore(state);
    DL_GPIO_clearInterruptStatus(
        port->gpio, GPIO_PIN_MASK(pin));
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_gpio_irq_enable(
    platform_gpio_pin_t pin)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    uint8_t port_index = GPIO_PORT_INDEX(pin);
    uint8_t pin_index = GPIO_PIN_INDEX(pin);

    if (!port ||
        !s_gpio_irq[port_index][pin_index].callback) {
        return !port
                   ? PLATFORM_STATUS_NO_DEVICE
                   : PLATFORM_STATUS_INVALID_STATE;
    }

    DL_GPIO_clearInterruptStatus(
        port->gpio, GPIO_PIN_MASK(pin));
    DL_GPIO_enableInterrupt(port->gpio, GPIO_PIN_MASK(pin));
    NVIC_EnableIRQ(port->irq);
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_gpio_irq_disable(
    platform_gpio_pin_t pin)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    DL_GPIO_disableInterrupt(port->gpio, GPIO_PIN_MASK(pin));
    DL_GPIO_clearInterruptStatus(
        port->gpio, GPIO_PIN_MASK(pin));
    return PLATFORM_STATUS_OK;
}

static platform_status_t mspm0_gpio_irq_detach(
    platform_gpio_pin_t pin)
{
    uint8_t port_index = GPIO_PORT_INDEX(pin);
    uint8_t pin_index = GPIO_PIN_INDEX(pin);
    platform_irq_state_t state;
    platform_status_t rc;

    if (!mspm0_gpio_get_port(pin)) {
        return PLATFORM_STATUS_NO_DEVICE;
    }

    rc = mspm0_gpio_irq_disable(pin);
    if (rc != 0) {
        return rc;
    }

    state = platform_irq_save();
    s_gpio_irq[port_index][pin_index].callback = NULL;
    s_gpio_irq[port_index][pin_index].context = NULL;
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

static void mspm0_gpio_dispatch(
    struct mspm0_gpio_port *port)
{
    DL_GPIO_IIDX index;
    uint32_t event_count = 0U;

    while (event_count < MSPM0_GPIO_ISR_EVENT_LIMIT &&
           (index = DL_GPIO_getPendingInterrupt(port->gpio)) !=
               DL_GPIO_IIDX_NO_INTR) {
        uint8_t pin_index = (uint8_t)index - 1U;
        struct mspm0_gpio_irq_entry *entry;
        platform_gpio_irq_callback_t callback;
        void *context;

        event_count++;
        port->diagnostics.irq_dispatch_count++;

        if (pin_index >= GPIO_PIN_COUNT) {
            DL_GPIO_clearInterruptStatus(
                port->gpio, UINT32_MAX);
            port->diagnostics.irq_unhandled_count++;
            continue;
        }

        entry = &s_gpio_irq[port->index][pin_index];
        callback = entry->callback;
        context = entry->context;
        DL_GPIO_clearInterruptStatus(
            port->gpio, 1UL << pin_index);
        if (callback) {
            callback(
                (platform_gpio_pin_t)(
                    (port->index << 5U) |
                    pin_index),
                context);
        } else {
            port->diagnostics.irq_unhandled_count++;
        }
    }
    if (event_count == MSPM0_GPIO_ISR_EVENT_LIMIT) {
        port->diagnostics.isr_limit_hit_count++;
    }
}

static platform_status_t mspm0_gpio_get_diagnostics(
    platform_gpio_pin_t pin,
    struct platform_gpio_diagnostics *diagnostics)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
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

static platform_status_t mspm0_gpio_reset_diagnostics(
    platform_gpio_pin_t pin)
{
    struct mspm0_gpio_port *port = mspm0_gpio_get_port(pin);
    platform_irq_state_t state;

    if (!port) {
        return PLATFORM_STATUS_NO_DEVICE;
    }
    state = platform_irq_save();
    port->diagnostics =
        (struct platform_gpio_diagnostics){0};
    platform_irq_restore(state);
    return PLATFORM_STATUS_OK;
}

void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(
        DL_INTERRUPT_GROUP_1)) {
    case DL_INTERRUPT_GROUP1_IIDX_GPIOA:
        mspm0_gpio_dispatch(&s_gpio_ports[0]);
        break;
    case DL_INTERRUPT_GROUP1_IIDX_GPIOB:
        mspm0_gpio_dispatch(&s_gpio_ports[1]);
        break;
    default:
        break;
    }
}

static const struct platform_gpio_ops s_mspm0_gpio_ops = {
    .set_direction = mspm0_gpio_set_direction,
    .write = mspm0_gpio_write,
    .read = mspm0_gpio_read,
    .irq_attach = mspm0_gpio_irq_attach,
    .irq_enable = mspm0_gpio_irq_enable,
    .irq_disable = mspm0_gpio_irq_disable,
    .irq_detach = mspm0_gpio_irq_detach,
    .get_diagnostics = mspm0_gpio_get_diagnostics,
    .reset_diagnostics = mspm0_gpio_reset_diagnostics,
};

platform_status_t mspm0_gpio_provider_init(void)
{
    return platform_gpio_register(&s_mspm0_gpio_ops);
}
