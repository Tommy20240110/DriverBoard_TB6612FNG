# Platform layer

The Platform layer is the MCU-facing contract used by Device code. It is
inspired by the Linux driver model, but intentionally keeps only the pieces
that are useful in a statically linked bare-metal firmware.

## Dependency direction

```text
Application
    |
Board        static device enumeration and resource assignment
    |
Device       motor, encoder, TB6612FNG, sensors
    |
Platform     GPIO, timers and communication contracts
    |
arch/MSPM0   TI DriverLib provider
```

Device code must not include TI headers or use timer instance names. Board code
assigns opaque Platform IDs to Device instances. The MSPM0 provider is the only
code that knows which ID maps to a concrete MSPM0 peripheral instance.

This follows two Linux ideas:

- board or firmware code enumerates devices and resources independently from
  the driver;
- consumers use subsystem APIs such as GPIO, PWM and Counter instead of one
  generic "platform timer" API.

It does not copy Linux hotplug, sysfs, reference counting, dynamic allocation
or automatic driver matching because this firmware has one fixed board image.

## Directory layout

```text
Platform/
├── core/            status and IRQ critical sections
├── gpio/            digital I/O and optional edge interrupts
├── timers/          clock, counter, capture, PWM and periodic timers
├── communications/ UART, I2C, SPI and CAN
├── analog/          reserved for future ADC and DAC contracts
├── arch/MSPM0/      TI DriverLib providers and static instance tables
├── tests/           host-side fake providers and contract tests
└── platform_init.c  delegates registration to the selected arch port
```

Public contracts are separated by service. One physical timer may back PWM,
clock or capture services, but consumers only see the service they need. The
MSPM0 implementation may keep several timer services in one source file.

## Contracts

- `platform_gpio`: digital direction, level and edge interrupt callbacks.
- `platform_pwm`: atomically applies period, pulse width and enable state.
- `platform_clock`: reads a free-running raw tick counter.
- `platform_periodic_timer`: provides periodic callbacks for scheduling.
- `platform_counter`: represents a hardware counter or QEI peripheral.
- `platform_input_capture`: captures timer ticks at configured signal edges.
- `platform_uart`: byte-stream configuration, FIFO I/O and receive events.
- `platform_i2c`: synchronous transfers to 7-bit addressed clients.
- `platform_can`: CAN/CAN FD frames, controller state and error status.
- `platform_spi`: synchronous transfers to devices on a shared SPI bus.
- `platform_irq`: provides short critical sections for shared ISR data.

All values remain in quantized hardware units. Platform returns ticks and raw
levels; conversion to angle, speed, distance, voltage or current belongs in
Device or Application code.

`platform_counter`, `platform_input_capture` and `platform_spi` currently have
no MSPM0 provider. Calling them returns
`PLATFORM_STATUS_NOT_SUPPORTED` instead of asserting. This board's encoder uses
external logic to produce pulse and direction signals, so it composes GPIO
interrupts with `platform_clock`.

## Status contract

All subsystems use `core/platform_status.h`:

| Status | Meaning |
| --- | --- |
| `PLATFORM_STATUS_INVALID_ARGUMENT` | Bad pointer, range or format |
| `PLATFORM_STATUS_NO_DEVICE` | Provider exists, instance ID does not |
| `PLATFORM_STATUS_NOT_SUPPORTED` | Provider or optional capability absent |
| `PLATFORM_STATUS_BUSY` | Resource is temporarily owned or conflicting |
| `PLATFORM_STATUS_TIMEOUT` | Bounded hardware wait expired |
| `PLATFORM_STATUS_IO_ERROR` | Peripheral reported a transfer error |
| `PLATFORM_STATUS_TRY_AGAIN` | Valid non-blocking call has no data yet |
| `PLATFORM_STATUS_INVALID_STATE` | Operation is invalid in current state |

Provider absence is a supported configuration. Public dispatchers do not
assert. GPIO interrupts, UART callbacks, input-capture callbacks and CAN status
reporting are optional provider capabilities.

Callbacks execute in interrupt context. They must not block, allocate memory,
or call code that can sleep. Shared hardware restrictions are provider
constraints: for example, two PWM channels on one timer must use one period.

## Current MSPM0 resource map

| Platform resource | Hardware | Configuration |
| --- | --- | --- |
| PWM 0 | TIMA0 CC0, TB6612FNG A | 10 MHz ticks |
| PWM 1 | TIMA0 CC1, TB6612FNG B | 10 MHz ticks |
| PWM 2 | TIMG6 CC0, steering servo | 1 MHz ticks |
| Clock 0 | TIMG12 free-running counter | 80 MHz ticks |
| Periodic timer 0 | TIMG8 encoder sampling | 1 MHz ticks |
| UART 0 | UART1, PA17/PA18 | default 9600 8N1 |
| I2C bus 0 | I2C1, PA15/PA16 | 400 kbit/s, 7-bit |
| CAN controller 0 | CANFD0, PA12/PA13 | CAN FD and BRS |

