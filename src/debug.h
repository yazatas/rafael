#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>

#define debug_fp(fp, fmt, ...) fprintf(fp, "[%s] "fmt, __func__, __VA_ARGS__)
#define debug(fmt, ...) fprintf(stderr, "[%s] "fmt, __func__, __VA_ARGS__)

#endif /* end of include guard: __DEBUG_H__ */
