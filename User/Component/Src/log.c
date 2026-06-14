#include "log.h"

#include <stdarg.h>
#include <string.h>

/* RTOS */
#include "cmsis_os2.h"

/* 串口驱动 */
#include "bsp_uart1.h"

/* 互斥锁管理 */
#include "app_mutex_manager.h"

/**
 * @description: 等级标签字符串匹配
 * @param {debug_level_e} level 日志等级
 * @return {*}
 */
static const char *s_level_str(LOG_level_e level)
{
    switch (level)
    {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARN ";
    case LOG_LEVEL_INFO:
        return "INFO ";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    default:
        return "?????";
    }
}

/**
 * @description: 从路径中提取文件名（去掉目录前缀）
 * @param {char} *path 路径字符串指针
 * @return {*}
 */
static const char *s_basename(const char *path)
{
    const char *p = path + strlen(path);
    while (p > path)
    {
        if (*(p - 1) == '\\' || *(p - 1) == '/')
        {
            return p;
        }
        p--;
    }
    return p;
}

/**
 * @description: 日志打印接口函数
 * @param {char} *buf
 * @return {*}
 */
static void s_log_interface_print(const char *buf)
{
    if (osMutexAcquire(uart1_mutex, 200) != osOK)
    {
        return;
    }

    bsp_uart1_printf("%s", buf);

    osMutexRelease(uart1_mutex);
}

/**
 * @description: 日志打印初始化
 * @return {*}
 */
void log_init(void)
{
    /* 初始化UART1 */
    bsp_uart1_init();
}

/**
 * @description: 通用格式化日志输出
 * @param {char *} fmt 格式化字符串
 * @param {...} ... 可变参数
 * @return {*}
 */
void LOG_PRINTF(const char *fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    s_log_interface_print(buf);
}

/**
 * @description:  统一日志输出
 * @param {LOG_level_e} level 日志等级
 * @param {char} *tag 日志标记
 * @param {char} *file 目标文件
 * @param {int} line 目标行号
 * @param {char} *fmt 格式化字符串
 * @return {*}
 */
void log_output(LOG_level_e level, const char *tag, const char *file, int line, const char *fmt, ...)
{
    char buf[256];
    int pos = 0;

    /* 等级 */
    pos += snprintf(buf + pos, sizeof(buf) - pos, "%s ", s_level_str(level));

    /* 模块标签 */
#if LOG_TAG_ENABLE
    if (tag != NULL && tag[0] != '\0')
    {
        pos += snprintf(buf + pos, sizeof(buf) - pos, "[%s] ", tag);
    }
#endif

    /* 文件名:行号 */
    pos += snprintf(buf + pos, sizeof(buf) - pos, "%s:%d: ",
                    s_basename(file), line);

    /* 用户消息 */
    if (pos < (int)(sizeof(buf) - 2))
    {
        va_list args;
        va_start(args, fmt);
        pos += vsnprintf(buf + pos, sizeof(buf) - pos, fmt, args);
        va_end(args);
    }

    /* 追加结束标志位 */
    if (pos < (int)(sizeof(buf) - 2))
    {
        buf[pos] = '\0';
    }

    s_log_interface_print(buf);
}
