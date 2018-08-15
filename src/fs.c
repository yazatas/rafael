#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "debug.h"
#include "disk.h"
#include "fs.h"
#include "fs_errno.h"
#include "io.h"
#include "inode.h"

#define BOOTLOADER_SIZE  512 
#define SB_SIZE          sizeof(superblock_t)

static inline size_t bytes_to_blocks(size_t bytes)
{
    return bytes / RFS_BLOCK_SIZE + ((bytes % RFS_BLOCK_SIZE) != 0);
}

/* rafael disk layout:
 *
 * 1) Booting stuff (1 block)
 *    * Basic bootloader (512 bytes)
 *    * Second stage boot loader (3584 bytes [max]) 
 *
 * 2) Superblock
 *    * This is a little overkill as superblock is below 256 bytes
 *      but for simplicity's sake make it 4KB long
 *
 * 3) Inode bitmap (N blocks [depends on the disk size])
 *
 * 3) Data block bitmap (N blocks [depends on the disk size])
 *
 * 3) Inode map (N blocks [depends on the disk size])
 *
 * 4) Data blocks (rest of the disk) */
fs_t *rfs_mkfs(const char *device)
{
    size_t avail_space, disk_size, offset, num_blocks,
           total, bm_size, nwritten, ino_map_size, num_bytes;
    size_t byte_offset, bytes_written;

    fs_t *fs = malloc(sizeof(fs_t));
    fs->sb   = malloc(sizeof(superblock_t));

    LOG_DEBUG("superblock size: %u", sizeof(superblock_t));

    fs->sb->magic1 = RFS_SB_MAGIC1;
    fs->sb->magic2 = RFS_SB_MAGIC2;
    fs->sb->flag = RFS_SB_CLEAN;

    if ((fs->fd = disk_init(device)) == -1) {
        LOG_EMERG("failed to open disk: %s", strerror(errno));
        return NULL;
    }

    fs->sb->used_blocks = fs->sb->used_inodes = 0;
    fs->sb->dev_block_size = disk_get_block_size(fs->fd);
    fs->sb->num_blocks     = disk_get_size(fs->fd) / fs->sb->dev_block_size;

    disk_size   = fs->sb->dev_block_size * fs->sb->num_blocks;
    avail_space = disk_size - BOOTLOADER_SIZE - sizeof(superblock_t);

    LOG_INFO("disk size: %u bytes", disk_size);
    LOG_INFO("available space: %u bytes", avail_space);

    fs->sb->num_inodes = avail_space / (BYTES_PER_INODE + sizeof(inode_t));
    ino_map_size       = fs->sb->num_inodes * sizeof(inode_t);
    fs->sb->num_blocks = (avail_space - ino_map_size - TO_BM_LEN(fs->sb->num_inodes)) / RFS_BLOCK_SIZE;

    LOG_INFO("%u inodes take %u bytes of space", fs->sb->num_inodes, ino_map_size);
    LOG_INFO("file system has %u data blocks", fs->sb->num_blocks);

    /* sanity check, for debugging only */
    total = BOOTLOADER_SIZE + sizeof(superblock_t) + ino_map_size +
            TO_BM_LEN(fs->sb->num_inodes) + TO_BM_LEN(fs->sb->num_blocks) + 
            fs->sb->num_blocks * RFS_BLOCK_SIZE;

    LOG_DEBUG("total space used: %u bytes", total);

    if (total > disk_size) {
        LOG_EMERG("file system total size exceeds disk size!");
        return NULL;
    }

    bitmap_t *bm_inode = bm_alloc_bitmap(fs->sb->num_inodes);
    bitmap_t *bm_data  = bm_alloc_bitmap(fs->sb->num_blocks);
    inode_t *inode_map = malloc(ino_map_size);

    LOG_INFO("bm_inode size %u", bm_inode->len);
    LOG_INFO("bm_data size %u", bm_data->len);

    /* DEBUG ONLY: write bootloader block full of garbage */
    uint8_t *bootloader = malloc(RFS_BLOCK_SIZE);
    memset(bootloader, 0xff, RFS_BLOCK_SIZE);
    
    LOG_DEBUG("writing temporary boot loader to disk");

    byte_offset   = 0;
    bytes_written = rfs_write_buf(fs, byte_offset, bootloader, RFS_BLOCK_SIZE);
    bytes_written = RFS_BLOCK_SIZE;

    if (bytes_written != RFS_BLOCK_SIZE) {
        LOG_EMERG("failed to write bootloader block");
        goto error;
    }

    free(bootloader);

    /* ******************************************************************** */

    LOG_INFO("writing inode bitmap to disk");

    byte_offset += bytes_written;
    fs->sb->ino_bm_start = byte_offset = 2 * RFS_BLOCK_SIZE;
    num_bytes = BM_GET_SIZE(bm_inode);

    LOG_DEBUG("inode bitmap takes %u bytes", num_bytes);
    LOG_DEBUG("inode bitmap starts at block %u (0x%x)", 
            fs->sb->ino_bm_start, fs->sb->ino_bm_start);

    /* FIXME: should the offsets be in fs blocks or bytes?? */

    bytes_written = bm_write_to_disk(fs, byte_offset, bm_inode);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write inode bitmap to disk: %s!", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

    LOG_INFO("writing block bitmap to disk");

    fs->sb->block_bm_start = byte_offset += bytes_written;
    num_bytes = BM_GET_SIZE(bm_data);

    LOG_DEBUG("data block bitmap takes %u bytes", num_bytes);
    LOG_DEBUG("data block bitmap starts at block %u (0x%x)",
            fs->sb->block_bm_start, fs->sb->block_bm_start);

    bytes_written = bm_write_to_disk(fs, byte_offset, bm_data);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write data block bitmap to disk: %s", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

    LOG_INFO("writing inode map to disk");

    fs->sb->ino_map_start = byte_offset += bytes_written;

    LOG_DEBUG("inode map takes %u bytes", ino_map_size);
    LOG_DEBUG("inode map starts at block %u", fs->sb->ino_map_start);

    bytes_written = rfs_write_buf(fs, byte_offset, inode_map, ino_map_size);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write inode map to disk: %s!", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

    LOG_INFO("writing superblock to disk");

    fs->sb->block_map_start = byte_offset + bytes_written;

    LOG_DEBUG("data blocks start at block %u", fs->sb->block_map_start);
    LOG_DEBUG("writing superblock to disk");

    memset(fs->sb->unused, 0xaa, sizeof(fs->sb->unused));

    byte_offset = RFS_BLOCK_SIZE;
    bytes_written = rfs_write_buf(fs, byte_offset, fs->sb, SB_SIZE);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write superblock to disk");
        goto error;
    }

    return fs;

