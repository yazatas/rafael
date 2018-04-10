#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>

#define LOG_EMERG "EMERGENCY"
#define LOG_WARN  "WARNING"
#define LOG_INFO  "INFO"

#define debug(level, fmt, ...) \
	fprintf(stderr, "[%s][%s] "fmt"\n", level, __func__, ##__VA_ARGS__)

#endif /* end of include guard: __DEBUG_H__ */
