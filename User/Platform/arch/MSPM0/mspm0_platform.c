#include "../platform_arch.h"

#include "mspm0_internal.h"
#include "mspm0_providers.h"

#include <stdint.h>

typedef platform_status_t (*mspm0_provider_init_t)(void);

static mspm0_provider_init_t const s_provider_initializers[] = {
    mspm0_gpio_provider_init,
    mspm0_pwm_provider_init,
    mspm0_clock_provider_init,
    mspm0_periodic_timer_provider_init,
    mspm0_uart_provider_init,
    mspm0_i2c_provider_init,
    mspm0_can_provider_init,
};

platform_status_t platform_arch_init(void)
{
    uint32_t index;

    for (index = 0U;
         index <
             (uint32_t)MSPM0_ARRAY_COUNT(
                 s_provider_initializers);
         index++) {
        platform_status_t rc =
            s_provider_initializers[index]();

        if (rc != PLATFORM_STATUS_OK) {
            return rc;
        }
    }
    return PLATFORM_STATUS_OK;
}
