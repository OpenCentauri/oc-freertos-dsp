/*
 * Example file showing how to use rtos-hal headers
 * This demonstrates the integration of rtos-hal into the project
 */

#include <stdint.h>
#include <stdio.h>

/* Include HAL headers - now available after integration */
#include "hal_msgbox.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_timer.h"

/* Include OSAL headers - fully implemented! */
#include "hal_log.h"
#include "hal_sem.h"
#include "hal_mutex.h"
#include "hal_queue.h"
#include "hal_interrupt.h"
#include "hal_cache.h"
#include "hal_atomic.h"

/*
 * Example function demonstrating OSAL usage
 */
void osal_example_init(void)
{
    /* Example: Create a semaphore */
    hal_sem_t sem = hal_sem_create(0);
    if (sem) {
        hal_sem_post(sem);
        hal_sem_wait(sem);
        hal_sem_delete(sem);
    }
    
    /* Example: Create a mutex */
    hal_mutex_t mutex = hal_mutex_create();
    if (mutex) {
        hal_mutex_lock(mutex);
        // Critical section
        hal_mutex_unlock(mutex);
        hal_mutex_delete(mutex);
    }
    
    /* Example: Create a queue */
    hal_queue_t queue = hal_queue_create("test_queue", sizeof(int), 10);
    if (queue) {
        int data = 42;
        hal_queue_send(queue, &data);
        hal_queue_recv(queue, &data, 100);
        hal_queue_delete(queue);
    }
    
    printf("RTOS HAL and OSAL are now fully integrated!\n");
}

/*
 * Example: Cache operations
 */
void cache_example(void *buffer, unsigned long size)
{
    /* Clean data cache (write back) */
    hal_dcache_clean((unsigned long)buffer, size);
    
    /* Invalidate data cache */
    hal_dcache_invalidate((unsigned long)buffer, size);
    
    /* Clean and invalidate */
    hal_dcache_clean_invalidate((unsigned long)buffer, size);
}

/*
 * Example: Interrupt management
 */
void interrupt_example(void)
{
    /* Save and disable interrupts */
    uint32_t flags = hal_interrupt_save();
    
    /* Critical section */
    
    /* Restore interrupts */
    hal_interrupt_restore(flags);
}

/*
 * Example msgbox endpoint configuration
 */
void hal_example_msgbox(void)
{
    /*
    struct msg_endpoint ep;
    ep.local_amp = DSP0_MSG_CORE;
    ep.remote_amp = ARM_MSG_CORE;
    ep.write_ch = 0;
    ep.read_ch = 1;
    
    hal_msgbox_init();
    hal_msgbox_alloc_channel(&ep, ARM_MSG_CORE, 1, 0);
    
    uint8_t msg[] = "Hello from DSP";
    hal_msgbox_channel_send(&ep, msg, sizeof(msg));
    */
}
