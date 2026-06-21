#ifndef PLATFORM_STATUS_H
#define PLATFORM_STATUS_H

/*
 * Platform APIs use one status vocabulary across every MCU provider.
 * Providers may return these values directly; callers must not interpret
 * unrelated negative integers as subsystem-specific meanings.
 */
typedef enum platform_status
{
    PLATFORM_STATUS_OK = 0,
    PLATFORM_STATUS_INVALID_ARGUMENT = -1,
    PLATFORM_STATUS_NO_DEVICE = -2,
    PLATFORM_STATUS_NOT_SUPPORTED = -3,
    PLATFORM_STATUS_BUSY = -4,
    PLATFORM_STATUS_TIMEOUT = -5,
    PLATFORM_STATUS_IO_ERROR = -6,
    PLATFORM_STATUS_TRY_AGAIN = -7,
    PLATFORM_STATUS_INVALID_STATE = -8,
    PLATFORM_STATUS_ALREADY_REGISTERED = -9,
} platform_status_t;

#endif /* PLATFORM_STATUS_H */
