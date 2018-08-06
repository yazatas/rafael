#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>

void disk_init(const char *device);
void disk_close(void);

void disk_write(uint32_t blocknum, uint8_t *buf);
void disk_read(uint32_t blocknum, uint8_t *buf);

#endif /* __DISK_H__ */
