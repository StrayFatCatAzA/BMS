#include "driver_bq76940.h"

/* C库头文件 */
#include <stdio.h>
#include <stddef.h>
/* HAL 库头文件 */
#include "stm32f1xx_hal.h"
/* BSP 层头文件 */
#include "bsp_iic.h"
#include "bsp_uart1.h"
/* BQ76940 驱动码 头文件 */
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

/* ================================ 串口宏定义 ================================ */

#define LOG bsp_uart1_printf
#define LOG_E bsp_uart1_printf

/* ================================ BQ76940 结构体定义 ================================ */

typedef struct
{
    uint16_t gain_uv; // 667
    int8_t offset_mv; // 47
} bq79640_compensation_t;

/* ================================ 内部静态变量声明 ================================ */

static bq79640_compensation_t s_bq79640_compensation_strcut;

/* ================================ IIC 接口函数声明 ================================ */

static bq76940_state_e s_bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
static bq76940_state_e s_bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ================================ 内部静态函数声明 ================================ */

static void s_bq76940_gpio_init(void);
static bq76940_state_e s_bq76940_write_byte_with_CRC(uint8_t reg_addr, uint8_t byte);
static bq76940_state_e s_bq76940_read_byte_with_CRC(uint8_t reg_addr, uint8_t *byte);
static bq76940_state_e s_bq76940_read_halfword_with_CRC(uint8_t reg_addr, uint16_t *halfword);
static uint8_t s_bq76940_crc(uint8_t *data, uint16_t len);

/* ================================ IIC 接口函数实现 ================================ */

/**
 * @description: bq76940 IIC写入字节函数接口
 * @param {uint8_t} dev_addr 设备地址
 * @param {uint8_t} reg 写入的寄存器地址
 * @param {uint8_t} *data 写入的数据
 * @param {uint16_t} len 数据长度
 * @return {*}
 */
static bq76940_state_e s_bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (bsp_iic_soft_mem_write_data(dev_addr, reg_addr, data, len) != IIC_OK)
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
static bq76940_state_e s_bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    if (bsp_iic_soft_mem_read_data(dev_addr, reg_addr, data, len) != IIC_OK)
        return BQ76940_STATE_ERR;

    return BQ76940_STATE_OK;
}

/* ================================ 内部静态函数实现 ================================ */

void s_bq76940_gpio_init(void)
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
static bq76940_state_e s_bq76940_write_byte_with_CRC(uint8_t reg_addr, uint8_t byte)
{
    uint8_t crc_buf[3] = {0};
    uint8_t send_buf[2] = {0};
    uint8_t crc = 0;

    crc_buf[0] = BQ76940_DEVICE_ADDR << 1; // Write operation
    crc_buf[1] = reg_addr;
    crc_buf[2] = byte;

    crc = s_bq76940_crc(crc_buf, 3);

    send_buf[0] = byte;
    send_buf[1] = crc;

    if (s_bq76940_interface_write_byte(BQ76940_DEVICE_ADDR, reg_addr, send_buf, 2) != BQ76940_STATE_OK)
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
static bq76940_state_e s_bq76940_read_byte_with_CRC(uint8_t reg_addr, uint8_t *byte)
{
    uint8_t crc_buf[2] = {0};
    uint8_t crc;
    uint8_t recv_data[2] = {0};

    if (s_bq76940_interface_read_byte(BQ76940_DEVICE_ADDR, reg_addr, recv_data, 2) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read I2C data\r\n");

        return BQ76940_STATE_ERR; // Return an invalid value to indicate failure
    }
    // CRC校验：先构造CRC输入数据（设备地址 + 读位 + 寄存器地址 + 读到的数据）
    crc_buf[0] = (BQ76940_DEVICE_ADDR << 1) | 0x01; // Read operation
    crc_buf[1] = recv_data[0];                      // Received data
    crc = s_bq76940_crc(crc_buf, 2);

    // LOG_E("Received byte: 0x%02X, CRC from device: 0x%02X\r\n", recv_data[0], recv_data[1]);

    if (crc != recv_data[1])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[1]);

        return BQ76940_STATE_ERR;
    }

    *byte = recv_data[0];

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940 向寄存器读半字数据 带CRC校验
 * @param {uint8_t} reg_addr 寄存器地址
 * @param {uint16_t} halfword 读取的半字数据
 * @return {*}
 */
