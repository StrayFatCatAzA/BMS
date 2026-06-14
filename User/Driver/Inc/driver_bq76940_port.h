#ifndef __DRIVER_BQ76940_PORT_H__
#define __DRIVER_BQ76940_PORT_H__

/* 该文件为BQ76940内部接口声明文件 只能在 driver_bq76940.c 中导入 */

#include "driver_bq76940.h"

/* ==================== I2C Hardware Interface ==================== */
bq76940_state_e s_bq76940_interface_iic_init(void);

bq76940_state_e s_bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
bq76940_state_e s_bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ==================== GPIO / Chip Control ==================== */

void s_bq76940_interface_gpio_init(void);
bq76940_state_e s_bq76940_interface_wake_up(void);
void s_bq76940_interface_delay_ms(uint32_t ms);

#endif
