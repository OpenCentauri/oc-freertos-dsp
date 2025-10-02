#include <sunxi_hal_common.h>
#include <hal_uart.h>
#include <string.h>
#include <platform.h>
#include <script.h>
#include <hal_cfg.h>

int32_t Hal_Cfg_GetGPIOSecKeyCount(char *GPIOSecName)
{
    int32_t i = 0;
    char name[20];

    /* uart */
    for (i = 0; i < UART_MAX; i++) {
        memset(name, 0, sizeof(name));
        sprintf(name, "uart%d", i);
        if (strcmp(name, GPIOSecName) == 0)
            return 2;
    }

    return -1;
}

int32_t Hal_Cfg_GetGPIOSecData(char *GPIOSecName, void *pGPIOCfg, int32_t GPIONum)
{
    volatile struct spare_rtos_head_t *pstr = platform_head;
    volatile struct dts_msg_t *pdts = &pstr->rtos_img_hdr.dts_msg;
    volatile user_gpio_set_t* pcfg = (user_gpio_set_t*)pGPIOCfg;
    int32_t i = 0, j = 0;
    char name[20];
    /* uart */
    for (i = 0; i < UART_MAX; i++) {
        memset(name,0,sizeof(name));
        sprintf(name, "uart%d", i);
        if (strcmp(name, GPIOSecName) == 0) {
            for (j = 0; j < GPIONum; j++)
            {
                pcfg[j].port = pdts->uart_msg.uart_pin_msg[j].port;
                pcfg[j].port_num = pdts->uart_msg.uart_pin_msg[j].port_num;
                pcfg[j].mul_sel = pdts->uart_msg.uart_pin_msg[j].mul_sel;
            }

            return 1;
        }
    }

    return -1;
}

