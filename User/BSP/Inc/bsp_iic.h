#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__

#include <stdint.h>

#define IIC_OK    0
#define IIC_ERR   1

uint8_t iic_soft_init(void);

uint8_t iic_soft_write_data(uint8_t dev_addr, const uint8_t *data, uint16_t len);

uint8_t iic_soft_read_data(uint8_t dev_addr, uint8_t *data, uint16_t len);

uint8_t iic_soft_mem_write_data(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint16_t len);

uint8_t iic_soft_mem_read_data(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif
