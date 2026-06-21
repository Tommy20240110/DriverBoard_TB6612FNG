#ifndef PLATFORM_FAKE_COMMUNICATIONS_H
#define PLATFORM_FAKE_COMMUNICATIONS_H

#include "Platform/core/platform_status.h"

platform_status_t platform_fake_communications_install(void);
void platform_fake_uart_record_rx(void);

#endif /* PLATFORM_FAKE_COMMUNICATIONS_H */
