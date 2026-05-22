#include "ring_buffer.h"

/**
 * @brief 检查缓冲区大小是否为2的次方数
 * @param size 缓冲区大小
 * @return 是否为2的次方数
 * @retval 0 不是2的次方数
 * @retval 1 是2的次方数
 */
static uint8_t buffer_is_power_of_two(uint32_t size)
{
    if (size == 0)
        return 0;
    return (size & (size - 1)) == 0;
}

/**
 * @description: 缓冲区初始化
 * @param buffer 缓冲区结构体指针
 * @param buffer_array 缓冲区数组指针
 * @param buffer_size 缓冲区数组大小
 * @return 初始化是否成功
 * @retval 0 缓冲区初始化失败
 * @retval 1 缓冲区初始化成功
 */
uint8_t buffer_init(Buffer_t *buffer, uint8_t *buffer_array, uint32_t buffer_size)
{
    if (buffer == NULL || buffer_array == NULL)
        return 0;

    if (!buffer_is_power_of_two(buffer_size))
        return 0;

    buffer->buffer_ptr = buffer_array;
    buffer->buffer_size = buffer_size;
    buffer->writeIndex = 0;
    buffer->readIndex = 0;

    buffer->rx_busy = 0;
    buffer->tx_busy = 0;

    return 1;
}

/**
 * @description: 获取缓冲区写索引值
 * @param buffer 缓冲区结构体指针
 * @return 写索引值
 * @retval 0~BUFFER_SIZE-1 有效写索引值长度
 */
uint32_t buffer_get_writeIndex(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->writeIndex;
}

/**
 * @description: 获取缓冲区读索引值
 * @param buffer 缓冲区结构体指针
 * @return 读索引值
 * @retval 0~BUFFER_SIZE-1 有效写读引值长度
 */
uint32_t buffer_get_readIndex(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->readIndex;
}

/**
 * @description: 获取缓冲区大小
 * @param buffer 缓冲区结构体指针
 * @return 缓冲区大小
 * @retval BUFFER_SIZE 缓冲区大小
 */
uint32_t buffer_get_buffer_size(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->buffer_size;
}

/**
 * @description: 计算缓冲区未处理的数据长度
 * @param buffer 缓冲区结构体指针
 * @return 未处理的数据长度
 * @retval 0 缓冲区为空
 * @retval 1~BUFFER_SIZE-1 有效数据长度
 */
uint32_t buffer_get_curr_length(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;

    uint32_t curr_len;

    curr_len = (buffer->writeIndex + buffer->buffer_size - buffer->readIndex) % buffer->buffer_size;

    return curr_len;
}

/**
 * @description: 计算缓冲区剩余空间
 * @param buffer 缓冲区结构体指针
 * @return 剩余空间
 * @retval 0 缓冲区已满
 * @retval 1~BUFFER_SIZE-1 剩余空间
 */
uint32_t buffer_get_free(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;
    return buffer->buffer_size - buffer_get_curr_length(buffer) - 1;
}

/**
 * @description: 检查缓冲区是否为空
 * @param buffer 缓冲区结构体指针
 * @return 是否为空
 * @retval 0 缓冲区不为空
 * @retval 1 缓冲区为空
 */
uint8_t buffer_is_empty(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 1;

    uint8_t result;

    result = (buffer->readIndex == buffer->writeIndex);

    return result;
}

/**
 * @description: 检查缓冲区是否已满
 * @param buffer 缓冲区指针
 * @return 是否已满
 * @retval 0 缓冲区未满
 * @retval 1 缓冲区已满
 */
uint8_t buffer_is_full(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;

    uint8_t result;

    result = ((buffer->writeIndex + 1) % buffer->buffer_size == buffer->readIndex);

    return result;
}

/**
 * @description: 查看pos位数据 超过缓存区长度自动循环
 * @param buffer 缓冲区结构体指针
 * @param pos 要查看的数据索引
 * @return pos位的数据
 */
uint8_t buffer_peek_at(Buffer_t *buffer, uint32_t pos)
{
    if (buffer == NULL)
        return 0;

    uint32_t curr_len = buffer_get_curr_length(buffer);
    if (pos >= curr_len)
    {
        return 0;
    }
    uint32_t index = (buffer->readIndex + pos) % buffer->buffer_size;
    uint8_t data = buffer->buffer_ptr[index];

    return data;
}

/**
 * @description: 向缓冲区写入单个字节
 * @param buffer 要写入的缓冲区指针
 * @param data 要写入的字节
 * @return 写入是否成功
 * @retval 0 写入失败（缓冲区已满）
 * @retval 1 写入成功
 */
