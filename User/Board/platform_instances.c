#include "platform_resources.h"

#include "Platform/arch/MSPM0/mspm0_resource_ids.h"

const platform_gpio_pin_t
    g_board_gpio_instances[BOARD_GPIO_COUNT] = {
        [BOARD_GPIO_ENCODER_LEFT_PULSE] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 28U),
        [BOARD_GPIO_ENCODER_LEFT_DIRECTION] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 8U),
        [BOARD_GPIO_ENCODER_RIGHT_PULSE] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 31U),
        [BOARD_GPIO_ENCODER_RIGHT_DIRECTION] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 9U),
        [BOARD_GPIO_TB6612_AIN1] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 21U),
        [BOARD_GPIO_TB6612_AIN2] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 22U),
        [BOARD_GPIO_TB6612_BIN1] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 23U),
        [BOARD_GPIO_TB6612_BIN2] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 24U),
        [BOARD_GPIO_TB6612_STBY] =
            MSPM0_PLATFORM_GPIO_ID(
                MSPM0_PLATFORM_GPIO_PORT_A_INDEX, 25U),
    };

const platform_pwm_channel_t
    g_board_pwm_instances[BOARD_PWM_COUNT] = {
        [BOARD_PWM_MOTOR_LEFT] = 0U,
        [BOARD_PWM_MOTOR_RIGHT] = 1U,
        [BOARD_PWM_STEERING_SERVO] = 2U,
    };

const platform_clock_id_t
    g_board_clock_instances[BOARD_CLOCK_COUNT] = {
        [BOARD_CLOCK_TIMESTAMP] = 0U,
    };

const platform_periodic_timer_id_t
    g_board_periodic_timer_instances[
        BOARD_PERIODIC_TIMER_COUNT] = {
            [BOARD_TIMER_ENCODER_SAMPLE] = 0U,
        };

const platform_uart_port_t
    g_board_uart_instances[BOARD_UART_COUNT] = {
        [BOARD_UART_DEBUG] = 0U,
    };

const platform_i2c_bus_t
    g_board_i2c_instances[BOARD_I2C_COUNT] = {
        [BOARD_I2C_EXTERNAL] = 0U,
    };

const platform_can_controller_t
    g_board_can_instances[BOARD_CAN_COUNT] = {
        [BOARD_CAN_MAIN] = 0U,
    };
