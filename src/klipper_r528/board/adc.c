
#include "../../include/board/gpio.h" // 
#include "sunxi_hal_gpadc.h"
#include "../../include/printer/basecmd.h"

extern hal_gpadc_t hal_gpadc;
int gpio_adc_setup(struct gpio_adc* g,uint32_t channel)
{
    int ret = hal_gpadc_init();
    if (ret) {
        printf("gpadc init failed!\n");
        return -2;
    }
    hal_gpadc_t *gpadc = &hal_gpadc;
     if(channel >= gpadc->channel_num )
    {
        printf("gpadc init channel %d\n", channel);
        return -1;
    }
    hal_gpadc_channel_init(channel);
    // hal_gpadc_register_callback
    
    g->adc = (void *)gpadc;
    g->chan = channel;
    return 0;
}

int gpio_low_rate_adc_setup(struct gpio_adc* g,uint32_t channel)
{
    int ret = hal_gpadc_init();
    if (ret) {
        printf("gpadc init failed!\n");
         return -2;
    }
    hal_gpadc_t *gpadc = &hal_gpadc;
    if(channel >= gpadc->channel_num )
    {
        printf("gpadc init channel %d\n", channel);
         return -1;
    }
    hal_gpadc_channel_init(channel);
    g->adc = (void *)gpadc;
    g->chan = channel;
    return 0;
}
uint32_t gpio_adc_sample(struct gpio_adc* g)
{
return 0;
    hal_gpadc_t *gpadc = g->adc;

    uint32_t reg_val, reg_low, reg_high;
    uint32_t reg_enable, reg_enable_low, reg_enable_high;
    uint32_t i, data = 0;

    reg_enable = m_gpadc_read_channel_irq_enable(gpadc->reg_base);
    reg_enable_low = m_gpadc_read_channel_lowirq_enable(gpadc->reg_base);
    reg_enable_high = m_gpadc_read_channel_highirq_enable(gpadc->reg_base);

    reg_val = m_gpadc_channel_irq_status(gpadc->reg_base);
    m_gpadc_channel_clear_irq(gpadc->reg_base, reg_val);
    reg_low = m_gpadc_channel_lowirq_status(gpadc->reg_base);
    m_gpadc_channel_clear_lowirq(gpadc->reg_base, reg_val);
    reg_high = m_gpadc_channel_highirq_status(gpadc->reg_base);
    m_gpadc_channel_clear_highirq(gpadc->reg_base, reg_val);
    return 0;
 
    {
        if (reg_low & (1 << g->chan) && reg_enable_low)
        {
            return 0;
        }
        else if (reg_high & (1 << g->chan) && reg_enable_high)
        {
            return 0;
        }
        else 
            return timer_from_us(20);
    }
}

uint16_t gpio_adc_read(struct gpio_adc* g)
{
    // int data = read_gpadc_data(g->adc->reg_base, g->chan);
    // return data;
     return readl((unsigned long)( ((hal_gpadc_t*)(g->adc))->reg_base) + GP_CH0_DATA_REG + 4 * g->chan) & GP_CH_DATA_MASK;
}

int gpio_adc_read_value(struct gpio_adc* g)
{
    // int data = read_gpadc_data(hal_gpadc.reg_base, g->chan);
    // return data;
    return readl((unsigned long)( ((hal_gpadc_t*)(g->adc))->reg_base) + GP_CH0_DATA_REG + 4 * g->chan) & GP_CH_DATA_MASK;
}

void gpio_adc_cancel_sample(struct gpio_adc* g)
{
    // hal_gpadc_channel_exit(g->chan);
    // hal_gpadc_deinit();
}
