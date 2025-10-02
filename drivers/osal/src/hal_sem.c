#include <hal_sem.h>
#include <hal_interrupt.h>

hal_sem_t hal_sem_create(unsigned int cnt)
{
	return (hal_sem_t)xSemaphoreCreateCounting(0xffffffffU, cnt);
}

int hal_sem_delete(hal_sem_t sem)
{
	vSemaphoreDelete(sem);

	return 0;
}

int hal_sem_getvalue(hal_sem_t sem, int *val)
{
	if (val == NULL) {
		printf("hal_sem_getvalue error, val is NULL\n");
		return -1;
	}
	*val = uxSemaphoreGetCount(sem);

	return 0;
}

int hal_sem_post(hal_sem_t sem)
{
	BaseType_t xHigherPriorityTaskWoken;

	/* Give the semaphore using the FreeRTOS API. */
	if (hal_interrupt_get_nest())
		xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
	else
		xSemaphoreGive(sem);

	return 0;
}

int hal_sem_timedwait(hal_sem_t sem, int ticks)
{
	TickType_t xDelay = (TickType_t)ticks;
	BaseType_t xHigherPriorityTaskWoken;
	BaseType_t ret;

	if (hal_interrupt_get_nest())
		ret = xSemaphoreTakeFromISR(sem, &xHigherPriorityTaskWoken);
	else
		ret = xSemaphoreTake(sem, xDelay);

	if (ret != pdTRUE)
		return -1;

	return 0;
}

int hal_sem_trywait(hal_sem_t sem)
{
	return hal_sem_timedwait(sem, 0);
}


int hal_sem_wait(hal_sem_t sem)
{
	return hal_sem_timedwait(sem, portMAX_DELAY);
}

int hal_sem_clear(hal_sem_t sem)
{
	xQueueReset(sem);

	return 0;
}

