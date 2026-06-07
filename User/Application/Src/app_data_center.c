#include "app_data_center.h"

/* 调试工具 */
#include "debug.h"
/* RTOS */
#include "cmsis_os2.h"

#define TAG "data center"

/* 数据互斥量 */
osMutexId_t battery_data_mutex;
const osMutexAttr_t battery_data_attributes = {
    .name = "battery_data_mutex"};

/* 电池数据结构体 */
static app_battery_data_t s_battery_data_struct = {
    .battery_alert = APP_SYS_STAT_NONE,

    .battery_temp = 0,
    .chip_temp = 0,
    .current_ma = 0,
    .battery_voltage_mv = 0,

    .max_voltage_mv = 0,
    .min_voltage_mv = 0,
    .avg_voltage_mv = 0,
    .max_voltage_diff_mv = 0,
    .max_voltage_index = 0
};

/**
 * @description: 数据中心初始化
 * @return {*}
 */
void app_data_center_init(void)
{
    /* 互斥量初始化 */
    battery_data_mutex = osMutexNew(&battery_data_attributes);

    if (battery_data_mutex == NULL)
    {
        DEBUG_ERROR(TAG, "battery data mutex init err\r\n");
        
        return;
    }
}

/**
 * @description: 获取电池数据
 * @param {app_battery_data_t} *data 电池数据结构体指针
 * @return {*}
 */
app_data_status_e app_data_center_set_battery_data(app_battery_data_t *data)
{
    if(osMutexAcquire(battery_data_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 结构体赋值 */
    s_battery_data_struct = *data;

    osMutexRelease(battery_data_mutex);

    return APP_DATA_OK;
}

/**
 * @description: 设置电池数据
 * @param {app_battery_data_t} *data 电池数据结构体指针
 * @return {*}
 */
app_data_status_e app_data_center_get_battery_data(app_battery_data_t *data)
{
    if(osMutexAcquire(battery_data_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 结构体赋值 */
    *data = s_battery_data_struct;

    osMutexRelease(battery_data_mutex);

    return APP_DATA_OK;
}
