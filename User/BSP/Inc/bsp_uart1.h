#ifndef __BSP_UART1_H__
#define __BSP_UART1_H__

#include <stdint.h>

typedef enum
{
    UART_STATE_OK = 0,
    UART_STATE_ERR = 1
}bsp_uart_state_e;


typedef void (*uart1_rx_callback_t)(const uint8_t *data, uint16_t len);
void bsp_uart1_set_rx_callback(uart1_rx_callback_t cb);


typedef void (*uart1_tx_callback_t)(void);
void bsp_uart1_set_tx_callback(uart1_tx_callback_t cb);

bsp_uart_state_e bsp_uart1_init(void);
bsp_uart_state_e bsp_uart1_send_byte(uint8_t byte);
bsp_uart_state_e bsp_uart1_send_bytes(uint8_t *bytes, uint16_t len);
bsp_uart_state_e bsp_uart1_receive_byte(uint8_t *byte);
bsp_uart_state_e bsp_uart1_receive_bytes(uint8_t *bytes, uint16_t len);
void bsp_uart1_printf(const char* format, ...);

#endif
