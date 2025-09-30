#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <hal_log.h>
#include <hal_cmd.h>
#include <hal_timer.h>
#include <hal_uart.h>



int do_read_uart(uint8_t* buf, uint32_t size)
{
    hal_uart_receive_no_block(UART_1, buf, size,0);
    GAM_DEBUG_printf_HEX(0,buf,size);
}