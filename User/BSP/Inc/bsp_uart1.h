#ifndef __BSP_UART1_H__
#define __BSP_UART1_H__

#include <stdint.h>


typedef void (*uart1_rx_callback_t)(const uint8_t *data, uint16_t len);
void uart1_set_rx_callback(uart1_rx_callback_t cb);


typedef void (*uart1_tx_callback_t)(void);
void uart1_set_tx_callback(uart1_tx_callback_t cb);

uint8_t uart1_init(void);
uint8_t uart1_send_byte(uint8_t *byte);
uint8_t uart1_send_bytes(uint8_t *bytes, uint16_t len);
uint8_t uart1_receive_byte(uint8_t *byte);
uint8_t uart1_receive_bytes(uint8_t *bytes, uint16_t len);

#endif
