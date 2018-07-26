#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

#define LOG_LEVEL_EMERG "EMERGENCY"
#define LOG_LEVEL_WARN  "WARNING"
#define LOG_LEVEL_INFO  "INFO"

#define debug(level, fmt, ...) \
	fprintf(stderr, "[%s][%s] "fmt"\n", level, __func__, ##__VA_ARGS__)

#define LOG_EMERG(fmt, ...) debug(LOG_LEVEL_EMERG, fmt, ##__VA_ARGS__)

#ifdef LEVEL_EMERG
#define LOG_INFO(fmt, ...) ;
#define LOG_WARN(fmt, ...) ;
#elif defined(LEVEL_WARN)
#define LOG_INFO(fmt, ...) ;
#define LOG_WARN(fmt, ...) debug(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...) debug(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) debug(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#endif



#endif /* end of include guard: __DEBUG_H__ */
