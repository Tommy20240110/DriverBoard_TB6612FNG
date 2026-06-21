# Platform examples

These files expose reusable functions and intentionally do not define
`main()`. They are excluded from the firmware source glob, while host contract
tests compile them with strict warnings.

Application code should pass purpose-based Board mappings rather than raw MCU
instance names:

```c
#include "Board/platform_resources.h"
#include "Platform/examples/platform_examples.h"

struct platform_uart_echo_example echo = {
    .port = g_board_uart_instances[BOARD_UART_DEBUG],
};

struct platform_i2c_client sensor = {
    .bus = g_board_i2c_instances[BOARD_I2C_EXTERNAL],
    .address = 0x50U,
};
```

- `platform_gpio_example.c`: output setup and edge interrupt lifecycle.
- `platform_timer_example.c`: raw clock read, periodic timer and PWM setup.
- `platform_uart_echo_example.c`: ISR receives one byte; foreground polling
  echoes it without blocking in the callback.
- `platform_i2c_example.c`: register address write followed by repeated START
  read.
- `platform_can_example.c`: controller start, send and polling receive.
- `platform_diagnostics_example.c`: takes one snapshot for Application logging,
  warning thresholds or recovery policy. The example never resets hardware.
