#ifndef __DRIVER_BQ76940_PORT_H__
#define __DRIVER_BQ76940_PORT_H__

#include "driver_bq76940.h"

/* ==================== I2C Hardware Interface ==================== */

bq76940_state_e s_bq76940_interface_write_byte(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
bq76940_state_e s_bq76940_interface_read_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ==================== GPIO / Chip Control ==================== */

void s_bq76940_interface_gpio_init(void);
bq76940_state_e s_bq76940_interface_wake_up(void);
void s_bq76940_interface_delay_ms(uint32_t ms);

/* ==================== Debug Log Output ==================== */

void s_bq76940_interface_log(const char *fmt, ...);

#define LOG   s_bq76940_interface_log

#define LOG_E s_bq76940_interface_log

#endif
