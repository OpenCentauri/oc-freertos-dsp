#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/config/core.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/xtruntime.h>

#include <platform.h>
#include <irqs.h>
#include <aw_io.h>
#include <console.h>
#include <string.h>
#include <components/aw/linux_debug/debug_common.h>
#include <sunxi_hal_common.h>
#include <hal_uart.h>
#include <hal_intc.h>
#include <hal_dma.h>
#include <hal_gpio.h>
#include <hal_clk.h>
#include <hal_msgbox.h>
#include <hal_reset.h>
#include <hal_clk.h>
#include "spinlock.h"
//#include "xtensa_timer.h"

int32_t console_uart = UART_UNVALID;

static void _cache_config(void) {
	/* 0x0~0x20000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x00000000, 0x20000000, XCHAL_CA_WRITEBACK, 0);
	xthal_set_region_attribute((void *)0x00000000, 0x20000000, XCHAL_CA_BYPASS, 0);

	/* 0x20000000~0x40000000-1 is cacheable */
	xthal_set_region_attribute((void *)0x20000000, 0x20000000, XCHAL_CA_WRITEBACK, 0);

	/* 0x4000000~0x80000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x40000000, 0x40000000, XCHAL_CA_WRITEBACK, 0);
	xthal_set_region_attribute((void *)0x40000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* 0x80000000~0xC0000000-1 is non-cacheable */
	xthal_set_region_attribute((void *)0x80000000, 0x40000000, XCHAL_CA_WRITEBACK, 0);
	xthal_set_region_attribute((void *)0x80000000, 0x40000000, XCHAL_CA_BYPASS, 0);

	/* 0xC0000000~0xFFFFFFFF is  cacheable */
	xthal_set_region_attribute((void *)0xC0000000, 0x40000000, XCHAL_CA_WRITEBACK, 0);

	/* set prefetch level */
	xthal_set_cache_prefetch(XTHAL_PREFETCH_BLOCKS(8) |XTHAL_DCACHE_PREFETCH_HIGH | XTHAL_ICACHE_PREFETCH_HIGH |XTHAL_DCACHE_PREFETCH_L1);
}

static void _console_uart_init() {
	volatile struct spare_rtos_head_t *pstr = platform_head;
	volatile struct dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
	int val = 0;
	val = pdts->uart_msg.status;
	if(val == DTS_OPEN) {
		val = pdts->uart_msg.uart_port;
		hal_uart_init(val);
		console_uart = val;

	}else {
		console_uart = UART_UNVALID;
	}

}

#ifdef CONFIG_LINUX_DEBUG
extern struct dts_sharespace_t dts_sharespace;
static void _sharespace_init() {
	volatile struct spare_rtos_head_t *pstr = platform_head;
	volatile struct dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
	int val = 0;
	val = pdts->dts_sharespace.status;
	if(val == DTS_OPEN) {
		dts_sharespace.dsp_write_addr = pdts->dts_sharespace.dsp_write_addr;
		dts_sharespace.dsp_write_size = pdts->dts_sharespace.dsp_write_size;
		dts_sharespace.arm_write_addr = pdts->dts_sharespace.arm_write_addr;
		dts_sharespace.arm_write_size = pdts->dts_sharespace.arm_write_size;
		dts_sharespace.dsp_log_addr = pdts->dts_sharespace.dsp_log_addr;
		dts_sharespace.dsp_log_size = pdts->dts_sharespace.dsp_log_size;
		debug_common_init();
	}
}
#endif


unsigned int xtbsp_clock_freq_hz(void)
{
	unsigned int dsp_clk_freq_hz = 600000000;//must init value
	int       clk_rate = 0;

	extern int dsp_get_freq(void); /*define by dspfreq.c*/
	clk_rate = dsp_get_freq();

	if (clk_rate <= 0) {
		/* return default freq.*/
		goto err_clk_get;
	}

	dsp_clk_freq_hz = clk_rate;

err_clk_get:
	return dsp_clk_freq_hz;
}

static unsigned int div_of_us_cycle = 0;
extern void     _xt_tick_divisor_init(void);
void arch_freq_update(void)
{
	unsigned int lock;

	div_of_us_cycle = xtbsp_clock_freq_hz() / 1000000;

	/* system tick my use glabal var */
	spin_lock_irqsave(lock);
	_xt_tick_divisor_init();
	spin_unlock_irqrestore(lock);
}

void board_init(void)
{
	/* cache configuration */
	_cache_config();

#ifdef CONFIG_LINUX_DEBUG
	_sharespace_init();
#endif

/*
* All peripherals should be initialized by DSP0.
*/
#ifdef CONFIG_CORE_DSP0
	/* clk init */
	hal_clock_init();

#ifdef CONFIG_DRIVERS_INTC
	/* intc init */
	hal_intc_init(SUNXI_DSP_IRQ_R_INTC);
#endif

#ifdef CONFIG_DRIVERS_GPIO
	/* gpio init */
	hal_gpio_init();
	hal_gpio_r_all_irq_disable();
#endif

#ifdef CONFIG_DRIVERS_DMA
	/* dma init */
	hal_dma_init();
#endif

#ifdef CONFIG_DRIVERS_MSGBOX_AMP
	hal_msgbox_init();
#endif

	/* console uart init */
	_console_uart_init();
#endif /* CONFIG_CORE_DSP0 */

	arch_freq_update();
}
