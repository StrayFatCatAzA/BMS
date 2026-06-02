#include "bsp_iic.h"

#include "stm32f1xx_hal.h"

#include "cmsis_os2.h"

/* ========================== 引脚配置 ========================== */

#define IIC_SCL_PORT GPIOB
#define IIC_SCL_PIN GPIO_PIN_8
#define IIC_SDA_PORT GPIOB
#define IIC_SDA_PIN GPIO_PIN_9

/* ========================== 内部函数声明 ========================== */

static void s_SCL_H(void);
static void s_SCL_L(void);
static void s_SDA_H(void);
static void s_SDA_L(void);
static uint8_t s_SDA_IN(void);

static void s_iic_delay(void);

static void s_iic_start(void);
static void s_iic_stop(void);

static void s_iic_send_ack(void);
static void s_iic_send_nack(void);

static uint8_t s_iic_wait_ack(void);

static void s_iic_send_byte(uint8_t byte);
static uint8_t s_iic_recv_byte(void);

static void s_iicENTER_CRITICAL(void);
static void s_iicEXIT_CRITICAL(void);

/* ========================== 接口函数开始 ========================== */

/* ========================== 操作系统函数接口 ========================== */

/**
 * @description: iic进入临界区
 * @return {*}
 */
static void s_iicENTER_CRITICAL(void)
{
    osKernelLock();
}

/**
 * @description: iic退出临界区
 * @return {*}
 */
static void s_iicEXIT_CRITICAL(void)
{
    osKernelUnlock();
}

/* ========================== 延时函数 ========================== */

/**
 * @description: I2C 半周期延时 (~5us @ 72MHz, 约合 100kHz 标准模式)
 */
static void s_iic_delay(void)
{
    volatile uint16_t i = 60;
    while (i--)
    {
        __NOP();
    }
}

/* ========================= IIC 数据线控制 ========================= */

/**
 * @description: SCL拉高
 * @return {*}
 */
static void s_SCL_H(void)
{
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
    s_iic_delay();
}

/**
 * @description: SCL拉低
 * @return {*}
 */
static void s_SCL_L(void)
{
    HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
    s_iic_delay();
}

/**
 * @description: SDA拉高
 * @return {*}
 */
static void s_SDA_H(void)
{
    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
    s_iic_delay();
}

/**
 * @description: SDA拉低
 * @return {*}
 */
static void s_SDA_L(void)
{
    HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_RESET);
    s_iic_delay();
}

/**
 * @description: 读SDA
 * @return SDA电平状态 1 - 高电平 0 -低电平
 */
static uint8_t s_SDA_IN(void)
{
    return HAL_GPIO_ReadPin(IIC_SDA_PORT, IIC_SDA_PIN);
}

/* ========================== 接口函数结束 ========================== */


/* ========================== 总线信号 ========================== */

/**
 * @description: 产生 I2C 起始信号
 *   SCL 高电平期间 SDA 拉低
 */
static void s_iic_start(void)
{
    s_SDA_H();
    s_SCL_H();

    s_SDA_L();
    s_SCL_L();
}

/**
 * @description: 产生 I2C 停止信号
 *   SCL 高电平期间 SDA 拉高
 */
static void s_iic_stop(void)
{
    s_SDA_L();
    s_SCL_H();
    s_SDA_H();
}

/* ========================== ACK / NACK ========================== */

/**
 * @description: 主机发送 ACK (第 9 个 SCL 期间 SDA 拉低)
 */
static void s_iic_send_ack(void)
{
    s_SDA_L();
    s_SCL_H();
    s_SCL_L();
    s_SDA_H(); /* 释放 SDA */
}

/**
 * @description: 主机发送 NACK (第 9 个 SCL 期间 SDA 保持高)
 */
static void s_iic_send_nack(void)
{
    s_SDA_H(); /* 拉高 = NACK */
    s_SCL_H();
    s_SCL_L();
}

/**
 * @description: 主机等待从机 ACK
 * @return 0 = ACK, 1 = NACK
 */
static uint8_t s_iic_wait_ack(void)
{
    uint8_t ack;

    s_SDA_H(); /* 释放 SDA，让从机驱动 */
    s_SCL_H();

    ack = s_SDA_IN(); /* 0 = ACK, 1 = NACK */

    s_SCL_L();

    return ack;
}

/* ========================== 字节收发 ========================== */

/**
 * @description: 发送一个字节
 * @param byte 待发送的字节
 */
static void s_iic_send_byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            s_SDA_H();
        }
        else
        {
            s_SDA_L();
        }
        s_SCL_H();
        s_SCL_L();

        byte <<= 1;
    }
    s_SDA_H(); /* 释放 SDA，等待 ACK */
}

