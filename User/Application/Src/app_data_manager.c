#include "app_data_manager.h"

/* RTOS */
#include "cmsis_os2.h"

/* 调试工具 */
#include "log.h"
#define TAG "data manager"

/* 电池数据互斥量 */
osMutexId_t battery_data_mutex;
const osMutexAttr_t battery_data_mutex_attributes = {
    .name = "battery_data_mutex"};

/* 电池系统状态互斥量 */
osMutexId_t bms_state_mutex;
const osMutexAttr_t bms_state_attributes = {
    .name = "bms_state_mutex"};

/* 电池数据结构体 */
static app_battery_data_t s_battery_data_struct = {
    .fault_code = APP_FAULT_CODE_NONE,

    .battery_temp = 0,
    .chip_temp = 0,
    .current_ma = 0,
    .battery_voltage_mv = 0,

    .update_time = 0
};

/* 电池状态信息结构体 */
static app_bms_system_info_t s_bms_system_info = {
    .bms_state = BMS_STATE_IDEL,
    .is_charge = 0,
    .is_discharge = 0,
    .is_balance = 0
};

/**
 * @description: 数据中心初始化
 * @return {*}
 */
void app_data_manager_init(void)
{
    /* 互斥量初始化 */
    battery_data_mutex = osMutexNew(&battery_data_mutex_attributes);
    if (battery_data_mutex == NULL)
    {
        LOG_ERROR(TAG, "battery data mutex init err\r\n");
        return;
    }

    bms_state_mutex = osMutexNew(&bms_state_attributes);
    if (bms_state_mutex == NULL)
    {
        LOG_ERROR(TAG, "BMS state mutex init err\r\n");
        return;
    }

    return;
}

/**
 * @description: 设置电池系统状态
 * @param {app_bms_state_e} *newstate 新状态
 * @return {*}
 */
app_data_status_e app_data_manager_set_bms_state(app_bms_state_e *newstate)
{
    if (osMutexAcquire(bms_state_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 赋值 */
    s_bms_system_info.bms_state = *newstate;

    osMutexRelease(bms_state_mutex);

    return APP_DATA_OK;
}

/**
 * @description: 获取电池系统状态
 * @param {app_bms_state_e} *state 状态
 * @return {*}
 */
app_data_status_e app_data_manager_get_bms_state(app_bms_state_e *state)
{
    if (osMutexAcquire(bms_state_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 赋值 */
    *state = s_bms_system_info.bms_state;

    osMutexRelease(bms_state_mutex);

    return APP_DATA_OK;
}

/**
 * @description: 设置电池数据
 * @param {app_battery_data_t} *data 电池数据结构体指针
 * @return {*}
 */
app_data_status_e app_data_manager_set_battery_data(app_battery_data_t *data)
{
    if (osMutexAcquire(battery_data_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 结构体赋值 */
    s_battery_data_struct = *data;

    osMutexRelease(battery_data_mutex);

    return APP_DATA_OK;
}

/**
 * @description: 获取电池数据
 * @param {app_battery_data_t} *data 电池数据结构体指针
 * @return {*}
 */
app_data_status_e app_data_manager_get_battery_data(app_battery_data_t *data)
{
    if (osMutexAcquire(battery_data_mutex, 100) != osOK)
    {
        return APP_DATA_TIMEOUT;
    }

    /* 结构体赋值 */
    *data = s_battery_data_struct;

    osMutexRelease(battery_data_mutex);

    return APP_DATA_OK;
}
