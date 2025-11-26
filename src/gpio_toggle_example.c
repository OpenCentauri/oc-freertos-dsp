/*
 * GPIO Toggle Example
 * Toggles GPIO pin PE12 (gpio140) every second
 */

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "hal_gpio.h"
#include "hal_timer.h"

/* Define the GPIO pin - PE12 is gpio 140 */
#define LED_PIN     GPIOE(12)   /* PE12 = 128 + 12 = 140 */

/*
 * GPIO toggle task
 * This task runs in a loop toggling the GPIO pin every second
 */
static void gpio_toggle_task(void *pvParameters)
{
    gpio_data_t state = GPIO_DATA_LOW;
    
    printf("GPIO Toggle Task Started - Toggling PE12 (gpio140) every second\n");
    
    while (1) {
        /* Toggle the GPIO state */
        state = (state == GPIO_DATA_LOW) ? GPIO_DATA_HIGH : GPIO_DATA_LOW;
        
        /* Set the GPIO output */
        if (hal_gpio_set_data(LED_PIN, state) == 0) {
            printf("GPIO PE12 set to %s\n", state ? "HIGH" : "LOW");
        } else {
            printf("Error setting GPIO PE12\n");
        }
        
        /* Delay for 1 second */
        hal_sleep(1);
    }
}

/*
 * Initialize and start the GPIO toggle example
 */
void gpio_toggle_example_init(void)
{
    int ret;
    
    printf("Initializing GPIO toggle example for PE12 (gpio140)\n");
    
    /* Initialize GPIO subsystem */
    ret = hal_gpio_init();
    if (ret != 0) {
        printf("Failed to initialize GPIO subsystem\n");
        return;
    }
    
    /* Configure PE12 as output */
    ret = hal_gpio_pinmux_set_function(LED_PIN, GPIO_MUXSEL_OUT);
    if (ret != 0) {
        printf("Failed to set PE12 pinmux to output\n");
        return;
    }
    
    ret = hal_gpio_set_direction(LED_PIN, GPIO_DIRECTION_OUTPUT);
    if (ret != 0) {
        printf("Failed to set PE12 direction to output\n");
        return;
    }
    
    /* Set initial state to LOW */
    hal_gpio_set_data(LED_PIN, GPIO_DATA_LOW);
    
    /* Create the toggle task */
    if (xTaskCreate(gpio_toggle_task, 
                    "GPIO_Toggle",
                    configMINIMAL_STACK_SIZE * 2,
                    NULL,
                    tskIDLE_PRIORITY + 1,
                    NULL) != pdPASS) {
        printf("Failed to create GPIO toggle task\n");
        return;
    }
    
    printf("GPIO toggle task created successfully\n");
}

/*
 * Simple polling version (no FreeRTOS task)
 * Call this function in a loop or from another task
 */
void gpio_toggle_once(void)
{
    static gpio_data_t state = GPIO_DATA_LOW;
    static int initialized = 0;
    
    /* Initialize on first call */
    if (!initialized) {
        hal_gpio_init();
        hal_gpio_pinmux_set_function(LED_PIN, GPIO_MUXSEL_OUT);
        hal_gpio_set_direction(LED_PIN, GPIO_DIRECTION_OUTPUT);
        initialized = 1;
        printf("GPIO PE12 initialized for toggle\n");
    }
    
    /* Toggle state */
    state = (state == GPIO_DATA_LOW) ? GPIO_DATA_HIGH : GPIO_DATA_LOW;
    hal_gpio_set_data(LED_PIN, state);
}
