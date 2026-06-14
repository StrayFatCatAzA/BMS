#ifndef __BATTERY_STATE_H__
#define __BATTERY_STATE_H__

#include <stdint.h>

/*===========================================================================
 * 电池状态类 (BatteryState)
 *
 * 面向对象风格封装，遵循以下约定：
 *   - 不透明指针 (opaque pointer) 隐藏实现细节
 *   - 构造函数 / 析构函数管理生命周期
 *   - 所有属性通过 getter / setter 访问
 *   - 内部自带互斥锁，线程安全
 *=========================================================================== */

/* 前置声明 — 外部只看到指针，看不到内部结构 */
typedef struct BatteryState *BatteryState_t;

/*===========================================================================
 * 枚举类型
 *=========================================================================== */

/** 电池系统运行状态 */
typedef enum {
    BATT_STATE_IDLE      = 0,  /* 空闲 */
    BATT_STATE_CHARGE    = 1,  /* 充电中 */
    BATT_STATE_DISCHARGE = 2,  /* 放电中 */
    BATT_STATE_FAULT     = 3   /* 故障 */
} BatteryState_SystemState_e;

/** API 返回状态 */
typedef enum {
    BATT_OK      = 0,  /* 成功 */
    BATT_ERR     = 1,  /* 一般错误 */
    BATT_TIMEOUT = 2,  /* 互斥锁超时 */
    BATT_NULL    = 3   /* 空指针 */
} BatteryState_Status_e;

/** 故障码位定义 (与 BQ76940 对齐) */
enum {
    BATT_FAULT_NONE        = 0x00,
    BATT_FAULT_OCD         = 0x01,  /* 放电过流 */
    BATT_FAULT_SCD         = 0x02,  /* 短路 */
    BATT_FAULT_OV          = 0x04,  /* 过压 */
    BATT_FAULT_UV          = 0x08,  /* 欠压 */
    BATT_FAULT_OVER_ALERT  = 0x10,  /* ALERT 引脚故障 */
    BATT_FAULT_DEVICE_ERR  = 0x20   /* 芯片内部故障 */
};

/*===========================================================================
 * 生命周期
 *=========================================================================== */

/**
 * @brief  创建电池状态对象
 * @param  cell_count  电芯串数 (1~16)
 * @return 对象句柄，失败返回 NULL
 */
BatteryState_t BatteryState_Create(uint8_t cell_count);

/**
 * @brief  销毁电池状态对象
 * @param  self  对象句柄
 */
void BatteryState_Destroy(BatteryState_t self);

/*===========================================================================
 * 系统状态  getter / setter
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetSystemState(BatteryState_t self,
                                                  BatteryState_SystemState_e state);
BatteryState_Status_e BatteryState_GetSystemState(BatteryState_t self,
                                                  BatteryState_SystemState_e *state);

/*===========================================================================
 * 电池总电压  getter / setter  (单位: mV)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetPackVoltage(BatteryState_t self,
                                                   uint16_t voltage_mv);
BatteryState_Status_e BatteryState_GetPackVoltage(BatteryState_t self,
                                                   uint16_t *voltage_mv);

/*===========================================================================
 * 电芯电压  getter / setter  (单位: mV)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetCellVoltage(BatteryState_t self,
                                                   uint8_t index,
                                                   uint16_t voltage_mv);
BatteryState_Status_e BatteryState_GetCellVoltage(BatteryState_t self,
                                                   uint8_t index,
                                                   uint16_t *voltage_mv);

/** 获取电芯串数 */
uint8_t BatteryState_GetCellCount(BatteryState_t self);

/** 获取最高/最低电芯电压 */
BatteryState_Status_e BatteryState_GetCellVoltageMax(BatteryState_t self,
                                                      uint16_t *voltage_mv);
BatteryState_Status_e BatteryState_GetCellVoltageMin(BatteryState_t self,
                                                      uint16_t *voltage_mv);