uint8_t buffer_write_byte(Buffer_t *buffer, uint8_t data)
{
    if (buffer == NULL)
        return 0;

    if (buffer_is_full(buffer))
    {
        return 0;
    }

    buffer->buffer_ptr[buffer->writeIndex] = data;
    buffer->writeIndex = (buffer->writeIndex + 1) % buffer->buffer_size;

    return 1;
}

/**
 * @description: 从缓冲区读取单个字节
 * @param buffer 要读取的缓冲区指针
 * @param data 接收数据的指针
 * @return 读取是否成功
 * @retval 0 读取失败（缓冲区为空）
 * @retval 1 读取成功
 */
uint8_t buffer_read_byte(Buffer_t *buffer, uint8_t *data)
{
    if (buffer == NULL || data == NULL)
        return 0;

    if (buffer_is_empty(buffer))
    {
        return 0;
    }

    *data = buffer->buffer_ptr[buffer->readIndex];
    buffer->readIndex = (buffer->readIndex + 1) % buffer->buffer_size;

    return 1;
}

/**
 * @description: 向缓冲区写入多个字节
 * @param buffer 要写入的缓冲区指针
 * @param data 写入的数据指针
 * @param length 写入的数据长度
 * @return 实际写入的字节数
 */
uint32_t buffer_write_data(Buffer_t *buffer, uint8_t *data, uint32_t length)
{
    if (buffer == NULL || data == NULL)
        return 0;

    // 如果缓冲区剩余空间不足 则不写入数据 返回0
    if (buffer_get_free(buffer) < length)
    {
        return 0;
    }

    uint32_t firstLength = buffer->buffer_size - buffer->writeIndex;

    if (length <= firstLength) // 情况1 不发生回环
    {
        // 不发生回环 直接将数据复制到缓冲区
        memcpy(buffer->buffer_ptr + buffer->writeIndex, data, length);
    }
    else // 情况2 发生回环
    {
        // 发生回环 将数据分成两部分复制到缓冲区
        // 第一部分 从写索引开始复制到缓冲区末尾
        memcpy(buffer->buffer_ptr + buffer->writeIndex, data, firstLength);
        // 第二部分 从缓冲区开始复制到数据末尾
        memcpy(buffer->buffer_ptr, data + firstLength, length - firstLength);
    }

    buffer->writeIndex = (buffer->writeIndex + length) % buffer->buffer_size;

    // 返回成功写入的字节数
    return length;
}

/**
 * @description: 从缓冲区读取多个字节
 * @param buffer 缓冲区指针
 * @param data 接收数据的指针
 * @param length 接收数据的长度
 * @return 实际读取的字节数
 */
uint32_t buffer_read_data(Buffer_t *buffer, uint8_t *data, uint32_t length)
{
    if (buffer == NULL || data == NULL)
        return 0;

    uint32_t curr_len = buffer_get_curr_length(buffer);
    if (curr_len == 0)
    {
        return 0;
    }

    // 实际可读取长度 = min(requested, available)
    uint32_t read_len = (length < curr_len) ? length : curr_len;

    uint32_t first_len = buffer->buffer_size - buffer->readIndex;

    if (read_len <= first_len) // 情况1: 不发生回环
    {
        memcpy(data, buffer->buffer_ptr + buffer->readIndex, read_len);
    }
    else // 情况2: 发生回环
    {
        // 第一段: 从读索引复制到缓冲区末尾
        memcpy(data, buffer->buffer_ptr + buffer->readIndex, first_len);
        // 第二段: 从缓冲区开头复制剩余数据
        memcpy(data + first_len, buffer->buffer_ptr, read_len - first_len);
    }

    buffer->readIndex = (buffer->readIndex + read_len) % buffer->buffer_size;

    return read_len;
}

/**
 * @description: 清空缓冲区全部内容
 * @param buffer 缓冲区结构体指针
 * @return 是否成功 1：成功 0：失败
 */
uint8_t buffer_drop_all(Buffer_t *buffer)
{
    if (buffer == NULL)
        return 0;

    buffer->readIndex = buffer->writeIndex;

    return 1;
}

/**
 * @description: 清空指定长度的缓冲区内容（从读指针位置开始）
 * @param buffer 缓冲区结构体指针
 * @param length 要清空的长度
 * @return 实际清空的字节数
 */
uint32_t buffer_drop_from_curr(Buffer_t *buffer, uint32_t length)
{
    if (buffer == NULL)
        return 0;

    uint32_t curr_len = buffer_get_curr_length(buffer);
    if (curr_len == 0)
    {
        return 0;
    }

    // 实际可清空的长度 = min(requested, available)
    uint32_t clear_len = (length < curr_len) ? length : curr_len;

    buffer->readIndex = (buffer->readIndex + clear_len) % buffer->buffer_size;

    return clear_len;
}
