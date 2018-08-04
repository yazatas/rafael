#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdio.h>
#include "fs.h"

/* return device block size (512 bytes for testing) */
size_t dev_get_block_size(void);

/* return  */
size_t dev_get_num_blocks(int fd);

/* write size bytes from buf to memory location starting from start 
 * 
 * return how many bytes was written */
size_t write_blocks(fs_t *fs, uint32_t offset, void *buf, size_t size);

/* read size bytes from disk starting at location start to buf 
 * read_blocks assumes buf points to an allocated block of memory
 *
 * return how many bytes was read */
size_t read_blocks(fs_t *fs, uint32_t offset, void *buf, size_t size);

#endif /* end of include guard: __DRIVER_H__ */
