# RTOS HAL Integration Guide

The `rtos-hal` folder has been successfully integrated into the project build system with **full OSAL (Operating System Abstraction Layer) support**.

## What Was Changed

### 1. Makefile Updates

#### Include Paths Added:
- `-I ./rtos-hal/include` - Main HAL include directory
- `-I ./rtos-hal/include/hal` - HAL driver headers
- `-I ./rtos-hal/include/osal` - OSAL (OS Abstraction Layer) headers
- `-I ./rtos-hal/hal/source` - HAL source internal headers

#### Configuration Defines Added:
- `-DCONFIG_KERNEL_FREERTOS` - Enables FreeRTOS-specific code paths in HAL
- `-DCONFIG_CORE_DSP0` - Enables DSP0 core-specific features

#### Build Directories Added:
Build directories for rtos-hal and OSAL modules are now created automatically:
- `build/src/osal` - OSAL implementation files
- `build/rtos-hal/hal/source/common`
- `build/rtos-hal/hal/source/msgbox/msgbox_sx`
- `build/rtos-hal/hal/source/uart`
- `build/rtos-hal/hal/source/gpio`
- `build/rtos-hal/hal/source/timer`

### 2. New OSAL Implementation Files

Complete OSAL layer has been added in `src/osal/`:

- **hal_mem.c** - Memory allocation (malloc/free wrappers)
- **hal_interrupt.c** - Interrupt management and control
- **hal_atomic.c** - Atomic operations and spinlocks
- **hal_cache.c** - Cache management (clean, invalidate, writeback)
- **hal_sem.c** - Semaphore operations (create, post, wait, delete)
- **hal_mutex.c** - Mutex operations (create, lock, unlock, delete)
- **hal_queue.c** - Queue and mailbox operations
- **hal_timer.c** - Timer and delay functions (sleep, usleep, msleep)
- **hal_thread.c** - Thread management (currently stubbed)

### 3. New System Headers

Added essential system headers in `include/`:

- **barrier.h** - Memory barrier macros for Xtensa (mb, rmb, wmb, etc.)
- **interrupt.h** - Interrupt request/free, enable/disable wrappers for Xtensa
- **spinlock.h** - Spinlock macros (implemented as critical sections for single-core)

## How to Use RTOS HAL in Your Code

### Available Headers

You can now include any HAL headers in your `.c` files in the `src/` folder:

```c
#include "hal_msgbox.h"      // Message box communication
#include "hal_gpio.h"        // GPIO control
#include "hal_uart.h"        // UART communication
#include "hal_timer.h"       // Timer functions
#include "hal_dma.h"         // DMA operations
#include "hal_spi.h"         // SPI communication
#include "hal_twi.h"         // TWI/I2C communication
#include "hal_pwm.h"         // PWM control
#include "hal_watchdog.h"    // Watchdog timer
#include "hal_intc.h"        // Interrupt controller
#include "hal_prcm.h"        // Power/reset/clock management
```

### OSAL Headers

**Operating System Abstraction Layer headers are now fully implemented:**

#### Synchronization Primitives:

```c
#include "hal_sem.h"         // Semaphore operations

// Create semaphore with initial count
hal_sem_t sem = hal_sem_create(0);
hal_sem_post(sem);           // Signal semaphore
hal_sem_wait(sem);           // Wait forever
hal_sem_timedwait(sem, 100); // Wait with timeout (ticks)
hal_sem_trywait(sem);        // Non-blocking wait
hal_sem_delete(sem);         // Delete semaphore
```

```c
#include "hal_mutex.h"       // Mutex operations

hal_mutex_t mutex = hal_mutex_create();
hal_mutex_lock(mutex);       // Lock mutex (blocking)
hal_mutex_trylock(mutex);    // Try lock (non-blocking)
hal_mutex_unlock(mutex);     // Unlock mutex
hal_mutex_delete(mutex);     // Delete mutex
```

#### Message Queues:

```c
#include "hal_queue.h"       // Queue operations

// Create queue: name, item_size, queue_size
hal_queue_t queue = hal_queue_create("myqueue", sizeof(int), 10);
int data = 42;
hal_queue_send(queue, &data);              // Send (non-blocking)
hal_queue_send_wait(queue, &data, 100);    // Send with timeout
hal_queue_recv(queue, &data, 100);         // Receive with timeout
hal_is_queue_empty(queue);                 // Check if empty
hal_queue_delete(queue);                   // Delete queue

// Mailbox (simplified queue for integers)
hal_mailbox_t mbox = hal_mailbox_create("mymbox", 10);
hal_mailbox_send(mbox, 42);
unsigned int value;
hal_mailbox_recv(mbox, &value, 100);
hal_mailbox_delete(mbox);
```

