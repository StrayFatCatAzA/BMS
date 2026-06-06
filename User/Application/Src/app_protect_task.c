#include "app_protect_task.h"

/* 调试文件 */
#include "debug.h"

/* RTOS */
#include "cmsis_os2.h"

#define TAG "protect task"

/* 电池保护任务 */
osThreadId_t protect_task_handle;
const osThreadAttr_t protect_task_attributes = {
    .name = "protect_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal6,
};

/**
 * @description: 电池保护任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_protect_task(void *argument)
{

    for (;;)
    {
    }
}


/**
 * @description: 电池保护任务初始化
 * @return {*}
 */
void app_protect_task_init(void)
{
    protect_task_handle = osThreadNew(app_protect_task, NULL, &protect_task_attributes);
    if (protect_task_handle == NULL)
    {
        /* 任务初始化失败 */
        DEBUG_ERROR(TAG, "protect task init err\r\n");
        return;
    }
}
