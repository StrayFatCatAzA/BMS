#ifndef __DRIVER_BQ76940_H__
#define __DRIVER_BQ76940_H__

#include <stdint.h>

/**
 * @description: bq76940 系统模式枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_MODE_NORMAL = 0,
    BQ76940_MODE_SHIP = 1,
} bq76940_system_mode_e;

/**
 * @description: bq76940 状态枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_STATE_OK = 0, /* 成功 */
    BQ76940_STATE_ERR     /* 失败 */
} bq76940_state_e;

/**
 * @description: bq76940 错误码枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_ERR_CODE_XREADY = 0x80,        /* 新的库仑计数器读数可用 */
    BQ76940_ERR_CODE_DEVICE_XREADY = 0x20, /* 内部芯片故障指示器 */
    BQ76940_ERR_CODE_OVRD_ALERT = 0x10,    /* ALERT引脚的外部上拉指示 */
    BQ76940_ERR_CODE_UV = 0x08,            /* 欠压故障事件指示器 */
    BQ76940_ERR_CODE_OV = 0x04,            /* 过压故障事件指示器 */
    BQ76940_ERR_CODE_SCD = 0x02,           /* 放电故障事件 短路指示 */
    BQ76940_ERR_CODE_OCD = 0x01,           /* 放电故障事件 过流指示 */
} bq76940_err_code_e;

/**
 * @description: 功能状态枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_FUNC_DISENABLE = 0, /* 失能功能 */
    BQ76940_FUNC_ENABLE = 1     /* 使能功能 */
} bq76940_function_state_e;

/**
 * @description: 温度获取模式
 * @return {*}
 */
typedef enum
{
    BQ76940_TEMP_MODE_INTERNAL = 0, /* 内部模式 */
    BQ76940_TEMP_MODE_EXTERNAL = 1  /* 外部模式 */
} bq76940_temp_mode_e;

/**
 * @description: 过流和短路等级枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_OCD_SCD_LOW_LEVEL = 0,
    BQ76940_OCD_SCD_HIGH_LEVEL = 1
} bq76940_ocd_scd_level_e;

/**
 * @description: 过流延迟时间 OCD 延迟时间枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_OCD_DELAY_8MS = 0x0,    /* 8 ms   */
    BQ76940_OCD_DELAY_20MS = 0x1,   /* 20 ms  */
    BQ76940_OCD_DELAY_40MS = 0x2,   /* 40 ms  */
    BQ76940_OCD_DELAY_80MS = 0x3,   /* 80 ms  */
    BQ76940_OCD_DELAY_160MS = 0x4,  /* 160 ms */
    BQ76940_OCD_DELAY_320MS = 0x5,  /* 320 ms */
    BQ76940_OCD_DELAY_640MS = 0x6,  /* 640 ms */
    BQ76940_OCD_DELAY_1280MS = 0x7, /* 1280 ms */
} bq76940_ocd_threshold_delay_e;

/**
 * @description: 过流阈值 OCD 阈值枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_OCD_VALUE_8MV = 0x0,  /* RSNS=0:  8 mV | RSNS=1: 17 mV */
    BQ76940_OCD_VALUE_11MV = 0x1, /* RSNS=0: 11 mV | RSNS=1: 22 mV */
    BQ76940_OCD_VALUE_14MV = 0x2, /* RSNS=0: 14 mV | RSNS=1: 28 mV */
    BQ76940_OCD_VALUE_17MV = 0x3, /* RSNS=0: 17 mV | RSNS=1: 33 mV */
    BQ76940_OCD_VALUE_19MV = 0x4, /* RSNS=0: 19 mV | RSNS=1: 39 mV */
    BQ76940_OCD_VALUE_22MV = 0x5, /* RSNS=0: 22 mV | RSNS=1: 44 mV */
    BQ76940_OCD_VALUE_25MV = 0x6, /* RSNS=0: 25 mV | RSNS=1: 50 mV */
    BQ76940_OCD_VALUE_28MV = 0x7, /* RSNS=0: 28 mV | RSNS=1: 56 mV */
    BQ76940_OCD_VALUE_31MV = 0x8, /* RSNS=0: 31 mV | RSNS=1: 61 mV */
    BQ76940_OCD_VALUE_33MV = 0x9, /* RSNS=0: 33 mV | RSNS=1: 67 mV */
    BQ76940_OCD_VALUE_36MV = 0xA, /* RSNS=0: 36 mV | RSNS=1: 72 mV */
    BQ76940_OCD_VALUE_39MV = 0xB, /* RSNS=0: 39 mV | RSNS=1: 78 mV */
    BQ76940_OCD_VALUE_42MV = 0xC, /* RSNS=0: 42 mV | RSNS=1: 83 mV */
    BQ76940_OCD_VALUE_44MV = 0xD, /* RSNS=0: 44 mV | RSNS=1: 89 mV */
    BQ76940_OCD_VALUE_47MV = 0xE, /* RSNS=0: 47 mV | RSNS=1: 94 mV */
    BQ76940_OCD_VALUE_50MV = 0xF, /* RSNS=0: 50 mV | RSNS=1: 100 mV */
} bq76940_ocd_threshold_value_e;

