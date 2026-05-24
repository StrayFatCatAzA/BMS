#include "bsp_led.h"

#include "stm32f1xx_hal.h"

/* ================================ LED 引脚定义 ================================ */

#define LED_GPIO_PORT GPIOA
#define LED_PIN GPIO_PIN_15

/* ================================ LED 公开函数 ================================ */

/**
 * @description: LED GPIO 初始化
 * @return {*}
 */
void bsp_led_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启 AFIO 时钟 */
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* 关闭 JTAG，保留 SWD */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    /* 开启 GPIO 时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 配置 LED 引脚 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    /* 启动后默认高电平 熄灭 */
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @description: 设置LED状态
 * @param state 新的LED状态
 * @return {*}
 */
void bsp_led_set_state(bsp_led_state_e state)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, (GPIO_PinState)state);
}

/**
 * @description: LED反转电平
 * @return {*}
 */
void bsp_led_toggle(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}
