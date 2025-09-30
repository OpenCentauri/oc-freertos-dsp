#ifndef __GENERIC_IRQ_H
#define __GENERIC_IRQ_H

typedef unsigned long irqstatus_t;  

void irq_disable_all();
void irq_enable_all();
irqstatus_t irq_save(void);
void irq_restore(irqstatus_t flag);
void irq_wait(void);
void irq_poll(void);

#endif // irq.h
