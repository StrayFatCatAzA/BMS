#ifndef __BSP_LED_H__
#define __BSP_LED_H__

typedef enum
{
    LED_ENABLE = 0,
    LED_DISABLE = 1
} bsp_led_state_e;

void bsp_led_gpio_init(void);

void bsp_led_set_state(bsp_led_state_e state);

void bsp_led_toggle(void);

#endif