error:
    LOG_EMERG("failed to write file system to disk!");
    fs_set_errno(FS_WRITE_FAILED);
    return NULL;
}

fs_t *rfs_mount(const char *device)
{
    LOG_INFO("mounting file system from device '%s'", device);

    fs_t *fs        = malloc(sizeof(fs_t));
    fs->sb          = malloc(sizeof(superblock_t));
    fs->fd          = disk_init(device);
    fs->bm_inode    = bm_alloc_bitmap(0);
    fs->bm_data     = bm_alloc_bitmap(0);

    fs->sb->num_blocks = 1;
    if (rfs_read_buf(fs, RFS_BLOCK_SIZE, fs->sb, SB_SIZE) == 0) {
        LOG_EMERG("failed to read superblock from disk!");
        fs_set_errno(FS_SB_READ_FAILED);
        return NULL;
    }

    if (fs->sb->magic1 != RFS_SB_MAGIC1 || fs->sb->magic2 != RFS_SB_MAGIC2) {
        LOG_EMERG("Invalid magic number: 0x%x 0x%x", fs->sb->magic1, fs->sb->magic2);
        fs_set_errno(FS_SB_INVALID_MAGIC);
        return NULL;
    }

    if (bm_read_from_disk(fs, fs->sb->ino_bm_start, fs->bm_inode) == 0) {
        LOG_EMERG("failed to read inode bitmap from disk!");
        fs_set_errno(FS_READ_FAILED);
        return NULL;
    }

    if (bm_read_from_disk(fs, fs->sb->block_bm_start, fs->bm_data) == 0) {
        LOG_EMERG("failed to read data bitmap from disk!");
        fs_set_errno(FS_READ_FAILED);
        return NULL;
    }

    fs->inode_map   = malloc(sizeof(inode_t *) * 30);
    fs->ino_map_len = 0; /* TODO: len, not count */

    LOG_INFO("\nsuperblock:\n"
             "\tmagic1:          0x%08x\n"
             "\tmagic2:          0x%08x\n"
             "\tnum_blocks:      %10u\n"
             "\tused_blocks:     %10u\n"
             "\tdev_block_size:  %10u\n"
             "\tblock_bm_start:  0x%08x\n"
             "\tblock_map_start: 0x%08x\n"
             "\tnum_inodes:      %10u\n"
             "\tused_inodes:     %10u\n"
             "\tino_bm_start:    0x%08x\n"
             "\tino_map_start:   0x%08x\n", fs->sb->magic1, fs->sb->magic2,
             fs->sb->num_blocks, fs->sb->used_blocks, fs->sb->dev_block_size,
             fs->sb->block_bm_start, fs->sb->block_map_start, fs->sb->num_inodes,
             fs->sb->used_inodes, fs->sb->ino_bm_start, fs->sb->ino_map_start);

    
    return fs;
}

fs_status_t rfs_umount(fs_t *fs)
{
    LOG_INFO("unmounting disk...");
    fs->sb->flag = RFS_SB_CLEAN;

    LOG_DEBUG("writing inode bitmap to disk at offset %u", fs->sb->ino_bm_start);
    if (bm_write_to_disk(fs, fs->sb->ino_bm_start, fs->bm_inode) == 0) {
        LOG_EMERG("failed to write inode bitmap to disk");
        return FS_WRITE_FAILED;
    }

    LOG_DEBUG("writing block bitmap to disk at offset %u", fs->sb->block_bm_start);
    if (bm_write_to_disk(fs, fs->sb->block_bm_start, fs->bm_data) == 0) {
        LOG_EMERG("failed to write block bitmap to disk");
        return FS_WRITE_FAILED;
    }

    LOG_DEBUG("writing superblock to disk at offset %u", BLOCK_TO_BYTE(1));
    if (rfs_write_buf(fs, BLOCK_TO_BYTE(1), fs->sb, sizeof(superblock_t)) == 0) {
        LOG_EMERG("failed to write superblock to disk");
        return FS_WRITE_FAILED;
    }

    LOG_DEBUG("writing inodes from the inode map to disk");
    for (size_t i = 0; i < fs->ino_map_len; ++i) {
        if (rfs_write_inode(fs, fs->inode_map[i]) == 0)
            LOG_WARN("failed to write inode to disk");
        free(fs->inode_map[i]);
    }

    bm_dealloc_bitmap(fs->bm_inode);
    bm_dealloc_bitmap(fs->bm_data);
    disk_close(fs->fd);
    free(fs->inode_map);
    free(fs->sb);
    free(fs);

    return FS_OK;
}
