#ifndef PLATFORM_IRQ_H
#define PLATFORM_IRQ_H

#include <stdint.h>

typedef uint32_t platform_irq_state_t;

platform_irq_state_t platform_irq_save(void);
void platform_irq_restore(platform_irq_state_t state);

#endif /* PLATFORM_IRQ_H */
