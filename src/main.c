#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/config/core.h>
#include <xtensa/core-macros.h>
#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/xtruntime.h>
#include <xtensa_api.h>

#include "FreeRTOS.h"
#include "platform.h"
#include "task.h"

/*
 *  External function prototypes
 */
extern int linpack_main(void);
extern int dhry_main(int t);
extern void coremark_main(void);

/*
 *  Main task function
 */
extern void vPortDumpTimerStatus(void);

void vTaskMain(void *pvParameters) {
    (void) pvParameters;
    printf("DSP_log: vTaskMain started\n");
    dsp_msgbox_init(0x00);
    printf("DSP_log: vTaskMain loop: 0\n");
    vPortDumpTimerStatus();
    vTaskDelay(500);
    printf("DSP_log: vTaskMain loop: 1 (after first delay)\n");
    dhry_main(10000000);
    linpack_main();
    coremark_main();
    uint32_t sdata = 0;
    char* msg = "Hello, World!\n";
    while (1) {
        dsp_msgbox_channel_send(0x02, (uint8_t *)msg, strlen(msg));
        //printf("task led run on task\n");
        sdata++;
        vTaskDelay(500);
    }
}

/*
 *  Function to print banner
 */
void print_banner(void) {
    printf("\n\n");
    printf("  ___                    ____           _                   _              \n");
    printf(" / _ \\ _ __   ___ _ __  / ___|___ _ __ | |_ __ _ _   _ _ __(_)            \n");
    printf("| | | | '_ \\ / _ \\ '_ \\| |   / _ \\ '_ \\| __/ _` | | | | '__| |        \n");
    printf("| |_| | |_) |  __/ | | | |__|  __/ | | | || (_| | |_| | |  | |             \n");
    printf(" \\___/| .__/ \\___|_| |_|\\____\\___|_| |_|\\__\\__,_|\\__,_|_|  |_|      \n");
    printf("      |_|                                                                  \n");
    printf("===========================================================================\n");
    printf(" OpenCentauri FreeRTOS for HIFI4 DSP v0.0.0, Build on xtensa-hifi4-elf-gcc \n");
    printf("===========================================================================\n");
}

/*
 *  Main function
 */
int main(void) {
    xTaskHandle xHandleTaskMain;

    print_banner();

    xTaskCreate(vTaskMain, "Task Main", 4096, NULL, 1, &xHandleTaskMain);
    printf("vTaskStartScheduler\n");
    vTaskStartScheduler();

    printf("vTaskStartScheduler FAILED!\n");
    return 1;
}
