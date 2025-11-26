#include <stdio.h>
#include <interrupt.h>
#include <hal_interrupt.h>
#include <sunxi_hal_common.h>

#include <portmacro.h>

int request_irq(unsigned int irq, irq_handler_t handler,
		unsigned long flags, const char *name, void *dev)
{
	if (irq_request(irq, (interrupt_handler_t)handler, dev) == FAIL) {
		printf("irq request failure.\n");
		return -1;
	}

	return 0;
}

void *free_irq(unsigned int irq, void *data)
{
	if (irq_free(irq) == FAIL) {
		printf("irq free failure.\n");
		return NULL;
	}

	return NULL;
}

void enable_irq(unsigned int irq)
{
	irq_enable(irq);
}

void disable_irq(unsigned int irq)
{
	irq_disable(irq);
}

extern unsigned port_interruptNesting;	/* defined in port.c */
uint32_t hal_interrupt_get_nest(void)
{
	return port_interruptNesting;
}

void hal_interrupt_enable(void)
{
	portENABLE_INTERRUPTS();
}

void hal_interrupt_disable(void)
{
	portDISABLE_INTERRUPTS();
}

uint32_t hal_interrupt_save(void)
{
	return portENTER_CRITICAL_NESTED();
}

void hal_interrupt_restore(uint32_t flag)
{
	portEXIT_CRITICAL_NESTED(flag);
}
