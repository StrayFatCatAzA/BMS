#include "app_can_task.h"

/* RTOS */
#include "cmsis_os2.h"

/* debug 头 */
#include "log.h"
#define TAG "can task"

/* CAN通信任务 */
osThreadId_t can_task_handle;
const osThreadAttr_t can_task_attributes = {
    .name = "can_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal2,
};

/**
 * @description: CAN 通信任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_can_task(void *argument)
{
    for (;;)
    {
    }
}


/**
 * @description: CAN 通信任务初始化
 * @return {*}
 */
void app_can_task_init(void)
{
    can_task_handle = osThreadNew(app_can_task, NULL, &can_task_attributes);
    if (can_task_handle == NULL)
    {
        /* 任务初始化失败 */
        LOG_ERROR(TAG, "can task init err\r\n");
        return;
    }
}
