#include "timer.h"
#include "platform.h"
#include "FreeRTOSConfig.h"
#include "xtensa_api.h"

#define RWDOG_BASE 0x01701000
#define RWDOG_MODE_REG (RWDOG_BASE + 0x0018)

#define TIMER_BASE 0x01700C00
#define TIMER_IRQ_EN_REG (TIMER_BASE + 0x00)
#define TIMER_IRQ_STA_REG (TIMER_BASE + 0x04)
#define TIMER1_CTRL_REG (TIMER_BASE + 0x20)
#define TIMER1_INTV_REG (TIMER_BASE + 0x24)
#define TIMER1_CUR_REG (TIMER_BASE + 0x28)

void timer1_interrupt_handler(void);
extern void xPortSysTickHandler(void);

void timer_init(void) {
    // Disable the watchdog timer
    writel(RWDOG_MODE_REG, readl(RWDOG_MODE_REG) & ~1);

    // Set the timer interval
    writel(TIMER1_INTV_REG, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));

    // Enable the timer and set it to periodic mode
    writel(TIMER1_CTRL_REG, (1 << 0) | (1 << 1));

    // Enable the timer interrupt
    writel(TIMER_IRQ_EN_REG, readl(TIMER_IRQ_EN_REG) | (1 << 1));

    // Register the interrupt handler
    xt_set_interrupt_handler(19, (xt_handler)timer1_interrupt_handler, NULL);
}

void timer1_interrupt_handler(void) {
    // Clear the timer interrupt pending bit
    writel(TIMER_IRQ_STA_REG, (1 << 1));

    // Call the FreeRTOS tick handler
    xPortSysTickHandler();
}
