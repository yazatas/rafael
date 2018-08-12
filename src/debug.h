#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

#define LOG_LEVEL_EMERG "EMERGENCY"
#define LOG_LEVEL_WARN  "WARNING"
#define LOG_LEVEL_INFO  "INFO"
#define LOG_LEVEL_DEBUG "DEBUG"

#define debug(level, fmt, ...) \
	fprintf(stderr, "[%s][%s] "fmt"\n", level, __func__, ##__VA_ARGS__)

#define LOG_EMERG(fmt, ...)  debug(LOG_LEVEL_EMERG, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt,  ...)  debug(LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt,  ...)  debug(LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt,  ...) debug(LOG_LEVEL_DEBUG,  fmt, ##__VA_ARGS__)

#ifdef LEVEL_EMERG
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#define LOG_DEBUG(fmt, ...) ;
#define LOG_INFO(fmt, ...) ;
#define LOG_WARN(fmt, ...) ;
#elif defined(LEVEL_WARN)
#undef LOG_DEBUG
#undef LOG_INFO
#define LOG_INFO(fmt, ...) ;
#define LOG_DEBUG(fmt, ...) ;
#elif defined(LEVEL_INFO)
#undef LOG_DEBUG
#define LOG_DEBUG(fmt, ...) ;
#endif

#endif /* end of include guard: __DEBUG_H__ */
