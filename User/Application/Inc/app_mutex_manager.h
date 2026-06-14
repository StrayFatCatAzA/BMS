#ifndef __APP_MUTEX_MANAGER_H__
#define __APP_MUTEX_MANAGER_H__



/* RTOS 头 */
#include "cmsis_os2.h"


extern osMutexId_t iic_soft_mutex;
extern osMutexId_t uart1_mutex;

void app_mutex_manager_init(void);



#endif