#### Interrupt Management:

```c
#include "hal_interrupt.h"   // Interrupt handling

uint32_t hal_interrupt_get_nest(void);     // Get interrupt nesting level
void hal_interrupt_enable(void);           // Enable interrupts globally
void hal_interrupt_disable(void);          // Disable interrupts globally
uint32_t flags = hal_interrupt_save(void); // Save and disable interrupts
hal_interrupt_restore(flags);              // Restore interrupt state

// Standard Linux-style IRQ API
request_irq(irq_num, handler, flags, "name", dev);
enable_irq(irq_num);
disable_irq(irq_num);
free_irq(irq_num, dev);
```

#### Cache Operations:

```c
#include "hal_cache.h"       // Cache management

void hal_dcache_clean(unsigned long vaddr, unsigned long size);
void hal_dcache_invalidate(unsigned long vaddr, unsigned long size);
void hal_dcache_clean_invalidate(unsigned long vaddr, unsigned long size);
void hal_icache_invalidate_all(void);
void hal_dcache_invalidate_all(void);
void hal_dcache_clean_all(void);
```

#### Atomic Operations:

```c
#include "hal_atomic.h"      // Spinlocks and atomic operations

hal_spinlock_t lock;
uint32_t flags = hal_spin_lock_irqsave(&lock);
// Critical section
hal_spin_unlock_irqrestore(&lock, flags);

// Or without IRQ save/restore
hal_spin_lock(&lock);
// Critical section
hal_spin_unlock(&lock);
```

#### Memory Management:

```c
#include "hal_mem.h"         // Memory allocation

void *ptr = hal_malloc(size);
hal_free(ptr);

// Aligned allocation
void *aligned = hal_malloc_align(size, 64); // 64-byte alignment
hal_free_align(aligned);

// Address translation (identity mapped by default)
unsigned long phys = hal_virt_to_phys(virt_addr);
unsigned long virt = hal_phys_to_virt(phys_addr);
```

#### Timers and Delays:

```c
#include "hal_timer.h"       // Timer management

hal_sleep(1);        // Sleep for 1 second
hal_msleep(100);     // Sleep for 100 milliseconds
hal_usleep(1000);    // Sleep for 1000 microseconds
```

### Example Usage

See `src/hal_example.c` for complete examples showing how to use OSAL functions.

## Adding More HAL Modules

To enable additional HAL driver modules, edit the `Makefile` and uncomment or add lines in the `RTOS_HAL_SRC` section:

```makefile
# RTOS HAL source files
RTOS_HAL_SRC := rtos-hal/hal/source/common/dma_alloc

# Add more HAL modules as needed:
RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/msgbox_sx
RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/msgbox_adapt
RTOS_HAL_SRC += rtos-hal/hal/source/msgbox/msgbox_sx/hal_msgbox_sx
RTOS_HAL_SRC += rtos-hal/hal/source/uart/hal_uart
RTOS_HAL_SRC += rtos-hal/hal/source/gpio/hal_gpio
RTOS_HAL_SRC += rtos-hal/hal/source/timer/hal_timer
```

### Available Module Paths:

- **UART**: `rtos-hal/hal/source/uart/hal_uart`
- **GPIO**: `rtos-hal/hal/source/gpio/` (check for specific .c files)
- **Timer**: `rtos-hal/hal/source/timer/hal_timer`
- **SPI**: `rtos-hal/hal/source/spi/hal_spi`
- **TWI/I2C**: `rtos-hal/hal/source/twi/hal_twi`
- **PWM**: `rtos-hal/hal/source/pwm/hal_pwm`
- **DMA**: `rtos-hal/hal/source/dma/` (check for specific .c files)
- **Watchdog**: `rtos-hal/hal/source/watchdog/hal_watchdog`
- **Msgbox (AMP)**: `rtos-hal/hal/source/msgbox/msgbox_amp/msgbox_amp`
- **Msgbox (SX)**: `rtos-hal/hal/source/msgbox/msgbox_sx/msgbox_sx`

Remember to also add corresponding build directories if you enable new modules from subdirectories.

## Building

Simply run `make` as usual:

```bash
make
```

The build system will now compile the enabled RTOS HAL modules along with your application code.

## Testing the Integration

1. Include a HAL header in any of your source files in `src/`
2. Run `make clean && make`
3. Verify that compilation succeeds without header not found errors

## Notes

- HAL headers are now in the include path, so you only need to use the header filename (e.g., `#include "hal_gpio.h"`)
- The integration uses the existing build system structure
- Additional HAL modules can be enabled by uncommenting or adding source files in the Makefile
- Some HAL functions may require platform-specific initialization or configuration
