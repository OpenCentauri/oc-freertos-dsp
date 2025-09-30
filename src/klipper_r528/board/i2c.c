// I2C functions on stm32
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/board/gpio.h"

struct i2c_config
i2c_setup(uint32_t bus, uint32_t rate, uint8_t addr)
{
    // // Lookup requested i2c bus
    // if (bus >= ARRAY_SIZE(i2c_bus))
    //     shutdown("Unsupported i2c bus");
    // const struct i2c_info *ii = &i2c_bus[bus];
    // I2C_TypeDef *i2c = ii->i2c;

    // if (!is_enabled_pclock((uint32_t)i2c)) {
    //     // Enable i2c clock and gpio
    //     enable_pclock((uint32_t)i2c);
    //     i2c_busy_errata(ii->scl_pin, ii->sda_pin);
    //     gpio_peripheral(ii->scl_pin, GPIO_FUNCTION(4) | GPIO_OPEN_DRAIN, 1);
    //     gpio_peripheral(ii->sda_pin, GPIO_FUNCTION(4) | GPIO_OPEN_DRAIN, 1);
    //     i2c->CR1 = I2C_CR1_SWRST;
    //     i2c->CR1 = 0;

    //     // Set 100Khz frequency and enable
    //     uint32_t pclk = get_pclock_frequency((uint32_t)i2c);
    //     i2c->CR2 = pclk / 1000000;
    //     i2c->CCR = pclk / 100000 / 2;
    //     i2c->TRISE = (pclk / 1000000) + 1;
    //     i2c->CR1 = I2C_CR1_PE;
    // }

    // return (struct i2c_config){ .i2c=i2c, .addr=addr<<1 };
}


void
i2c_write(struct i2c_config config, uint8_t write_len, uint8_t *write)
{
    // I2C_TypeDef *i2c = config.i2c;
    // uint32_t timeout = timer_read_time() + timer_from_us(5000);

    // i2c_start(i2c, config.addr, write_len, timeout);
    // while (write_len--)
    //     i2c_send_byte(i2c, *write++, timeout);
    // i2c_stop(i2c, timeout);
}

void
i2c_read(struct i2c_config config, uint8_t reg_len, uint8_t *reg
         , uint8_t read_len, uint8_t *read)
{
    // I2C_TypeDef *i2c = config.i2c;
    // uint32_t timeout = timer_read_time() + timer_from_us(5000);
    // uint8_t addr = config.addr | 0x01;

    // if (reg_len) {
    //     // write the register
    //     i2c_start(i2c, config.addr, reg_len, timeout);
    //     while(reg_len--)
    //         i2c_send_byte(i2c, *reg++, timeout);
    // }
    // // start/re-start and read data
    // i2c_start(i2c, addr, read_len, timeout);
    // while(read_len--) {
    //     *read = i2c_read_byte(i2c, timeout, read_len);
    //     read++;
    // }
    // i2c_wait(i2c, 0, I2C_SR1_RXNE, timeout);
}
