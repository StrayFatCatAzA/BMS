#include "app_sample_task.h"

/* C库*/
#include <string.h>

/* BQ76940 驱动文件 */
#include "driver_bq76940.h"
/* RTOS 文件 */
#include "cmsis_os2.h"
/* 数据管理 */
#include "app_data_manager.h"
/* 互斥锁管锁管理 */
#include "app_mutex_manager.h"

/* 任务 */
#include "app_protect_task.h"

/* DEBUG */
#include "log.h"
#define TAG "sample task"

/* 数据读取错误位定义 */
#define CHIP_TEMP_DATA_ERR 0x01            // 芯片温度读取错误
#define BATTERY_TEMP_DATA_ERR 0x02         // 电池温度数据读取错误
#define BATTERY_CURRENT_DATA_ERR 0x04      // 电池电流读取错误
#define BATTERY_VOLTAGE_DATA_ERR 0x08      // 电池电压读取错误
#define BATTERY_CELL_VOLTAGE_DATA_ERR 0x10 // 电芯电压读取错误
#define CHIP_STAT_DATA_ERR 0x20            // 芯片状态读取错误

/* 采样任务周期 250 ms */
#define SAMPLE_TASK_PERIOD 250

/* 电池采样任务 */
osThreadId_t sample_task_handle;
const osThreadAttr_t sample_task_attributes = {
    .name = "sample_task_handle",
    .stack_size = 128 * 4, // 512 Byte
    .priority = (osPriority_t)osPriorityNormal5,
};

/* 消息数据结构体 */
osMemoryPoolId_t battery_data_handle;
const osMemoryPoolAttr_t battery_data_attributes = {
    .name = "battery_data_handle",
    .mp_size = 128};

/* 静态函数声明*/

/* 静态变量声明*/
static uint16_t s_data_collect_err_code;            // 数据读取错误码

/* 电芯索引图 对应实际电芯位置 */
static const uint8_t s_cell_id_index_map[APP_CELL_NUM] = {
    1, 2,
    5, 6, 7,
    10, 11, 12,
    15};

/**
 * @description: 电池采样任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_sample_task(void *argument)
{
    app_battery_data_t app_battery_data_tmp;                                // 电池数据缓存
    uint32_t next_wake_time = osKernelGetTickCount();                       // 下一次读取时间 初始化为任务开始时间

    for (;;)
    {
        do
        {
            /* 数据读取错误码初始化 */
            s_data_collect_err_code = 0x00;

            /* 获取互斥锁准备读取数据 */
            if (osMutexAcquire(iic_soft_mutex, osWaitForever) != osOK)
            {
                LOG_ERROR(TAG, "i2c mutex acquire failed\r\n");
                break;
            }

            /* 获取芯片温度 */
            if (bq76940_get_internal_temperature(&app_battery_data_tmp.chip_temp) != BQ76940_OK)
            {
                s_data_collect_err_code |= CHIP_TEMP_DATA_ERR;
            }
            /* 获取电池温度 */
            if (bq76940_get_external_temperature_ch(1, &app_battery_data_tmp.battery_temp) != BQ76940_OK)
            {
                s_data_collect_err_code |= BATTERY_TEMP_DATA_ERR;
            }
            /* 获取当前电池电流 */
            if (bq76940_get_current(&app_battery_data_tmp.current_ma) != BQ76940_OK)
            {
                s_data_collect_err_code |= BATTERY_CURRENT_DATA_ERR;
            }
            /* 获取电池电压 */
            if (bq76940_get_battery_voltage(&app_battery_data_tmp.battery_voltage_mv, APP_CELL_NUM) != BQ76940_OK)
            {
                s_data_collect_err_code |= BATTERY_VOLTAGE_DATA_ERR;
            }
            /* 获取电芯电压 */
            for (uint8_t i = 0; i < APP_CELL_NUM; i++)
            {
                if (bq76940_get_cell_voltage(s_cell_id_index_map[i], &app_battery_data_tmp.cell_voltage_mv[i]) != BQ76940_OK)
                {
                    s_data_collect_err_code |= BATTERY_CELL_VOLTAGE_DATA_ERR;
                    break;
                }
            }

            /* 获取芯片错误码*/
            if (bq76940_get_fault_status(&app_battery_data_tmp.fault_code) != BQ76940_OK)
            {
                s_data_collect_err_code |= CHIP_STAT_DATA_ERR;
            }

            /* 读取数据完成 释放互斥锁*/
            osMutexRelease(iic_soft_mutex);

            /* 检查数据是否正确*/
            if (s_data_collect_err_code != 0)
            {
                LOG_ERROR(TAG, "get data err, code:0x%02X", s_data_collect_err_code);
                break;
            }
            else
            {
                /* 设置数据更新时间 */
                app_battery_data_tmp.update_time = osKernelGetTickCount();
            }

            /* 更新数据 */
            if (app_data_manager_set_battery_data(&app_battery_data_tmp) != APP_DATA_OK)
            {
                LOG_ERROR(TAG, "set battery data err\r\n");
                break;
            }
        } while (0);

        /* 清空数据缓存 */
        memset(&app_battery_data_tmp, 0, sizeof(app_battery_data_tmp));

        /* 采样任务延时 */
        next_wake_time += SAMPLE_TASK_PERIOD;
        osDelayUntil(next_wake_time);
    }
}
/**
 * @description: 数据采样任务初始化
 * @return {*}
 */
void app_sample_task_init(void)
{
    sample_task_handle = osThreadNew(app_sample_task, NULL, &sample_task_attributes);
    if (sample_task_handle == NULL)
    {
        /* 任务创建失败 */
        LOG_ERROR(TAG, "sample task init err\r\n");
        return;
    }
}
