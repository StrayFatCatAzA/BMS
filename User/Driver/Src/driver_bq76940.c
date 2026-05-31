#include "driver_bq76940.h"

/* C库头文件 */
#include <stdio.h>
#include <stddef.h>
#include <math.h>
/* 接口层头文件 (hardware abstraction) */
#include "driver_bq76940_port.h"
/* BQ76940 寄存器头文件 */
#include "driver_bq76940_reg.h"

/* ================================ BQ76940 设备地址定义 ================================ */

#define BQ76940_DEVICE_ADDR 0x08

/* ================================ BQ76940 结构体定义 ================================ */

typedef struct
{
    uint16_t gain_uv; // 667
    int8_t offset_mv; // 47
    uint8_t inited;
} bq79640_compensation_t;

/* ================================ 内部静态变量声明 ================================ */

/* BQ76940 校准数据结构体 */
static bq79640_compensation_t s_bq79640_compensation_struct = {
    .gain_uv = 0,
    .offset_mv = 0,
    .inited = 0};

/* 热敏电阻参数 */
static const float Rp = 10000;       // 热敏电阻25℃标称阻值（10kΩ）
static const float T2 = 273.15 + 25; // 热敏电阻25℃的绝对温度（开尔文）
static const float Bx = 3380;        // 热敏电阻的B值（单位：K）
static const float Ka = 273.15;      // 开尔文与摄氏度的转换常数

/* ================================ 内部静态函数声明 ================================ */

static bq76940_state_e s_bq76940_write_byte_with_CRC(uint8_t reg_addr, uint8_t byte);
static bq76940_state_e s_bq76940_read_byte_with_CRC(uint8_t reg_addr, uint8_t *byte);
static bq76940_state_e s_bq76940_read_halfword_with_CRC(uint8_t reg_addr, uint16_t *halfword);
static uint8_t s_bq76940_crc(uint8_t *data, uint16_t len);

