#include "app_data_collect_task.h"

/* C库 */
#include <string.h>

/* DEBUG文件 */
#include "debug.h"
/* BQ76940 驱动文件 */
#include "driver_bq76940.h"
/* RTOS 文件 */
#include "cmsis_os2.h"
/* 数据中心 */
#include "app_data_center.h"

/* DEBUG 标签定义 */
#define TAG "data collect"

/* 数据错误位定义 */
#define CHIP_TEMP_DATA_ERR 0x01
#define BATTERY_TEMP_DATA_ERR 0x02
#define BATTERY_CURRENT_DATA_ERR 0x04
#define BATTERY_VOLTAGE_DATA_ERR 0x08
#define BATTERY_CELL_VOLTAGE_DATA_ERR 0x10

/* 数据采集任务 */
osThreadId_t data_collect_task_handle;
const osThreadAttr_t bq76940_core_task_attributes = {
    .name = "bq76940_core_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal5,
};

/* 静态函数声明 */
static void s_battery_data_calc(app_battery_data_t *data);
static void s_battery_data_print(app_battery_data_t *data_struct);

/* 静态变量声明 */
static uint8_t s_data_err_status;

/* 电芯索引图 */
static const uint8_t s_cell_id_index_map[APP_CELL_NUM] = {
    1, 2,
    5, 6, 7,
    10, 11, 12,
    15};

/**
 * @description: 电池数据采集任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_data_collect_task(void *argument)
{
    app_battery_data_t app_battery_data_tmp;

    for (;;)
    {
        /* 获取芯片温度 */
        if (bq76940_get_internal_temperature(&app_battery_data_tmp.chip_temp) != BQ76940_STATE_OK)
        {
            DEBUG_ERROR(TAG, "get chip temperature err\r\n");
            s_data_err_status |= CHIP_TEMP_DATA_ERR;
        }
        /* 获取电池温度 */
        if (bq76940_get_external_temperature_ch(1, &app_battery_data_tmp.battery_temp) != BQ76940_STATE_OK)
        {
            DEBUG_ERROR(TAG, "get battery temperature err\r\n");
            s_data_err_status |= BATTERY_TEMP_DATA_ERR;
        }
        /* 获取当前电池电流 */
        if (bq76940_get_current(&app_battery_data_tmp.current_ma) != BQ76940_STATE_OK)
        {
            DEBUG_ERROR(TAG, "get battery current err\r\n");
            s_data_err_status |= BATTERY_CURRENT_DATA_ERR;
        }
        /* 获取电池电压 */
        if (bq76940_get_battery_voltage(&app_battery_data_tmp.battery_voltage_mv, APP_CELL_NUM) != BQ76940_STATE_OK)
        {
            DEBUG_ERROR(TAG, "get battery voltage err\r\n");
            s_data_err_status |= BATTERY_VOLTAGE_DATA_ERR;
        }
        /* 获取电芯电压 */
        for (uint8_t i = 0; i < APP_CELL_NUM; i++)
        {
            if (bq76940_get_cell_voltage(s_cell_id_index_map[i], &app_battery_data_tmp.cell_voltage_mv[i]) != BQ76940_STATE_OK)
            {
                DEBUG_ERROR(TAG, "get cell [%d] voltage err\r\n", i);
                s_data_err_status |= BATTERY_CELL_VOLTAGE_DATA_ERR;
                break;
            }
        }

        /* 检查数据是否正确 */
        if (s_data_err_status != 0)
        {
            DEBUG_ERROR(TAG, "get data err, code:0x%02X", s_data_err_status);
            continue;
        }

        /* 计算电压 最大 最小 平均值 */
        s_battery_data_calc(&app_battery_data_tmp);

        /* 设置数据值 */
        if (app_data_center_set_battery_data(&app_battery_data_tmp) != APP_DATA_OK)
        {
            DEBUG_ERROR(TAG, "set battery data err\r\n");
        }
        else
        {
            s_battery_data_print(&app_battery_data_tmp);
        }

        /* 清空数据缓存 */
        memset(&app_battery_data_tmp, 0, sizeof(app_battery_data_tmp));
        
        /* 延迟250ms */
        osDelay(250);
    }
}

