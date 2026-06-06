#include "app_task_init.h"
/* RTOS头 */
#include "cmsis_os2.h"
/* BSP层 */
#include "bsp_can.h"
#include "bsp_iic.h"
#include "bsp_uart1.h"
/* Driver层 */
#include "driver_led.h"
#include "driver_bq76940.h"
/* 任务层 */
#include "app_data_collect_task.h"
#include "app_balance_task.h"
#include "app_protect_task.h"
#include "app_battery_calc_task.h"
#include "app_can_task.h"
#include "app_data_center.h"
/* debug */
#include "debug.h"
#define TAG "APP_Init"

/* ================================= 内部静态函数声明 ================================= */

static void s_hardware_init(void);
static void s_bq76940_config(void);

void app_task_init(void)
{
  /* 1. 硬件初始化 初始化 */
  s_hardware_init();

  /* 2. BQ76940 配置 */
  s_bq76940_config();

  /* 3. 任务初始化 */

  osKernelLock(); // 初始化开始 锁定调度器 不允许切换
  
  app_data_center_init(); // 数据中心初始化

  app_data_collect_task_init(); // 电池数据采集任务初始化

  app_protect_task_init(); // 电池保护任务初始化

  app_battery_calc_task_init(); // 电池容量计算任务初始化

  app_balance_task_init(); // 电池均衡任务初始化

  app_can_task_init(); // CAN 通信任务初始化

  osKernelUnlock(); // 初始化完成 解锁调度器
}

/**
 * @description: 硬件初始化
 * @return {*}
 */
static void s_hardware_init(void)
{
  /* BSP层初始化 */
  bsp_uart1_init();
  // CAN

  /* Driver层初始化 */
  drv_led_init();
  bq76940_init();
}

/**
 * @description: BQ76940配置
 * @return {*}
 */
static void s_bq76940_config(void)
{
  /* 使能电流采集 */
  bq76940_set_current_collection(BQ76940_FUNC_ENABLE);
  /* 使能电压采集 */
  bq76940_set_voltage_collection(BQ76940_FUNC_ENABLE);
  /* 使能温度采集 外部温度模式 */
  bq76940_set_temperature_collection(BQ76940_TEMP_MODE_EXTERNAL);

  /* 配置过压 欠压电压阈值 */
  /* 过压延迟: 4s 过压电压: 4200mV */
  bq76940_set_ov_threshold(4200, BQ76940_OV_DELAY_4S);

  /* 欠压延迟: 4s 欠压电压: 3100mV */
  bq76940_set_uv_threshold(3100, BQ76940_UV_DELAY_4S);

  /* 配置过流 短路电流阈值 */
  bq76940_set_ocd_scd_level(BQ76940_OCD_SCD_LOW_LEVEL); /* 低等级 */

  /* 放电过流延迟：320ms, 过流电压：11mV 换算过流电流: 11mV/4mΩ = 2.75A */
  bq76940_set_ocd_threshold(BQ76940_OCD_VALUE_11MV, BQ76940_OCD_DELAY_320MS);

  /* 放电短路延迟：400us, 短路电压：22mV 换算短路电流: 22mV/4mΩ = 5.5A */
  bq76940_set_scd_threshold(BQ76940_SCD_VALUE_22MV, BQ76940_SCD_DELAY_400US);
}
