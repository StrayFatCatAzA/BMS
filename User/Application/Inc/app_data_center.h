#ifndef __APP_DATA_CENTER_TASK_H__
#define __APP_DATA_CENTER_TASK_H__

#include <stdint.h>

#define APP_CELL_NUM 9 /* 电芯数量 */

/**
 * @description: 数据读取状态
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
 * @description: BQ76940 状态
 * @return {*}
 */
enum
{
    APP_SYS_STAT_NONE = 0x00,
    APP_SYS_STAT_OCD = 0x01, // 过流
    APP_SYS_STAT_SCD = 0x02, // 短路
    APP_SYS_STAT_OV = 0x04, // 过压
    APP_SYS_STAT_UV = 0x08, // 欠压
    APP_SYS_STAT_OVER_ALERT = 0x10, // ALERT引脚故障
    APP_SYS_STAT_DEVICE_XREADY = 0x20 // 内部芯片故障
};

/**
 * @description: 电池数据结构体
 * @return {*}
 */
typedef struct
{
    uint8_t battery_alert;      /* 电池警报结构体  */

    int16_t chip_temp;                      /* 芯片温度 0.1℃  */
    int16_t battery_temp;                   /* 电池温度 0.1℃  */
    int16_t current_ma;                     /* 当前电流 mA     */
    uint16_t battery_voltage_mv;            /* 当前电池电压 mV */
    uint16_t cell_voltage_mv[APP_CELL_NUM]; /* 当前电芯电压 mV */

    uint16_t max_voltage_mv;      /* 最大电压 mV  */
    uint16_t min_voltage_mv;      /* 最小电压 mV  */
    uint16_t avg_voltage_mv;      /* 平均电压 mV  */
    uint16_t max_voltage_diff_mv; /* 最大压差 mV  */
    uint8_t max_voltage_index;    /* 最大电压索引 */
} app_battery_data_t;



void app_data_center_init(void);

app_data_status_e app_data_center_set_battery_data(app_battery_data_t *data);
app_data_status_e app_data_center_get_battery_data(app_battery_data_t *data);

#endif
