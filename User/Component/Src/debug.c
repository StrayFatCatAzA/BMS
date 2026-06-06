#include "debug.h"

#include <stdarg.h>
#include <string.h>

#include "bsp_uart1.h"

/**
 * @description: 等级标签字符串匹配
 * @param {debug_level_e} level 日志等级
 * @return {*}
 */
static const char *s_level_str(debug_level_e level)
{
    switch (level)
    {
    case DEBUG_LEVEL_ERROR:
        return "ERROR";
    case DEBUG_LEVEL_WARN:
        return "WARN ";
    case DEBUG_LEVEL_INFO:
        return "INFO ";
    case DEBUG_LEVEL_DEBUG:
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
 * @description:  统一日志输出
 * @param {debug_level_e} level 日志等级
 * @param {char} *tag 日志标记
 * @param {char} *file 目标文件
 * @param {int} line 目标行号
 * @param {char} *fmt 格式化字符串
 * @return {*}
 */
void debug_output(debug_level_e level, const char *tag, const char *file, int line, const char *fmt, ...)
{
    char buf[256];
    int pos = 0;

    /* 等级 */
    pos += snprintf(buf + pos, sizeof(buf) - pos, "%s ", s_level_str(level));

    /* 模块标签 */
#if DEBUG_TAG_ENABLE
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

    /* 追加换行 */
    if (pos < (int)(sizeof(buf) - 2))
    {
        buf[pos++] = '\r';
        buf[pos++] = '\n';
        buf[pos] = '\0';
    }

    bsp_uart1_printf("%s", buf);
}
