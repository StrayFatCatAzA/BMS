#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__


#include <stdint.h>


uint8_t iic_s_init(void);
uint8_t iic_s_send_byte(uint8_t *byte);
uint8_t iic_s_receive_byte(uint8_t *byte);




#endif

