/*
 * Interrupt wrapper for Xtensa/FreeRTOS
 */
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <stdint.h>
#include "xtensa_api.h"
#include "xtensa/hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Interrupt handler type */
typedef void (*interrupt_handler_t)(void *data);

/* IRQ request/free functions */
static inline int irq_request(unsigned int irq, interrupt_handler_t handler, void *dev)
{
    xt_handler old = xt_set_interrupt_handler(irq, (xt_handler)handler, dev);
    return (old != NULL) ? 0 : -1;
}

static inline int irq_free(unsigned int irq)
{
    xt_set_interrupt_handler(irq, NULL, NULL);
    return 0;
}

static inline void irq_enable(unsigned int irq)
{
    xthal_int_enable(1 << irq);
}

static inline void irq_disable(unsigned int irq)
{
    xthal_int_disable(1 << irq);
}

#ifdef __cplusplus
}
#endif

#endif /* _INTERRUPT_H_ */
