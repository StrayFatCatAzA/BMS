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
    BQ76940_STATE_OK = 0,
    BQ76940_STATE_ERR
} bq76940_state_e;

/**
 * @description: bq76940 错误码枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_ERR_CODE_XREADY = 0,
    BQ76940_ERR_CODE_DEVICE_XREADY,
    BQ76940_ERR_CODE_OVRD_ALERT,
    BQ76940_ERR_CODE_UV,
    BQ76940_ERR_CODE_OV,
    BQ76940_ERR_CODE_SCD,
    BQ76940_ERR_CODE_OCD,
} bq76940_err_code_e;

/**
 * @description: 功能状态枚举
 * @return {*}
 */
typedef enum
{
    BQ76940_DISENABLE = 0,
    BQ76940_ENABLE = 1
} bq76940_function_state_e;


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
 * @param {bq76940_function_state_e} state 温度采样功能状态
 * @return {*}
 */
bq76940_state_e bq76940_set_temperature_collection(bq76940_function_state_e state);

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
bq76940_state_e bq76940_get_battery_voltage(uint16_t *total_voltage,uint16_t cell_num);

/**
 * @description: bq76940获取芯片外部温度
 * @param {uint8_t} *NTC_temperature 芯片外部温度
 * @return {*}
 */
bq76940_state_e bq76940_get_external_temperature(int16_t *temperature);

/**
 * @description: bq76940获取芯片内部温度
 * @param {uint8_t} *temperature 芯片内部温度
 * @return {*}
 */
bq76940_state_e bq76940_get_internal_temperature(int16_t *temperature);

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
 * @param {uint16_t} cell_index 电芯索引
 * @param {uint8_t} *cell 电池数组指针
 * @param {uint16_t} cells_len 电池数组长度
 * @return {*}
 */
bq76940_state_e bq76940_start_banlance(uint16_t cell_index, uint8_t *cell, uint16_t cells_len);

/**
 * @description: 停止均衡指定电芯
 * @param {uint16_t} cell_index 电芯索引
 * @param {uint8_t} *cell 电池数组指针
 * @param {uint16_t} cells_len 电池数组长度
 * @return {*}
 */
bq76940_state_e bq76940_stop_banlance(uint16_t cell_index, uint8_t *cell, uint16_t cells_len);

void bq76940_static_test(void);

#endif
