#ifndef __APP_PROTECT_TASK_H__
#define __APP_PROTECT_TASK_H__

#include "cmsis_os2.h"

/**
 * @description: 错误事件枚举
 * @return {*}
 */
#define BMS_EVENT_NONE_FAULT 0
#define BMS_EVENT_CHIP_FAULT (1 << 0) // 芯片内部错误
#define BMS_EVENT_TEMP_FAULT (1 << 1) // 温度错误
#define BMS_EVENT_OV_FAULT (1 << 2)   // 过压
#define BMS_EVENT_UV_FAULT (1 << 3)   // 欠压
#define BMS_EVENT_SCD_FAULT (1 << 4)  // 短路
#define BMS_EVENT_OCD_FAULT (1 << 5)  // 过流

#define BMS_EVENT_DATA_FAULT (1 << 6) // 数据过期

#define BMS_EVENT_CLEAR       (1 << 7)   // 清除所有故障

#define BMS_EVENT_ALL_FAULTS  (BMS_EVENT_CHIP_FAULT | BMS_EVENT_TEMP_FAULT | \
                               BMS_EVENT_OV_FAULT | BMS_EVENT_UV_FAULT |   \
                               BMS_EVENT_SCD_FAULT | BMS_EVENT_OCD_FAULT |  \
                               BMS_EVENT_DATA_FAULT)

#define BMS_EVENT_MASK_ALL    (BMS_EVENT_ALL_FAULTS | BMS_EVENT_CLEAR)

void app_protect_task_init(void);

osEventFlagsId_t app_protect_task_get_fault_event_handle(void);

#endif
