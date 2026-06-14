#ifndef __APP_DATA_MANAGER_TASK_H__
#define __APP_DATA_MANAGER_TASK_H__

#include "cmsis_os2.h"

#define APP_CELL_NUM 9 /* 电芯数量 由实际电路设计决定 */

/**
 * @description: 数据状态返回值
 * @return {*}
 */
typedef enum
{
    APP_DATA_OK = 0,
    APP_DATA_ERR,
    APP_DATA_TIMEOUT,
    APP_DATA_NULL
} app_data_status_e;

/**
 * @description: BQ76940 错误状态枚举
 * @return {*}
 */
enum
{
    APP_FAULT_CODE_NONE = 0x00,
    APP_FAULT_CODE_OCD = 0x01,          // 过流
    APP_FAULT_CODE_SCD = 0x02,          // 短路
    APP_FAULT_CODE_OV = 0x04,           // 过压
    APP_FAULT_CODE_UV = 0x08,           // 欠压
    APP_FAULT_CODE_OVER_ALERT = 0x10,   // ALERT引脚故障
    APP_FAULT_CODE_DEVICE_XREADY = 0x20 // 内部芯片故障
};

/**
 * @description: 电池数据结构体
 * @return {*}
 */
typedef struct
{
    int16_t chip_temp;                      /* 芯片温度 0.1℃  */
    int16_t battery_temp;                   /* 电池温度 0.1℃  */
    int16_t current_ma;                     /* 当前电流 mA     */
    uint16_t battery_voltage_mv;            /* 当前电池电压 mV */
    uint16_t cell_voltage_mv[APP_CELL_NUM]; /* 当前电芯组电压 mV */

    uint8_t fault_code; /* 错误码 */

    uint32_t update_time; /* 数据更新时间 */
} app_battery_data_t;

/* 电池系统状态机 */
typedef enum
{
    BMS_STATE_IDEL = 0,      /* 空闲 */
    BMS_STATE_CHARGE = 1,    /* 充电 */
    BMS_STATE_DISCHARGE = 3, /* 放电 */
    BMS_STATE_FAULT = 4,     /* 错误 */
} app_bms_state_e;

typedef struct
{
    app_bms_state_e bms_state; /* 当前系统状态 */
    uint8_t is_charge;         /* 充电标志位 */
    uint8_t is_discharge;      /* 放电标志位 */
    uint8_t is_balance;        /* 均衡标志位 */
} app_bms_system_info_t;

void app_data_manager_init(void);

app_data_status_e app_data_manager_set_bms_state(app_bms_state_e *newstate);
app_data_status_e app_data_manager_get_bms_state(app_bms_state_e *state);

app_data_status_e app_data_manager_set_battery_data(app_battery_data_t *data);
app_data_status_e app_data_manager_get_battery_data(app_battery_data_t *data);

#endif
