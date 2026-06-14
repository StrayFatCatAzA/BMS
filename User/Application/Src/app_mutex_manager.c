#include "app_mutex_manager.h"

/* debug 头 */
#include "log.h"
#define TAG "mutex manager"

/* 软件IIC互斥量定义 */
osMutexId_t iic_soft_mutex;
const osMutexAttr_t iic_mutex_attributes = {
    .name = "iic_soft_mutex"
};

/* UART1互斥量定义 */
osMutexId_t uart1_mutex;
const osMutexAttr_t uart1_mutex_attributes = {
    .name = "uart1_mutex"
};

/**
 * @description: 硬件互斥量初始化
 * @return {*}
 */
void app_mutex_manager_init(void)
{
    /* 软件IIC互斥量初始化 */
    iic_soft_mutex = osMutexNew(&iic_mutex_attributes);
    if (iic_soft_mutex == NULL)
    {
        return;
    }

    /* UART1互斥量初始化 */
    uart1_mutex = osMutexNew(&uart1_mutex_attributes);
    if (uart1_mutex == NULL)
    {
        return;
    }

    return;
}








