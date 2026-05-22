#include "bsp_uart1.h"

#include <stdio.h>

#include "usart.h"
#include "ring_buffer.h"
#include "freertos.h"


/* 发送 DMA设置 */
#define DMA_TX_BUF_SIZE 256
uint8_t dma_tx_buf[DMA_TX_BUF_SIZE];
/* 发送 环形缓冲区设置 */
#define RING_BUF_TX_SIZE 512
Buffer_t tx_buffer_handle;
uint8_t tx_buffer[RING_BUF_TX_SIZE];

/* 接收 DMA设置 */
#define DMA_RX_BUF_SIZE 256
uint8_t dma_rx_buf[DMA_RX_BUF_SIZE];
/* 接收 环形缓冲区设置 */
#define RING_BUF_RX_SIZE 512
Buffer_t rx_buffer_handle;
uint8_t rx_buffer[RING_BUF_RX_SIZE];

/* 回调函数 */
static uart1_rx_callback_t rx_callback = NULL;
static uart1_tx_callback_t tx_callback = NULL;

/* 内部辅助函数声明 */
static void uart1_rx_restart(void);
static void uart1_tx_flush(void);
static void uart1_rx_push_to_buffer(uint16_t len);

/* 临界区接口 */
static void uartENTER_CRITICAL()
{
    // portENTER_CRITICAL();
}

static void uartEXIT_CRITICAL()
{
    // portEXIT_CRITICAL();
}

/**
 * @description: 串口初始化函数
 * @return {*}
 */
uint8_t uart1_init(void)
{
    /* 串口初始化 */
    MX_USART1_UART_Init();

    /* 环形缓冲区初始化 */
    buffer_init(&tx_buffer_handle, tx_buffer, RING_BUF_TX_SIZE);
    buffer_init(&rx_buffer_handle, rx_buffer, RING_BUF_RX_SIZE);

    /* DMA+IDLE 启动 */
    uart1_rx_restart();

    return 0;
}

/**
 * @description: 串口发送字节函数
 * @param  byte 字节指针
 * @return 1 失败 0 成功
 */
uint8_t uart1_send_byte(uint8_t *byte)
{
    if (byte == NULL)
    {
        return 1;
    }

    uartENTER_CRITICAL();
    if (buffer_write_byte(&tx_buffer_handle, *byte) != 1)
    {
        uartEXIT_CRITICAL();
        return 1;
    }

    uart1_tx_flush();
    uartEXIT_CRITICAL();

    return 0;
}

/**
 * @description: 串口发送连续字节函数
 * @param bytes 字节指针
 * @param  len 发送长度
 * @return 1 失败 0 成功
 */
uint8_t uart1_send_bytes(uint8_t *bytes, uint16_t len)
{
    if (bytes == NULL)
    {
        return 1;
    }
    uartENTER_CRITICAL();
    if (buffer_write_data(&tx_buffer_handle, bytes, len) == 0)
    {
        uartEXIT_CRITICAL();
        return 1;
    }

    uart1_tx_flush();
    uartEXIT_CRITICAL();
    return 0;
}

/**
 * @description: 串口接收字节函数
 * @param byte 接收字节指针
 * @return 1 失败 0 成功
 */
uint8_t uart1_receive_byte(uint8_t *byte)
{
    if (byte == NULL)
    {
        return 1;
    }
    uartENTER_CRITICAL();
    uint8_t res = buffer_read_byte(&rx_buffer_handle, byte);
    uartEXIT_CRITICAL();
    return res;
}

/**
 * @description: 串口接收连续字节函数
 * @param bytes 接收字节指针
 * @param len 接收长度
 * @return 1 失败 0 成功
 */
uint8_t uart1_receive_bytes(uint8_t *bytes, uint16_t len)
{
    if (bytes == NULL)
    {
        return 1;
    }
    uartENTER_CRITICAL();
    uint32_t r_len = buffer_read_data(&rx_buffer_handle, bytes, len);
    uartEXIT_CRITICAL();
    return (r_len > 0 ? 0 : 1);
}

/**
 * @description: 设置接收完成串口回调函数
 * @param cb 函数指针
 * @return {*}
 */
void uart1_set_rx_callback(uart1_rx_callback_t cb)
{
    rx_callback = cb;
}

/**
 * @description: 设置发送完成串口回调函数
 * @param cb 函数指针
 * @return {*}
 */
void uart1_set_tx_callback(uart1_tx_callback_t cb)
{
    tx_callback = cb;
}

/**
 * @description: printf重定向为 USART1 （需要启动 MicroLib）
 */
int fputc(int ch, FILE *f)
{
    uart1_send_byte((uint8_t *)&ch);
    return ch;
}

/**
 * @description: 重启 DMA + IDLE 的中断接收
 * @return {*}
 */
static void uart1_rx_restart(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, dma_rx_buf, DMA_RX_BUF_SIZE);
}

/**
 * @description: 将 DMA 缓冲区收到的数据写入环形缓冲区
 * @param len 实际接收的字节数
 */
static void uart1_rx_push_to_buffer(uint16_t len)
{
    if (len > 0 && len <= DMA_RX_BUF_SIZE)
    {
        uint32_t written = buffer_write_data(&rx_buffer_handle, dma_rx_buf, len);
    }
}

/**
 * @description: TX 刷新: 若 DMA 空闲且环形缓冲区有数据，启动一次 DMA 发送
 * @return {*}
 */
static void uart1_tx_flush(void)
{
    uint32_t avail = buffer_get_curr_length(&tx_buffer_handle);

    if (avail == 0)
        return;

    /* 从 TX 环形缓冲区取出不超过 DMA_TX_BUF_SIZE 的数据 */
    uint16_t len = (avail > DMA_TX_BUF_SIZE) ? DMA_TX_BUF_SIZE : (uint16_t)avail;
    buffer_read_data(&tx_buffer_handle, dma_tx_buf, len);

    /* 启动 DMA 发送 */
    HAL_UART_Transmit_DMA(&huart1, dma_tx_buf, len);
}

/*---------------------------------- 串口空闲中断 --------------------------------*/

/**
 * @description: 串口线路空闲时触发（变长数据帧结束标志）
 * @param huart HAL库串口句柄
 * @param Size 本次实际接收到的字节数
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1 && Size > 0)
    {
        /* 将实际收到的 N 字节写入环形缓冲区 */
        uart1_rx_push_to_buffer(Size);

        /* 重启 DMA + 空闲中断 */
        uart1_rx_restart();

        /* 回调函数触发 */
        if (rx_callback)
        {
            rx_callback(dma_rx_buf, Size);
        }
    }
}

/*---------------------------------- DMA 接收全满中断 -------------------------------*/

/**
 * @description: DMA 收满 DMA_RX_BUF_SIZE 字节时触发
 * @param huart HAL库串口句柄
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 将全部 256 字节写入环形缓冲区 */
        uart1_rx_push_to_buffer(DMA_RX_BUF_SIZE);

        /* 重启 DMA + 空闲中断 */
        uart1_rx_restart();

        /* 回调函数触发 */
        if (rx_callback)
        {
            rx_callback(dma_rx_buf, DMA_RX_BUF_SIZE);
        }
    }
}

/*---------------------------------- DMA 发送完成中断 -------------------------------*/

/**
 * @description: DMA 发送完成时触发 → 检查 TX 缓冲区是否有剩余数据 → 链式发送
 * @param huart HAL库串口句柄
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 发送 TX 环形缓冲区中的下一块数据 */
        uart1_tx_flush();

        /* 回调函数触发 */
        if (tx_callback)
        {
            tx_callback();
        }
    }
}
