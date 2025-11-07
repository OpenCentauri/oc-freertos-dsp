#include <stdint.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/config/core.h>
#include <xtensa/core-macros.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/tie/xt_interrupt.h>
#include <xtensa/tie/xt_timer.h>
#include <xtensa/xtruntime.h>
#include <xtensa/hal.h>

#include "xtensa_api.h"
#include "xtensa_timer.h"
#include "platform.h"

static void _cache_config(void) {
    /* 0x0~0x20000000-1 is non-cacheable */
    xthal_set_region_attribute((void *) 0x00000000, 0x20000000, XCHAL_CA_WRITEBACK,
                               0);
    xthal_set_region_attribute((void *) 0x00000000, 0x20000000, XCHAL_CA_BYPASS,
                               0);

    /* 0x20000000~0x40000000-1 is cacheable */
    xthal_set_region_attribute((void *) 0x20000000, 0x20000000, XCHAL_CA_WRITEBACK,
                               0);

    /* 0x4000000~0x80000000-1 is non-cacheable */
    xthal_set_region_attribute((void *) 0x40000000, 0x40000000, XCHAL_CA_WRITEBACK,
                               0);
    xthal_set_region_attribute((void *) 0x40000000, 0x40000000, XCHAL_CA_BYPASS,
                               0);

    /* 0x80000000~0xC0000000-1 is non-cacheable */
    xthal_set_region_attribute((void *) 0x80000000, 0x40000000, XCHAL_CA_WRITEBACK,
                               0);
    xthal_set_region_attribute((void *) 0x80000000, 0x40000000, XCHAL_CA_BYPASS,
                               0);

    /* 0xC0000000~0xFFFFFFFF is  cacheable */
    xthal_set_region_attribute((void *) 0xC0000000, 0x40000000, XCHAL_CA_WRITEBACK,
                               0);

    /* set prefetch level */
    xthal_set_cache_prefetch(XTHAL_PREFETCH_BLOCKS(8) | XTHAL_DCACHE_PREFETCH_HIGH | XTHAL_ICACHE_PREFETCH_HIGH | XTHAL_DCACHE_PREFETCH_L1);
}

void _exit(int no) {
    (void) no;
    while (1)
        ;
}

#define SUNXI_DSP_IRQ_NMI 0
#define SUNXI_DSP_IRQ_DSP_TIMER1 1
#define SUNXI_DSP_IRQ_DSP_TIMER0 2

#define RINTC_IRQ_MASK 0xffff0000

#define SUNXI_R_INTC_PBASE (0x01700800)

#define SUNXI_DSP_IRQ_R_INTC 20

#define SUNXI_RINTC_IRQ_NMI (RINTC_IRQ_MASK | 0) /* not use */
#define SUNXI_RINTC_IRQ_SOURCE_MAX 88            /* FIXME: can be decreased */

struct intc_regs {
    /*offset 0x00 */
    volatile uint32_t vector;
    volatile uint32_t base_addr;
    volatile uint32_t reserved0;
    volatile uint32_t control;

    /*offset 0x10 */
    volatile uint32_t pending;
    volatile uint32_t pending1;
    volatile uint32_t pending2;
    volatile uint32_t reserved1[9];

    /*offset 0x40 */
    volatile uint32_t enable;
    volatile uint32_t enable1;
    volatile uint32_t enable2;
    volatile uint32_t reserved2[1];

    /*offset 0x50 */
    volatile uint32_t mask;
    volatile uint32_t mask1;
    volatile uint32_t mask2;
    volatile uint32_t reserved3[5];

    /*offset 0x70 */
    volatile uint32_t fast_forcing;
    volatile uint32_t reserved4[3];

    /*offset 0x80 */
    volatile uint32_t priority0;
    volatile uint32_t priority1;
    volatile uint32_t reserved5[14];

    /*offset 0xc0 */
    volatile uint32_t group_config0;
    volatile uint32_t group_config1;
    volatile uint32_t group_config2;
    volatile uint32_t group_config3;
};

static volatile struct intc_regs *(pintc_regs) = (volatile struct intc_regs *)
        SUNXI_R_INTC_PBASE;

#define writel(v, a) (*(volatile unsigned int *)(a) = (v))
#define readl(a) (*(volatile unsigned int *)(a))

#define SUNXI_CCU_BASE      0x02001000
#define SUNXI_R_CCU_BASE    0x07010000
#define SUNXI_RTC_CCU_REG   0x07090000

/* Main CCU registers */
#define SUN8IW20_PLL_CPUX_REG       0x000
#define SUN8IW20_PLL_DDR0_REG       0x010
#define SUN8IW20_PLL_PERIPH0_REG    0x020
#define SUN8IW20_PLL_VIDEO0_REG     0x040
#define SUN8IW20_PLL_VIDEO1_REG     0x048
#define SUN8IW20_PLL_VE_REG         0x058
#define SUN8IW20_PLL_AUDIO0_REG     0x078
#define SUN8IW20_PLL_AUDIO1_REG     0x080
#define SUN8IW20_USB0_CLK_REG       0xa70
#define SUN8IW20_USB1_CLK_REG       0xa74
#define DSP_CLK_REG                 0xc70

/* RTC CCU registers */
#define LOSC_CTRL_REG               0x00
#define KEY_FIELD_MAGIC_NUM_RTC     0x16AA0000
#define LOSC_OUT_GATING_REG         0x60
#define XO_CTRL_REG                 0x160

#define BIT(nr)         (1UL << (nr))
#define GENMASK(h, l) \
    (((~0UL) << (l)) & (~0UL >> (32 - 1 - (h))))

