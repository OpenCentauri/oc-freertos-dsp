#include <stdint.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/config/core.h>
#include <xtensa/core-macros.h>
#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/xtruntime.h>

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
void board_init(void) {
    _cache_config();
    pintc_regs->enable = 0x0;
    pintc_regs->mask = 0x0;
    pintc_regs->pending = 0xffffffff;

    pintc_regs->enable1 = 0x0;
    pintc_regs->mask1 = 0x0;
    pintc_regs->pending1 = 0xffffffff;

    pintc_regs->enable2 = 0x0;
    pintc_regs->mask2 = 0x0;
    pintc_regs->pending2 = 0xffffffff;
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
