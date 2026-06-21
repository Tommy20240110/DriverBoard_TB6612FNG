#ifndef MSPM0_PROVIDERS_H
#define MSPM0_PROVIDERS_H

#include "../../core/platform_status.h"

platform_status_t mspm0_gpio_provider_init(void);
platform_status_t mspm0_pwm_provider_init(void);
platform_status_t mspm0_clock_provider_init(void);
platform_status_t mspm0_periodic_timer_provider_init(void);
platform_status_t mspm0_uart_provider_init(void);
platform_status_t mspm0_i2c_provider_init(void);
platform_status_t mspm0_can_provider_init(void);

#endif /* MSPM0_PROVIDERS_H */
