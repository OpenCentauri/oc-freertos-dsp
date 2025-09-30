// Commands for sending messages to a 4-bit hd44780 lcd driver
//
// Copyright (C) 2018  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "../../include/printer/autoconf.h" // 
#include "../../include/printer/basecmd.h" // oid_alloc
#include "../../include/board/gpio.h" // gpio_out_write
#include "../../include/board/irq.h" // irq_disable
#include "../../include/board/misc.h" // timer_from_us
#include "../../include/printer/command.h" // DECL_COMMAND
#include "../../include/printer/scheder.h" // DECL_SHUTDOWN

struct hd44780 {
    uint32_t last_cmd_time, cmd_wait_ticks;
    uint8_t last;
    struct gpio_out rs, e, d4, d5, d6, d7;
};


/****************************************************************
 * Transmit functions
 ****************************************************************/

static uint32_t
nsecs_to_ticks(uint32_t ns)
{
    return timer_from_us(ns * 1000) / 1000000;
}

static inline void ndelay(uint32_t nsecs)
{
    uint32_t end = timer_read_time() + nsecs_to_ticks(nsecs);
    while (timer_is_before(timer_read_time(), end))
        irq_poll();
}

// Write 4 bits to the hd44780 using the 4bit parallel interface
static __always_inline void
hd44780_xmit_bits(uint8_t toggle, struct gpio_out e, struct gpio_out d4
                  , struct gpio_out d5, struct gpio_out d6, struct gpio_out d7)
{
 //---G-G-2022-07-29---
}

// Transmit 8 bits to the chip
static void
hd44780_xmit_byte(struct hd44780 *h, uint8_t data)
{
    struct gpio_out e = h->e, d4 = h->d4, d5 = h->d5, d6 = h->d6, d7 = h->d7;
    hd44780_xmit_bits(h->last ^ data, e, d4, d5, d6, d7);
    h->last = data << 4;
    ndelay(500 - 230);
    hd44780_xmit_bits(data ^ h->last, e, d4, d5, d6, d7);
}

// Transmit a series of bytes to the chip
static void
hd44780_xmit(struct hd44780 *h, uint8_t len, uint8_t *data)
{
    uint32_t last_cmd_time=h->last_cmd_time, cmd_wait_ticks=h->cmd_wait_ticks;
    while (len--) {
        uint8_t b = *data++;
        while (timer_read_time() - last_cmd_time < cmd_wait_ticks)
            irq_poll();
        hd44780_xmit_byte(h, b);
        last_cmd_time = timer_read_time();
    }
    h->last_cmd_time = last_cmd_time;
}


/****************************************************************
 * Interface
 ****************************************************************/

void
command_config_hd44780(uint32_t *args)
{
    struct hd44780 *h = oid_alloc(args[0], command_config_hd44780, sizeof(*h));
    h->rs = gpio_out_setup(args[1], 0);
    h->e = gpio_out_setup(args[2], 0);
    h->d4 = gpio_out_setup(args[3], 0);
    h->d5 = gpio_out_setup(args[4], 0);
    h->d6 = gpio_out_setup(args[5], 0);
    h->d7 = gpio_out_setup(args[6], 0);

    if (!CONFIG_HAVE_STRICT_TIMING) {
        h->cmd_wait_ticks = args[7];
        return;
    }

    // Calibrate cmd_wait_ticks
    irq_disable_all();
    uint32_t start = timer_read_time();
    hd44780_xmit_byte(h, 0);
    uint32_t end = timer_read_time();
    irq_enable_all();
    uint32_t diff = end - start, delay_ticks = args[7];
    if (delay_ticks > diff)
        h->cmd_wait_ticks = delay_ticks - diff;
}
DECL_COMMAND(command_config_hd44780,
             "config_hd44780 oid=%c rs_pin=%u e_pin=%u"
             " d4_pin=%u d5_pin=%u d6_pin=%u d7_pin=%u delay_ticks=%u");

void
command_hd44780_send_cmds(uint32_t *args)
{
//---G-G-2022-07-29---
}
DECL_COMMAND(command_hd44780_send_cmds, "hd44780_send_cmds oid=%c cmds=%*s");

void
command_hd44780_send_data(uint32_t *args)
{
//---G-G-2022-07-29---
}
DECL_COMMAND(command_hd44780_send_data, "hd44780_send_data oid=%c data=%*s");

void
hd44780_shutdown(void)
{
//---G-G-2022-07-29---
}
DECL_SHUTDOWN(hd44780_shutdown);