/**
 * @description: 数据采集任务初始化
 * @return {*}
 */
void app_data_collect_task_init(void)
{
    data_collect_task_handle = osThreadNew(app_data_collect_task, NULL, &bq76940_core_task_attributes);
    if (data_collect_task_handle == NULL)
    {
        /* 任务创建失败 */
        DEBUG_ERROR(TAG, "data collect task init err\r\n");
        return;
    }
}

/**
 * @description: 电池数据计算
 * @param {app_battery_data_t} *data 电池数据结构体指针
 * @return {*}
 */
static void s_battery_data_calc(app_battery_data_t *data)
{
    uint16_t max_mv = data->cell_voltage_mv[0];
    uint16_t min_mv = data->cell_voltage_mv[0];
    uint32_t sum_mv = 0;
    uint8_t max_index = 0;

    for (uint8_t i = 0; i < APP_CELL_NUM; i++)
    {
        uint16_t voltage = data->cell_voltage_mv[i];

        sum_mv += voltage;

        /* 比当前值大 */
        if (voltage > max_mv)
        {
            max_mv = voltage;
            max_index = i;
        }
        /* 比当前值小 */
        if (voltage < min_mv)
        {
            min_mv = voltage;
        }
    }

    data->max_voltage_mv = max_mv;
    data->min_voltage_mv = min_mv;
    data->avg_voltage_mv = (uint16_t)(sum_mv / APP_CELL_NUM);
    data->max_voltage_diff_mv = max_mv - min_mv;
    data->max_voltage_index = max_index;
}

/**
 * @description: 电池数据串口打印
 * @param {app_battery_data_t} *data_struct 电池数据结构体指针
 * @return {*}
 */
static void s_battery_data_print(app_battery_data_t *data_struct)
{
    DEBUG_PRINTF(" =========== Battery Data =========== \r\n");
    DEBUG_PRINTF(" Chip temperature: %.1f C \r\n", data_struct->chip_temp * 0.1f);                                                    // 芯片温度
    DEBUG_PRINTF(" Battery temperature: %.1f C \r\n", data_struct->battery_temp * 0.1f);                                              // 电池温度
    DEBUG_PRINTF(" Battery current: %.3f A \r\n", data_struct->current_ma * 0.001f);                                                  // 电池电流
    DEBUG_PRINTF(" Battery voltage: %d.%03d V \r\n", data_struct->battery_voltage_mv / 1000, data_struct->battery_voltage_mv % 1000); // 电池电压
    for (uint8_t i = 0; i < APP_CELL_NUM; i++)
    {
        DEBUG_PRINTF("Cell [%d] voltage: %d.%03d\r\n", i + 1, data_struct->cell_voltage_mv[i] / 1000, data_struct->cell_voltage_mv[i] % 1000); // 单个电芯电压
    }
    DEBUG_PRINTF("Max voltage:  %d.%03d V \r\n", data_struct->max_voltage_mv / 1000, data_struct->max_voltage_mv % 1000);
    DEBUG_PRINTF("Min voltage:  %d.%03d V \r\n", data_struct->min_voltage_mv / 1000, data_struct->min_voltage_mv % 1000);
    DEBUG_PRINTF("Avg voltage:  %d.%03d V \r\n", data_struct->avg_voltage_mv / 1000, data_struct->avg_voltage_mv % 1000);
    DEBUG_PRINTF("Max voltage diff: %d.%03d V \r\n", data_struct->max_voltage_diff_mv / 1000, data_struct->max_voltage_diff_mv % 1000);
    DEBUG_PRINTF("Max voltage index: %d \r\n", data_struct->max_voltage_index);
}