/**
 * @description: 短路延迟时间 SCD 延迟时间枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_SCD_DELAY_70US = 0x0,  /* 70 µs  */
    BQ76940_SCD_DELAY_100US = 0x1, /* 100 µs */
    BQ76940_SCD_DELAY_200US = 0x2, /* 200 µs */
    BQ76940_SCD_DELAY_400US = 0x3, /* 400 µs（仅推荐 Rc=1kΩ 系统） */
} bq76940_scd_threshold_delay_e;

/**
 * @description: 短路电流 SCD 阈值枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_SCD_VALUE_22MV = 0x0,  /* RSNS=0:  22 mV | RSNS=1:  44 mV */
    BQ76940_SCD_VALUE_33MV = 0x1,  /* RSNS=0:  33 mV | RSNS=1:  67 mV */
    BQ76940_SCD_VALUE_44MV = 0x2,  /* RSNS=0:  44 mV | RSNS=1:  89 mV */
    BQ76940_SCD_VALUE_56MV = 0x3,  /* RSNS=0:  56 mV | RSNS=1: 111 mV */
    BQ76940_SCD_VALUE_67MV = 0x4,  /* RSNS=0:  67 mV | RSNS=1: 133 mV */
    BQ76940_SCD_VALUE_78MV = 0x5,  /* RSNS=0:  78 mV | RSNS=1: 155 mV */
    BQ76940_SCD_VALUE_89MV = 0x6,  /* RSNS=0:  89 mV | RSNS=1: 178 mV */
    BQ76940_SCD_VALUE_100MV = 0x7, /* RSNS=0: 100 mV | RSNS=1: 200 mV */
} bq76940_scd_threshold_value_e;

/**
 * @description: 过压延迟时间 SCD 延迟时间枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_OV_DELAY_1S = 0x0, /* 1 秒 */
    BQ76940_OV_DELAY_2S = 0x1, /* 2 秒 */
    BQ76940_OV_DELAY_4S = 0x2, /* 4 秒 */
    BQ76940_OV_DELAY_8S = 0x3, /* 8 秒 */
} bq76940_ov_threshold_delay_e;

/**
 * @description: 欠压延迟时间 SCD 延迟时间枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_UV_DELAY_1S = 0x0,  /* 1 秒  */
    BQ76940_UV_DELAY_4S = 0x1,  /* 4 秒  */
    BQ76940_UV_DELAY_8S = 0x2,  /* 8 秒  */
    BQ76940_UV_DELAY_16S = 0x3, /* 16 秒 */
} bq76940_uv_threshold_delay_e;

/**
 * @description: bq76940初始化
 * @return {*}
 */
bq76940_state_e bq76940_init(void);

/**
 * @description: bq76940进入低功耗模式
 * @return {*}
 */
bq76940_state_e bq76940_enter_ship(void);

/**
 * @description: bq76940唤醒
 * @return {*}
 */
bq76940_state_e bq76940_wake_up(void);

/**
 * @description: bq76940设置电压采样状态
 * @param {bq76940_ADC_state} state 电压采集功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_voltage_collection(bq76940_function_state_e state);

/**
 * @description: bq76940设置电流采样状态
 * @param {bq76940_function_state_e} state 电流采样功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_current_collection(bq76940_function_state_e state);

/**
 * @description: bq76940设置温度采样状态
 * @param {bq76940_temp_mode_e} mode 采集模式
 * @return {*}
 */
bq76940_state_e bq76940_set_temperature_collection(bq76940_temp_mode_e mode);

/**
 * @description: bq76940获取校准参数
 * @param {uint16_t} *adc_gain ADC增益指针
 * @param {int8_t} *adc_offset ADC偏置指针
 * @return {*}
 */
bq76940_state_e bq76940_get_calibration(uint16_t *adc_gain, int8_t *adc_offset);

/**
 * @description: bq76940获取指定电芯电压
 * @param {uint16_t} cell_index 电芯索引 0-14
 * @param {uint16_t} *voltage 电芯电压
 * @return {*}
 */
bq76940_state_e bq76940_get_cell_voltage(uint16_t cell_index, uint16_t *voltage);

/**
 * @description: bq76940获取全部电池电压
 * @param {uint16_t} *voltage 接收电芯电压数组
 * @param {uint16_t} vol_len 电芯数量
 * @return {*}
 */
