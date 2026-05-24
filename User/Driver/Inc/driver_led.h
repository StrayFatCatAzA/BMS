#ifndef __DRIVER_LED_H__
#define __DRIVER_LED_H__

typedef enum
{
    DRV_LED_ON = 0,
    DRV_LED_OFF = 1
} drv_led_state_e;

void drv_led_init(void);

void drv_led_set_state(drv_led_state_e state);

void drv_led_toggle_state(void);

#endif
