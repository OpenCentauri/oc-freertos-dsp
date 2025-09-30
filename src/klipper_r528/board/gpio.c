// GPIO functions on stm32f4
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // ffs
#include "../../include/board/irq.h" // irq_save
#include "../../include/printer/command.h" // DECL_ENUMERATION_RANGE
#include "../../include/board/gpio.h" // gpio_out_setup
#include "../../include/printer/scheder.h" // sched_shutdown
#include "../../include/printer/basecmd.h"
#include "../../include/board/debug.h"


#define GPIOA_BASE            (SUNXI_GPIO_PBASE + 0x00000000)
#define GPIOB_BASE            (SUNXI_GPIO_PBASE + BANK_MEM_SIZE)
#define GPIOC_BASE            (GPIOB_BASE + BANK_MEM_SIZE)
#define GPIOD_BASE            (GPIOC_BASE + BANK_MEM_SIZE)
#define GPIOE_BASE            (GPIOD_BASE + BANK_MEM_SIZE)
#define GPIOF_BASE            (GPIOE_BASE + BANK_MEM_SIZE)
#define GPIOG_BASE            (GPIOF_BASE + BANK_MEM_SIZE)

#define GPIOA_REGS               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB_REGS               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC_REGS               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD_REGS               ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE_REGS               ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF_REGS               ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG_REGS               ((GPIO_TypeDef *) GPIOG_BASE)
GPIO_TypeDef * const digital_regs[] = {
    ['A' - 'A'] = GPIOA_REGS, GPIOB_REGS, GPIOC_REGS,
    ['D' - 'A'] = GPIOD_REGS,
    ['E' - 'A'] = GPIOE_REGS,
    ['F' - 'A'] = GPIOF_REGS,
    ['G' - 'A'] = GPIOG_REGS,
};


#define GPIO2PORT(PIN) ((PIN) / PINS_PER_BANK)
#define GPIO2BIT(PIN) (1<<((PIN) % PINS_PER_BANK))
  struct gpio_out g_GAM_DBG_gpio1;
   struct gpio_out g_GAM_DBG_gpio2;
   struct gpio_out g_GAM_DBG_gpio3;
   struct gpio_out g_GAM_DBG_gpio4;

int hal_gpio_pinmux_set_function(gpio_pin_t pin, gpio_muxsel_t function_index)
{
    int ret = 0;

   if (pin >= BANK_BOUNDARY)       //224
    {
        shutdown("Not an output pin");
    }
    GPIO_TypeDef *regs = digital_regs[GPIO2PORT(pin)];
    uint32_t offset = (pin >> MUX_OFFSET_BITS)&MUX_OFFSET_MASK;
    uint32_t shift = (pin & MUX_SHIFT_MASK) << MUX_SHIFT_BITS;
    uint32_t reg = regs->AFR[ offset];
    reg  &= ~(MUX_PINS_MASK << shift);
    regs->AFR[ offset] = reg | (function_index << shift) ;
    return ret;
}


struct gpio_out gpio_out_setup(gpio_pin_t pin, gpio_data_t data)
{
    if (pin >= BANK_BOUNDARY)       //224
    {
        shutdown("Not an output pin");
    }
    GPIO_TypeDef *regs = digital_regs[GPIO2PORT(pin)];
    struct gpio_out g = {regs, GPIO2BIT(pin),pin&PINS_BANK_MASK };
    gpio_out_reset(&g, data);
    return g; 
}

void gpio_out_reset(struct gpio_out *g, uint32_t data)
{
    irqstatus_t flag = irq_save();
    GPIO_TypeDef *regs = g->regs;
    // hal_gpio_set_direction_cbd(g, GPIO_DIRECTION_OUTPUT);//GPIO_TYPE_FUNC  GPIO_MUXSEL_OUT
    uint32_t offset = (g->pin >> MUX_OFFSET_BITS)&MUX_OFFSET_MASK;
    uint32_t shift = (g->pin & MUX_SHIFT_MASK) << MUX_SHIFT_BITS;
    uint32_t reg = regs->AFR[ offset];
    reg  &= ~(MUX_PINS_MASK << shift);
    regs->AFR[ offset] = reg | (GPIO_MUXSEL_OUT << shift) ;       //GPIO_MUXSEL_OUT  GPIO_DIRECTION_OUTPUT

    offset = (g->pin >> PULL_OFFSET_BITS)&PULL_OFFSET_MASK;
    shift = (g->pin & PULL_SHIFT_MASK) << PULL_SHIFT_BITS;
    reg = regs->PUPDR[offset] & (~(PULL_PINS_MASK << shift));
    regs->PUPDR[ offset] = reg | (GPIO_PULL_UP << shift) ;        //0 GPIO_PULL_DOWN_DISABLED GPIO_PULL_UP  GPIO_PULL_DOWN

    if(data)
        regs->ODR |= g->bit;
    else
         regs->ODR &= ~(g->bit);
    //  g->gpio_cur_data = data;
    
    irq_restore(flag);
}

void gpio_out_toggle_noirq(struct gpio_out* g)
{
    // gpio_data_t data = (g->gpio_cur_data)?0:1;
    // hal_gpio_set_data_cbd(g->pin, data);
    // g->gpio_cur_data = data;

    // hal_gpio_out_toggle_noirq_cbd(g);
   GPIO_TypeDef *regs = g->regs;
    regs->ODR ^= g->bit;

}

void gpio_out_toggle(struct gpio_out *g)
{
    irqstatus_t flag = irq_save();
    gpio_out_toggle_noirq(g);
    irq_restore(flag);
}

void gpio_out_write(struct gpio_out* g, gpio_data_t data)
{
    // hal_gpio_set_data_cbd(g->pin, data);
    GPIO_TypeDef *regs = g->regs;
    if(data)
        regs->ODR |= g->bit;
    else
         regs->ODR &= ~(g->bit);
    // g->gpio_cur_data = data;
}

struct gpio_in gpio_in_setup(gpio_pin_t pin, int32_t pull)
{
    if (pin >= BANK_BOUNDARY)       //224
    {
        shutdown("Not an output pin");
    }
    gpio_pull_status_t pull_up = pull ? (pull > 0 ? GPIO_PULL_UP : GPIO_PULL_DOWN) : GPIO_PULL_DOWN_DISABLED;
    GPIO_TypeDef *regs = digital_regs[GPIO2PORT(pin)];
    struct gpio_in g =  {regs, GPIO2BIT(pin),pin&PINS_BANK_MASK };
    gpio_in_reset(&g, pull_up);
    return g;
}

void gpio_in_reset(struct gpio_in *g, gpio_pull_status_t pull_up)
{
    irqstatus_t flag = irq_save();
    // hal_gpio_set_direction_cbd(g, GPIO_MUXSEL_IN);//GPIO_MUXSEL_IN  GPIO_DIRECTION_INPUT
    // hal_gpio_set_pull_cbd(g, pull_up);
    gpio_peripheral(g, GPIO_MUXSEL_IN, pull_up);
    irq_restore(flag);
}

uint8_t gpio_in_read(struct gpio_in *g)
{
    
    GPIO_TypeDef *regs = g->regs;
    return !!(regs->ODR & g->bit);  //
}









