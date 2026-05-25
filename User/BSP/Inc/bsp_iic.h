#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__

#include <stdint.h>

typedef enum{
    IIC_OK = 0,
    IIC_ERR
}iic_state_e;

iic_state_e bsp_iic_soft_init(void);

iic_state_e bsp_iic_soft_write_data(uint8_t dev_addr, const uint8_t *data, uint16_t len);

iic_state_e bsp_iic_soft_read_data(uint8_t dev_addr, uint8_t *data, uint16_t len);

iic_state_e bsp_iic_soft_mem_write_data(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint16_t len);

iic_state_e bsp_iic_soft_mem_read_data(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif
