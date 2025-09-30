// Timer based on ARM Cortex-M3/M4 SysTick and DWT logic
//
// Copyright (C) 2017-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <hal_gpio.h>
#include <sound/snd_dma.h>
#include <sound/dma_wrap.h>
#include <hal_dma.h>
#include <hal_clk.h>
#include <hal_reset.h>
#include "../../include/board/gpio.h"


#include "../../include/printer/autoconf.h" // 
// #include "armcm_boot.h" // 
// #include "board/internal.h" // 
#include "../../include/board/irq.h" // 
#include "../../include/board/misc.h" // 
#include "../../include/printer/command.h" // 
#include "../../include/printer/scheder.h" // 
#include "../../include/printer/basecmd.h" //

#include "sunxi_hal_htimer.h"
#include "hal_clk.h"
#include "delay.h"



// Return the number of clock ticks for a given number of microseconds
uint32_t timer_from_us(uint32_t us)
{
    return us * (CONFIG_CLOCK_FREQ / 1000000);
}

// Return true if time1 is before time2.  Always use this function to
// compare times as regular C comparisons can fail if the counter
// rolls over.
uint8_t timer_is_before(uint32_t time1, uint32_t time2)
{
    return (int32_t)(time1 - time2) < 0;       //----G-G-2022-07-29-----
}

// Set the next irq time
// static 
void timer_set_diff(uint32_t value)
{
    set_htimer_intval(value, HAL_HRTIMER0);
}

// Return the current time (in absolute clock ticks).
uint32_t timer_read_time(void)
{
    uint32_t ret = read_htimer_current_value(HAL_HRTIMER1);         //180M-200M 32bits
    ret = 0xffffffff - ret;
    return ret;
}

// Activate timer dispatch as soon as possible
void timer_kick(void)
{
    timer_set_diff(1);
}

//实现简单的早期引导延迟机制  Implement simple early-boot delay mechanism 
void udelay1(uint32_t usecs)
{
    // if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
    //     CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    //     DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    // }
    uint32_t end = timer_read_time() + timer_from_us(usecs);
    while (timer_is_before(timer_read_time(), end))
        ;
}
void udelay_timer(unsigned int us)
{
	unsigned expiry = timer_read_time() + 200 * us;
    //  printf( "udelay: %u \n",xthal_get_ccount() );
	while( (long)(expiry - timer_read_time()) > 0 );
    //  printf( "udelay: %u \n",xthal_get_ccount() );
	return;
}
// 虚拟计时器，以避免调度系统的 irq 大于0xffffff   Dummy timer to avoid scheduling a SysTick irq greater than 0xffffff
static uint_fast8_t
timer_wrap_event(struct timer *t)
{
    t->waketime += 0xffffff;
    return SF_RESCHEDULE;
}
static struct timer wrap_timer = {
    .func = timer_wrap_event,
    .waketime = 0xffffff,
};

void timer_reset(void)
{
    if (timer_from_us(100000) <= 0xffffff)  //100ms  滴答数 Timer in sched.c already ensures SysTick wont overflow
    {
        DisTraceMsg();      //--G-G-2022-07-28--
        return;
    }
    if(sched_add_timer(&wrap_timer))
    {
        GAM_DEBUG_errer( "--timer_reset--1----" );
    }
}
DECL_SHUTDOWN(timer_reset);

void SysTick_Handler(void *param);
static uint32_t timer1_total_times;
#include <hal_gpio.h>

//    #define GPIO_TEST_SYS1		GPIO_PC1
//     #define GPIO_TEST_SYS2		GPIO_PC0
//      #define GPIO_TEST_SYS3		GPIO_PF5
//       #define GPIO_TEST_SYS4		GPIO_PG11
 
void GAM_DBG_gpio_init(void)
{
    {
        // g_GAM_DBG_gpio1 = gpio_out_setup(GPIO_TEST_SYS1,1 );
        // g_GAM_DBG_gpio2 = gpio_out_setup(GPIO_TEST_SYS2,1 );
        // g_GAM_DBG_gpio3 = gpio_out_setup(GPIO_TEST_SYS3,1 );
        // g_GAM_DBG_gpio4 = gpio_out_setup(GPIO_TEST_SYS4,1 );
    }
}
void GAM_DBG_gpio_write(gpio_data_t data)
{
    // gpio_out_toggle_noirq(&g_GAM_DBG_gpio1);            //250ns----HLH
    // gpio_out_write(&g_GAM_DBG_gpio2,!data);                 //-250ns----HHL----LHL
    // gpio_out_write(&g_GAM_DBG_gpio3,data);                  //250nsHLH
    // gpio_out_write(&g_GAM_DBG_gpio4,!data);
}


