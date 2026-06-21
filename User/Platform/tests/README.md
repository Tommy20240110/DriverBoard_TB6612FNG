# Platform host tests

These tests compile every contract dispatcher on the host. They verify missing
provider behavior for all current subsystems and use fake providers for
stateful dispatch tests. They do not include TI DriverLib.

```text
cmake -S User/Platform/tests -B build/platform-tests -G Ninja
cmake --build build/platform-tests
ctest --test-dir build/platform-tests --output-on-failure
```

Hardware-in-the-loop tests remain separate: UART loopback, I2C register access,
CAN loopback or two-node traffic, PWM measurement and timer timing checks.
