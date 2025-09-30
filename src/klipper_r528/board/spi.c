// SPI functions on STM32
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/board/gpio.h"


struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
    struct spi_config g;
    hal_spi_master_port_t  port;
    hal_spi_master_config_t cfg;

    cfg.clock_frequency = 5000000;
    cfg.slave_port = HAL_SPI_MASTER_SLAVE_0;
    cfg.cpha = HAL_SPI_MASTER_CLOCK_PHASE0;
    cfg.cpol = HAL_SPI_MASTER_CLOCK_POLARITY0;
    // hal_spi_init(port, &cfg);      //---G-G-2022-08-09---
    
    g.cfg = cfg;
    g.port = port;

    return g;
}

void
spi_prepare(struct spi_config config)
{
    // SPI_TypeDef *spi = config.spi;
    // spi->CR1 = config.spi_cr1;
}

void
spi_transfer(struct spi_config config, uint8_t receive_data,
             uint8_t len, uint8_t *data)
{
    // hal_spi_write(config.port, len, data);           //---G-G-2022-08-09---
    // hal_spi_read(config.port, len, receive_data);
}
