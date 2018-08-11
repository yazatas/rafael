#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io.h"
#include "fs.h"
#include "debug.h"

/* TODO: pass fs object and check that the calculated
 * offset is within limits of the file system block count */
/* convert byte offset to block offset */
static size_t get_block_offset(size_t offset)
{
    return offset / RFS_BLOCK_SIZE;
}

static size_t get_block_count(size_t nbytes)
{
    return nbytes / RFS_BLOCK_SIZE + ((nbytes % RFS_BLOCK_SIZE) != 0);
}

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


size_t rfs_read_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t size)
{

}

size_t rfs_write_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t size)
{

}

/* If size is evenly divisible by RFS_BLOCK_SIZE then this routine is very simple
 * We simply call rfs_read_blocks and store the data to "buf"
 *
 * This, however, won't be the case most of the time so this is how we manage uneven reads
 *
 * 1) if size < RFS_BLOCK_SIZE 
 *    * Call rfs_read_blocks with fs_block buffer
 *    * Copy size bytes from fs_block to buf
 *
 * 2) if size % RFS_BLOCK_SIZE != 0 
 *    * read "size / RFS_BLOCK_SIZE" blocks from disk right to buf
 *    * read one block of data (the remainder) from disk to fs_block
 *    * copy contents from fs_block to buf  */

size_t rfs_write_buf(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    size_t b_offset = get_block_offset(offset), nblocks;
    static uint8_t fs_block[RFS_BLOCK_SIZE];

    if (size % RFS_BLOCK_SIZE == 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        if (rfs_write_blocks(fs, b_offset, buf, nblocks) != nblocks) {
            goto error;
        }

        return size;
    }

    if (size < RFS_BLOCK_SIZE) {
        memset(fs_block, 0, RFS_BLOCK_SIZE);
        memcpy(buf, fs_block, size);

        if (rfs_write_blocks(fs, b_offset, fs_block, 1) != 1) {
            goto error;
        }

        return size;
    }

    if (size % RFS_BLOCK_SIZE != 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        LOG_INFO("size %u | blocks %u | remaining %u",
                size, nblocks, size % RFS_BLOCK_SIZE);

        if (rfs_read_blocks(fs, b_offset, buf, nblocks) != nblocks) {
            goto error;
        }

        memcpy(buf + nblocks * RFS_BLOCK_SIZE, fs_block, size % RFS_BLOCK_SIZE);
        if (rfs_read_blocks(fs, b_offset + nblocks, fs_block, 1) != 1) {
            goto error;
        }

        return size;
    }

error:
    LOG_EMERG("failed to write blocks to disk!");
    return 0;
}

/* If size is evenly divisible by RFS_BLOCK_SIZE then this routine is very simple
 * We simply call rfs_read_blocks and store the data to "buf"
 *
 * This, however, won't be the case most of the time so this is how we manage uneven reads
 *
 * 1) if size < RFS_BLOCK_SIZE 
 *    * Call rfs_read_blocks with fs_block buffer
 *    * Copy size bytes from fs_block to buf
 *
 * 2) if size % RFS_BLOCK_SIZE != 0 
 *    * read "size / RFS_BLOCK_SIZE" blocks from disk right to buf
 *    * read one block of data (the remainder) from disk to fs_block
 *    * copy contents from fs_block to buf  */
size_t rfs_read_buf(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    size_t b_offset = get_block_offset(offset), nblocks;
    static uint8_t fs_block[RFS_BLOCK_SIZE];

    if (size % RFS_BLOCK_SIZE == 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        if (rfs_read_blocks(fs, b_offset, buf, nblocks) != nblocks) {
            goto error;
        }
    }

    if (size < RFS_BLOCK_SIZE) {
        if (rfs_read_blocks(fs, b_offset, fs_block, 1) != 1) {
            goto error;
        }

        memcpy(buf, fs_block, size);
        return size;
    }

    if (size % RFS_BLOCK_SIZE != 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        LOG_INFO("size %u | blocks %u | remaining %u",
                size, nblocks, size % RFS_BLOCK_SIZE);

        if (rfs_read_blocks(fs, b_offset, buf, nblocks) != nblocks || 
            rfs_read_blocks(fs, b_offset + nblocks, fs_block, 1) != 1) {
            goto error;
        }

        memcpy(buf + nblocks * RFS_BLOCK_SIZE, fs_block, size % RFS_BLOCK_SIZE);
        return size;
    }

error:
    LOG_EMERG("failed to read from disk!");
    return 0;
}
