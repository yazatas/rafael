#ifndef __DISK_H__
#define __DISK_H__

#include <stdint.h>

int disk_init(const char *device);
void disk_close(int fd);

void disk_write(int fd, uint32_t sector, uint8_t *buf);
void disk_read(int fd, uint32_t sector, uint8_t *buf);

uint32_t disk_get_size(int fd);
uint32_t disk_get_block_size(int fd);

#endif /* __DISK_H__ */
