#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdio.h>

/**
 * @description: 日志等级定义
 * @return {*}
 */
typedef enum
{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_ALL = 5
} LOG_level_e;

/*===========================================================================
 * 全局编译时等级阈值（可在项目预处理器宏中覆盖）
 *===========================================================================*/
#ifndef LOG_GLOBAL_LEVEL
#define LOG_GLOBAL_LEVEL LOG_LEVEL_ALL
#endif

/*===========================================================================
 * 日志格式配置
 *===========================================================================*/

/* 是否输出模块标签 */
#ifndef LOG_TAG_ENABLE
#define LOG_TAG_ENABLE 1
#endif

/*===========================================================================
 * 接口函数
 *=========================================================================== */

void log_init(void);

void LOG_PRINTF(const char *fmt, ...);

void log_output(LOG_level_e level, const char *tag, const char *file, int line, const char *fmt, ...);

/*===========================================================================
 * 宏定义 - 编译期等级裁剪
 *=========================================================================== */

#if (LOG_GLOBAL_LEVEL >= LOG_LEVEL_ERROR)
#define LOG_ERROR(tag, fmt, ...) \
    log_output(LOG_LEVEL_ERROR, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(tag, fmt, ...) ((void)0)
#endif

#if (LOG_GLOBAL_LEVEL >= LOG_LEVEL_WARN)
#define LOG_WARN(tag, fmt, ...) \
    log_output(LOG_LEVEL_WARN, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(tag, fmt, ...) ((void)0)
#endif

#if (LOG_GLOBAL_LEVEL >= LOG_LEVEL_INFO)
#define LOG_INFO(tag, fmt, ...) \
    log_output(LOG_LEVEL_INFO, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(tag, fmt, ...) ((void)0)
#endif

#if (LOG_GLOBAL_LEVEL >= LOG_LEVEL_DEBUG)
#define LOG_DEBUG(tag, fmt, ...) \
    log_output(LOG_LEVEL_DEBUG, tag, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(tag, fmt, ...) ((void)0)
#endif

#endif /* __LOG_H__ */
