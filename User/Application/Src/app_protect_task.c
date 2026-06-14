#include "app_protect_task.h"

/* RTOS */
#include "cmsis_os2.h"

/* 驱动 */
#include "driver_bq76940.h"
/* 数据管理 */
#include "app_data_manager.h"

/* debug */
#include "log.h"
#define TAG "protect task"

/* 内部静态函数 */
static void s_event_handle(uint32_t event);

/* 电池保护任务 */
osThreadId_t protect_task_handle;
const osThreadAttr_t protect_task_attributes = {
    .name = "protect_task_handle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal6,
};

/* 故障事件 */
osEventFlagsId_t fault_event_handle;
const osEventFlagsAttr_t fault_event_attributes = {
    .name = "fault_event_handle"};

/**
 * @description: 故障事件判断
 * @return {*}
 */
static void s_event_handle(uint32_t event)
{
    if (event & BMS_EVENT_SCD_FAULT)
    {
        /* 短路：立即断放电 FET */
        bq76940_disable_discharge();
    }
    else if (event & BMS_EVENT_OCD_FAULT)
    {
        /* 过流：断放电 FET */
        bq76940_disable_discharge();
    }
    else if (event & BMS_EVENT_OV_FAULT)
    {
        /* 过压：断充电 FET */
        bq76940_disable_charge();
    }
    else if (event & BMS_EVENT_UV_FAULT)
    {
        /* 欠压：断放电 FET */
        bq76940_disable_discharge();
    }
    else if (event & BMS_EVENT_CHIP_FAULT)
    {
        /* 芯片内部故障：全断 */
        bq76940_disable_charge();
        bq76940_disable_discharge();
    }
    else if (event & BMS_EVENT_TEMP_FAULT)
    {
        /* 温度：全断 */
        bq76940_disable_charge();
        bq76940_disable_discharge();
    }
    else if (event & BMS_EVENT_DATA_FAULT)
    {
        /* 数据过期：全断（保守策略） */
        bq76940_disable_charge();
        bq76940_disable_discharge();
    }

    /* 进入故障状态 */
    app_bms_state_e new_state = BMS_STATE_FAULT;
    app_data_manager_set_bms_state(&new_state);

    LOG_INFO(TAG,"BMS system is fault state now\r\n");
    LOG_INFO(TAG,"Fault Code: 0x%08lX\r\n",event);
}

/**
 * @description: 故障恢复函数
 * @return {*}
 */
static void s_recovery_handle(void)
{
    /* 启用充放电 */
    bq76940_enable_charge();
    bq76940_enable_discharge();

    /* 设置空闲状态 */
    app_bms_state_e new_state = BMS_STATE_IDEL;
    app_data_manager_set_bms_state(&new_state);

    LOG_INFO(TAG, "BMS recovered, FETs re-enabled\r\n");
}

/**
 * @description: 电池保护任务函数
 * @param {void} *argument
 * @return {*}
 */
void app_protect_task(void *argument)
{
    uint32_t fault_event;
    osStatus_t stat;
    for (;;)
    {
        stat = osOK;

        /* 阻塞等待事件 */
        fault_event = osEventFlagsWait(
            fault_event_handle,
            BMS_EVENT_MASK_ALL,
            osFlagsWaitAny,
            osWaitForever);

        /* 检查事件是不是错误  */
        if ((fault_event & 0x80000000) != 0)
        {
            stat = osError;
        }

        if (stat != osError)
        {
            /* 判断故障是否恢复 */
            if (fault_event & BMS_EVENT_CLEAR)
            {
                s_recovery_handle(); // 恢复正常 切回正常状态
            }
            else
            {
                s_event_handle(fault_event); // 存在故障 故障处理
            }
        }
    }
}

/**
 * @description: 获取错误事件句柄
 * @return {*}
 */
osEventFlagsId_t app_protect_task_get_fault_event_handle(void)
{
    return fault_event_handle;
}

/**
 * @description: 电池保护任务初始化
 * @return {*}
 */
void app_protect_task_init(void)
{
    /* 任务初始化 */
    protect_task_handle = osThreadNew(app_protect_task, NULL, &protect_task_attributes);
    if (protect_task_handle == NULL)
    {
        /* 任务初始化失败 */
        LOG_ERROR(TAG, "protect task init err\r\n");
        return;
    }

    /* 事件初始化 */
    fault_event_handle = osEventFlagsNew(&fault_event_attributes);
    if (fault_event_handle == NULL)
    {
        /* 事件初始化失败 */
        LOG_ERROR(TAG, "fault event init err\r\n");
        return;
    }

    return;
}