GPIO IDs encode the port in the upper bits and the pin number in the lower
five bits. `arch/MSPM0/mspm0_resource_ids.h` defines that encoding, while
`Board/platform_resources.h` declares purpose-based indices such as
`BOARD_PWM_MOTOR_LEFT`. `Board/platform_instances.c` maps those indices to
opaque Platform IDs. Device code only stores the resulting ID.

## Static instances

Each provider owns a statically allocated instance table. Immutable hardware
fields and mutable callback or busy state may share one entry on small ports:

```c
static struct mspm0_uart_port s_uart_ports[] = {
    {
        .uart = UART1,
        .irq = UART1_INT_IRQn,
        .clock_hz = 40000000U,
    },
};
```

The table index is the Platform ID. To add an instance:

1. configure the peripheral and pins in SysConfig;
2. append one entry to the appropriate MSPM0 instance table;
3. add a purpose index to `Board/platform_resources.h`;
4. map it to the Platform ID in `Board/platform_instances.c`;
5. add or extend a host contract test and run a hardware loopback test.

Instance counts are derived from the arrays. A new MCU port supplies one
`platform_arch_init()` and registers only the providers that MCU implements.
The top-level CMake build links only `User/Platform/arch/${PLATFORM_PORT}`.

## Communication behavior

UART `write` and `read` are non-blocking FIFO operations. The returned count
can be shorter than the requested length. Attached UART callbacks run in
interrupt context and receive one byte per `RX_DATA` event.

I2C `transfer` is synchronous. Multiple messages use repeated START, with STOP
after the final message. Each hardware wait has a bounded 100 ms timeout. A
message owns its raw byte buffer; register interpretation belongs to Device.
The PA15/PA16 bus requires external pull-up resistors.

CAN `send` queues into one of two dedicated transmit buffers. `receive` polls
RX FIFO 0 and returns `PLATFORM_STATUS_TRY_AGAIN` when empty. `get_status`
reports bus errors and RX FIFO overflow. The provider accepts classic CAN and
CAN FD frames; remote frames are classic CAN only. The current SysConfig global
filter accepts non-matching frames into FIFO 0. The board must provide an
enabled external CAN transceiver compatible with the configured CAN FD rate.

Blocking configuration, I2C transfer and CAN mode-control functions are
foreground APIs and must not be called from an interrupt callback.

SPI is deliberately contract-only until an SPI instance is added in SysConfig.
Its device descriptor keeps bus, chip-select, mode, word size, bit order and
maximum clock separate from each transfer buffer.

## Tests

## Runtime diagnostics

GPIO, UART, I2C and CAN expose optional diagnostics snapshots backed by static
counters. They complement normal return codes: a return code describes one
call, while diagnostics show trends such as repeated ISR pressure, framing
errors, bus contention, timeouts, RX overflow and CAN bus-off observations.

Queries take a short atomic snapshot. Reset uses a short critical section.
Neither operation resets hardware or invokes user callbacks. Application code
owns logging, warning thresholds, degraded-mode and recovery decisions. If a
provider does not implement diagnostics, the public API returns
`PLATFORM_STATUS_NOT_SUPPORTED`.

## Examples

`examples/` contains reusable functions without a second `main()`. It is
excluded from the default firmware image and compiled by the host test project
with strict warnings. See `examples/README.md` for Board resource mappings.

Host tests use fake providers and do not include TI headers:

```text
cmake -S User/Platform/tests -B build/platform-tests -G Ninja
cmake --build build/platform-tests
ctest --test-dir build/platform-tests --output-on-failure
```

They verify missing providers, missing optional capabilities, invalid instance
IDs and normal dispatch. Hardware tests are still required for pin routing,
electrical behavior, timing and peripheral errata.

## References

- Linux Platform Devices and Drivers:
  <https://docs.kernel.org/driver-api/driver-model/platform.html>
- Linux GPIO consumer interface:
  <https://docs.kernel.org/driver-api/gpio/consumer.html>
- Linux PWM interface:
  <https://docs.kernel.org/driver-api/pwm.html>
- Linux Generic Counter interface:
  <https://docs.kernel.org/driver-api/generic-counter.html>
- Linux serial driver API:
  <https://docs.kernel.org/driver-api/serial/driver.html>
- Linux I2C subsystem:
  <https://docs.kernel.org/driver-api/i2c.html>
- Linux SPI subsystem:
  <https://docs.kernel.org/driver-api/spi.html>
- Linux SocketCAN:
  <https://docs.kernel.org/networking/can.html>
