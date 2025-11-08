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
    printf("DSP Clock Frequency: %u Hz\n", (unsigned int)xtbsp_clock_freq_hz());
    printf("OEM Header Address: %p\n", (void*)_oemhead_text_start);
    printf("FreeRTOS Tick Rate: %d Hz (1 tick = %d ms)\n", configTICK_RATE_HZ, 1000/configTICK_RATE_HZ);
    printf("Platform: Allwinner R528 SoC\n");
    printf("---------------------------------------------------------------------------\n");
    printf("Initializing shared memory regions...\n\n");
    sharespace_fake_init();
}

/*
 *  Main task function
 */
void vTaskMain(void *pvParameters) {
    (void) pvParameters;

    lprintf("vTaskMain: Task started, entering main loop.\n");
    for(unsigned int i=0;;++i) {
        lprintf("vTaskMain loop: %u\n", i);
        vTaskDelay(1000); // Delay 1000 ticks = 1 second (configTICK_RATE_HZ = 1000)
    }
}

/*
 *  Main function
 */
int main(void) {
    xTaskHandle xHandleTaskMain;

    hw_usleep(200);
    print_banner();
    
    lprintf("main: Starting DSP initialization sequence...\n");

    char* msg = "Hello, World";
    int ret;

    // Initialize the kbuf shared memory communication
    lprintf("main: Calling log_init()...\n");
    log_init();
    lprintf("main: log_init() complete.\n");
    lprintf("This is a test log message from the DSP.");

    /*lprintf("Counting to 15...\n");
    for(unsigned int i=0;i<12;++i) {
        lprintf("%s: %u\n", msg, i);
        hw_usleep(5000);
    }*/

    // Initialize the kbuf shared memory communication
    lprintf("main: Calling sharespace_init()...\n");
    sharespace_init();
    lprintf("main: sharespace_init() complete.\n");

    lprintf("Counting to 15...\n");
    for(unsigned int i=0;i<12;++i) {
        lprintf("%s: %u\n", msg, i);
        hw_usleep(1000);
    }

    lprintf("main: Creating vTaskMain...\n");
    xTaskCreate(vTaskMain, "Task Main", 4096, NULL, 1, &xHandleTaskMain);
    lprintf("main: Calling vTaskStartScheduler...\n");
    printf("vTaskStartScheduler\n");
    vTaskStartScheduler();

    lprintf("main: ERROR - vTaskStartScheduler returned!\n");
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
