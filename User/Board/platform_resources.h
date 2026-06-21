#ifndef BOARD_PLATFORM_RESOURCES_H
#define BOARD_PLATFORM_RESOURCES_H

#include "Platform/communications/platform_can.h"
#include "Platform/communications/platform_i2c.h"
#include "Platform/communications/platform_uart.h"
#include "Platform/gpio/platform_gpio.h"
#include "Platform/timers/platform_clock.h"
#include "Platform/timers/platform_periodic_timer.h"
#include "Platform/timers/platform_pwm.h"

/*
 * These enums are purpose-based indices into the static maps defined by
 * platform_instances.c. They are not MCU peripheral numbers.
 */
typedef enum board_gpio_resource
{
    BOARD_GPIO_ENCODER_LEFT_PULSE = 0,
    BOARD_GPIO_ENCODER_LEFT_DIRECTION,
    BOARD_GPIO_ENCODER_RIGHT_PULSE,
    BOARD_GPIO_ENCODER_RIGHT_DIRECTION,
    BOARD_GPIO_TB6612_AIN1,
    BOARD_GPIO_TB6612_AIN2,
    BOARD_GPIO_TB6612_BIN1,
    BOARD_GPIO_TB6612_BIN2,
    BOARD_GPIO_TB6612_STBY,
    BOARD_GPIO_COUNT,
} board_gpio_resource_t;

typedef enum board_pwm_resource
{
    BOARD_PWM_MOTOR_LEFT = 0,
    BOARD_PWM_MOTOR_RIGHT,
    BOARD_PWM_STEERING_SERVO,
    BOARD_PWM_COUNT,
} board_pwm_resource_t;

typedef enum board_clock_resource
{
    BOARD_CLOCK_TIMESTAMP = 0,
    BOARD_CLOCK_COUNT,
} board_clock_resource_t;

typedef enum board_periodic_timer_resource
{
    BOARD_TIMER_ENCODER_SAMPLE = 0,
    BOARD_PERIODIC_TIMER_COUNT,
} board_periodic_timer_resource_t;

typedef enum board_uart_resource
{
    BOARD_UART_DEBUG = 0,
    BOARD_UART_COUNT,
} board_uart_resource_t;

typedef enum board_i2c_resource
{
    BOARD_I2C_EXTERNAL = 0,
    BOARD_I2C_COUNT,
} board_i2c_resource_t;

typedef enum board_can_resource
{
    BOARD_CAN_MAIN = 0,
    BOARD_CAN_COUNT,
} board_can_resource_t;

extern const platform_gpio_pin_t
    g_board_gpio_instances[BOARD_GPIO_COUNT];
extern const platform_pwm_channel_t
    g_board_pwm_instances[BOARD_PWM_COUNT];
extern const platform_clock_id_t
    g_board_clock_instances[BOARD_CLOCK_COUNT];
extern const platform_periodic_timer_id_t
    g_board_periodic_timer_instances[
        BOARD_PERIODIC_TIMER_COUNT];
extern const platform_uart_port_t
    g_board_uart_instances[BOARD_UART_COUNT];
extern const platform_i2c_bus_t
    g_board_i2c_instances[BOARD_I2C_COUNT];
extern const platform_can_controller_t
    g_board_can_instances[BOARD_CAN_COUNT];

#endif /* BOARD_PLATFORM_RESOURCES_H */
