#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdio.h>

#define DEV_BLOCK_SIZE 512

typedef struct hdd_handle {
	FILE *fp;
	size_t size;
	size_t dev_bsize;
} hdd_handle_t;

/* open device, for now just fopen(device), read handle, 
 * max size and dev size to hdd_handle_t and return the handle */
hdd_handle_t *open_device(const char *device, const char *mode);

/* close device, for now just fclose */
int close_device(hdd_handle_t *fd);

/* write size bytes from buf to memory location starting from start 
 * 
 * return how many bytes was written */
size_t write_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size);

/* read size bytes from disk starting at location start to buf 
 * read_blocks assumes buf points to an allocated block of memory
 *
 * return how many bytes was read */
size_t read_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size);

#endif /* end of include guard: __DRIVER_H__ */
