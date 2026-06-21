/*
 * UART Motor & Servo Control Demo
 *
 * Listens on the debug UART for host commands, parses a binary protocol,
 * and drives two TB6612FNG BDC motors plus one PWM servo.
 *
 * Hardware bootstrap (SYSCFG_DL_init) is the only TI-specific call;
 * everything else uses the Platform abstraction layer or Device-level
 * motor/servo drivers built on it.
 *
 * Protocol: see uart_protocol.h for the binary frame format.
 *
 * Wiring: host TX → PA18 (MCU RX), host RX ← PA17 (MCU TX), common GND.
 * Default baud rate: 115200 8N1.
 */

#include "Platform/communications/platform_uart.h"
#include "Platform/platform_init.h"

#include "Board/motors.h"
#include "Board/platform_resources.h"

#include "Application/uart_protocol.h"

#include "ti_msp_dl_config.h" /* SYSCFG_DL_init() — HW bootstrap only */

/* --------------------------------------------------------------------------
 * Motor / servo constants
 * -------------------------------------------------------------------------- */

/*
 * Motor PWM runs at 10 MHz tick rate (TIMA0).  period_ticks = 10000 gives
 * a 1 kHz PWM frequency — same as the existing motor_board_init default.
 */
#define MOTOR_PERIOD_TICKS  ((uint32_t)10000)
#define MOTOR_SPEED_MAX     ((uint8_t)100)

/* Centre angle for the steering servo (135° in the original demo). */
#define SERVO_CENTER_DEG ((q16_16_t)INT_TO_Q16_16(135))

/* Debug UART configuration. */
#define DEMO_UART_BAUD ((uint32_t)115200)

/* --------------------------------------------------------------------------
 * UART RX callback — invoked per byte in ISR context
 * -------------------------------------------------------------------------- */

static void demo_uart_rx_callback(platform_uart_port_t port,
                                  platform_uart_event_t event,
                                  uint8_t data,
                                  void *context)
{
    (void) port;
    (void) context;

    if (event == PLATFORM_UART_EVENT_RX_DATA) {
        uart_protocol_feed_byte(data);
    }
    /* Error events (OVERRUN, FRAMING, …) are silently ignored; the
     * protocol parser resyncs on the next SYNC1/SYNC2 pair. */
}

/* --------------------------------------------------------------------------
 * Command executors
 * -------------------------------------------------------------------------- */

static int demo_exec_set_motor(const struct uart_proto_command *cmd)
{
    uint8_t motor_id;
    uint8_t dir;
    uint8_t speed_pct;
    motor_bdc_mode_t mode;
    uint32_t pulse_ticks;
    int rc;

    if (cmd->data_len < 3U) {
        return -1;
    }

    motor_id  = cmd->data[0];
    dir       = cmd->data[1];
    speed_pct = cmd->data[2];

    /* Map protocol direction to motor_bdc_mode. */
    switch (dir) {
    case UART_PROTO_MOTOR_COAST:   mode = MOTOR_BDC_COAST;   break;
    case UART_PROTO_MOTOR_FORWARD: mode = MOTOR_BDC_FORWARD; break;
    case UART_PROTO_MOTOR_REVERSE: mode = MOTOR_BDC_REVERSE; break;
    case UART_PROTO_MOTOR_BRAKE:   mode = MOTOR_BDC_BRAKE;   break;
    default:                       return -1;
    }

    /* Map percentage 0–100 to PWM pulse ticks. */
    if (speed_pct > MOTOR_SPEED_MAX) {
        speed_pct = MOTOR_SPEED_MAX;
    }
    pulse_ticks =
        ((uint32_t) speed_pct * MOTOR_PERIOD_TICKS) /
        ((uint32_t) MOTOR_SPEED_MAX);

    const struct motor_bdc_output output = {
        .mode         = mode,
        .period_ticks = MOTOR_PERIOD_TICKS,
        .pulse_ticks  = pulse_ticks,
    };

    /* Apply to the selected motor(s). */
    switch (motor_id) {
    case UART_PROTO_MOTOR_LEFT:
        rc = motor_bdc_enable(g_motor_left);
        if (rc != 0) { return rc; }
        return motor_bdc_apply_output(g_motor_left, &output);

    case UART_PROTO_MOTOR_RIGHT:
        rc = motor_bdc_enable(g_motor_right);
        if (rc != 0) { return rc; }
        return motor_bdc_apply_output(g_motor_right, &output);

    case UART_PROTO_MOTOR_BOTH: {
        rc = motor_bdc_enable(g_motor_left);
        if (rc != 0) { return rc; }
        rc = motor_bdc_enable(g_motor_right);
        if (rc != 0) {
            (void) motor_bdc_disable(g_motor_left);
            return rc;
        }
        rc = motor_bdc_apply_output(g_motor_left, &output);
        if (rc != 0) { return rc; }
        return motor_bdc_apply_output(g_motor_right, &output);
    }
    default:
        return -1;
    }
}

