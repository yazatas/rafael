#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdio.h>

#define DEV_BLOCK_SIZE 512

/* wew */
typedef struct superblock superblock_t;

typedef struct hdd_handle {
	FILE *fp;
	size_t size;
	size_t dev_bsize;
} hdd_handle_t;

/* TODO: better abstraction for device?!!? */
hdd_handle_t *open_device(const char *device, const char *mode);

int close_device(hdd_handle_t *fd);

/* write size bytes from buf to memory location starting from start */
int write_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size);

/* read size bytes from disk starting at location start to buf */
int read_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size);

#endif /* end of include guard: __DRIVER_H__ */
