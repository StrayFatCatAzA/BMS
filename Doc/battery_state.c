/**
 * @file    battery_state.c
 * @brief   电池状态类实现
 *
 * 面向对象 C 风格：
 *   - struct BatteryState 在 .c 中定义 (opaque pointer 模式)
 *   - 所有公开方法以 BatteryState_ 为前缀
 *   - 内部互斥锁保证线程安全
 *   - 构造函数动态分配内存以支持可变电芯数量
 */

#include "battery_state.h"
#include <stdlib.h>
#include <string.h>

/* =========================== 内部结构定义 =========================== */

#define BATT_CELL_MAX 16

struct BatteryState {
    /* ——— 测量数据 ——— */
    int16_t  chip_temp;                        /* 芯片温度       0.1°C */
    int16_t  battery_temp;                     /* 电池温度       0.1°C */
    int16_t  current_ma;                       /* 电流           mA   */
    uint16_t pack_voltage_mv;                  /* 电池总电压     mV   */
    uint16_t cell_voltage_mv[BATT_CELL_MAX];   /* 各电芯电压     mV   */

    /* ——— 系统状态 ——— */
    BatteryState_SystemState_e system_state;   /* 当前运行状态        */
    uint8_t  soc;                              /* SOC  0 ~ 100 %     */
    uint8_t  fault_code;                       /* 故障码位掩码       */

    /* ——— 标志位 ——— */
    uint8_t  is_charge;                        /* 充电标志 */
    uint8_t  is_discharge;                     /* 放电标志 */
    uint8_t  is_balance;                       /* 均衡标志 */

    /* ——— 元数据 ——— */
    uint8_t  cell_count;                       /* 实际电芯串数 */
    uint32_t update_time;                      /* 最后更新时间戳  */

    /* ——— 线程安全 ——— */
    /* 注：实际集成时替换为 RTOS 互斥锁句柄 */
    int      _lock;                            /* 简化锁标记 (0=空闲, 1=锁定) */
};

/* =========================== 内部辅助方法 =========================== */

/**
 * @brief  加锁 (简化实现，集成时替换为 osMutexAcquire)
 */
static BatteryState_Status_e s_lock(struct BatteryState *self)
{
    if (self == NULL) return BATT_NULL;
    /* 单线程文档示例用；集成时替换:
     * if (osMutexAcquire(self->_mutex, osWaitForever) != osOK)
     *     return BATT_TIMEOUT;
     */
    self->_lock = 1;
    return BATT_OK;
}

/**
 * @brief  解锁 (简化实现，集成时替换为 osMutexRelease)
 */
static void s_unlock(struct BatteryState *self)
{
    if (self == NULL) return;
    self->_lock = 0;
    /* 集成时替换: osMutexRelease(self->_mutex); */
}

/* =========================== 生命周期 =========================== */

BatteryState_t BatteryState_Create(uint8_t cell_count)
{
    if (cell_count == 0 || cell_count > BATT_CELL_MAX) {
        return NULL;
    }

    struct BatteryState *self = (struct BatteryState *)malloc(sizeof(struct BatteryState));
    if (self == NULL) {
        return NULL;
    }

    /* 零初始化所有字段 */
    memset(self, 0, sizeof(struct BatteryState));

    self->cell_count   = cell_count;
    self->system_state = BATT_STATE_IDLE;
    self->soc          = 0;
    self->fault_code   = BATT_FAULT_NONE;

    return (BatteryState_t)self;
}

void BatteryState_Destroy(BatteryState_t self)
{
    if (self == NULL) return;
    free(self);
}

/* =========================== 系统状态 =========================== */