static int demo_exec_set_servo(const struct uart_proto_command *cmd)
{
    uint16_t angle_raw;
    q16_16_t angle;

    if (cmd->data_len < 2U) {
        return -1;
    }

    /* Little-endian u16: hundredths of a degree (0 .. 27000). */
    angle_raw  = (uint16_t) cmd->data[0];
    angle_raw |= (uint16_t) cmd->data[1] << 8U;

    /*
     * Clamp to the servo's configured MAX_ANGLE (270° = 27000 centidegrees).
     * motor_servo_pwm_set_angle applies its own saturation internally, but
     * we clamp early to avoid wrap-around in the Q16.16 conversion.
     */
    if (angle_raw > 27000U) {
        angle_raw = 27000U;
    }

    /* Convert centidegrees → Q16.16.  angle_raw * 65536 / 100, keeping
     * enough precision for the integer→fixed conversion. */
    angle = (q16_16_t)(
        ((int64_t) angle_raw * INT64_C(65536)) / INT64_C(100));

    return motor_servo_pwm_set_angle(g_motor_steering, angle);
}

static int demo_exec_emergency_stop(void)
{
    const struct motor_bdc_output coast = {
        .mode         = MOTOR_BDC_COAST,
        .period_ticks = MOTOR_PERIOD_TICKS,
        .pulse_ticks  = 0U,
    };
    int rc;
    int ret = 0;

    /* Coast both motors. */
    rc = motor_bdc_apply_output(g_motor_left, &coast);
    if (rc != 0) { ret = rc; }
    rc = motor_bdc_apply_output(g_motor_right, &coast);
    if (rc != 0) { ret = rc; }
    rc = motor_bdc_disable(g_motor_left);
    if (rc != 0) { ret = rc; }
    rc = motor_bdc_disable(g_motor_right);
    if (rc != 0) { ret = rc; }

    /* Centre the steering servo. */
    rc = motor_servo_pwm_set_angle(g_motor_steering, SERVO_CENTER_DEG);
    if (rc != 0) { ret = rc; }

    return ret;
}

/* --------------------------------------------------------------------------
 * Command dispatch
 * -------------------------------------------------------------------------- */

static int demo_dispatch(const struct uart_proto_command *cmd)
{
    switch (cmd->cmd) {
    case UART_PROTO_CMD_SET_MOTOR:
        return demo_exec_set_motor(cmd);
    case UART_PROTO_CMD_SET_SERVO:
        return demo_exec_set_servo(cmd);
    case UART_PROTO_CMD_EMERGENCY_STOP:
        return demo_exec_emergency_stop();
    default:
        /* Unknown command — silently ignored. */
        return 0;
    }
}

/* --------------------------------------------------------------------------
 * Initialization
 * -------------------------------------------------------------------------- */

static int demo_init_uart(void)
{
    const struct platform_uart_config config = {
        .baud_rate = DEMO_UART_BAUD,
        .data_bits = 8U,
        .parity    = PLATFORM_UART_PARITY_NONE,
        .stop_bits = PLATFORM_UART_STOP_BITS_ONE,
    };
    platform_uart_port_t uart_port;
    platform_status_t rc;

    uart_port = g_board_uart_instances[BOARD_UART_DEBUG];

    rc = platform_uart_configure(uart_port, &config);
    if (rc != PLATFORM_STATUS_OK) {
        return (int) rc;
    }

    rc = platform_uart_attach(
        uart_port, demo_uart_rx_callback, NULL);
    if (rc != PLATFORM_STATUS_OK) {
        return (int) rc;
    }

    return 0;
}

/* --------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */

int main(void)
{
    struct uart_proto_command cmd;
    int rc;

    /* ---- Hardware bootstrap (TI SysConfig — unavoidable) ---- */
    SYSCFG_DL_init();

    /* ---- Platform & board ---- */
    rc = (int) platform_init();
    if (rc != 0) {
        return -1;
    }

    rc = motor_board_init();
    if (rc != 0) {
        return -1;
    }

    /* ---- Enable servo (it starts at centre angle) ---- */
    rc = motor_servo_pwm_enable(g_motor_steering);
    if (rc != 0) {
        return -1;
    }
    rc = (int) motor_servo_pwm_set_angle(
        g_motor_steering, SERVO_CENTER_DEG);
    if (rc != 0) {
        return -1;
    }

    /* ---- Initialise protocol parser & UART ---- */
    uart_protocol_init();

    rc = demo_init_uart();
    if (rc != 0) {
        return -1;
    }

    /* ---- Main loop: poll & dispatch ---- */
    for (;;) {
        if (uart_protocol_poll_command(&cmd)) {
            (void) demo_dispatch(&cmd);
        }
        __WFI();
    }
}
