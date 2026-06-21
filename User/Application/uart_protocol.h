#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/*
 * UART Protocol for Motor/Servo Control
 *
 * Frame format (variable length):
 * ┌────────┬────────┬──────────┬──────────┬────────────────────┬──────────┐
 * │ SYNC1  │ SYNC2  │ Command  │ Data Len │ Data (0 .. N bytes)│ Checksum │
 * │  0xAA  │  0x55  │  (1 B)   │  (1 B)   │                    │  (1 B)   │
 * └────────┴────────┴──────────┴──────────┴────────────────────┴──────────┘
 *
 *   Checksum = XOR of [Command, Data_Len, Data[0] .. Data[N-1]]
 *
 * Commands:
 *   0x01  Set Motor   Data: [Motor_ID(1B)] [Direction(1B)] [Speed(1B)]
 *   0x02  Set Servo   Data: [Angle_Lo(1B)] [Angle_Hi(1B)]  (LE u16, 0.01°)
 *   0x03  Emerg. Stop Data: none
 */

/* --- Protocol constants --- */

#define UART_PROTO_SYNC1 ((uint8_t)0xAA)
#define UART_PROTO_SYNC2 ((uint8_t)0x55)

/* Commands */
#define UART_PROTO_CMD_SET_MOTOR       ((uint8_t)0x01)
#define UART_PROTO_CMD_SET_SERVO       ((uint8_t)0x02)
#define UART_PROTO_CMD_EMERGENCY_STOP  ((uint8_t)0x03)

/* Motor IDs for SET_MOTOR */
#define UART_PROTO_MOTOR_LEFT    ((uint8_t)0)
#define UART_PROTO_MOTOR_RIGHT   ((uint8_t)1)
#define UART_PROTO_MOTOR_BOTH    ((uint8_t)2)

/* Directions for SET_MOTOR */
#define UART_PROTO_MOTOR_COAST   ((uint8_t)0)
#define UART_PROTO_MOTOR_FORWARD ((uint8_t)1)
#define UART_PROTO_MOTOR_REVERSE ((uint8_t)2)
#define UART_PROTO_MOTOR_BRAKE   ((uint8_t)3)

/* Maximum data payload per frame */
#define UART_PROTO_MAX_DATA_LEN  ((uint8_t)8)

/* --- Decoded command --- */

struct uart_proto_command {
    uint8_t cmd;
    uint8_t data_len;
    uint8_t data[UART_PROTO_MAX_DATA_LEN];
};

/* --- Public API --- */

/*
 * Initialise the protocol parser state machine.
 * Must be called once before any uart_protocol_feed_byte().
 */
void uart_protocol_init(void);

/*
 * Feed one received byte into the parser.
 * Safe to call from ISR context (UART RX callback).
 */
void uart_protocol_feed_byte(uint8_t byte);

/*
 * Non-blocking poll for a complete, valid command.
 * Returns true and fills *cmd when one is ready; returns false otherwise.
 * Call from main loop only (not ISR-safe).
 */
bool uart_protocol_poll_command(struct uart_proto_command *cmd);

#endif /* UART_PROTOCOL_H */