static bq76940_state_e s_bq76940_read_halfword_with_CRC(uint8_t reg_addr, uint16_t *halfword)
{
    uint8_t crc_buf[2] = {0};
    uint8_t crc = 0;
    uint8_t recv_data[4] = {0};

    if (s_bq76940_interface_read_byte(BQ76940_DEVICE_ADDR, reg_addr, recv_data, 4) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read I2C data\r\n");

        return BQ76940_STATE_ERR;
    }

    /* 第一个字节校验 构造CRC输入数据（设备地址 + 读位 + 读到的数据） */
    crc_buf[0] = (BQ76940_DEVICE_ADDR << 1) | 0x01;
    crc_buf[1] = recv_data[0];
    /* CRC8校验 */
    crc = s_bq76940_crc(crc_buf, 2);
    /* 校验结构判断 */
    if (crc != recv_data[1])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[1]);
    }
    /* 第二个字节校验 */
    crc_buf[0] = recv_data[2];
    crc = s_bq76940_crc(crc_buf, 1);
    if (crc != recv_data[3])
    {
        LOG_E("CRC check failed! Expected: 0x%02X, Received: 0x%02X\r\n", crc, recv_data[3]);
    }

    *halfword = ((recv_data[0] << 8) | recv_data[2]);

    return BQ76940_STATE_OK;
}

/**
 * @description: 生成CRC8校验码
 * @param {uint8_t} *data 要计算的数据
 * @param {uint16_t} len 数据长度
 * @return CRC校验值
 */
static uint8_t s_bq76940_crc(uint8_t *data, uint16_t len)
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
bq76940_state_e bq76940_init(void)
{
    /* 0. GPIO 初始化 */
    s_bq76940_gpio_init();

    /* 1. 唤醒芯片 */
    bq76940_wake_up();

    /* 2. 等待芯片启动完成 */
    HAL_Delay(10);

    /* 3. 获取 gain和 offest 值 */
    bq76940_get_calibration(&s_bq79640_compensation_strcut.gain_uv, &s_bq79640_compensation_strcut.offset_mv);

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940进入低功耗模式
 * @return {*}
 */
bq76940_state_e bq76940_enter_ship(void)
{
    /* 连续向 SYS_CTRL1 的 SHUT_A/SHUT_B 位写入 */
    uint8_t shut_a = 0x08;
    uint8_t shut_b = 0x10;

    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL1, shut_a) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enter ship! SHUT_A error\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL1, shut_b) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enter ship! SHUT_B error\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940唤醒
 * @return {*}
 */
bq76940_state_e bq76940_wake_up(void)
{
    /* 1. 唤醒芯片 */
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(BQ76940_WAKE_PORT, BQ76940_WAKE_PIN, GPIO_PIN_RESET);

    /* 2. 等待芯片启动完成 */
    HAL_Delay(10);

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940设置电压采样状态
 * @param {bq76940_function_state_e} state 电压采集功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_voltage_collection(bq76940_function_state_e state)
{
    uint8_t reg_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &reg_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set voltage collection\r\n");

        return BQ76940_STATE_ERR;
    }

    if (state == BQ76940_ENABLE)
    {
        reg_val |= (BQ76940_ADC_EN_MASK | BQ76940_TEMP_SEL_MASK);
    }
    else
    {
        reg_val &= ~(BQ76940_ADC_EN_MASK | BQ76940_TEMP_SEL_MASK);
    }

    return s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL1, reg_val);
}

/**
 * @description: bq76940设置电流采样状态
 * @param {bq76940_function_state_e} state 电流采样功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_current_collection(bq76940_function_state_e state)
{
    uint8_t reg_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_CC_CFG, &reg_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set current collection\r\n");

        return BQ76940_STATE_ERR;
    }

    if (state == BQ76940_ENABLE)
    {
        reg_val |= BQ76940_CC_EN_MASK;
    }
    else
    {
        reg_val &= ~BQ76940_CC_EN_MASK;
    }

    return s_bq76940_write_byte_with_CRC(BQ76940_CC_CFG, reg_val);
}

/**
 * @description: bq76940设置温度采样状态
 * @param {bq76940_function_state_e} state 温度采样功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_temperature_collection(bq76940_function_state_e state)
{
    uint8_t reg_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &reg_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set temperature collection\r\n");

        return BQ76940_STATE_ERR;
    }

    if (state == BQ76940_ENABLE)
    {
        reg_val |= BQ76940_TEMP_SEL_MASK;
    }
    else
    {
        reg_val &= ~BQ76940_TEMP_SEL_MASK;
    }

    return s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL1, reg_val);
}

/**
 * @description: bq76940获取校准参数
 * @param {uint16_t} *adc_gain ADC增益指针
 * @param {int8_t} *adc_offset ADC偏置指针
 * @return {*}
 */
