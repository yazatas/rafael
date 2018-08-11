#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdio.h>
#include "fs.h"

/* return device block size (right now a dummy function) */
size_t dev_get_block_size(void);

/* return device block count (right now size is hard coded to 65mb) */
size_t dev_get_num_blocks(int fd);

/* TODO: provide byte interface and behind abstraction operate with blocks
 *       that is: user can ask to write 64 bytes to offset 112244
 *
 *       Calculate which block this is.
 *
 *       If it's block start aligned, it's safe to overwrite
 *       (ie. just call write_blocks with block offset)
 *
 *       If it's NOT block start aligned, determine which block is
 *       in question (CHECK CACHE??), read the block contents 
 *       rewrite the part from the read block that user wants updated
 *       and flush the changes to disk
 */

/* read "size" bytes from disk to "buf"
 *
 * conversion to disk sized blocks is done internally so the caller
 * need'n't to worry about the disk interface and its intricacies */
size_t rfs_read_buf(fs_t *fs, uint32_t offset, void *buf, size_t size);

/* write "size" bytes from "buf" to disk */
size_t rfs_write_buf(fs_t *fs, uint32_t offset, void *buf, size_t size);

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
