#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "driver.h"
#include "fs.h"
#include "debug.h"

/* this is the low level API for accessing disk, one below this is 
 * the block cache and one above is the dstream  */

/* return the number of blocks written to disk  */
size_t write_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t num_blocks)
{
    size_t block_size = disk_get_block_size();
    size_t phys_blocks = RFS_BLOCK_SIZE / block_size;

    if (fs->sb->num_blocks < b_offset + num_blocks) {
        LOG_EMERG("Trying to write to block %u but disk has only %u blocks",
                b_offset + num_blocks, fs->sb->num_blocks);
        fs_set_errno(FS_NOT_ENOUGH_SPACE);
        return SIZE_MAX;
    }

    LOG_INFO("writing %u FS blocks and %u disk blocks", num_blocks, phys_blocks);

    /* one file system block must be written in parts as the disk doesn't (necessarily)
     * support as large writes (currently 512 bytes but the FS block size is 4KB) */

    /* TODO: is there any way to cluster writes to disk? */
    for (size_t i = 0; i < num_blocks; ++i) {

        for (size_t k = 0; k < phys_blocks; ++k) {

        }
    }

    /* for (size_t lb = 0; lb < num_blocks; ++lb) { */
    /*     LOG_INFO("write to block %u", lb + b_offset); */
    /*     /1* for (size_t pb = 0; pb < *1/ */ 
    /*     /1*     disk_write(i + b_offset, buf + lb * block_size); *1/ */
    /* } */

    return 0;
}

/* This functions emulates the disk writing routine of the real driver as much as possible.
 * What that means is that all writes happen in 512 chunks ie. if user wants to write 
 * 1024 bytes to disk, this functions splits the action in to two consecutive writes.  
 *
 *
 * Read num_blocks * dev_block_size bytes of data from disk to buffer pointed to by buf 
 * Return total bytes read if reading succeeded.
 * If reading failed, set fs_errno and return SIZE_MAX */
size_t read_blocks_new(fs_t *fs, uint32_t b_offset, void *buf, size_t num_blocks)
{
    size_t block_size = disk_get_block_size();

    if (fs->sb->num_blocks < b_offset + num_blocks) {
        LOG_EMERG("Trying to read from block %u but disk has only %u blocks",
                b_offset + num_blocks, fs->sb->num_blocks);
        fs_set_errno(FS_NOT_ENOUGH_SPACE);
        return SIZE_MAX;
    }

    for (size_t i = 0; i < num_blocks; ++i) {
        LOG_INFO("write to block %u", i + b_offset);

        disk_read(i + b_offset, buf + i * block_size);
    }

    return 0;
}

size_t read_blocks(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    lseek(fs->fd, offset, SEEK_SET);
    return read(fs->fd, buf, size);
}

size_t write_blocks_old(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    lseek(fs->fd, offset, SEEK_SET);
    return write(fs->fd, buf, size);
}

