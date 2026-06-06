#ifndef __APP_DATA_CENTER_TASK_H__
#define __APP_DATA_CENTER_TASK_H__

#include <stdint.h>

#define APP_CELL_NUM 9 /* 电芯数量 */

typedef struct
{
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

typedef enum
{
    APP_DATA_OK = 0,
    APP_DATA_ERR,
    APP_DATA_TIMEOUT,
    APP_DATA_NULL
} app_data_status_e;

void app_data_center_init(void);

app_data_status_e app_data_center_set_battery_data(app_battery_data_t *data);
app_data_status_e app_data_center_get_battery_data(app_battery_data_t *data);

#endif
