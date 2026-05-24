#ifndef __BSP_UART1_H__
#define __BSP_UART1_H__

#include <stdint.h>

typedef enum
{
    UART_STATE_OK = 0,
    UART_STATE_ERR = 1
}uart_state;


typedef void (*uart1_rx_callback_t)(const uint8_t *data, uint16_t len);
void uart1_set_rx_callback(uart1_rx_callback_t cb);


typedef void (*uart1_tx_callback_t)(void);
void uart1_set_tx_callback(uart1_tx_callback_t cb);

uart_state uart1_init(void);
uart_state uart1_send_byte(uint8_t byte);
uart_state uart1_send_bytes(uint8_t *bytes, uint16_t len);
uart_state uart1_receive_byte(uint8_t *byte);
uart_state uart1_receive_bytes(uint8_t *bytes, uint16_t len);
void uart1_printf(const char* format, ...);

#endif
