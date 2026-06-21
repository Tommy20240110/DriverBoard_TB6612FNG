#include "../../core/platform_irq.h"

#include "cmsis_gcc.h"

platform_irq_state_t platform_irq_save(void)
{
    platform_irq_state_t state = __get_PRIMASK();

    __disable_irq();
    return state;
}

void platform_irq_restore(platform_irq_state_t state)
{
    __set_PRIMASK(state);
}
