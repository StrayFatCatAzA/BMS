#include "app_monitor_task.h"

/* RTOS */
#include "cmsis_os2.h"
/* 数据管理 */
#include "app_data_manager.h"
/* 任务 */
#include "app_protect_task.h"

/* debug */
#include "log.h"
#define TAG "monitor task"

/* 监控任务周期 750 ms */
#define MONITOR_TASK_PERIOD 750
/* 数据过期时间 5000 ms */
#define DATA_EXPIREATION_TIME 5000

/* 电池监控任务 */
osThreadId_t monitor_task_handle;
const osThreadAttr_t monitor_task_attributes = {
    .name = "monitor_task_handle",
    .stack_size = 256 * 4, // 1024 Byte
    .priority = (osPriority_t)osPriorityNormal5,
};

/* 静态函数声明 */
static void s_battery_data_print(app_battery_data_t *data_struct);

/* 内部静态变量 */
static osEventFlagsId_t s_fault_event_handle = NULL;

/**
 * @description: 电池监控任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_monitor_task(void *argument)
{
    app_battery_data_t battery_data;                                  // 数据缓冲
    uint32_t next_wake_time = osKernelGetTickCount();                 // 下一次读取时间 初始化为任务开始时间
    s_fault_event_handle = app_protect_task_get_fault_event_handle(); // 获取错误事件句柄

    for (;;)
    {
        /* 事件初始化 */
        uint32_t event = BMS_EVENT_NONE_FAULT;

        /* 数据获取 */
        if (app_data_manager_get_battery_data(&battery_data) != APP_DATA_OK)
        {
            LOG_ERROR(TAG, "get data err\r\n");
        }
        else
        {
            /* 判断数据时效 */
            uint32_t data_update_time = battery_data.update_time;
            uint32_t data_curr_time = osKernelGetTickCount();

            if (data_curr_time - data_update_time > DATA_EXPIREATION_TIME)
            {
                /* 数据过时 */
                event |= BMS_EVENT_DATA_FAULT;
            }

            /* 发现错误 */
            if (battery_data.fault_code != 0x00)
            {
                /* 判断错误事件 */
                if (battery_data.fault_code & APP_FAULT_CODE_DEVICE_XREADY)
                    event |= BMS_EVENT_CHIP_FAULT;

                if (battery_data.fault_code & APP_FAULT_CODE_OV)
                    event |= BMS_EVENT_OV_FAULT;

                if (battery_data.fault_code & APP_FAULT_CODE_UV)
                    event |= BMS_EVENT_UV_FAULT;

                if (battery_data.fault_code & APP_FAULT_CODE_OCD)
                    event |= BMS_EVENT_OCD_FAULT;

                if (battery_data.fault_code & APP_FAULT_CODE_SCD)
                    event |= BMS_EVENT_SCD_FAULT;
            }

            /* 温度过高 */
            if (battery_data.chip_temp > 600 || battery_data.battery_temp > 700)
            {
                event |= BMS_EVENT_TEMP_FAULT;
            }

            if (event == BMS_EVENT_NONE_FAULT)
            {
                /* 无故障：清除所有旧故障位，设置 CLEAR */
                osEventFlagsClear(s_fault_event_handle, BMS_EVENT_ALL_FAULTS);
                osEventFlagsSet(s_fault_event_handle, BMS_EVENT_CLEAR);
            }
            else
            {
                /* 有故障：清除 CLEAR 位，设置故障位 */
                osEventFlagsClear(s_fault_event_handle, BMS_EVENT_CLEAR);
                osEventFlagsSet(s_fault_event_handle, event);
            }

            /* 数据打印 */
            s_battery_data_print(&battery_data);
        }

        /* 监控任务延迟 */
        next_wake_time += MONITOR_TASK_PERIOD;
        osDelayUntil(next_wake_time);
    }
}

/**
 * @description: 电池监控任务初始化
 * @return {*}
 */
void app_monitor_task_init(void)
{
    monitor_task_handle = osThreadNew(app_monitor_task, NULL, &monitor_task_attributes);
    if (monitor_task_handle == NULL)
    {
        LOG_ERROR(TAG, "monitor task init err\r\n");
        return;
    }
}

/**
 * @description: 电池数据串口打印
 * @param {app_battery_data_t} *data_struct 电池数据结构体指针
 * @return {*}
 */
static void s_battery_data_print(app_battery_data_t *data_struct)
{
    LOG_PRINTF(" =========== Battery Data =========== \r\n");
    LOG_PRINTF(" Chip temperature: %.1f C \r\n", data_struct->chip_temp * 0.1f);                                                    // 芯片温度
    LOG_PRINTF(" Battery temperature: %.1f C \r\n", data_struct->battery_temp * 0.1f);                                              // 电池温度
    LOG_PRINTF(" Battery current: %.3f A \r\n", data_struct->current_ma * 0.001f);                                                  // 电池电流
    LOG_PRINTF(" Battery voltage: %d.%03d V \r\n", data_struct->battery_voltage_mv / 1000, data_struct->battery_voltage_mv % 1000); // 电池电压
    for (uint8_t i = 0; i < APP_CELL_NUM; i++)
    {
        LOG_PRINTF("Cell [%d] voltage: %d.%03d\r\n", i + 1, data_struct->cell_voltage_mv[i] / 1000, data_struct->cell_voltage_mv[i] % 1000); // 单个电芯电压
    }
}
