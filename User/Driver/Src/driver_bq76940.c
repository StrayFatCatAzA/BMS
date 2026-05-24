#include "driver_bq76940.h"

#include <stdio.h>
#include <stddef.h>

#include "stm32f1xx_hal.h"

#include "bsp_iic.h"
#include "bsp_uart1.h"

#include "driver_bq76940_reg.h"

/* ================================ BQ76940 引脚定义 ================================ */

#define BQ76940_IIC_SCL_PIN GPIO_PIN_8
#define BQ76940_IIC_SCL_PORT GPIOB
#define BQ76940_IIC_SDA_PIN GPIO_PIN_9
#define BQ76940_IIC_SDA_PORT GPIOB

#define BQ76940_WAKE_PIN GPIO_PIN_8
#define BQ76940_WAKE_PORT GPIOA

/* ================================ BQ76940 寄存器定义 ================================ */

#define BQ76940_DEVICE_ADDR 0x08

#define LOG uart1_printf
#define LOG_E uart1_printf

/* ================================ IIC 接口函数声明 ================================ */

static bq76940_state bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
static bq76940_state bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ================================ 内部静态函数声明 ================================ */

static void bq76940_gpio_init(void);
static bq76940_state bq76940_write_byte_with_CRC(uint8_t reg_addr, uint8_t byte);
static bq76940_state bq76940_read_byte_with_CRC(uint8_t reg_addr, uint8_t *byte);
static bq76940_state bq76940_read_halfword_with_CRC(uint8_t reg_addr, uint16_t *halfword);
static uint8_t bq76940_crc(uint8_t *data, uint16_t len);

/* ================================ IIC 接口函数实现 ================================ */

/**
 * @description: bq76940 IIC写入字节函数接口
 * @param {uint8_t} dev_addr 设备地址
 * @param {uint8_t} reg 写入的寄存器地址
 * @param {uint8_t} *data 写入的数据
 * @param {uint16_t} len 数据长度
 * @return {*}
 */
static bq76940_state bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (iic_soft_mem_write_data(dev_addr, reg_addr, data, len) != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940 IIC读取字节函数接口
 * @param {uint8_t} dev_addr 设备地址
 * @param {uint8_t} reg 要读取的寄存器地址
 * @param {uint8_t} *data 要读取的数据
 * @param {uint16_t} len 要读取数据长度
 * @return {*}
 */
static bq76940_state bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    if (iic_soft_mem_read_data(dev_addr, reg_addr, data, len) != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

/* ================================ 内部静态函数实现 ================================ */


void bq76940_gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* GPIO 引脚初始化 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = BQ76940_IIC_SCL_PIN | BQ76940_IIC_SDA_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_IIC_SCL_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_IIC_SCL_PORT, BQ76940_IIC_SCL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BQ76940_IIC_SDA_PORT, BQ76940_IIC_SDA_PIN, GPIO_PIN_SET);

    /* WAKE 引脚初始化 */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = BQ76940_WAKE_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BQ76940_WAKE_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);    
}

/**
 * @description: bq76940 向寄存器写字节数据 带CRC校验
 * @param {uint8_t} reg 寄存器地址
 * @param {uint8_t} byte 写入的字节数据
 * @return {*}
 */
static bq76940_state bq76940_write_byte_with_CRC(uint8_t reg_addr, uint8_t byte)
{
    uint8_t crc_buf[3] = {0};
    uint8_t send_buf[2] = {0};
    uint8_t crc = 0;

    crc_buf[0] = BQ76940_DEVICE_ADDR << 1; // Write operation
    crc_buf[1] = reg_addr;
    crc_buf[2] = byte;

    crc = bq76940_crc(crc_buf, 3);

    send_buf[0] = byte;
    send_buf[1] = crc;

    if (bq76940_interface_write_byte(BQ76940_DEVICE_ADDR, reg_addr, send_buf, 2) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to write I2C data\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940 向寄存器读字节数据 带CRC校验
 * @param {uint8_t} reg_addr 寄存器地址
 * @param {uint8_t} *byte 读取的字节数据
 * @return {*}
 */
static bq76940_state bq76940_read_byte_with_CRC(uint8_t reg_addr, uint8_t *byte)
{
    // uint8_t crc_buf[2] = {0};
    // uint8_t crc;
    uint8_t recv_data[2] = {0};

    if (bq76940_interface_read_byte(BQ76940_DEVICE_ADDR, reg_addr, recv_data, 2) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read I2C data\r\n");

        return BQ76940_STATE_ERR; // Return an invalid value to indicate failure
    }
    // CRC校验：先构造CRC输入数据（设备地址 + 读位 + 寄存器地址 + 读到的数据）
    // crc_buf[0] = (BQ76940_DEVICE_ADDR << 1) | 0x01; // Read operation
    // crc_buf[1] = recv_data[0];                      // Received data
    // LOG_I("Received byte: 0x%02X, CRC from device: 0x%02X\r\n", recv_data[0], recv_data[1]);
    // crc = CRC8_Calculate(crc_buf, 2);
    // if (crc != recv_data[1])
    // {
    //     LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[1]);
    //     // return 0;
    // }

    *byte = recv_data[0];

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940 向寄存器读半字数据 带CRC校验
 * @param {uint8_t} reg_addr 寄存器地址
 * @param {uint16_t} halfword 读取的半字数据
 * @return {*}
 */
static bq76940_state bq76940_read_halfword_with_CRC(uint8_t reg_addr, uint16_t *halfword)
{
    uint8_t high_byte = 0, low_byte = 0;

    if (bq76940_read_byte_with_CRC(reg_addr, &high_byte) != IIC_OK)
    {
        LOG_E("Failed to read high byte\r\n");
        return BQ76940_STATE_ERR; // Return an invalid value to indicate failure
    }

    if (bq76940_read_byte_with_CRC(reg_addr + 1, &low_byte) != IIC_OK)
    {
        LOG_E("Failed to read low byte\r\n");
        return BQ76940_STATE_ERR; // Return an invalid value to indicate failure
    }

    *halfword = ((high_byte << 8) | low_byte);

    return BQ76940_STATE_OK;
}

/**
 * @description: 生成CRC8校验码
 * @param {uint8_t} *data 要计算的数据
 * @param {uint16_t} len 数据长度
 * @return CRC校验值
 */
static uint8_t bq76940_crc(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    /* CRC 计算 */
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }

    return crc;
}


/* ================================ BQ76940 公开接口函数 ================================ */

/**
 * @description: BQ76940 初始化函数
 * @return {*}
 */
bq76940_state bq76940_init(void)
{
    /* 0. GPIO 初始化 */
    bq76940_gpio_init();


    /* 1. 唤醒芯片 */
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);

    /* 2. 等待芯片启动完成 */
    HAL_Delay(10);

    return BQ76940_STATE_OK;
}


/**
 * @description: 测试函数 负责测试static函数
 * @return {*}
 */
void bq76940_test(void)
{
    LOG("\r\n========== BQ76940 Static Function Tests ==========\r\n\r\n");

    uint8_t tmp = 0;
    bq76940_read_byte_with_CRC(0x00,&tmp);


    LOG("========== Tests Complete ==========\r\n\r\n");
}