bq76940_state_e bq76940_get_calibration(uint16_t *adc_gain, int8_t *adc_offset)
{
    uint8_t gain_buf[2] = {0};
    uint8_t offset = 0;

    if (adc_gain == NULL || adc_offset == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_ADCGAIN1, &gain_buf[0]) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read ADC GAIN1\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_ADCGAIN2, &gain_buf[1]) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read ADC GAIN2\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_ADCOFFSET, &offset) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read ADC OFFSET\r\n");

        return BQ76940_STATE_ERR;
    }

    *adc_gain = ((gain_buf[0] & 0x0C) << 1) | ((gain_buf[1] & 0xE0) >> 5) + 365;
    *adc_offset = (int8_t)offset;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取指定电芯电压
 * @param {uint16_t} cell_index 电芯索引0-14 (对应第1-15节)
 * @param {uint16_t} *voltage 电芯电压
 * @return {*}
 */
bq76940_state_e bq76940_get_cell_voltage(uint16_t cell_index, uint16_t *voltage)
{
    uint16_t raw;
    uint16_t mv;

    if (cell_index > 14 || voltage == NULL)
    {
        LOG_E("Failed to get cell voltage:cell or voltage err\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_halfword_with_CRC(BQ76940_VC1_HI + (cell_index * 2), &raw) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get cell voltage: read reg err\r\n");

        return BQ76940_STATE_ERR;
    }

    /* Vcell(mV) = ADC × Gain(μV) / 1000 + Offset(mV) */
    mv = ((raw * s_bq79640_compensation_strcut.gain_uv) * 0.001f) + s_bq79640_compensation_strcut.offset_mv;

    *voltage = mv;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取电池全部电芯电压
 * @param {uint16_t} *voltage 接收电芯电压数组
 * @param {uint16_t} cell_num 电芯数量
 * @return {*}
 */
bq76940_state_e bq76940_get_all_cell_voltage(uint16_t *voltage, uint16_t cell_num)
{
    uint16_t raw;
    int32_t mv;
    uint16_t count = (cell_num > 15) ? 15 : cell_num;

    if (count == 0 || voltage == NULL)
    {
        LOG_E("Failed to get all cell voltage:voltage or cell_num err\r\n");

        return BQ76940_STATE_ERR;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        if (s_bq76940_read_halfword_with_CRC((uint8_t)(BQ76940_VC1_HI + i * 2 + 1), &raw) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to get cell voltage: read reg err\r\n");

            return BQ76940_STATE_ERR;
        }

        mv = ((raw * s_bq79640_compensation_strcut.gain_uv) * 0.001f) + s_bq79640_compensation_strcut.offset_mv;

        voltage[i] = (uint16_t)mv;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取电池总电压
 * @param {uint16_t} *total_voltage 电池总电压 单位mV
 * @param {uint16_t} cell_num 电芯数量
 * @return {*}
 */
bq76940_state_e bq76940_get_battery_voltage(uint16_t *battery_voltage,uint16_t cell_num)
{
    uint16_t raw;
    uint16_t mv;

    if (battery_voltage == NULL || cell_num == 0)
    {
        LOG_E("Failed to get total voltage: null pointer\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_halfword_with_CRC(BQ76940_BAT_HI, &raw) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get read BAT_HI");

        return BQ76940_STATE_ERR;
    }

    mv = 4 * s_bq79640_compensation_strcut.gain_uv * raw * 0.001f + (cell_num * s_bq79640_compensation_strcut.offset_mv);

    *battery_voltage = (uint16_t)mv;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取NTC温度
 * @param {uint8_t} *NTC_temperature NTC温度
 * @return {*}
 */
bq76940_state_e bq76940_get_NTC_temperature(uint8_t *NTC_temperature);

/**
 * @description: bq76940获取芯片温度
 * @param {uint8_t} *temperature 芯片温度
 * @return {*}
 */
bq76940_state_e bq76940_get_die_temperature(uint8_t *temperature);

/**
 * @description: 获取电流值
 * @param {uint8_t} *current 电流值
 * @return {*}
 */
bq76940_state_e bq76940_get_current(uint8_t *current);

/**
 * @description: 获取CC原始值
 * @param {uint8_t} *cc_raw 原始CC值
 * @return {*}
 */
bq76940_state_e bq76940_get_current_raw(uint8_t *cc_raw);

/**
 * @description: 设置过压值
 * @param {uint16_t} mv 过压值
 * @return {*}
 */
bq76940_state_e bq76940_set_ov_threshold(uint16_t mv);

/**
 * @description: 设置欠压值
 * @param {uint16_t} mv 欠压值
 * @return {*}
 */
bq76940_state_e bq76940_set_uv_threshold(uint16_t mv);

/**
 * @description: 设置过流值
 * @param {uint16_t} ma 过流值
 * @return {*}
 */
bq76940_state_e bq76940_set_ocd_threshold(uint16_t ma);

/**
 * @description: 设置短路电流
 * @param {uint16_t} ma 短路电流值
 * @return {*}
 */
bq76940_state_e bq76940_set_scd_threshold(uint16_t ma);

/**
 * @description: 获取故障状态
 * @return {*}
 */
bq76940_err_code_e bq76940_get_fault_status(void);

/**
 * @description: 清除故障码
 * @param {uint8_t} mask 故障掩码
 * @return {*}
 */
bq76940_state_e bq76940_clear_fault(uint8_t mask);

/**
 * @description: 启用充电
 * @return {*}
 */
bq76940_state_e bq76940_enable_charge(void);

/**
 * @description: 禁用充电
 * @return {*}
 */
bq76940_state_e bq76940_disable_charge(void);

/**
 * @description: 启用放电
 * @return {*}
 */
bq76940_state_e bq76940_enable_discharge(void);

/**
 * @description: 禁用放电
 * @return {*}
 */
bq76940_state_e bq76940_disable_discharge(void);

/**
 * @description: 开始均衡指定电芯
 * @param {uint8_t} cell_index 电芯索引
 * @return {*}
 */
bq76940_state_e bq76940_start_banlance(uint16_t cell_index, uint8_t *cell, uint16_t cells_len);

/**
 * @description: 停止均衡指定电芯
 * @param {uint8_t} cell_index 电芯索引
 * @return {*}
 */
bq76940_state_e bq76940_stop_banlance(uint16_t cell_index, uint8_t *cell, uint16_t cells_len);

/* ================================ BQ76940 测试函数 ================================ */

/**
 * @description: 测试函数 负责测试static函数
 * @return {*}
 */
void bq76940_static_test(void)
{
    LOG("\r\n========== BQ76940 Register R/W Test ==========\r\n\r\n");

    /* Test 1: 读 SYS_STAT (0x00) 验证基本读 */
    {
        uint8_t val = 0xFF;
        bq76940_state_e ret = s_bq76940_read_byte_with_CRC(0x00, &val);
        LOG("  [READ] SYS_STAT(0x00) = 0x%02X  %s\r\n",
            val, ret == BQ76940_STATE_OK ? "OK" : "ERR");
    }

    /* Test 2: SYS_CTRL1 (0x04) 写读回环 */
    {
        uint8_t orig = 0, rback = 0;
        bq76940_state_e ret;

        ret = s_bq76940_read_byte_with_CRC(0x04, &orig);
        LOG("  [READ] SYS_CTRL1(0x04) orig = 0x%02X  %s\r\n",
            orig, ret == BQ76940_STATE_OK ? "OK" : "ERR");

        uint8_t test = orig ^ 0x01;
        ret = s_bq76940_write_byte_with_CRC(0x04, test);
        LOG("  [WRITE] SYS_CTRL1(0x04) <- 0x%02X  %s\r\n",
            test, ret == BQ76940_STATE_OK ? "OK" : "ERR");

        ret = s_bq76940_read_byte_with_CRC(0x04, &rback);
        LOG("  [READ] SYS_CTRL1(0x04) rback = 0x%02X  %s\r\n",
            rback, ret == BQ76940_STATE_OK ? "OK" : "ERR");

        if (rback == test)
            LOG("  [PASS] round-trip matched\r\n");
        else
            LOG("  [FAIL] wrote 0x%02X got 0x%02X\r\n", test, rback);

        s_bq76940_write_byte_with_CRC(0x04, orig);
        LOG("  [RESTORE] SYS_CTRL1(0x04) <- 0x%02X\r\n", orig);
    }

    /* Test 3: 读半字 CC_CFG (0x0B) */
    {
        uint16_t val = 0;
        bq76940_state_e ret = s_bq76940_read_halfword_with_CRC(0x0B, &val);
        LOG("  [READ] CC_CFG(0x0B) = 0x%04X  %s\r\n",
            val, ret == BQ76940_STATE_OK ? "OK" : "ERR");
    }

    LOG("\r\n========== Test Complete ==========\r\n\r\n");
}
