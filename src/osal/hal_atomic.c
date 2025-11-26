#include <FreeRTOS.h>
#include <task.h>

#include <stdint.h>
#include <spinlock.h>
#include <hal_atomic.h>
#include <hal_interrupt.h>

uint32_t hal_spin_lock_irqsave(hal_spinlock_t *lock)
{
	unsigned cpu_sr;

	spin_lock_irqsave(cpu_sr);

	return cpu_sr;
}

void hal_spin_unlock_irqrestore(hal_spinlock_t *lock, uint32_t __cpsr)
{
	spin_unlock_irqrestore(__cpsr);
}

void hal_spin_lock(hal_spinlock_t *lock)
{
	if (hal_interrupt_get_nest() == 0)
		vTaskSuspendAll();
}

void hal_spin_unlock(hal_spinlock_t *lock)
{
	if (hal_interrupt_get_nest() == 0)
		xTaskResumeAll();
}
