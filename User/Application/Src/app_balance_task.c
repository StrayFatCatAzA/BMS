#include "app_balance_task.h"

/* 调试文件 */
#include "debug.h"
/* RTOS 文件 */
#include "cmsis_os2.h"

#define TAG "balance task"

/* 电池均衡任务 */
osThreadId_t balance_task_handle;
const osThreadAttr_t balance_task_attributes = {
    .name = "balance_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal1,
};

/**
 * @description: 电池均衡任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_balance_task(void *argument)
{

    for (;;)
    {
    }
}

/**
 * @description: 电池均衡任务初始化
 * @return {*}
 */
void app_balance_task_init(void)
{
    balance_task_handle = osThreadNew(app_balance_task, NULL, &balance_task_attributes);
    if (balance_task_handle == NULL)
    {
        /* 任务创建失败 */
        DEBUG_ERROR(TAG, "balance task init err\r\n");
        return;
    }
}
