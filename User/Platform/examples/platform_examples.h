#ifndef PLATFORM_EXAMPLES_H
#define PLATFORM_EXAMPLES_H

#include "../communications/platform_can.h"
#include "../communications/platform_i2c.h"
#include "../communications/platform_uart.h"
#include "../gpio/platform_gpio.h"
#include "../timers/platform_clock.h"
#include "../timers/platform_periodic_timer.h"
#include "../timers/platform_pwm.h"

struct platform_gpio_example
{
    platform_gpio_pin_t output_pin;
    platform_gpio_pin_t input_pin;
    volatile uint32_t edge_count;
};

struct platform_uart_echo_example
{
    platform_uart_port_t port;
    volatile uint8_t byte;
    volatile bool byte_pending;
};

struct platform_diagnostics_snapshot
{
    struct platform_gpio_diagnostics gpio;
    struct platform_uart_diagnostics uart;
    struct platform_i2c_diagnostics i2c;
    struct platform_can_diagnostics can;
};

platform_status_t platform_example_gpio_start(
    struct platform_gpio_example *example);
platform_status_t platform_example_gpio_stop(
    struct platform_gpio_example *example);
platform_status_t platform_example_timer_configure(
    platform_clock_id_t clock,
    platform_periodic_timer_id_t timer,
    platform_pwm_channel_t pwm,
    uint32_t period_ticks,
    uint32_t pulse_ticks,
    platform_periodic_timer_callback_t callback,
    void *context);
platform_status_t platform_example_uart_echo_start(
    struct platform_uart_echo_example *example,
    const struct platform_uart_config *config);
platform_status_t platform_example_uart_echo_poll(
    struct platform_uart_echo_example *example);
platform_status_t platform_example_i2c_read_register(
    platform_i2c_bus_t bus,
    uint16_t address,
    uint8_t register_address,
    uint8_t *data,
    uint16_t length);
platform_status_t platform_example_can_send(
    platform_can_controller_t controller,
    const struct platform_can_frame *frame);
platform_status_t platform_example_can_poll(
    platform_can_controller_t controller,
    struct platform_can_frame *frame);
platform_status_t platform_example_get_diagnostics(
    platform_gpio_pin_t gpio_pin,
    platform_uart_port_t uart,
    platform_i2c_bus_t i2c,
    platform_can_controller_t can,
    struct platform_diagnostics_snapshot *snapshot);

#endif /* PLATFORM_EXAMPLES_H */
