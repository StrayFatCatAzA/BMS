#include "driver_bq76940_port.h"

#include "stm32f1xx_hal.h"
#include "bsp_iic.h"
#include "bsp_uart1.h"

/* ================================ BQ76940 引脚定义 ================================ */

#define BQ76940_IIC_SCL_PIN GPIO_PIN_8
#define BQ76940_IIC_SCL_PORT GPIOB
#define BQ76940_IIC_SDA_PIN GPIO_PIN_9
#define BQ76940_IIC_SDA_PORT GPIOB

#define BQ76940_WAKE_PIN GPIO_PIN_8
#define BQ76940_WAKE_PORT GPIOA

/* ================================ I2C 接口 ================================ */

bq76940_state_e s_bq76940_interface_iic_init(void)
{
    if (bsp_iic_soft_init() != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

bq76940_state_e s_bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (bsp_iic_soft_mem_write_data(dev_addr, reg_addr, data, len) != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

bq76940_state_e s_bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    if (bsp_iic_soft_mem_read_data(dev_addr, reg_addr, data, len) != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

/* ================================ GPIO 初始化函数接口 ================================ */

void s_bq76940_interface_gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* I2C SCL/SDA: open-drain output */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = BQ76940_IIC_SCL_PIN | BQ76940_IIC_SDA_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_IIC_SCL_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_IIC_SCL_PORT, BQ76940_IIC_SCL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BQ76940_IIC_SDA_PORT, BQ76940_IIC_SDA_PIN, GPIO_PIN_SET);

    /* WAKE pin: push-pull output */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = BQ76940_WAKE_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_WAKE_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);
}

/* ================================ 芯片唤醒函数接口 ================================ */

bq76940_state_e s_bq76940_interface_wake_up(void)
{
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);

    HAL_Delay(10);

    return BQ76940_STATE_OK;
}

/* ================================ 延迟函数接口 ================================ */

void s_bq76940_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* ================================ 日志输出接口 ================================ */

#include <stdarg.h>
#include <stdio.h>

void s_bq76940_interface_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[255];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    bsp_uart1_printf("%s", buf);
}
