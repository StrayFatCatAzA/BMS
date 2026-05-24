#include "driver_led.h"

#include "bsp_led.h"

/**
 * @description: LED 初始化
 * @return {*}
 */
void drv_led_init(void)
{
    bsp_led_gpio_init();
}

/**
 * @description: LED 设置状态
 * @param state 新状态
 * @return {*}
 */
void drv_led_set_state(drv_led_state_e state)
{
    bsp_led_set_state((bsp_led_state_e)state);
}

/**
 * @description: LED 翻转状态
 * @return {*}
 */
void drv_led_toggle_state(void)
{
    bsp_led_toggle();
}
