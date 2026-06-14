#include "app_sox_task.h"

/* RTOS */
#include "cmsis_os.h"
/* 数据管理 */
#include "app_data_manager.h"

/* debug */
#include "log.h"
#define TAG "sox task"

/* 监控任务周期 750 ms */
#define SOX_TASK_PERIOD 750
/* 数据过期时间 5000 ms */
#define DATA_EXPIREATION_TIME 5000

/* 电池容量计算任务 */
osThreadId_t sox_task_handle;
const osThreadAttr_t sox_task_attributes = {
    .name = "sox_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

/**
 * @description: 电池容量计算任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_sox_task(void *argument)
{
    app_battery_data_t battery_data;

    uint32_t next_wake_time = osKernelGetTickCount();
    for (;;)
    {
        if (app_data_manager_get_battery_data(&battery_data) != APP_DATA_OK)
        {
            goto delay;
        }

        

    delay:
        next_wake_time += SOX_TASK_PERIOD;
        osDelayUntil(next_wake_time); 
    }
}

/**
 * @description: 电池均衡任务初始化
 * @return {*}
 */
void app_sox_task_init(void)
{
    sox_task_handle = osThreadNew(app_sox_task, NULL, &sox_task_attributes);
    if (sox_task_handle == NULL)
    {
        /* 任务初始化失败 */
        LOG_ERROR(TAG, "batterry calc task init err\r\n");
        return;
    }
}
