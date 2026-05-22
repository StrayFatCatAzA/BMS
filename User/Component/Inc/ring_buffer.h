#ifndef __CIR_BUFFER_H__
#define __CIR_BUFFER_H__

#include <stdint.h>
#include <string.h>

/*-----------------------------------------------------------*/

typedef struct
{
    uint8_t *buffer_ptr;          // 缓冲区指针 指向缓冲区数组
    uint32_t buffer_size;         // 缓冲区大小
    volatile uint32_t writeIndex; // 缓冲区写指针
    volatile uint32_t readIndex;  // 缓冲区读指针
    volatile uint8_t rx_busy;
    volatile uint8_t tx_busy;
} Buffer_t;

/*-----------------------------------------------------------*/

/* 函数声明 */

uint8_t buffer_init(Buffer_t *buffer, uint8_t *buffer_array, uint32_t buffer_size);

uint32_t buffer_get_writeIndex(Buffer_t *buffer);

uint32_t buffer_get_readIndex(Buffer_t *buffer);

uint32_t buffer_get_buffer_size(Buffer_t *buffer);

uint32_t buffer_get_curr_length(Buffer_t *buffer);

uint32_t buffer_get_free(Buffer_t *buffer);

uint8_t buffer_is_empty(Buffer_t *buffer);

uint8_t buffer_is_full(Buffer_t *buffer);

uint8_t buffer_peek_at(Buffer_t *buffer, uint32_t pos);

uint8_t buffer_write_byte(Buffer_t *buffer, uint8_t data);

uint8_t buffer_read_byte(Buffer_t *buffer, uint8_t *data);

uint32_t buffer_write_data(Buffer_t *buffer, uint8_t *data, uint32_t length);

uint32_t buffer_read_data(Buffer_t *buffer, uint8_t *data, uint32_t length);

uint8_t buffer_drop_all(Buffer_t *buffer);

uint32_t buffer_drop_from_curr(Buffer_t *buffer, uint32_t length);



#endif
