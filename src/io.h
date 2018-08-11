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

/* write "size" bytes from "buf" to disk
 *
 * conversion to disk sized blocks is done internally so the caller
 * need'n't to worry about the disk interface and its intricacies 
 *
 * Returns the number of bytes written to disk on success and 0 on error 
 *
 * NOTE: return value may differ from "size" as it denotes the buffer 
 * size but rfs_write_buf() returns the actual amount of bytes written 
 * (the size may have been round up to satisfty the disk requirements) 
 *
 * Caller should check that the return values is not 0 and if it is, check fs_errno */
size_t rfs_write_buf(fs_t *fs, uint32_t offset, void *buf, size_t size);

/* read "nblocks" * RFS_BLOCK_SIZE of bytes from disk to buffer "buf" */
size_t rfs_read_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t nblocks);

/* write "nblocks" * RFS_BLOCK_SIZE of bytes from buffer "buf" to disk */
size_t rfs_write_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t nblocks);

#endif /* end of include guard: __DRIVER_H__ */
