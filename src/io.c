#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#include "disk.h"
#include "fs.h"
#include "io.h"

/* TODO: pass fs object and check that the calculated
 * offset is within limits of the file system block count */
/* convert byte offset to block offset */
static size_t get_block_offset(size_t offset)
{
    return offset / RFS_BLOCK_SIZE;
}

static size_t get_sector_offset(int fd, size_t offset)
{
    return offset / disk_get_block_size(fd);
}

static size_t get_block_count(size_t nbytes)
{
    return nbytes / RFS_BLOCK_SIZE + ((nbytes % RFS_BLOCK_SIZE) != 0);
}

size_t rfs_read_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t nblocks)
{
    const size_t BLOCK_SIZE = disk_get_block_size(fs->fd);

    if (b_offset > fs->sb->num_blocks) {
        LOG_EMERG("Trying to read from block %u but file system has only %u blocks",
                b_offset, fs->sb->num_blocks);
        return 0;
    }

    LOG_DEBUG("reading %u blocks, block offset %u",  nblocks, b_offset);

    for (size_t i = 0, ptr = 0; i < nblocks; ++i) {

        size_t start = (b_offset + i) * RFS_BLOCK_SIZE;
        size_t end   = (b_offset + i) * RFS_BLOCK_SIZE + RFS_BLOCK_SIZE;

        for (; start < end; start+=BLOCK_SIZE, ptr+=BLOCK_SIZE) {
            LOG_DEBUG("reading from disk sector %u", start / BLOCK_SIZE);
            disk_read(fs->fd, start / BLOCK_SIZE, &((uint8_t *)buf)[ptr]);
        }
    }

    return nblocks;
}

size_t rfs_write_blocks(fs_t *fs, uint32_t b_offset, void *buf, size_t nblocks)
{
    const size_t BLOCK_SIZE = disk_get_block_size(fs->fd);

    if (b_offset > fs->sb->num_blocks) {
        LOG_EMERG("Trying to write to block %u but file system has only %u blocks",
                b_offset, fs->sb->num_blocks);
        return 0;
    }

    LOG_DEBUG("writing %u blocks, block offset %u",  nblocks, b_offset);

    for (size_t i = 0, ptr = 0; i < nblocks; ++i) {

        size_t start = (b_offset + i) * RFS_BLOCK_SIZE;
        size_t end   = (b_offset + i) * RFS_BLOCK_SIZE + RFS_BLOCK_SIZE;

        for (; start < end; start+=BLOCK_SIZE, ptr += BLOCK_SIZE) {
            LOG_DEBUG("writing to disk sector %u", start / BLOCK_SIZE);
            disk_write(fs->fd, start / BLOCK_SIZE, &((uint8_t *)buf)[ptr]);
        }
    }

    return nblocks;
}

/* If size is evenly divisible by RFS_BLOCK_SIZE then this routine is very simple
 * We simply call rfs_write_blocks and store the data to "buf"
 *
 * This, however, won't be the case most of the time so this is how we manage uneven reads */
size_t rfs_write_buf(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    static uint8_t fs_block[RFS_BLOCK_SIZE];
    size_t b_offset = get_block_offset(offset);
    size_t nblocks, nwritten;

    if (buf == NULL || size == 0) {
        goto error;
    }

    LOG_DEBUG("writing %u bytes, byte offset %u", size, offset);

    if (size % RFS_BLOCK_SIZE == 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        if ((nwritten = rfs_write_blocks(fs, b_offset, buf, nblocks)) != nblocks)
            goto error;

        return nwritten * RFS_BLOCK_SIZE;
    }

    if (size < RFS_BLOCK_SIZE) {
        memset(fs_block, 0, RFS_BLOCK_SIZE);
        memcpy(fs_block, buf, size);

        if ((nwritten = rfs_write_blocks(fs, b_offset, fs_block, 1)) != 1)
            goto error;

        return nwritten * RFS_BLOCK_SIZE;
    }

    if (size % RFS_BLOCK_SIZE != 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        LOG_DEBUG("size %u | blocks %u | remaining %u",
                size, nblocks, size % RFS_BLOCK_SIZE);

        if ((nwritten = rfs_write_blocks(fs, b_offset, buf, nblocks)) != nblocks)
            goto error;

        memcpy(buf + nblocks * RFS_BLOCK_SIZE, fs_block, size % RFS_BLOCK_SIZE);

        if (rfs_write_blocks(fs, b_offset + nblocks, fs_block, 1) != 1)
            goto error;

        return (nwritten + 1) * RFS_BLOCK_SIZE;
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
 *    * As the rfs_read_blocks() reads an entire fs block,
 *      remember to copy bytes starting from byte offset
 *
 * 2) if size % RFS_BLOCK_SIZE != 0 
 *    * read "size / RFS_BLOCK_SIZE" blocks from disk right to buf
 *    * read one block of data (the remainder) from disk to fs_block
 *    * copy contents from fs_block to buf  */
size_t rfs_read_buf(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    size_t b_offset = get_block_offset(offset), nblocks;
    static uint8_t fs_block[RFS_BLOCK_SIZE];

    if (buf == NULL || size == 0) {
        goto error;
    }

    LOG_DEBUG("reading %u bytes, byte offset %u", size, offset);

    if (size % RFS_BLOCK_SIZE == 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        if (rfs_read_blocks(fs, b_offset, buf, nblocks) != nblocks) {
            goto error;
        }

        return size;
    }

    if (size < RFS_BLOCK_SIZE) {
        if (rfs_read_blocks(fs, b_offset, fs_block, 1) != 1) {
            goto error;
        }

        memcpy(buf, fs_block + offset % RFS_BLOCK_SIZE, size);
        return size;
    }

    if (size % RFS_BLOCK_SIZE != 0) {
        nblocks = size / RFS_BLOCK_SIZE;

        LOG_DEBUG("size %u | blocks %u | remaining %u",
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