/**
 * @description: 接收一个字节
 * @return 接收到的字节
 */
static uint8_t s_iic_recv_byte(void)
{
    uint8_t byte = 0;

    s_SDA_H(); /* 释放 SDA */

    for (uint8_t i = 0; i < 8; i++)
    {
        s_SCL_H();

        byte <<= 1;
        if (s_SDA_IN())
        {
            byte |= 0x01;
        }

        s_SCL_L();
    }

    return byte;
}

/* ========================== 公开接口 ========================== */

/**
 * @description: 软件 I2C 初始化
 *   SCL: PB8 推挽输出
 *   SDA: PB9 开漏输出
 * @return 初始化是否成功 0 - 成功 1 - 失败
 */
iic_state_e bsp_iic_soft_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* SCL - 推挽输出 */
    GPIO_InitStruct.Pin = IIC_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IIC_SCL_PORT, &GPIO_InitStruct);

    /* SDA - 开漏输出 */
    GPIO_InitStruct.Pin = IIC_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);

    /* 总线释放（高电平） */
    s_SCL_H();
    s_SDA_H();

    return IIC_OK;
}

/**
 * @description: 向从机写入连续数据（无寄存器地址）
 * @param  dev_addr 从机地址
 * @param  data 要写入的数据指针
 * @param  len 数据长度
 * @return 写入是否成功 0 - 成功 1 - 失败
 */
iic_state_e bsp_iic_soft_write_data(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0)
    {
        return IIC_ERR;
    }

    s_iicENTER_CRITICAL();

    s_iic_start();

    s_iic_send_byte(dev_addr << 1);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        s_iic_send_byte(data[i]);
        if (s_iic_wait_ack())
        {
            s_iic_stop();
            s_iicEXIT_CRITICAL();

            return IIC_ERR;
        }
    }

    s_iic_stop();
    s_iicEXIT_CRITICAL();
    return IIC_OK;
}

/**
 * @description: 从从机读取连续数据（无寄存器地址）
 * @param  dev_addr 从机地址
 * @param  data 接收数据的指针
 * @param  len 接收数据的长度
 * @return 读取是否成功 0 - 成功 1 - 失败
 */
iic_state_e bsp_iic_soft_read_data(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0)
    {
        return IIC_ERR;
    }

    s_iicENTER_CRITICAL();

    s_iic_start();

    s_iic_send_byte((dev_addr << 1) | 0x01);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = s_iic_recv_byte();

        if (i < len - 1)
        {
            s_iic_send_ack(); /* 非最后一字节 → ACK */
        }
        else
        {
            s_iic_send_nack(); /* 最后一字节   → NACK */
        }
    }

    s_iic_stop();
    s_iicEXIT_CRITICAL();

    return IIC_OK;
}

/**
 * @description: 向从机寄存器写入数据
 * @param dev_addr 从机地址
 * @param reg 从机寄存器地址
 * @param data 要写入的数据指针
 * @param len 写入数据长度
 * @return 写入是否成功 0 - 成功 1 - 失败
 */
iic_state_e bsp_iic_soft_mem_write_data(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0)
    {
        return IIC_ERR;
    }

    s_iicENTER_CRITICAL();

    s_iic_start();

    s_iic_send_byte(dev_addr << 1);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    s_iic_send_byte(reg);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        s_iic_send_byte(data[i]);
        if (s_iic_wait_ack())
        {
            s_iic_stop();
            s_iicEXIT_CRITICAL();

            return IIC_ERR;
        }
    }

    s_iic_stop();
    s_iicEXIT_CRITICAL();

    return IIC_OK;
}

/**
 * @description: 从从机寄存器读取数据
 * @param dev_addr 从机地址
 * @param reg 要读取的寄存器地址
 * @param data 接收数据的指针
 * @param len 接收数据的长度
 * @return 读取是否成功 0 - 成功 1 - 失败
 */
iic_state_e bsp_iic_soft_mem_read_data(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0)
    {
        return IIC_ERR;
    }

    /* 第一阶段: 写入寄存器地址 */
    s_iicENTER_CRITICAL();
    s_iic_start();
    s_iic_send_byte(dev_addr << 1);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }
    s_iic_send_byte(reg);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    /* 第二阶段: 重新开始 + 读取数据 */
    s_iic_start();
    s_iic_send_byte((dev_addr << 1) | 0x01);
    if (s_iic_wait_ack())
    {
        s_iic_stop();
        s_iicEXIT_CRITICAL();

        return IIC_ERR;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = s_iic_recv_byte();

        if (i < len - 1)
        {
            s_iic_send_ack();
        }
        else
        {
            s_iic_send_nack();
        }
    }

    s_iic_stop();
    s_iicEXIT_CRITICAL();
    
    return IIC_OK;
}
