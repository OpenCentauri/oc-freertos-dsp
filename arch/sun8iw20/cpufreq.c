#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/config/core.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/xtruntime.h>


#include <platform.h>
#include <irqs.h>
#include <aw_io.h>
#include <console.h>

#include <sunxi_hal_common.h>
#include <hal_uart.h>
#include <hal_intc.h>
#include <hal_dma.h>
#include <hal_gpio.h>
#include <hal_clk.h>
#include "delay.h"
#include <hal_msgbox.h>
#include <hal_reset.h>
#include <hal_clk.h>


struct dsp_freq {
	const int rate;
	int parent_type;
	int parent_id;
};

struct dsp_freq dsp_freq_table[] = {
	{     32768,	HAL_SUNXI_RTC_CCU,	CLK_OSC32K},
	/* Transition frequency for switching from low to high frequency */
	{  12000000,	HAL_SUNXI_FIXED_CCU,	CLK_SRC_HOSC24M},
	{  16000000,	HAL_SUNXI_RTC_CCU,	CLK_IOSC},
	{  24000000,	HAL_SUNXI_FIXED_CCU,	CLK_SRC_HOSC24M},
	/* when CLK_PLL_PERIPH0_2X is set to 1200000000*/
	/* DSP work 600M max, and 50M/60M/1200M can't be set*/
	/*{  50000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},*/
	/*{  60000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},*/
	{  75000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{  80000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 100000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 120000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 150000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 200000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 240000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 300000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 400000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	{ 600000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},
	/*{1200000000,	HAL_SUNXI_CCU,		CLK_PLL_PERIPH0_2X},*/
};

int get_dsp_freq_table_size(void)
{
	return sizeof(dsp_freq_table)/sizeof(dsp_freq_table[0]);
}

int get_dsp_freq_table_freq(int index)
{
	if (index<0 || index > get_dsp_freq_table_size())
		return -1;

	return dsp_freq_table[index].rate;
}

int dsp_set_freq(int clk_rate)
{
	int ret = 0, i = 0, size = get_dsp_freq_table_size();
	hal_clk_t  clk = NULL;
	hal_clk_t pclk = NULL;
	hal_clk_t sclk = NULL;

	for (i=0; i<size; i++) {
		if (dsp_freq_table[i].rate == clk_rate)
			break;
	}

	/* rate isn't match*/
	if (dsp_freq_table[i].rate == 0)
		return -1;

	/*
	 * When we need to switch the parent clock to CLK_PLL_periph0_2x(1.2G),
	 * we need to switch to 12M first to prevent a high-frequency state of 1.2G from crashing
	 * Because the 32K/24M frequency partition coefficient is 1
	 *
	 * example: 32K/24M -> 1.2G -> 75M/.../600M
	 * fixed:
	 *	32K/24M -> 12M -> 600M -> 75M/.../600M
	 */
	if (clk_rate > 24000000)
		dsp_set_freq(12000000);

	clk = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP);
	if (!clk) {
		ret = -1;
		goto err_get_clk;
	}

	pclk = hal_clock_get(dsp_freq_table[i].parent_type, dsp_freq_table[i].parent_id);
	if (!pclk) {
		ret = -1;
		goto err_get_pclk;
	}


	ret = hal_clk_set_parent(clk, pclk);
	sclk = hal_clk_get_parent(clk);
	if (ret || sclk != pclk) {
		ret = -1;
		goto err_set_parent;
	}

	ret = hal_clk_set_rate(clk, (unsigned int)clk_rate) ;

err_set_parent:
	hal_clock_put(pclk);

err_get_pclk:
	hal_clock_put(clk);

err_get_clk:
	return ret;
}

int  dsp_get_freq(void)
{
	hal_clk_t clk = NULL;
	int       clk_rate = 0;

	clk = hal_clock_get(HAL_SUNXI_CCU, CLK_DSP);
	if (!clk) {
		clk_rate = -1;
		goto err_get_clk;
	}

	clk_rate = (int)hal_clk_get_rate(clk);
	if (clk_rate <= 0) {
		clk_rate = -1;
	}

	hal_clock_put(clk);

err_get_clk:
	return clk_rate;

}

