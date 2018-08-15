#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define BYTE_TO_BLOCK(n) (n / RFS_BLOCK_SIZE)
#define BLOCK_TO_BYTE(n) (n * RFS_BLOCK_SIZE)

static inline void hex_dump(uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i+=10) {
        fprintf(stderr, "\t");
        for (size_t k = i; k < i + 10; ++k) {
            fprintf(stderr, "0x%02x ", buf[k]);
        }
        fprintf(stderr, "\n");
    }
}

#endif /* __COMMON_H__ */
