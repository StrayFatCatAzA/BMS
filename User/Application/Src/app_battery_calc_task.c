#include "app_battery_calc_task.h"


/* 调试文件 */
#include "debug.h"
/* RTOS */
#include "cmsis_os.h"

#define TAG "battery calc"

/* 电池容量计算任务 */
osThreadId_t battery_calc_task_handle;
const osThreadAttr_t battery_calc_task_attributes = {
    .name = "battery_calc_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

/**
 * @description: 电池容量计算任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_battery_calc_task(void *argument)
{

    for (;;)
    {
    }
}


/**
 * @description: 电池均衡任务初始化
 * @return {*}
 */
void app_battery_calc_task_init(void)
{
    battery_calc_task_handle = osThreadNew(app_battery_calc_task, NULL, &battery_calc_task_attributes);
    if (battery_calc_task_handle == NULL)
    {
        /* 任务初始化失败 */
        DEBUG_ERROR(TAG, "batterry calc task init err\r\n");
        return;
    }
}
