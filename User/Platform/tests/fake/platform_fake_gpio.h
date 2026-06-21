#ifndef PLATFORM_FAKE_GPIO_H
#define PLATFORM_FAKE_GPIO_H

#include "Platform/gpio/platform_gpio.h"

#define PLATFORM_FAKE_GPIO_PIN_COUNT (64U)

platform_status_t platform_fake_gpio_install(void);
void platform_fake_gpio_reset(void);
platform_status_t platform_fake_gpio_trigger(
    platform_gpio_pin_t pin);
platform_status_t platform_fake_gpio_force_isr_limit(
    platform_gpio_pin_t pin);

#endif /* PLATFORM_FAKE_GPIO_H */