BatteryState_Status_e BatteryState_SetSystemState(BatteryState_t self,
                                                   BatteryState_SystemState_e state)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->system_state = state;

    /* 保持标志位与状态一致 */
    if (state == BATT_STATE_FAULT) {
        self->is_charge    = 0;
        self->is_discharge = 0;
    }

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetSystemState(BatteryState_t self,
                                                   BatteryState_SystemState_e *state)
{
    if (state == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *state = self->system_state;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 电池总电压 =========================== */

BatteryState_Status_e BatteryState_SetPackVoltage(BatteryState_t self,
                                                   uint16_t voltage_mv)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->pack_voltage_mv = voltage_mv;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetPackVoltage(BatteryState_t self,
                                                   uint16_t *voltage_mv)
{
    if (voltage_mv == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *voltage_mv = self->pack_voltage_mv;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 电芯电压 =========================== */

BatteryState_Status_e BatteryState_SetCellVoltage(BatteryState_t self,
                                                   uint8_t index,
                                                   uint16_t voltage_mv)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    if (index >= self->cell_count) {
        s_unlock(self);
        return BATT_ERR;
    }

    self->cell_voltage_mv[index] = voltage_mv;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetCellVoltage(BatteryState_t self,
                                                   uint8_t index,
                                                   uint16_t *voltage_mv)
{
    if (voltage_mv == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    if (index >= self->cell_count) {
        s_unlock(self);
        return BATT_ERR;
    }

    *voltage_mv = self->cell_voltage_mv[index];

    s_unlock(self);
    return BATT_OK;
}

uint8_t BatteryState_GetCellCount(BatteryState_t self)
{
    if (self == NULL) return 0;
    return self->cell_count;
}

BatteryState_Status_e BatteryState_GetCellVoltageMax(BatteryState_t self,
                                                      uint16_t *voltage_mv)
{
    if (voltage_mv == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    uint16_t max_val = 0;
    for (uint8_t i = 0; i < self->cell_count; i++) {
        if (self->cell_voltage_mv[i] > max_val) {
            max_val = self->cell_voltage_mv[i];
        }
    }
    *voltage_mv = max_val;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetCellVoltageMin(BatteryState_t self,
                                                      uint16_t *voltage_mv)
{
    if (voltage_mv == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    uint16_t min_val = 0xFFFF;
    for (uint8_t i = 0; i < self->cell_count; i++) {
        if (self->cell_voltage_mv[i] < min_val) {
            min_val = self->cell_voltage_mv[i];
        }
    }
    *voltage_mv = (min_val == 0xFFFF) ? 0 : min_val;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 电流 =========================== */

BatteryState_Status_e BatteryState_SetCurrent(BatteryState_t self,
                                               int16_t current_ma)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->current_ma = current_ma;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetCurrent(BatteryState_t self,
                                               int16_t *current_ma)
{
    if (current_ma == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *current_ma = self->current_ma;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 温度 =========================== */

BatteryState_Status_e BatteryState_SetChipTemp(BatteryState_t self,
                                                int16_t temp)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->chip_temp = temp;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetChipTemp(BatteryState_t self,
                                                int16_t *temp)
{
    if (temp == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *temp = self->chip_temp;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_SetBatteryTemp(BatteryState_t self,
                                                   int16_t temp)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->battery_temp = temp;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetBatteryTemp(BatteryState_t self,
                                                   int16_t *temp)
{
    if (temp == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *temp = self->battery_temp;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== SOC =========================== */

BatteryState_Status_e BatteryState_SetSOC(BatteryState_t self, uint8_t soc)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    if (soc > 100) {
        self->soc = 100;
    } else {
        self->soc = soc;
    }

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetSOC(BatteryState_t self, uint8_t *soc)
{
    if (soc == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *soc = self->soc;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 故障码 =========================== */

BatteryState_Status_e BatteryState_SetFaultCode(BatteryState_t self,
                                                 uint8_t fault_code)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->fault_code = fault_code;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetFaultCode(BatteryState_t self,
                                                 uint8_t *fault_code)
{
    if (fault_code == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *fault_code = self->fault_code;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_HasFault(BatteryState_t self,
                                             uint8_t *has_fault)
{
    if (has_fault == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *has_fault = (self->fault_code != BATT_FAULT_NONE) ? 1 : 0;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 充/放电标志位 =========================== */

BatteryState_Status_e BatteryState_SetChargeFlag(BatteryState_t self,
                                                  uint8_t is_charge)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->is_charge = (is_charge != 0) ? 1 : 0;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetChargeFlag(BatteryState_t self,
                                                  uint8_t *is_charge)
{
    if (is_charge == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *is_charge = self->is_charge;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_SetDischargeFlag(BatteryState_t self,
                                                     uint8_t is_discharge)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->is_discharge = (is_discharge != 0) ? 1 : 0;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetDischargeFlag(BatteryState_t self,
                                                     uint8_t *is_discharge)
{
    if (is_discharge == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *is_discharge = self->is_discharge;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_SetBalanceFlag(BatteryState_t self,
                                                   uint8_t is_balance)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->is_balance = (is_balance != 0) ? 1 : 0;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetBalanceFlag(BatteryState_t self,
                                                   uint8_t *is_balance)
{
    if (is_balance == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *is_balance = self->is_balance;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 时间戳 =========================== */

BatteryState_Status_e BatteryState_SetUpdateTime(BatteryState_t self,
                                                  uint32_t tick)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->update_time = tick;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_GetUpdateTime(BatteryState_t self,
                                                  uint32_t *tick)
{
    if (tick == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    *tick = self->update_time;

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 批量操作 =========================== */

BatteryState_Status_e BatteryState_WriteMeasurementData(BatteryState_t self,
                                                         int16_t chip_temp,
                                                         int16_t battery_temp,
                                                         int16_t current_ma,
                                                         uint16_t pack_voltage_mv,
                                                         const uint16_t *cell_voltages_mv,
                                                         uint8_t fault_code,
                                                         uint32_t tick)
{
    if (cell_voltages_mv == NULL) return BATT_ERR;
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->chip_temp        = chip_temp;
    self->battery_temp     = battery_temp;
    self->current_ma       = current_ma;
    self->pack_voltage_mv  = pack_voltage_mv;
    self->fault_code       = fault_code;
    self->update_time      = tick;

    memcpy(self->cell_voltage_mv, cell_voltages_mv,
           self->cell_count * sizeof(uint16_t));

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_ReadMeasurementData(BatteryState_t self,
                                                        int16_t *chip_temp,
                                                        int16_t *battery_temp,
                                                        int16_t *current_ma,
                                                        uint16_t *pack_voltage_mv,
                                                        uint16_t *cell_voltages_mv,
                                                        uint8_t *fault_code,
                                                        uint32_t *tick)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    if (chip_temp)        *chip_temp        = self->chip_temp;
    if (battery_temp)     *battery_temp     = self->battery_temp;
    if (current_ma)       *current_ma       = self->current_ma;
    if (pack_voltage_mv)  *pack_voltage_mv  = self->pack_voltage_mv;
    if (fault_code)       *fault_code       = self->fault_code;
    if (tick)             *tick             = self->update_time;

    if (cell_voltages_mv) {
        memcpy(cell_voltages_mv, self->cell_voltage_mv,
               self->cell_count * sizeof(uint16_t));
    }

    s_unlock(self);
    return BATT_OK;
}

/* =========================== 状态机便捷方法 =========================== */

BatteryState_Status_e BatteryState_EnterFault(BatteryState_t self,
                                               uint8_t fault_code)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->system_state = BATT_STATE_FAULT;
    self->fault_code   = fault_code;
    self->is_charge    = 0;
    self->is_discharge = 0;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_ClearFault(BatteryState_t self)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    self->system_state = BATT_STATE_IDLE;
    self->fault_code   = BATT_FAULT_NONE;

    s_unlock(self);
    return BATT_OK;
}

BatteryState_Status_e BatteryState_UpdateChargeDischargeByCurrent(BatteryState_t self)
{
    BatteryState_Status_e ret = s_lock(self);
    if (ret != BATT_OK) return ret;

    /* 故障状态下不更新充放电标志 */
    if (self->system_state == BATT_STATE_FAULT) {
        s_unlock(self);
        return BATT_OK;
    }

    /* 小电流视为静置，清除充放电标志 */
    #define IDLE_CURRENT_MA 50
    if (self->current_ma > IDLE_CURRENT_MA) {
        self->is_charge    = 1;
        self->is_discharge = 0;
        self->system_state = BATT_STATE_CHARGE;
    } else if (self->current_ma < -IDLE_CURRENT_MA) {
        self->is_charge    = 0;
        self->is_discharge = 1;
        self->system_state = BATT_STATE_DISCHARGE;
    } else {
        self->is_charge    = 0;
        self->is_discharge = 0;
        /* 保持当前状态 (IDLE 或从前一状态恢复) */
    }

    s_unlock(self);
    return BATT_OK;
}