/*===========================================================================
 * 电流  getter / setter  (单位: mA, 充电为正, 放电为负)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetCurrent(BatteryState_t self,
                                               int16_t current_ma);
BatteryState_Status_e BatteryState_GetCurrent(BatteryState_t self,
                                               int16_t *current_ma);

/*===========================================================================
 * 温度  getter / setter  (单位: 0.1°C, 即 -100 = -10.0°C)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetChipTemp(BatteryState_t self,
                                                int16_t temp);
BatteryState_Status_e BatteryState_GetChipTemp(BatteryState_t self,
                                                int16_t *temp);

BatteryState_Status_e BatteryState_SetBatteryTemp(BatteryState_t self,
                                                   int16_t temp);
BatteryState_Status_e BatteryState_GetBatteryTemp(BatteryState_t self,
                                                   int16_t *temp);

/*===========================================================================
 * SOC (State of Charge)  getter / setter  (单位: %, 0~100)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetSOC(BatteryState_t self, uint8_t soc);
BatteryState_Status_e BatteryState_GetSOC(BatteryState_t self, uint8_t *soc);

/*===========================================================================
 * 故障码  getter / setter
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetFaultCode(BatteryState_t self,
                                                 uint8_t fault_code);
BatteryState_Status_e BatteryState_GetFaultCode(BatteryState_t self,
                                                 uint8_t *fault_code);

/** 便捷方法：判断是否存在故障 */
BatteryState_Status_e BatteryState_HasFault(BatteryState_t self, uint8_t *has_fault);

/*===========================================================================
 * 充/放电标志位  getter / setter
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetChargeFlag(BatteryState_t self,
                                                  uint8_t is_charge);
BatteryState_Status_e BatteryState_GetChargeFlag(BatteryState_t self,
                                                  uint8_t *is_charge);

BatteryState_Status_e BatteryState_SetDischargeFlag(BatteryState_t self,
                                                     uint8_t is_discharge);
BatteryState_Status_e BatteryState_GetDischargeFlag(BatteryState_t self,
                                                     uint8_t *is_discharge);

BatteryState_Status_e BatteryState_SetBalanceFlag(BatteryState_t self,
                                                   uint8_t is_balance);
BatteryState_Status_e BatteryState_GetBalanceFlag(BatteryState_t self,
                                                   uint8_t *is_balance);

/*===========================================================================
 * 时间戳  getter / setter  (tick 值)
 *=========================================================================== */

BatteryState_Status_e BatteryState_SetUpdateTime(BatteryState_t self,
                                                  uint32_t tick);
BatteryState_Status_e BatteryState_GetUpdateTime(BatteryState_t self,
                                                  uint32_t *tick);

/*===========================================================================
 * 批量操作 (高效：一次加锁完成读写)
 *=========================================================================== */

/**
 * @brief  批量写入全部测量数据 (sample task 使用)
 */
BatteryState_Status_e BatteryState_WriteMeasurementData(BatteryState_t self,
                                                         int16_t chip_temp,
                                                         int16_t battery_temp,
                                                         int16_t current_ma,
                                                         uint16_t pack_voltage_mv,
                                                         const uint16_t *cell_voltages_mv,
                                                         uint8_t fault_code,
                                                         uint32_t tick);

/**
 * @brief  批量读取全部测量数据 (monitor / sox 任务使用)
 */
BatteryState_Status_e BatteryState_ReadMeasurementData(BatteryState_t self,
                                                        int16_t *chip_temp,
                                                        int16_t *battery_temp,
                                                        int16_t *current_ma,
                                                        uint16_t *pack_voltage_mv,
                                                        uint16_t *cell_voltages_mv,
                                                        uint8_t *fault_code,
                                                        uint32_t *tick);

/*===========================================================================
 * 状态机便捷方法
 *=========================================================================== */

/** 进入故障状态 (同时清除充放电标志) */
BatteryState_Status_e BatteryState_EnterFault(BatteryState_t self,
                                               uint8_t fault_code);

/** 清除故障，回到空闲状态 */
BatteryState_Status_e BatteryState_ClearFault(BatteryState_t self);

/** 由电流方向自动判断充放电状态 */
BatteryState_Status_e BatteryState_UpdateChargeDischargeByCurrent(BatteryState_t self);

#endif /* __BATTERY_STATE_H__ */
