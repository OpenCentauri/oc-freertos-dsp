#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//#include <sys/unistd.h>
#include <time.h>
#include <xtensa/config/core-matmap.h>
#include <xtensa/config/core.h>
#include <xtensa/core-macros.h>
#include <xtensa/hal.h>
#include <xtensa/tie/xt_externalregisters.h>
#include <xtensa/xtruntime.h>
#include <xtensa_api.h>

#include "FreeRTOS.h"
#include "platform.h"
#include "sharespace.h"
#include "task.h"
#include "log.h"

/*
 *  External function prototypes
 */
extern int linpack_main(void);
extern int dhry_main(int t);
extern void coremark_main(void);
extern void vPortDumpTimerStatus(void);

/*
 *  Function to print banner
 */
void print_banner(void) {
    // Print the banner
    printf("                                                                           \n");
    printf("  ___                    ____           _                   _              \n");
    printf(" / _ \\ _ __   ___ _ __  / ___|___ _ __ | |_ __ _ _   _ _ __(_)            \n");
    printf("| | | | '_ \\ / _ \\ '_ \\| |   / _ \\ '_ \\| __/ _` | | | | '__| |        \n");
    printf("| |_| | |_) |  __/ | | | |__|  __/ | | | || (_| | |_| | |  | |             \n");
    printf(" \\___/| .__/ \\___|_| |_|\\____\\___|_| |_|\\__\\__,_|\\__,_|_|  |_|      \n");
    printf("      |_|                                                                  \n");
    printf("===========================================================================\n");
    printf(" OpenCentauri FreeRTOS for HIFI4 DSP v0.0.0, Build on xtensa-hifi4-elf-gcc \n");
    printf("===========================================================================\n");
    // Print the address stored IN the pointer (the address of myVariable)
    printf("Call xtbsp_clock_freq_hertz: %u Hz\n", (unsigned int)xtbsp_clock_freq_hz());
    printf("Address of *_oemhead_text_start: %p\n\n", (void*)_oemhead_text_start);
    sharespace_fake_init();
}

/*
 *  Main task function
 */
void vTaskMain(void *pvParameters) {
    (void) pvParameters;

    //dsp_msgbox_init(0x00);
    printf("DSP_log: vTaskMain loop: 0\n");
    vPortDumpTimerStatus();
    vTaskDelay(1000);
    printf("DSP_log: vTaskMain loop: 1 (after first delay)\n");
    //printf("Bob Dole Lives!!!\n");
    for(unsigned int i=2;;++i) {
        lprintf("vTaskMain loop: %u\n", i);
        vTaskDelay(1000);
    }
}

/*
 *  Main function
 */
int main(void) {
    xTaskHandle xHandleTaskMain;

    hw_usleep(200);
    print_banner();

    char* msg = "Hello, World";
    int ret;

    // Initialize the kbuf shared memory communication
    log_init();
    lprintf("This is a test log message from the DSP.");

    //dsp_msgbox_init(0x00);

    /*lprintf("Counting to 15...\n");
    for(unsigned int i=0;i<12;++i) {
        lprintf("%s: %u\n", msg, i);
        hw_usleep(5000);
    }*/

    // Initialize the kbuf shared memory communication
    sharespace_init();

    /*lprintf("Counting to 15...\n");
    for(unsigned int i=0;i<12;++i) {
        lprintf("%s: %u\n", msg, i);
        hw_usleep(1000);
    }*/

    xTaskCreate(vTaskMain, "Task Main", 4096, NULL, 1, &xHandleTaskMain);
    printf("vTaskStartScheduler\n");
    vTaskStartScheduler();

    printf("vTaskStartScheduler FAILED!\n");
/*
    // Initialize the shared memory communication
    //sharespace_init();
    const int maxsize = 1024;
    char outbuf[maxsize];

    char* msg = "Hello, World";
    int ret;

    // Initialize the shared memory communication
    sharespace_init();

    for(unsigned int i=0;;++i) {
        snprintf(outbuf, maxsize, "%s: %u\n", msg, i);
        ret = sharespace_write(outbuf, strlen(outbuf)+1);
        for(unsigned int j=2;j>=2;++j) ;
        // Delay until overflow of unsigned int, then continue
    }
*/
/*
    print_banner();

    xTaskCreate(vTaskMain, "Task Main", 4096, NULL, 1, &xHandleTaskMain);
    printf("vTaskStartScheduler\n");
    vTaskStartScheduler();

    printf("vTaskStartScheduler FAILED!\n");
*/
    return 1;
}
