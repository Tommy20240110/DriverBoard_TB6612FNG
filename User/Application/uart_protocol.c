#include "uart_protocol.h"

#include "Platform/core/platform_irq.h"

#include <string.h>

/* --- Parser state machine --- */

typedef enum {
    PARSER_STATE_IDLE,
    PARSER_STATE_GOT_SYNC1,
    PARSER_STATE_GOT_SYNC2,
    PARSER_STATE_GOT_CMD,
    PARSER_STATE_GOT_LEN,
    PARSER_STATE_COLLECTING_DATA,
} parser_state_t;

/* --- Static variables --- */

/* Parser state — only ever touched by the ISR (single UART, no nesting). */
static parser_state_t s_state;
static uint8_t s_cmd;
static uint8_t s_data_len;
static uint8_t s_data_index;
static uint8_t s_data_buf[UART_PROTO_MAX_DATA_LEN];
static uint8_t s_running_checksum;

/*
 * Hand-off buffer: ISR writes on valid frame, main loop reads.
 * Protected by irq_save / irq_restore.
 */
static struct uart_proto_command s_pending_cmd;
static volatile bool s_cmd_ready;

/* --- Helper: compute XOR checksum from a buffer --- */

static uint8_t uart_proto_compute_checksum(uint8_t cmd,
                                           uint8_t data_len,
                                           const uint8_t *data)
{
    uint8_t cksum = cmd;
    uint8_t i;

    cksum ^= data_len;
    for (i = 0U; i < data_len; i++) {
        cksum ^= data[i];
    }
    return cksum;
}

/* --- Helper: reset parser to idle --- */

static void uart_proto_reset(void)
{
    s_state      = PARSER_STATE_IDLE;
    s_data_index = 0U;
}

/* --- Helper: deliver a validated command to the hand-off buffer --- */

static void uart_proto_deliver(void)
{
    platform_irq_state_t irq_state;

    irq_state = platform_irq_save();
    s_pending_cmd.cmd      = s_cmd;
    s_pending_cmd.data_len = s_data_len;
    (void) memcpy(s_pending_cmd.data, s_data_buf, s_data_len);
    s_cmd_ready = true;
    platform_irq_restore(irq_state);
}

/* --- Public API --- */

void uart_protocol_init(void)
{
    uart_proto_reset();
    s_cmd_ready = false;
    (void) memset(&s_pending_cmd, 0, sizeof(s_pending_cmd));
}

/*
 * Per-byte state machine.
 *
 * Called from the UART RX ISR callback.  The callback processes one byte
 * at a time (one ISR invocation per byte), but the ISR is non-nesting on
 * Cortex-M0+, so the parser state is implicitly single-threaded.
 */
void uart_protocol_feed_byte(uint8_t byte)
{
    switch (s_state) {

    case PARSER_STATE_IDLE:
        if (byte == UART_PROTO_SYNC1) {
            s_state = PARSER_STATE_GOT_SYNC1;
        }
        /* else: stay idle, discard byte */
        break;

    case PARSER_STATE_GOT_SYNC1:
        if (byte == UART_PROTO_SYNC2) {
            s_state = PARSER_STATE_GOT_SYNC2;
        } else if (byte != UART_PROTO_SYNC1) {
            /*
             * Not a valid continuation.  If it's another SYNC1, stay in
             * GOT_SYNC1 (allows multiple AA bytes).  Otherwise reset.
             */
            s_state = PARSER_STATE_IDLE;
        }
        break;

    case PARSER_STATE_GOT_SYNC2:
        /*
         * After dual sync, accept any byte as the command.
         * Resync protection: if the byte happens to be SYNC1, treat it
         * as a real command value (0xAA) and stay; a host embedding AA in
         * a command byte is unusual but legal.
         */
        s_cmd              = byte;
        s_running_checksum = byte;
        s_state            = PARSER_STATE_GOT_CMD;
        break;

    case PARSER_STATE_GOT_CMD:
        s_data_len         = byte;
        s_running_checksum ^= byte;
        if (s_data_len == 0U) {
            /*
             * Zero-length payload — skip directly to checksum byte.
             * The next byte must be the checksum.
             */
            s_state = PARSER_STATE_GOT_LEN; /* wait for checksum */
        } else if (s_data_len <= UART_PROTO_MAX_DATA_LEN) {
            s_data_index = 0U;
            s_state      = PARSER_STATE_COLLECTING_DATA;
        } else {
            /* Payload too long — frame corrupted or unsupported. */
            uart_proto_reset();
        }
        break;

    case PARSER_STATE_GOT_LEN:
        /*
         * This state is reached when data_len == 0.
         * The arriving byte is the checksum.
         */
        {
            uint8_t expected = uart_proto_compute_checksum(
                s_cmd, s_data_len, s_data_buf);

            if (s_running_checksum == expected) {
                uart_proto_deliver();
            }
            uart_proto_reset();
        }
        break;

    case PARSER_STATE_COLLECTING_DATA:
        s_data_buf[s_data_index] = byte;
        s_running_checksum ^= byte;
        s_data_index++;

        if (s_data_index >= s_data_len) {
            /*
             * All payload bytes received.  The next byte is the checksum.
             * Re-use GOT_LEN state.
             */
            s_state = PARSER_STATE_GOT_LEN;
        }
        break;

    default:
        uart_proto_reset();
        break;
    }
}

bool uart_protocol_poll_command(struct uart_proto_command *cmd)
{
    platform_irq_state_t irq_state;
    bool ready;

    if (!cmd) {
        return false;
    }

    irq_state = platform_irq_save();
    ready     = s_cmd_ready;
    if (ready) {
        *cmd        = s_pending_cmd;
        s_cmd_ready = false;
    }
    platform_irq_restore(irq_state);

    return ready;
}
