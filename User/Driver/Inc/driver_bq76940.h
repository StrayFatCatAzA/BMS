#ifndef __DRIVER_BQ76940_H__
#define __DRIVER_BQ76940_H__

#include <stdint.h>

typedef enum
{
    BQ76940_MODE_NORMAL = 0,
    BQ76940_MODE_SHIP = 1,
}bq76940_mode_e;

typedef enum
{
    BQ76940_STATE_OK = 0,
    BQ76940_STATE_ERR
} bq76940_state_e;

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
 * @description: bq76940软复位
 * @return {*}
 */
bq76940_state_e bq76940_reset(void);

/**
 * @description: bq76940获取指定索引的电池电压
 * @param {uint8_t} cell_index 电池索引
 * @param {uint8_t} *cell 电池电压
 * @return {*}
 */
bq76940_state_e bq76940_get_cell_voltage(uint8_t cell_index, uint8_t *cell);

/**
 * @description: bq76940获取全部索引的电池电压
 * @param {uint8_t} *cells 电池电压数组指针
 * @return {*}
 */
bq76940_state_e bq76940_get_all_cell_voltage(uint8_t *cells);

/**
 * @description: bq76940获取总电压
 * @param {uint8_t} *pack_cells 总电压
 * @return {*}
 */
bq76940_state_e bq76940_get_pack_voltage(uint8_t *pack_cells);

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
bq76940_state_e bq76940_start_banlance(uint8_t cell_index); 

/**
 * @description: 停止均衡指定电芯
 * @param {uint8_t} cell_index 电芯索引
 * @return {*}
 */
bq76940_state_e bq76940_stop_banlance(uint8_t cell_index); 



void bq76940_static_test(void);

#endif