bq76940_state_e bq76940_get_all_cell_voltage(uint16_t *voltage, uint16_t vol_len);

/**
 * @description: bq76940获取电池总电压
 * @param {uint16_t} *battery_voltage 电池总电压
 * @param {uint16_t} cell_num 电芯数量
 * @return {*}
 */
bq76940_state_e bq76940_get_battery_voltage(uint16_t *total_voltage, uint16_t cell_num);

/**
 * @description: bq76940获取芯片外部温度
 * @param channel 温度通道 1=TS1, 2=TS2, 3=TS3
 * @param temperature 芯片外部温度  输出单位: 0.1℃
                                   例如 253 表示 25.3℃
 *
 * @return {*}
 */
bq76940_state_e bq76940_get_external_temperature_ch(uint8_t channel, int16_t *temperature);

/**
 * @description: bq76940获取芯片内部温度
 * @param temperature 芯片内部温度 输出单位: 0.1℃
 *                                例如 253 表示 25.3℃
 * @return {*}
 */
bq76940_state_e bq76940_get_internal_temperature(int16_t *temperature);

/**
 * @description: 获取电流值
 * @param {uint16_t} *current 电流值 单位: mA
 * @return {*}
 */
bq76940_state_e bq76940_get_current(int16_t *current);

/**
 * @description: 获取CC原始值
 * @param {uint16_t} *cc_raw 原始CC值 单位: uV
 * @return {*}
 */
bq76940_state_e bq76940_get_current_raw(int16_t *cc_raw);

/**
 * @description: 设置过压值和过压触发时间
 * @param {uint16_t} ov 过压值 单位mV 范围: 3.15V ~ 4.70V  3136mV ~ 4674mV
 * @param {bq76940_ov_threshold_delay_e} delay 过压触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_ov_threshold(uint16_t ov, bq76940_ov_threshold_delay_e delay);

/**
 * @description: 设置欠压值
 * @param {uint16_t} uv 欠压值 单位mV  范围：1.58V ~ 3.10V  1589mV ~ 3125mV
 * @param {bq76940_uv_threshold_delay_e} delay 欠压触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_uv_threshold(uint16_t uv, bq76940_uv_threshold_delay_e delay);

/**
 * @description: bq76940设置过流、短路等级
 * @param {bq76940_ocd_sed_level_e} level 过流、短路等级
 * @return {*}
 * @note 高档位 X2
 */
bq76940_state_e bq76940_set_ocd_scd_level(bq76940_ocd_scd_level_e level);

/**
 * @description: 设置过电流值
 * @param {bq76940_ocd_threshold_value_e} value 过电流值
 * @param {bq76940_ocd_threshold_delay_e} delay 过电流触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_ocd_threshold(bq76940_ocd_threshold_value_e value, bq76940_ocd_threshold_delay_e delay);

/**
 * @description: 设置短路电流值
 * @param {bq76940_scd_threshold_value_e} value 短路电流值
 * @param {bq76940_scd_threshold_delay_e} delay 短路电流触发时间
 * @return {*}
 */
bq76940_state_e bq76940_set_scd_threshold(bq76940_scd_threshold_value_e value, bq76940_scd_threshold_delay_e delay);

/**
 * @description: 获取过压值
 * @param {uint16_t} ov 过压值 单位mV
 * @return {*}
 */
bq76940_state_e bq76940_get_ov_threshold(uint16_t *ov);

/**
 * @description: 获取欠压值
 * @param {uint16_t} uv 欠压值 单位mV
 * @return {*}
 */
bq76940_state_e bq76940_get_uv_threshold(uint16_t *uv);

/**
 * @description: 获取过流值
 * @param {uint8_t} value 过流值 
 * @return {*}
 */
bq76940_state_e bq76940_get_ocd_threshold(uint8_t *value);

/**
 * @description: 获取短路电流
 * @param {uint8_t} value 短路电流值
 * @return {*}
 */
bq76940_state_e bq76940_get_scd_threshold(uint8_t *value);

/**
 * @description: 获取故障状态
 * @param {uint8_t} *fault_mask 故障码
 * @return {*}
 */
bq76940_state_e bq76940_get_fault_status(uint8_t *fault_mask);

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
 * @param {uint16_t} cell_index 电芯索引 范围:1 - 15
 * @return {*}
 */
bq76940_state_e bq76940_start_balance(uint16_t cell_index);

/**
 * @description: 停止均衡指定电芯
 * @param {uint16_t} cell_index 电芯索引 范围:1 - 15
 * @return {*}
 */
bq76940_state_e bq76940_stop_balance(uint16_t cell_inde);

void bq76940_static_test(void);

#endif
