#include "platform_init.h"
#include "arch/platform_arch.h"

platform_status_t platform_init(void)
{
    return platform_arch_init();
}
