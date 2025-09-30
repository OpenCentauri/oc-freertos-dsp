// // Definitions for irq enable/disable on ARM Cortex-M processors
// //
// // Copyright (C) 2017-2018  Kevin O'Connor <kevin@koconnor.net>
// //
// // This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/board/irq.h" // irqstatus_t
#include <portmacro.h>
#include <hal_interrupt.h>

void irq_disable_all()
{
    // asm volatile("cpsid i" ::: "memory");
    // hal_interrupt_disable();
    // portDISABLE_INTERRUPTS();
    XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);        //---------irq_disable_all-G-G-2022-08-10-------------
    
}

void irq_enable_all()
{
    // hal_interrupt_enable();
    // portENABLE_INTERRUPTS();
    XTOS_SET_INTLEVEL(0);   
    XT_RSIL(0);         //      repo/lichee/rtos-dsp/freertos/XtDevTools/RI-2020.4-linux/aw_axi_cfg0/xtensa-elf/arch/include/xtensa/tie/xt_core.h
    // _TIE_xt_core_RSIL(0);   
    // asm volatile("cpsie i" ::: "memory");
}

irqstatus_t irq_save(void)
{
    irqstatus_t flag;
    // asm volatile("mrs %0, primask" : "=r" (flag) :: "memory");
    // flag = hal_interrupt_save();
    flag = XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);
    //  hal_interrupt_disable();

     return flag;
}

void irq_restore(irqstatus_t flag)
{
    // hal_interrupt_restore(flag);  //---gg-6.30
    // XTOS_RESTORE_JUST_INTLEVEL(flag);
    // xthal_intlevel_set(flag);
    if(flag)
    {
        XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);
    }
    else{
        XTOS_SET_INTLEVEL(0);
    }
}

void irq_wait(void)
{
    //  asm("sei\n    nop\n    cli" : : : "memory");
    XTOS_SET_INTLEVEL(0);  
    asm("nop\n    nop\n    nop" : : : "memory");
    XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);  
}

void
irq_poll(void)
{
}

// // Clear the active irq if a shutdown happened in an irq handler
void
clear_active_irq(void)
{
    // uint32_t psr;
    // asm volatile("mrs %0, psr" : "=r" (psr));
    // if (!(psr & 0x1ff))
    //     // Shutdown did not occur in an irq - nothing to do.
    //     return;
    // // Clear active irq status
    // psr = 1<<24; // T-bit
    // uint32_t temp;
    // asm volatile(
    //     "  push { %1 }\n"
    //     "  adr %0, 1f\n"
    //     "  push { %0 }\n"
    //     "  push { r0, r1, r2, r3, r4, lr }\n"
    //     "  bx %2\n"
    //     ".balign 4\n"
    //     "1:\n"
    //     : "=&r"(temp) : "r"(psr), "r"(0xfffffff9) : "r12", "cc");
}
// DECL_SHUTDOWN(clear_active_irq);
