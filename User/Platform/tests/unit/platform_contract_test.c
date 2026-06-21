#include "Platform/communications/platform_can.h"
#include "Platform/communications/platform_i2c.h"
#include "Platform/communications/platform_spi.h"
#include "Platform/communications/platform_uart.h"
#include "Platform/gpio/platform_gpio.h"
#include "Platform/timers/platform_clock.h"
#include "Platform/timers/platform_counter.h"
#include "Platform/timers/platform_input_capture.h"
#include "Platform/timers/platform_periodic_timer.h"
#include "Platform/timers/platform_pwm.h"

#include "../fake/platform_fake_gpio.h"
#include "../fake/platform_fake_communications.h"

#include <stdio.h>

static int s_failures;
static uint32_t s_gpio_a_callbacks;
static uint32_t s_gpio_b_callbacks;

#define CHECK(expression)                         \
    do {                                          \
        if (!(expression)) {                      \
            (void)printf(                         \
                "FAIL %s:%d: %s\n",              \
                __FILE__, __LINE__, #expression); \
            s_failures++;                         \
        }                                         \
    } while (0)

static void test_missing_providers(void)
{
    struct platform_can_frame can_frame = {0};
    struct platform_i2c_client i2c_client = {
        .bus = 0U,
        .address = 0x50U,
    };
    uint8_t i2c_data = 0U;
    struct platform_i2c_msg i2c_message = {
        .flags = 0U,
        .length = 1U,
        .data = &i2c_data,
    };
    struct platform_pwm_state pwm_state = {
        .period_ticks = 100U,
        .pulse_ticks = 50U,
        .is_enabled = true,
    };
    struct platform_spi_device device = {
        .bus = 0U,
        .chip_select = 0U,
        .bits_per_word = 8U,
        .mode = PLATFORM_SPI_MODE_0,
        .bit_order = PLATFORM_SPI_MSB_FIRST,
        .max_frequency_hz = 1000000U,
    };
    uint8_t tx = 0U;
    uint32_t count;
    int64_t counter;
    struct platform_spi_transfer transfer = {
        .tx_data = &tx,
        .rx_data = NULL,
        .length = 1U,
    };
    platform_gpio_level_t level;
    struct platform_gpio_diagnostics gpio_diagnostics;
    struct platform_uart_diagnostics uart_diagnostics;
    struct platform_i2c_diagnostics i2c_diagnostics;
    struct platform_can_diagnostics can_diagnostics;

    CHECK(platform_gpio_read(0U, &level) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_clock_read(0U, &count) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_counter_read(0U, &counter) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_pwm_apply(0U, &pwm_state) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_periodic_timer_start(0U) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_uart_write(
              0U, &tx, 1U, &count) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_i2c_transfer(
              &i2c_client, &i2c_message, 1U) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_can_send(0U, &can_frame) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_spi_transfer(
              &device, &transfer, 1U) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_input_capture_start(0U) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_gpio_get_diagnostics(
              0U, &gpio_diagnostics) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_uart_get_diagnostics(
              0U, &uart_diagnostics) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_i2c_get_diagnostics(
              0U, &i2c_diagnostics) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
    CHECK(platform_can_get_diagnostics(
              0U, &can_diagnostics) ==
          PLATFORM_STATUS_NOT_SUPPORTED);
}

static void gpio_callback(
    platform_gpio_pin_t pin,
    void *context)
{
    uint32_t *count = context;

    (void)pin;
    (*count)++;
}

static void test_fake_gpio(void)
{
    platform_gpio_level_t level = PLATFORM_GPIO_LOW;
    struct platform_gpio_diagnostics diagnostics;

    CHECK(platform_fake_gpio_install() ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_irq_enable(0U) ==
          PLATFORM_STATUS_INVALID_STATE);
    CHECK(platform_gpio_write(
              0U, PLATFORM_GPIO_HIGH) ==
          PLATFORM_STATUS_INVALID_STATE);
    CHECK(platform_gpio_set_direction(
              0U, PLATFORM_GPIO_OUTPUT) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_write(
              0U, PLATFORM_GPIO_HIGH) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_read(0U, &level) ==
          PLATFORM_STATUS_OK);
    CHECK(level == PLATFORM_GPIO_HIGH);
    CHECK(platform_gpio_read(
              PLATFORM_FAKE_GPIO_PIN_COUNT,
              &level) ==
          PLATFORM_STATUS_NO_DEVICE);
    CHECK(platform_gpio_read(0U, NULL) ==
          PLATFORM_STATUS_INVALID_ARGUMENT);
    CHECK(platform_gpio_irq_attach(
              0U, PLATFORM_GPIO_IRQ_RISING,
              gpio_callback, &s_gpio_a_callbacks) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_irq_attach(
              32U, PLATFORM_GPIO_IRQ_FALLING,
              gpio_callback, &s_gpio_b_callbacks) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_irq_enable(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_irq_enable(32U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_fake_gpio_trigger(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_fake_gpio_trigger(32U) ==
          PLATFORM_STATUS_OK);
    CHECK(s_gpio_a_callbacks == 1U);
    CHECK(s_gpio_b_callbacks == 1U);
    CHECK(platform_fake_gpio_trigger(1U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_fake_gpio_force_isr_limit(32U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_get_diagnostics(
              0U, &diagnostics) == PLATFORM_STATUS_OK);
    CHECK(diagnostics.irq_dispatch_count == 2U);
    CHECK(diagnostics.irq_unhandled_count == 1U);
    CHECK(platform_gpio_get_diagnostics(
              32U, &diagnostics) == PLATFORM_STATUS_OK);
    CHECK(diagnostics.irq_dispatch_count == 1U);
    CHECK(diagnostics.isr_limit_hit_count == 1U);
    CHECK(platform_gpio_get_diagnostics(
              0U, NULL) ==
          PLATFORM_STATUS_INVALID_ARGUMENT);
    CHECK(platform_gpio_get_diagnostics(
              PLATFORM_FAKE_GPIO_PIN_COUNT,
              &diagnostics) == PLATFORM_STATUS_NO_DEVICE);
    CHECK(platform_gpio_reset_diagnostics(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_gpio_get_diagnostics(
              0U, &diagnostics) == PLATFORM_STATUS_OK);
    CHECK(diagnostics.irq_dispatch_count == 0U);
}

static void test_fake_communication_diagnostics(void)
{
    struct platform_uart_diagnostics uart_diagnostics;
    struct platform_i2c_diagnostics i2c_diagnostics;
    struct platform_can_diagnostics can_diagnostics;
    struct platform_i2c_client client = {
        .bus = 0U,
        .address = 0x50U,
    };
    uint8_t data = 0U;
    struct platform_i2c_msg message = {
        .flags = 0U,
        .length = 1U,
        .data = &data,
    };
    struct platform_can_frame frame = {0};

    CHECK(platform_fake_communications_install() ==
          PLATFORM_STATUS_OK);
    platform_fake_uart_record_rx();
    CHECK(platform_i2c_transfer(
              &client, &message, 1U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_can_send(0U, &frame) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_can_receive(0U, &frame) ==
          PLATFORM_STATUS_TRY_AGAIN);

    CHECK(platform_uart_get_diagnostics(
              0U, &uart_diagnostics) ==
          PLATFORM_STATUS_OK);
    CHECK(uart_diagnostics.rx_byte_count == 1U);
    CHECK(platform_i2c_get_diagnostics(
              0U, &i2c_diagnostics) ==
          PLATFORM_STATUS_OK);
    CHECK(i2c_diagnostics.transfer_count == 1U);
    CHECK(platform_can_get_diagnostics(
              0U, &can_diagnostics) ==
          PLATFORM_STATUS_OK);
    CHECK(can_diagnostics.send_count == 1U);
    CHECK(can_diagnostics.no_frame_count == 1U);

    CHECK(platform_uart_get_diagnostics(
              1U, &uart_diagnostics) ==
          PLATFORM_STATUS_NO_DEVICE);
    CHECK(platform_i2c_get_diagnostics(
              1U, &i2c_diagnostics) ==
          PLATFORM_STATUS_NO_DEVICE);
    CHECK(platform_can_get_diagnostics(
              1U, &can_diagnostics) ==
          PLATFORM_STATUS_NO_DEVICE);
    CHECK(platform_uart_get_diagnostics(0U, NULL) ==
          PLATFORM_STATUS_INVALID_ARGUMENT);
    CHECK(platform_i2c_get_diagnostics(0U, NULL) ==
          PLATFORM_STATUS_INVALID_ARGUMENT);
    CHECK(platform_can_get_diagnostics(0U, NULL) ==
          PLATFORM_STATUS_INVALID_ARGUMENT);

    CHECK(platform_uart_reset_diagnostics(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_i2c_reset_diagnostics(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_can_reset_diagnostics(0U) ==
          PLATFORM_STATUS_OK);
    CHECK(platform_uart_get_diagnostics(
              0U, &uart_diagnostics) ==
          PLATFORM_STATUS_OK);
    CHECK(uart_diagnostics.rx_byte_count == 0U);
}

int main(void)
{
    test_missing_providers();
    test_fake_gpio();
    test_fake_communication_diagnostics();

    if (s_failures != 0) {
        (void)printf("%d test(s) failed\n", s_failures);
        return 1;
    }

    (void)printf("Platform contract tests passed\n");
    return 0;
}
