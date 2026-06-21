#include "motors.h"
#include "platform_resources.h"

#include "Device/encoder/encoder_incremental.h"
#include "Device/interface/tb6612fng.h"
#include "Device/motor/motor_servo_pwm.h"
#include "Device/motor/motor_tb6612fng.h"

static struct tb6612fng s_motor_driver;

static struct motor_tb6612fng s_motor_left;
static struct motor_tb6612fng s_motor_right;
static const struct motor_tb6612fng_config
    s_motor_left_config = {
        .driver = &s_motor_driver,
        .channel = TB6612FNG_CHANNEL_A,
        .initial_period_ticks = 10000U,
    };
static const struct motor_tb6612fng_config
    s_motor_right_config = {
        .driver = &s_motor_driver,
        .channel = TB6612FNG_CHANNEL_B,
        .initial_period_ticks = 10000U,
    };

static struct motor_servo_pwm s_motor_steering;
static const struct motor_servo_pwm_config
    s_motor_steering_config = {
        .MAX_ANGLE = INT_TO_Q16_16(270),
        .MAX_ANGLE_INV =
            Q16_16_INV(INT_TO_Q16_16(270)),
        .SERVO_PULSE_MAX_US = INT_TO_Q16_16(2500),
        .SERVO_PULSE_MIN_US = INT_TO_Q16_16(500),
        .SERVO_PERIOD_US = INT_TO_Q16_16(20000),
    };

static struct encoder_incremental s_encoder_left;
static struct encoder_incremental s_encoder_right;
static const struct encoder_incremental_config
    s_encoder_config = {
        .PPR = 13U,
        .MULTIPLIER = 2U,
        .WHEEL_CIRC = INT_TO_Q16_16(0),
        .SAMPLE_PERIOD_TICKS = 10000U,
    };

struct motor_bdc *g_motor_left;
struct motor_bdc *g_motor_right;
struct motor_servo_pwm *g_motor_steering;
struct encoder_base *g_encoder_left;
struct encoder_base *g_encoder_right;

int motor_board_init(void)
{
    int rc;

    s_motor_driver.channel[TB6612FNG_CHANNEL_A] =
        (struct tb6612fng_channel_config){
            .in1_pin =
                g_board_gpio_instances[
                    BOARD_GPIO_TB6612_AIN1],
            .in2_pin =
                g_board_gpio_instances[
                    BOARD_GPIO_TB6612_AIN2],
            .pwm_channel =
                g_board_pwm_instances[
                    BOARD_PWM_MOTOR_LEFT],
        };
    s_motor_driver.channel[TB6612FNG_CHANNEL_B] =
        (struct tb6612fng_channel_config){
            .in1_pin =
                g_board_gpio_instances[
                    BOARD_GPIO_TB6612_BIN1],
            .in2_pin =
                g_board_gpio_instances[
                    BOARD_GPIO_TB6612_BIN2],
            .pwm_channel =
                g_board_pwm_instances[
                    BOARD_PWM_MOTOR_RIGHT],
    };
    s_motor_driver.stby_pin =
        g_board_gpio_instances[BOARD_GPIO_TB6612_STBY];
    s_motor_driver.enabled_channels = 0U;

    rc = tb6612fng_set_stby(&s_motor_driver, false);
    if (rc != 0) {
        return rc;
    }

    rc = motor_tb6612fng_init(
        &s_motor_left, "motor_left", &s_motor_left_config);
    if (rc != 0) {
        return rc;
    }

    rc = motor_tb6612fng_init(
        &s_motor_right,
        "motor_right",
        &s_motor_right_config);
    if (rc != 0) {
        return rc;
    }

    rc = motor_servo_pwm_init(
        &s_motor_steering,
        "motor_steering",
        g_board_pwm_instances[
            BOARD_PWM_STEERING_SERVO],
        &s_motor_steering_config);
    if (rc != 0) {
        return rc;
    }

    rc = encoder_incremental_init(
        &s_encoder_left,
        "encoder_left",
        g_board_gpio_instances[
            BOARD_GPIO_ENCODER_LEFT_PULSE],
        g_board_gpio_instances[
            BOARD_GPIO_ENCODER_LEFT_DIRECTION],
        g_board_clock_instances[
            BOARD_CLOCK_TIMESTAMP],
        g_board_periodic_timer_instances[
            BOARD_TIMER_ENCODER_SAMPLE],
        &s_encoder_config);
    if (rc != 0) {
        return rc;
    }

    rc = encoder_incremental_init(
        &s_encoder_right,
        "encoder_right",
        g_board_gpio_instances[
            BOARD_GPIO_ENCODER_RIGHT_PULSE],
        g_board_gpio_instances[
            BOARD_GPIO_ENCODER_RIGHT_DIRECTION],
        g_board_clock_instances[
            BOARD_CLOCK_TIMESTAMP],
        g_board_periodic_timer_instances[
            BOARD_TIMER_ENCODER_SAMPLE],
        &s_encoder_config);
    if (rc != 0) {
        return rc;
    }

    g_motor_left = &s_motor_left.bdc;
    g_motor_right = &s_motor_right.bdc;
    g_motor_steering = &s_motor_steering;
    g_encoder_left = &s_encoder_left.base;
    g_encoder_right = &s_encoder_right.base;

    return 0;
}
