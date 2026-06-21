cmake_minimum_required(VERSION 3.20)

project(platform_contract_tests C)

add_executable(platform_contract_tests
    unit/platform_contract_test.c
    fake/platform_fake_gpio.c
    fake/platform_fake_communications.c
    ../gpio/platform_gpio.c
    ../timers/platform_clock.c
    ../timers/platform_counter.c
    ../communications/platform_spi.c
    ../timers/platform_input_capture.c
    ../timers/platform_periodic_timer.c
    ../timers/platform_pwm.c
    ../communications/platform_uart.c
    ../communications/platform_i2c.c
    ../communications/platform_can.c
    ../examples/platform_gpio_example.c
    ../examples/platform_timer_example.c
    ../examples/platform_uart_echo_example.c
    ../examples/platform_i2c_example.c
    ../examples/platform_can_example.c
    ../examples/platform_diagnostics_example.c
)

target_include_directories(platform_contract_tests PRIVATE
    ../..
)

target_compile_features(platform_contract_tests PRIVATE c_std_99)

if(MSVC)
    target_compile_options(platform_contract_tests PRIVATE
        /W4
        /WX
    )
else()
    target_compile_options(platform_contract_tests PRIVATE
        -Wall
        -Wextra
        -Werror
        -pedantic
    )
endif()

enable_testing()
add_test(NAME platform_contract_tests
         COMMAND platform_contract_tests)