/* ================================ 内部静态函数实现 ================================ */

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
    s_bq76940_interface_gpio_init();

    /* 1. 唤醒芯片 */
    bq76940_wake_up();

    /* 2. 等待芯片启动完成 */
    s_bq76940_interface_delay_ms(10);

    /* 3. 获取 gain和 offest 值 */
    bq76940_get_calibration(&s_bq79640_compensation_struct.gain_uv, &s_bq79640_compensation_struct.offset_mv);
    s_bq79640_compensation_struct.inited = 1;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940进入低功耗模式
 * @return {*}
 * @note 退出低功耗模式时 需要延迟至少800ms
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
    return s_bq76940_interface_wake_up();
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

    if (state == BQ76940_FUNC_ENABLE)
    {
        reg_val |= BQ76940_ADC_EN_MASK;
    }
    else
    {
        reg_val &= ~BQ76940_ADC_EN_MASK;
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

    if (state == BQ76940_FUNC_ENABLE)
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
 * @description: bq76940设置温度采样模式
 * @param {bq76940_temp_mode_e} mode 温度模式（内部芯片温度 / 外部热敏电阻）
 * @return {*}
 * @note 使用该函数后 必须延迟至少2000ms便于ADC采样数据稳定
 */
bq76940_state_e bq76940_set_temperature_collection(bq76940_temp_mode_e mode)
{
    uint8_t reg_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &reg_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set temperature collection\r\n");
        return BQ76940_STATE_ERR;
    }

    if (mode == BQ76940_TEMP_MODE_EXTERNAL)
    {
        /* TEMP_SEL = 1: 外部热敏电阻模式 */
        reg_val |= BQ76940_TEMP_SEL_MASK;
    }
    else
    {
        /* TEMP_SEL = 0: 内部芯片温度模式 */
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
        LOG_E("Failed to get cell voltage: read VC1_HI err\r\n");

        return BQ76940_STATE_ERR;
    }

    /* Vcell(mV) = ADC × Gain(μV) / 1000 + Offset(mV) */
    mv = ((raw * s_bq79640_compensation_struct.gain_uv) * 0.001f) + s_bq79640_compensation_struct.offset_mv;

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
        LOG_E("Failed to get all cell voltage: voltage or cell_num err\r\n");

        return BQ76940_STATE_ERR;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        if (s_bq76940_read_halfword_with_CRC((uint8_t)(BQ76940_VC1_HI + i * 2 + 1), &raw) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to get cell voltage: read VC1_HI err\r\n");

            return BQ76940_STATE_ERR;
        }

        /* Vcell(mV) = ADC × Gain(μV) / 1000 + Offset(mV) */
        mv = ((raw * s_bq79640_compensation_struct.gain_uv) * 0.001f) + s_bq79640_compensation_struct.offset_mv;

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
bq76940_state_e bq76940_get_battery_voltage(uint16_t *battery_voltage, uint16_t cell_num)
{
    uint16_t raw; // 原始ADC数据
    uint16_t mv;  // 转换后电压数据 单位mV

    if (battery_voltage == NULL || cell_num == 0)
    {
        LOG_E("Failed to get battery voltage: null pointer\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_halfword_with_CRC(BQ76940_BAT_HI, &raw) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get battery voltage: read BAT_HI err\r\n");

        return BQ76940_STATE_ERR;
    }
    /* 计算公式 V(BAT) = 4 x GAIN x ADC(cell) + (#Cells x OFFSET) */
    mv = 4 * s_bq79640_compensation_struct.gain_uv * raw * 0.001f + (cell_num * s_bq79640_compensation_struct.offset_mv);

    *battery_voltage = (uint16_t)mv;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取芯片外部温度
 * @param channel 温度通道 1=TS1, 2=TS2, 3=TS3
 * @param temperature 芯片外部温度  输出单位: 0.1℃ 例如 253 表示 25.3℃
 * @return {*}
 */
bq76940_state_e bq76940_get_external_temperature_ch(uint8_t channel, int16_t *temperature)
{
    uint8_t sys_ctrl1;
    uint8_t reg_addr;

    if (temperature == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    /* 通道选择 */
    switch (channel)
    {
    case 1:
        reg_addr = BQ76940_TS1_HI;
        break;
    case 2:
        reg_addr = BQ76940_TS2_HI;
        break;
    case 3:
        reg_addr = BQ76940_TS3_HI;
        break;
    default:
        LOG_E("Failed to get external temperature: invalid channel %d\r\n", channel);
        return BQ76940_STATE_ERR;
    }

    /* 读取TEMP_SEL 判断是否启用外部温度模式 */
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &sys_ctrl1) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get external temperature: read TEMP_SEL err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 读取对应温度 */
    if (sys_ctrl1 & BQ76940_TEMP_SEL_MASK)
    {
        uint16_t raw_adc_temp_val = 0;

        if (s_bq76940_read_halfword_with_CRC(reg_addr, &raw_adc_temp_val) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to get external temperature: read TS%d err\r\n", channel);
            return BQ76940_STATE_ERR;
        }

        /* 14位ADC值: 寄存器低2位为状态位, 右移2位得到实际ADC值 */
        // raw_adc_temp_val >>= 2;
        /* 公式: VTSX = (ADC in Decimal) x 382 uV/LSB
         RTS = (10,000 x VTSX) / (3.3 - VTSX)
         T = 1 / (1/T2 + ln(Rt/Rp)/Bx) - Ka */
        float V_tsx = (raw_adc_temp_val * 382) * 0.001f;
        float Rt = (10000 * V_tsx) / (3300.0f - V_tsx);
        float temp_kelvin = 1 / (1 / T2 + (log(Rt / Rp)) / Bx);

        *temperature = (int16_t)((temp_kelvin - Ka + 0.5f) * 10);

        // LOG("Raw ADC value: %d \r\nV_tsx: %.2f mV \r\nRt: %.2f ohms \r\nTempKelvin: %.2f \r\nTemperature: %d\r\n",
        // raw_adc_temp_val, V_tsx, Rt, temp_kelvin, *temperature);
    }
    else
    {
        LOG_E("Failed to get external temperature: TS%d TEMP_SEL bit err\r\n", channel);
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取芯片内部温度
 * @param temperature 芯片内部温度 输出单位: 0.1℃  例如 253 表示 25.3℃
 * @return {*}
 */
bq76940_state_e bq76940_get_internal_temperature(int16_t *temperature)
{
    uint8_t sys_ctrl1;
    uint16_t raw_adc_ts1 = 0; // TS1 14位ADC 原始值
    uint16_t raw_adc_ts2 = 0; // TS2 14位ADC 原始值
    uint16_t raw_adc_ts3 = 0; // TS3 14位ADC 原始值
    float ave_adc_val = 0;    // TS1 TS2 TS3 平均值
    float V_tsx = 0;
    float V_25 = 1.200f;
    float temp_die = 0;

    /* 1. 判断功能是否启用 读取 TEMP_SEL 位判断是否为0 */
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &sys_ctrl1) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get internal temperature: read SYS_CTRL1 err\r\n");

        return BQ76940_STATE_ERR;
    }

    /* TEMP_SEL = 1 外部温度模式, TEMP_SEL = 0 内部温度模式 */
    if (sys_ctrl1 & BQ76940_TEMP_SEL_MASK)
    {
        LOG_E("Failed to get internal temperature: TEMP_SEL bit err\r\n");

        return BQ76940_STATE_ERR;
    }

    /* 2. 读取数据 读取寄存器 TS1 TS2 TS3 的数据 这些寄存器都是使用14位ADC采样 */
    if (s_bq76940_read_halfword_with_CRC(BQ76940_TS1_HI, &raw_adc_ts1) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get internal temperature: read TS1 err\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_halfword_with_CRC(BQ76940_TS2_HI, &raw_adc_ts2) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get internal temperature: read TS2 err\r\n");

        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_halfword_with_CRC(BQ76940_TS3_HI, &raw_adc_ts3) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get internal temperature: read TS3 err\r\n");

        return BQ76940_STATE_ERR;
    }

    /* 注意: 14位数据移位合成时保留全部16位数据 */
    raw_adc_ts1 >>= 2;
    raw_adc_ts2 >>= 2;
    raw_adc_ts3 >>= 2;

    /* 3. 数据转化 将读取到的寄存器数据进行转化 */
    /* 公式: V25 = 1.200V (标称值)
             VTSX = (ADC in Decimal) x 382 µV/LSB
             TEMPDIE = 25° – ((VTSX – V25) ÷ 0.0042) */
    ave_adc_val = (raw_adc_ts1 + raw_adc_ts2 + raw_adc_ts3) / 3.0f;

    V_tsx = (ave_adc_val * 0.000382f);
    temp_die = 25.0f - ((V_tsx - V_25) / 0.0042f);
    // LOG("raw adc ts1:%d \r\nraw adc ts2:%d \r\nraw adc ts3:%d\r\n", raw_adc_ts1, raw_adc_ts2, raw_adc_ts3);
    // LOG("ave adc val:%.1f\r\n", ave_adc_val);
    // LOG("V_tsx: %0.1f\r\ntemp_die:%0.1f\r\n", V_tsx, temp_die);
    *temperature = (int16_t)(temp_die);

    return BQ76940_STATE_OK;
}

/**
 * @description: 获取电流值
 * @param {uint8_t} *current 电流值 单位: mA
 * @return {*}
 */
bq76940_state_e bq76940_get_current(int16_t *current)
{
    int16_t cc_adc_val = 0; // 库仑计数寄存器的原始ADC值
    float cc_val = 0;       // 转换后的库仑计数值

    if (s_bq76940_read_halfword_with_CRC(BQ76940_CC_HI, (uint16_t *)&cc_adc_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read current\r\n");
        return BQ76940_STATE_ERR; // Return failure
    }

    cc_val = cc_adc_val * 8.44f;      // 转换为库仑计数值，单位：uV
    *current = (int16_t)(cc_val / 4); // 转换为电流值，单位：mA（采样电阻为4mΩ）
    // LOG("Raw ADC value: %d, CC value: %.2f uV, Current: %d mA\r\n", cc_adc_val, cc_val, *current);

    return BQ76940_STATE_OK;
}

/**
 * @description: 获取CC原始值
 * @param {uint8_t} *cc_raw 原始CC值 单位 uV
 * @return {*}
 */
bq76940_state_e bq76940_get_current_raw(int16_t *cc_raw)
{
    int16_t cc_adc_val = 0; // 库仑计数寄存器的原始ADC值
    float cc_val = 0;       // 转换后的库仑计数值

    if (s_bq76940_read_halfword_with_CRC(BQ76940_CC_HI, (uint16_t *)&cc_adc_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to read current\r\n");
        return BQ76940_STATE_ERR; // Return failure
    }

    cc_val = cc_adc_val * 8.44f; // 转换为库仑计数值，单位：uV

    *cc_raw = (int16_t)cc_val;

    return BQ76940_STATE_OK;
}

/**
 * @description: 设置过压值和过压触发时间
 * @param {uint16_t} ov 过压值 单位mV 范围: 3.15V ~ 4.70V  3136mV ~ 4674mV
 * @param {bq76940_ov_threshold_delay_e} delay 过压触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_ov_threshold(uint16_t ov, bq76940_ov_threshold_delay_e delay)
{
    uint16_t ov_trip_full = 0;
    uint8_t ov_trip = 0;

    /* 1. 读取SYS_CTRL1 判断ADC是否启用(ADC_EN=1) */
    {
        uint8_t sys_ctrl1 = 0;
        if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &sys_ctrl1) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set ov threshold: read SYS_CTRL1 err\r\n");
            return BQ76940_STATE_ERR;
        }
        if (!(sys_ctrl1 & BQ76940_ADC_EN_MASK))
        {
            LOG_E("Failed to set ov threshold: ADC not enabled\r\n");
            return BQ76940_STATE_ERR;
        }
    }

    /* 2. 校验ADCGAIN和ADCOFFSET校准值是否已加载 */
    if (s_bq79640_compensation_struct.inited != 1)
    {
        LOG_E("Failed to set ov threshold: calibration not loaded\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 3. 计算过压跳闸阈值
     *  公式: OV_TRIP_FULL = (OV – ADCOFFSET) ÷ ADCGAIN
     */
    ov_trip_full = (ov - s_bq79640_compensation_struct.offset_mv) / (s_bq79640_compensation_struct.gain_uv * 0.001f);

    /* 4. 从完整14位值中移除最高2MSB和最低4LSB，仅保留中间的8位 */
    ov_trip = (ov_trip_full >> 4) & 0xFF;

    /* 5. 写入对应寄存器 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_OV_TRIP, ov_trip) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set ov threshold: write OV_TRIP err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 6. 写入过压跳闸时间 */
    {
        uint8_t protect3_val = 0;
        if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT3, &protect3_val) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set ov delay: read PROTECT3 err\r\n");
            return BQ76940_STATE_ERR;
        }
        protect3_val &= ~BQ76940_OV_DELAY_MASK;
        protect3_val |= (delay << BQ76940_OV_DELAY_SHIFT);
        if (s_bq76940_write_byte_with_CRC(BQ76940_PROTECT3, protect3_val) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set ov delay: write PROTECT3 err\r\n");
            return BQ76940_STATE_ERR;
        }
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 设置欠压值
 * @param {uint16_t} uv 欠压值 单位mV  范围：1.58V ~ 3.10V  1589mV ~ 3125mV
 * @param {bq76940_uv_threshold_delay_e} delay 欠压触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_uv_threshold(uint16_t uv, bq76940_uv_threshold_delay_e delay)
{
    uint16_t uv_trip_full = 0;
    uint8_t uv_trip = 0;

    /* 1. 读取SYS_CTRL1 判断ADC是否启用(ADC_EN=1) */
    {
        uint8_t sys_ctrl1 = 0;
        if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL1, &sys_ctrl1) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set uv threshold: read SYS_CTRL1 err\r\n");
            return BQ76940_STATE_ERR;
        }
        if (!(sys_ctrl1 & BQ76940_ADC_EN_MASK))
        {
            LOG_E("Failed to set uv threshold: ADC not enabled\r\n");
            return BQ76940_STATE_ERR;
        }
    }

    /* 2. 校验ADCGAIN和ADCOFFSET校准值是否已加载 */
    if (s_bq79640_compensation_struct.inited != 1)
    {
        LOG_E("Failed to set uv threshold: calibration not loaded\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 3. 计算欠压跳闸阈值
     *  公式: UV_TRIP_FULL = (UV – ADCOFFSET) ÷ ADCGAIN
     */
    uv_trip_full = (uv - s_bq79640_compensation_struct.offset_mv) / (s_bq79640_compensation_struct.gain_uv * 0.001f);

    /* 4. 从完整14位值中移除最高2MSB和最低4LSB，仅保留中间的8位 */
    uv_trip = (uv_trip_full >> 4) & 0xFF;

    /* 5. 写入对应寄存器 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_UV_TRIP, uv_trip) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set uv threshold: write UV_TRIP err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 6. 写入欠压跳闸延迟时间 */
    {
        uint8_t protect3_val = 0;
        if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT3, &protect3_val) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set uv delay: read PROTECT3 err\r\n");
            return BQ76940_STATE_ERR;
        }
        protect3_val &= ~BQ76940_UV_DELAY_MASK;
        protect3_val |= (delay << BQ76940_UV_DELAY_SHIFT);
        if (s_bq76940_write_byte_with_CRC(BQ76940_PROTECT3, protect3_val) != BQ76940_STATE_OK)
        {
            LOG_E("Failed to set uv delay: write PROTECT3 err\r\n");
            return BQ76940_STATE_ERR;
        }
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940设置过流、短路等级
 * @param {bq76940_ocd_sed_level_e} level 过流、短路等级
 * @return {*}
 * @note 高档位 X2
 */
bq76940_state_e bq76940_set_ocd_scd_level(bq76940_ocd_scd_level_e level)
{
    uint8_t protect1_val = 0;

    /* 读取 PROTECT1 只修改bit7 (RSNS位) */
    if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT1, &protect1_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set ocd scd level: read PROTECT1 err\r\n");
        return BQ76940_STATE_ERR;
    }

    if (level == BQ76940_OCD_SCD_HIGH_LEVEL)
    {
        /* RSNS = 1: 高档位 (阈值 x2) */
        protect1_val |= 0x80;
    }
    else
    {
        /* RSNS = 0: 低档位 */
        protect1_val &= ~0x80;
    }

    if (s_bq76940_write_byte_with_CRC(BQ76940_PROTECT1, protect1_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set ocd scd level: write PROTECT1 err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 设置过电流值
 * @param {bq76940_ocd_threshold_value_e} value 过电流值 单位mV
 * @param {bq76940_ocd_threshold_delay_e} delay 过电流触发时间 单位μs
 * @return {*}
 */
bq76940_state_e bq76940_set_ocd_threshold(bq76940_ocd_threshold_value_e value, bq76940_ocd_threshold_delay_e delay)
{
    uint8_t protect2_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT2, &protect2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set ocd threshold: read PROTECT2 err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* PROTECT2 [6:4] 过流延迟时间设置  [3:0] 过流阈值设置 */
    protect2_val &= ~(BQ76940_OCD_DELAY_MASK | BQ76940_OCD_THRESH_MASK);
    protect2_val |= (delay << BQ76940_OCD_DELAY_SHIFT) | (value << BQ76940_OCD_THRESH_SHIFT);

    if (s_bq76940_write_byte_with_CRC(BQ76940_PROTECT2, protect2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set ocd threshold: write PROTECT2 err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 设置短路电流值
 * @param {bq76940_scd_threshold_value_e} value 短路电流值 单位mV
 * @param {bq76940_scd_threshold_delay_e} delay 短路电流触发时间 单位μs
 * @return {*}
 */
bq76940_state_e bq76940_set_scd_threshold(bq76940_scd_threshold_value_e value, bq76940_scd_threshold_delay_e delay)
{
    uint8_t protect1_val = 0;

    if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT1, &protect1_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set scd threshold: read PROTECT1 err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* PROTECT1 [4:3] 延迟时间设置  [2:0] 短路阈值设置 */
    protect1_val &= ~(BQ76940_SCD_DELAY_MASK | BQ76940_SCD_THRESH_MASK);
    protect1_val |= (delay << BQ76940_SCD_DELAY_SHIFT) | (value << BQ76940_SCD_THRESH_SHIFT);

    if (s_bq76940_write_byte_with_CRC(BQ76940_PROTECT1, protect1_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to set scd threshold: write PROTECT1 err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 获取过压值
 * @param {uint16_t} ov 过压值 单位mV
 * @return {*}
 */
bq76940_state_e bq76940_get_ov_threshold(uint16_t *ov)
{
    uint8_t ov_trip = 0;
    uint16_t ov_trip_full = 0;

    if (ov == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_OV_TRIP, &ov_trip) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get ov threshold: read OV_TRIP err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* Reconstruct 14-bit ADC: "10-OV_T<7:0>-1000" */
    ov_trip_full = 0x2000 | ((uint16_t)ov_trip << 4) | 0x0008;

    /* Convert to mV */
    *ov = (uint16_t)(ov_trip_full * s_bq79640_compensation_struct.gain_uv * 0.001f + s_bq79640_compensation_struct.offset_mv + 0.5f);

    return BQ76940_STATE_OK;
}

/**
 * @description: 获取欠压值
 * @param {uint16_t} uv 欠压值 单位mV
 * @return {*}
 */
bq76940_state_e bq76940_get_uv_threshold(uint16_t *uv)
{
    uint8_t uv_trip = 0;
    uint16_t uv_trip_full = 0;

    if (uv == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_UV_TRIP, &uv_trip) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get uv threshold: read UV_TRIP err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* Reconstruct 14-bit ADC: "01-UV_T<7:0>-0000" */
    uv_trip_full = 0x1000 | ((uint16_t)uv_trip << 4) | 0x0000;

    /* Convert to mV */
    *uv = (uint16_t)(uv_trip_full * s_bq79640_compensation_struct.gain_uv * 0.001f + s_bq79640_compensation_struct.offset_mv + 0.5f + 1.0f);

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取过流值
 * @param {uint8_t} value 过流值 参考@bq76940_ocd_threshold_value_e
 * @return {*}
 */
bq76940_state_e bq76940_get_ocd_threshold(uint8_t *value)
{
    uint8_t protect2_val = 0;

    if (value == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT2, &protect2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get ocd threshold: read PROTECT2 err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* Extract OCD_THRESH [3:0] */
    *value = (protect2_val & BQ76940_OCD_THRESH_MASK) >> BQ76940_OCD_THRESH_SHIFT;

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940获取短路电流
 * @param {uint8_t} value 短路电流值 参考@bq76940_scd_threshold_value_e
 * @return {*}
 */
bq76940_state_e bq76940_get_scd_threshold(uint8_t *value)
{
    uint8_t protect1_val = 0;

    if (value == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    if (s_bq76940_read_byte_with_CRC(BQ76940_PROTECT1, &protect1_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get scd threshold: read PROTECT1 err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* Extract SCD_THRESH [2:0] */
    *value = (protect1_val & BQ76940_SCD_THRESH_MASK) >> BQ76940_SCD_THRESH_SHIFT;

    return BQ76940_STATE_OK;
}

/**
 * @description: 获取故障状态
 * @return {*}
 */
bq76940_state_e bq76940_get_fault_status(uint8_t *fault_mask)
{
    uint8_t sys_stat_val = 0;

    if (fault_mask == NULL)
    {
        return BQ76940_STATE_ERR;
    }

    /* 读取 SYS_STAT 寄存器 */
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_STAT, &sys_stat_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to get fault status: read SYS_STAT err\r\n");
        return BQ76940_STATE_ERR;
    }

    *fault_mask = sys_stat_val;

    return BQ76940_STATE_OK;
}

/**
 * @description: 清除故障码
 * @param {uint8_t} mask 故障掩码
 * @return {*}
 */
bq76940_state_e bq76940_clear_fault(uint8_t mask)
{
    /* SYS_STAT 位 写1清除 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_STAT, mask) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to clear fault: write SYS_STAT err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940启用充电
 * @return {*}
 */
bq76940_state_e bq76940_enable_charge(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL2, &sys_ctrl2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enable charge: read SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }
    /* CHG_ON 置1 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL2, sys_ctrl2_val | BQ76940_CHG_ON) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enable charge: write SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940禁用充电
 * @return {*}
 */
bq76940_state_e bq76940_disable_charge(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL2, &sys_ctrl2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to disable charge: read SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }
    /* CHG_ON 清0 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL2, sys_ctrl2_val & ~BQ76940_CHG_ON) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to disable charge: write SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940启用放电
 * @return {*}
 */
bq76940_state_e bq76940_enable_discharge(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL2, &sys_ctrl2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enable discharge: read SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }
    /* DSG_ON 置1 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL2, sys_ctrl2_val | BQ76940_DSG_ON) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to enable discharge: write SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: bq76940禁用放电
 * @return {*}
 */
bq76940_state_e bq76940_disable_discharge(void)
{
    uint8_t sys_ctrl2_val = 0;
    if (s_bq76940_read_byte_with_CRC(BQ76940_SYS_CTRL2, &sys_ctrl2_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to disable discharge: read SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }
    /* DSG_ON 清0 */
    if (s_bq76940_write_byte_with_CRC(BQ76940_SYS_CTRL2, sys_ctrl2_val & ~BQ76940_DSG_ON) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to disable discharge: write SYS_CTRL2 err\r\n");

        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 开始均衡指定电芯
 * @param {uint8_t} cell_index 电芯索引 范围:1 - 15
 * @return {*}
 */
bq76940_state_e bq76940_start_balance(uint16_t cell_index)
{
    uint8_t bal_reg_addr = 0;
    uint8_t bal_bit_mask = 0;
    uint8_t cellbal[3] = {0}; /* 寄存器 CELLBAL 1  2  3 值*/
    uint16_t bal_map = 0;     /* 15-bit bitmap: bit[i]=1 表示电芯 i+1 正在均衡 */

    /* 1. 参数合法性检验 */
    if (cell_index < 1 || cell_index > 15)
    {
        LOG_E("Failed to start balance: cell index err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 2. 相邻电芯保护 不允许相邻两个电芯同时均衡: 读取全部 CELLBAL 寄存器, 构建 bitmap */
    if (s_bq76940_read_byte_with_CRC(BQ76940_CELLBAL1, &cellbal[0]) != BQ76940_STATE_OK ||
        s_bq76940_read_byte_with_CRC(BQ76940_CELLBAL2, &cellbal[1]) != BQ76940_STATE_OK ||
        s_bq76940_read_byte_with_CRC(BQ76940_CELLBAL3, &cellbal[2]) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to start balance: read CELLBAL err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 构建 15-bit bitmap: CELLBAL1[4:0]=电芯 5..1, CELLBAL2[4:0]=电芯 10..6, CELLBAL3[4:0]=电芯 15..11 */
    bal_map = (uint16_t)(cellbal[0] & 0x1F);        /* cells 1..5 */
    bal_map |= (uint16_t)(cellbal[1] & 0x1F) << 5;  /* cells 6..10 */
    bal_map |= (uint16_t)(cellbal[2] & 0x1F) << 10; /* cells 11..15 */

    /* 检查相邻两个电芯是否正在均衡 */
    if (cell_index > 1 && (bal_map & (1U << (cell_index - 2)))) /* 电芯 N-1 */
    {
        LOG_E("Failed to start balance: adjacent cell %d already balancing\r\n", cell_index - 1);
        return BQ76940_STATE_ERR;
    }
    if (cell_index < 15 && (bal_map & (1U << (cell_index)))) /* 电芯 N+1 */
    {
        LOG_E("Failed to start balance: adjacent cell %d already balancing\r\n", cell_index + 1);
        return BQ76940_STATE_ERR;
    }

    /* 3. 计算目标电芯寄存器和位掩码 */
    if (cell_index <= 5)
    {
        bal_reg_addr = BQ76940_CELLBAL1;
        bal_bit_mask = 1 << (cell_index - 1);
    }
    else if (cell_index <= 10)
    {
        bal_reg_addr = BQ76940_CELLBAL2;
        bal_bit_mask = 1 << (cell_index - 6);
    }
    else
    {
        bal_reg_addr = BQ76940_CELLBAL3;
        bal_bit_mask = 1 << (cell_index - 11);
    }

    cellbal[bal_reg_addr - BQ76940_CELLBAL1] |= bal_bit_mask;

    /* 4. 读-修改-写: 只设置目标电芯 */
    if (s_bq76940_write_byte_with_CRC(bal_reg_addr,
                                      cellbal[bal_reg_addr - BQ76940_CELLBAL1]) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to start balance: write reg err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

/**
 * @description: 停止均衡指定电芯
 * @param {uint8_t} cell_index 电芯索引 范围:1 - 15
 * @return {*}
 */
bq76940_state_e bq76940_stop_balance(uint16_t cell_index)
{
    uint8_t bal_reg_addr = 0;
    uint8_t bal_bit_mask = 0;
    uint8_t bal_val = 0;

    /* 1. 参数合法性检验 */
    if (cell_index < 1 || cell_index > 15)
    {
        LOG_E("Failed to stop balance: cell index err\r\n");
        return BQ76940_STATE_ERR;
    }

    /* 2. 计算目标电芯寄存器和位掩码 */
    if (cell_index <= 5)
    {
        bal_reg_addr = BQ76940_CELLBAL1;
        bal_bit_mask = 1 << (cell_index - 1);
    }
    else if (cell_index <= 10)
    {
        bal_reg_addr = BQ76940_CELLBAL2;
        bal_bit_mask = 1 << (cell_index - 6);
    }
    else
    {
        bal_reg_addr = BQ76940_CELLBAL3;
        bal_bit_mask = 1 << (cell_index - 11);
    }

    /* 3. 读-修改-写: 只设置目标电芯 */
    if (s_bq76940_read_byte_with_CRC(bal_reg_addr, &bal_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to stop balance: read reg err\r\n");
        return BQ76940_STATE_ERR;
    }

    bal_val &= ~bal_bit_mask;

    /* 写回寄存器 */
    if (s_bq76940_write_byte_with_CRC(bal_reg_addr, bal_val) != BQ76940_STATE_OK)
    {
        LOG_E("Failed to stop balance: write reg err\r\n");
        return BQ76940_STATE_ERR;
    }

    return BQ76940_STATE_OK;
}

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