static void hal_htimer_irq_callback(void *param)
{
   timer1_total_times += 1;
   DisTraceMsg();      //--G-G-2022-07-28--
}
void timer_init(void)
{
    hal_htimer_init(HAL_HRTIMER1);
    timer1_total_times = 0;
    hal_htimer_set_periodic(HAL_HRTIMER1, 0xffffffff, hal_htimer_irq_callback, NULL);//0xffffffff 20000000
    timer_reset();     //--G-G------
    
    irqstatus_t flag = irq_save();  
    hal_htimer_init(HAL_HRTIMER0);
    hal_htimer_set_periodic(HAL_HRTIMER0, 200000, SysTick_Handler, NULL);
    timer_kick();  //--G-G-6.30-----
    irq_restore(flag);

    GAM_DBG_gpio_init();

    // irq_enable_all();
	// int i = 0;
	// while(1)
	// {
	// 	uint32_t time =  timer_read_time();     //-CS-G-G-2022-07-28-----
	// 	printf( "DSP mian start!-%d-%x-\n",i++ ,time);
	// 	// udelay_timer(3000000);//3s
	// 	 udelay_timer(1000000);
	// }
}
DECL_INIT(timer_init);

static uint32_t timer_repeat_until;
#define TIMER_IDLE_REPEAT_TICKS timer_from_us(500)
#define TIMER_REPEAT_TICKS timer_from_us(100)
#define TIMER_MIN_TRY_TICKS timer_from_us(2)
#define TIMER_DEFER_REPEAT_TICKS timer_from_us(5)

// Invoke timers
static uint32_t
timer_dispatch_many(void)
{
    uint32_t tru = timer_repeat_until;
    for (;;) {
        uint32_t next = sched_timer_dispatch();
        uint32_t now = timer_read_time();
        int32_t diff = next - now;
   
        if (diff > (int32_t)TIMER_MIN_TRY_TICKS)
            return diff;
        
        if (unlikely(timer_is_before(tru, now))) {
            if (diff < (int32_t)(-timer_from_us(1000)))            // Check if there are too many repeat timers
            {
                // printf("next:%x now:%x diff:%d TIMER_MIN_TRY_TICKS:%u \n", next,now,diff,TIMER_MIN_TRY_TICKS  ); //-HTIMER-G-G-2022-07-28-----
                // printf( "timer_dispatch_many-0:%x-1:%x-\n",read_htimer_current_value(HAL_HRTIMER0) ,read_htimer_current_value(HAL_HRTIMER1));
                try_shutdown("Rescheduled timer in the past");
            }
            if (sched_tasks_busy()) {
                timer_repeat_until = now + TIMER_REPEAT_TICKS;
                return TIMER_DEFER_REPEAT_TICKS;
            }
            timer_repeat_until = tru = now + TIMER_IDLE_REPEAT_TICKS;
        }

        // Next timer in the past or  fnearuture - wait for it to be ready
        irq_enable_all();
        while (unlikely(diff > 0))
        {
            diff = next - timer_read_time();
        }
        irq_disable_all();
    }
}




void __visible __aligned(16) // aligning helps stabilize perf benchmarks
SysTick_Handler(void *param)
{
    GAM_DBG_gpio_write(0);
    irq_disable_all();
    uint32_t diff = timer_dispatch_many();  //-HTIMER-G-G-2022-07-28-----
    timer_set_diff(diff);  //--G-G------
    irq_enable_all();

GAM_DBG_gpio_write(1);
 
}
// DECL_ARMCM_IRQ(SysTick_Handler, SysTick_IRQn);

// Make sure timer_repeat_until doesn't wrap 32bit comparisons
void timer_task(void)
{
    uint32_t now = timer_read_time();
    irq_disable_all();
    if (timer_is_before(timer_repeat_until, now))
        timer_repeat_until = now;
    irq_enable_all();
}
DECL_TASK(timer_task);