static void set_reg_key(unsigned long addr, unsigned int key,
                               unsigned int kstart, unsigned int klen,
                               unsigned int val, unsigned int vstart,
                               unsigned int vlen)
{
    unsigned int temp;

    temp = readl(addr);
    temp &= ~(((1 << klen) - 1) << kstart);
    temp &= ~(((1 << vlen) - 1) << vstart);
    temp |= (key << kstart);
    temp |= (val << vstart);
    writel(temp, addr);
}

static void clock_init(void)
{
    volatile uint32_t val;
    int i;

    /* This is based on sunxi_ccu_init from rtos-hal */

    /* Enable the lock bits on all Plls */
    const uint32_t pll_regs[] = {
        SUN8IW20_PLL_CPUX_REG, SUN8IW20_PLL_DDR0_REG,
        SUN8IW20_PLL_PERIPH0_REG, SUN8IW20_PLL_VIDEO0_REG,
        SUN8IW20_PLL_VIDEO1_REG, SUN8IW20_PLL_VE_REG,
        SUN8IW20_PLL_AUDIO0_REG, SUN8IW20_PLL_AUDIO1_REG,
    };

    for (i = 0; i < sizeof(pll_regs)/sizeof(pll_regs[0]); i++) {
        val = readl(SUNXI_CCU_BASE + pll_regs[i]);
        val |= BIT(29);
        writel(val, SUNXI_CCU_BASE + pll_regs[i]);
    }

    /* This is based on sunxi_rtc_ccu_init and clock_source_init from rtos-hal */
    /* (1) enable DCXO */
    val = readl(SUNXI_RTC_CCU_REG + XO_CTRL_REG);
    val |= (1 << 1);
    writel(val, SUNXI_RTC_CCU_REG + XO_CTRL_REG);

    /* (2) enable auto switch function */
    set_reg_key(SUNXI_RTC_CCU_REG + LOSC_CTRL_REG,
                KEY_FIELD_MAGIC_NUM_RTC >> 16, 16, 16,
                0x1, 2, 14);

    /* (3) set the parent of osc32k-sys to ext-osc32k */
    set_reg_key(SUNXI_RTC_CCU_REG + LOSC_CTRL_REG,
                KEY_FIELD_MAGIC_NUM_RTC >> 16, 16, 16,
                0x1, 1, 0);

    /* (4) set the parent of osc32k-out to osc32k-sys */
    val = readl(SUNXI_RTC_CCU_REG + LOSC_OUT_GATING_REG);
    val &= ~GENMASK(1, 0);
    writel(val, SUNXI_RTC_CCU_REG + LOSC_OUT_GATING_REG);

    /* Configure DSP clock to 600MHz */
    /* Set parent to pll-periph0-2x (1200MHz) and divider to 2 */
    val = (1 << 31) | (3 << 24) | (1 << 0);
    writel(val, SUNXI_CCU_BASE + DSP_CLK_REG);
}

void board_init(void) {
    _cache_config();

    /*pintc_regs->enable = 0x0;
    pintc_regs->mask = 0x0;
    pintc_regs->pending = 0xffffffff;

    pintc_regs->enable1 = 0x0;
    pintc_regs->mask1 = 0x0;
    pintc_regs->pending1 = 0xffffffff;

    pintc_regs->enable2 = 0x0;
    pintc_regs->mask2 = 0x0;
    pintc_regs->pending2 = 0xffffffff;*/

    //clock_init();

    //xt_ints_on(XT_TIMER_INTEN);
}

int outbyte(char c) {
    while ((readl(SUNXI_UART0_BA + UART_LSR) & (1 << 5)) == 0)
        ;
    writel(SUNXI_UART0_BA + UART_THR, c);
    return 0;
}

// This actually is right, the function below querying registers returns the same thing...
uint32_t xtbsp_clock_freq_hz(void) { return 600000000; }
/*
static volatile uint32_t* const dspclkreg = (volatile uint32_t*)SUNXI_DSP_CLK_REG;
uint32_t xtbsp_clock_freq_hz(void)
{
    uint32_t val = *dspclkreg;
    uint32_t src = (val >> 24) & 0x7;
    uint32_t m = ((val >> 0) & 0x1F) + 1;
    uint32_t n = ((val >> 8) & 0x3) + 1;

    uint32_t freq = 0;

    switch (src) {
        case 0: // HOSC - 24MHz
            freq = SUNXI_HOSC_FREQ / m / n;
            break;
        case 1: // CLK32K - 32kHz
            freq = 32768 / m / n;
            break;
        case 2: // RC16M - 16MHz
            freq = 16000000UL / m / n;
            break;
        case 3: // PLLPERI2X (usually 1200 MHz)
            // Read PLL registers and calculate actual
            // For now, assume 1200 MHz
            freq = 1200000000UL / m / n;
            break;
        case 4: // PLLAUDIO1DIV2 (divided PLLAUDIO1)
            // Read PLLAUDIO1CTRLREG for actual value.
            // For now, assume 1536000000 / 2 = 768 MHz
            freq = 768000000UL / m / n;
            break;
        default:
            freq = 0; // Unknown/invalid source
    }

    return freq;
}
*/

uint64_t xbsp_get_ccount(void) {
    static uint64_t cnt = 0;
    static unsigned last_ccount = 0;
    unsigned ccount = XTHAL_GET_CCOUNT();

    if (ccount < last_ccount)
        cnt = cnt + 0xffffffff;
    cnt = cnt + ccount;

    last_ccount = ccount;
    return cnt;
}
