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

    lprintf("Bob Dole Lives!!!\n");
    for(unsigned int i=0;;++i) {
        lprintf("vTaskMain loop: %u\n", i);
        hw_usleep(1000);
        //vTaskDelay(1000);
    }
}

/*
 *  Main function
 */
int main(void) {
    xTaskHandle xHandleTaskMain;
    int ret;

    hw_usleep(200);
    print_banner();


    // Initialize the kbuf shared memory communication
    log_init();
    lprintf("This is a test log message from the DSP.");

    /*lprintf("Counting to 15...\n");
    for(unsigned int i=0;i<12;++i) {
        lprintf("%s: %u\n", msg, i);
        hw_usleep(5000);
    }*/

    // Initialize the kbuf shared memory communication
    sharespace_init();

    lprintf("Counting to 5...\n");
    for(unsigned int i=0;i<5;++i) {
        lprintf("Hello, World!: %u\n", i);
        hw_usleep(1000);
    }

    char buf[4096];

    for (int b = 0; b < 5; b++)
    {
        char* test_message = "Hello, world! %d";

        sprintf(buf, test_message, b);

        sharespace_write(buf, strlen(buf) + 1);
        sharespace_read(buf, 4096);
        hw_usleep(1000);
    }

    for (;;)
    {
        hw_usleep(1000);
        int read_len = sharespace_read(buf, 4096);
        if (read_len <= 0)
        {
            lprintf("Waiting...\n");
            continue;
        }

        sharespace_write(buf, read_len);
    }

    // Test out the DSP/ARM comms, wait for message and respond!
    int read_len;
    
    int buf_len = sizeof(buf);
    char buf2[buf_len * 2];
    int buf2_len;
    for(unsigned int i=0;;++i) {
        lprintf("Looping...\n");
        hw_usleep(1000);
        read_len = sharespace_read(buf, buf_len);
        lprintf("Read message #%u (bytes=%d, len=%d): %s\n", i, read_len, strlen(buf), buf);
        snprintf(buf2, buf_len, "ALLO GOVNAH %u: %s", i, buf);
        buf2_len=strlen(buf2)+1;
        lprintf("Writing message #%u (bytes=%d, len=%d): %s\n", i, buf2_len, strlen(buf2), buf2);
        sharespace_write(buf2, buf2_len);
    }

    lprintf("Broken free!\n");
    hw_usleep(1000);

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
