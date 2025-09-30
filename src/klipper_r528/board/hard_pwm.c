// Hardware PWM support on stm32
//
// Copyright (C) 2021  Michael Kurz <michi.kurz@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/board/irq.h" // irq_save
#include "../../include/printer/command.h" // shutdown
#include "../../include/board/gpio.h" // gpio_pwm_write
#include "../../include/printer/scheder.h" // sched_shutdown
#define MAX_PWM 255
DECL_CONSTANT("PWM_MAX", MAX_PWM);

struct gpio_pwm_info {
    // TIM_TypeDef* timer;
    // uint8_t pin, channel, function;
};


struct gpio_pwm
gpio_pwm_setup(uint8_t port, uint32_t cycle_time, uint8_t val){

    // struct pwm_config *config;
    // printf("port = %d", port);

    // config = (struct pwm_config *)malloc(sizeof(struct pwm_config));
    // if(config == NULL)
    // {
    //     GAM_DEBUG_errer("-ERR malloc:%d--\n",len);
    //     return;
    // }
    // config->duty_ns   = val;
    // config->period_ns = cycle_time;
    // config->polarity  = PWM_POLARITY_NORMAL;
    // printf("duty_ns = %d \n", config->duty_ns);
    // printf("period_ns = %d \n", config->period_ns);
    // printf("polarity = %d \n", config->polarity);

    // hal_pwm_init();
    // hal_pwm_control(port, config);
    // struct gpio_pwm g;
    // return g;

}

void
gpio_pwm_write(struct gpio_pwm g, uint32_t val) {
    // *(volatile uint32_t*) g.reg = val;
}
