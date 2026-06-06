#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdint.h>
#include <stdio.h>


/**
 * @description: 日志等级定义
 * @return {*}
 */
typedef enum {
    DEBUG_LEVEL_NONE  = 0,
    DEBUG_LEVEL_ERROR = 1,
    DEBUG_LEVEL_WARN  = 2,
    DEBUG_LEVEL_INFO  = 3,
    DEBUG_LEVEL_DEBUG = 4,
    DEBUG_LEVEL_ALL   = 5
} debug_level_e;

/*===========================================================================
 * 全局编译时等级阈值（可在项目预处理器宏中覆盖）
 *===========================================================================*/
#ifndef DEBUG_GLOBAL_LEVEL
#define DEBUG_GLOBAL_LEVEL  DEBUG_LEVEL_ALL
#endif

/*===========================================================================
 * 日志格式配置
 *===========================================================================*/

/* 是否输出模块标签 */
#ifndef DEBUG_TAG_ENABLE
#define DEBUG_TAG_ENABLE        1
#endif

/* 输出函数宏，默认使用 bsp_uart1_printf */
#define DEBUG_PRINTF            bsp_uart1_printf

/*===========================================================================
 * 接口函数
 *=========================================================================== */

/**
 * @brief 输出日志消息
 */
void debug_output(debug_level_e level, const char *tag, const char *file, int line, const char *fmt, ...);

/*===========================================================================
 * 宏定义 - 编译期等级裁剪
 *=========================================================================== */

#if (DEBUG_GLOBAL_LEVEL >= DEBUG_LEVEL_ERROR)
#define DEBUG_ERROR(tag, fmt, ...) \
    debug_output(DEBUG_LEVEL_ERROR, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_ERROR(tag, fmt, ...)  ((void)0)
#endif

#if (DEBUG_GLOBAL_LEVEL >= DEBUG_LEVEL_WARN)
#define DEBUG_WARN(tag, fmt, ...) \
    debug_output(DEBUG_LEVEL_WARN, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_WARN(tag, fmt, ...)  ((void)0)
#endif

#if (DEBUG_GLOBAL_LEVEL >= DEBUG_LEVEL_INFO)
#define DEBUG_INFO(tag, fmt, ...) \
    debug_output(DEBUG_LEVEL_INFO, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_INFO(tag, fmt, ...)  ((void)0)
#endif

#if (DEBUG_GLOBAL_LEVEL >= DEBUG_LEVEL_DEBUG)
#define DEBUG_DEBUG(tag, fmt, ...) \
    debug_output(DEBUG_LEVEL_DEBUG, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_DEBUG(tag, fmt, ...)  ((void)0)
#endif

#endif /* __DEBUG_H__ */
