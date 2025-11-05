#include "timer.h"
#include "platform.h"
#include "FreeRTOSConfig.h"
#include "xtensa_api.h"
#include "log.h"

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
    lprintf("timer_init: Disabling watchdog timer...\n");
    writel(RWDOG_MODE_REG, readl(RWDOG_MODE_REG) & ~1);
    lprintf("timer_init: RWDOG_MODE_REG = 0x%x\n", readl(RWDOG_MODE_REG));

    lprintf("timer_init: Setting timer interval...\n");
    writel(TIMER1_INTV_REG, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));
    lprintf("timer_init: TIMER1_INTV_REG = 0x%x\n", readl(TIMER1_INTV_REG));

    lprintf("timer_init: Enabling timer...\n");
    writel(TIMER1_CTRL_REG, (1 << 0) | (1 << 1));
    lprintf("timer_init: TIMER1_CTRL_REG = 0x%x\n", readl(TIMER1_CTRL_REG));

    lprintf("timer_init: Enabling timer interrupt...\n");
    writel(TIMER_IRQ_EN_REG, readl(TIMER_IRQ_EN_REG) | (1 << 1));
    lprintf("timer_init: TIMER_IRQ_EN_REG = 0x%x\n", readl(TIMER_IRQ_EN_REG));

    lprintf("timer_init: Registering interrupt handler...\n");
    xt_set_interrupt_handler(19, (xt_handler)timer1_interrupt_handler, NULL);
    lprintf("timer_init: Interrupt handler registered.\n");
}

void timer1_interrupt_handler(void) {
    lprintf("timer1_interrupt_handler: Interrupt received!\n");
    // Clear the timer interrupt pending bit
    writel(TIMER_IRQ_STA_REG, (1 << 1));

    // Call the FreeRTOS tick handler
    xPortSysTickHandler();
}
