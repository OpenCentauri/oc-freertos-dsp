/*
 * GPIO Toggle Example Header
 */
#ifndef __GPIO_TOGGLE_EXAMPLE_H__
#define __GPIO_TOGGLE_EXAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Initialize and start GPIO toggle task
 * This creates a FreeRTOS task that toggles PE12 every second
 */
void gpio_toggle_example_init(void);

/*
 * Toggle GPIO pin once (polling version)
 * Call this from your own task/loop if you don't want a separate task
 */
void gpio_toggle_once(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_TOGGLE_EXAMPLE_H__ */
