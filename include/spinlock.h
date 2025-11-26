/*
 * Spinlock definitions for Xtensa/FreeRTOS
 * On single-core systems, spinlocks are just interrupt disable/enable
 */
#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "portmacro.h"

#ifdef __cplusplus
extern "C" {
#endif

/* On single core, spinlock is just critical section */
#define spin_lock_irqsave(flags) \
    do { \
        flags = portENTER_CRITICAL_NESTED(); \
    } while (0)

#define spin_unlock_irqrestore(flags) \
    do { \
        portEXIT_CRITICAL_NESTED(flags); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _SPINLOCK_H_ */
