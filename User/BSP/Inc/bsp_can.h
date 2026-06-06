#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

#include "stdint.h"

/**
 * @description: CAN状态枚举
 * @return {*}
 */
typedef enum
{
    CAN_STATE_OK = 0,
    CAN_STATE_ERR = 1
}bsp_can_state_e;

#endif
