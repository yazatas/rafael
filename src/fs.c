#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "disk.h"
#include "io.h"
#include "inode.h"
#include "fs.h"
#include "fs_errno.h"

#define BOOTLOADER_SIZE  512 
#define BYTES_PER_INODE  (1024 * 16)
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

    LOG_DEBUG("disk size: %u bytes", disk_size);
    LOG_DEBUG("available space: %u bytes", avail_space);

    fs->sb->num_inodes = avail_space / (BYTES_PER_INODE + sizeof(inode_t));
    ino_map_size       = fs->sb->num_inodes * sizeof(inode_t);
    fs->sb->num_blocks = (avail_space - ino_map_size - TO_BM_LEN(fs->sb->num_inodes)) / RFS_BLOCK_SIZE;

    LOG_DEBUG("%u inodes take %u bytes of space", fs->sb->num_inodes, ino_map_size);
    LOG_WARN("file system has %u data blocks", fs->sb->num_blocks);

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

    LOG_DEBUG("writing inode bitmap to disk");

    byte_offset += bytes_written;
    fs->sb->ino_bm_start = byte_offset = 2 * RFS_BLOCK_SIZE;
    num_bytes = BM_GET_SIZE(bm_inode);

    LOG_DEBUG("inode bitmap takes %u bytes", num_bytes);
    LOG_DEBUG("inode bitmap starts at block %u (0x%x)", 
            fs->sb->ino_bm_start, fs->sb->ino_bm_start);
    LOG_DEBUG("writing inode bitmap to disk");

    bytes_written = rfs_write_buf(fs, byte_offset, bm_inode, num_bytes);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write inode bitmap to disk: %s!", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

    fs->sb->block_bm_start = byte_offset += bytes_written;
    num_bytes = BM_GET_SIZE(bm_data);

    LOG_DEBUG("data block bitmap takes %u bytes", num_bytes);
    LOG_DEBUG("data block bitmap starts at block %u (0x%x)",
            fs->sb->block_bm_start, fs->sb->block_bm_start);
    LOG_DEBUG("writing block bitmap to disk");

    bytes_written = rfs_write_buf(fs, byte_offset, bm_data, num_bytes);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write data block bitmap to disk: %s", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

    fs->sb->ino_map_start = byte_offset += bytes_written;

    LOG_DEBUG("inode map takes %u bytes", ino_map_size);
    LOG_DEBUG("inode map starts at block %u", fs->sb->ino_map_start);

    bytes_written = rfs_write_buf(fs, byte_offset, inode_map, ino_map_size);

    if (bytes_written == 0) {
        LOG_EMERG("failed to write inode map to disk: %s!", fs_strerror(0));
        goto error;
    }

    /* ******************************************************************** */

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
    fs_t *fs = malloc(sizeof(fs_t));
    fs->sb = malloc(sizeof(superblock_t));
    fs->fd = open(device, O_RDWR | O_CREAT);

    disk_init(device);

    fs->sb->num_blocks = 1;
    if (rfs_read_buf(fs, RFS_BLOCK_SIZE, fs->sb, SB_SIZE) == 0) {
        LOG_EMERG("failed to read superblock from disk!");
        goto error;
    }

    if (fs->sb->magic1 != RFS_SB_MAGIC1 || fs->sb->magic2 != RFS_SB_MAGIC2) {
        LOG_EMERG("Invalid magic number: 0x%x 0x%x", fs->sb->magic1, fs->sb->magic2);
        fs_set_errno(FS_SB_INVALID_MAGIC);
        return NULL;
    }

    LOG_DEBUG("superblock:\n"
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

    close(fs->fd);
    return fs;

error:
    fs_set_errno(FS_SB_READ_FAILED);
    return NULL;
}

fs_status_t rfs_umount(fs_t *fs)
{
}
