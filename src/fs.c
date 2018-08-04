#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "debug.h"
#include "driver.h"
#include "fs.h"
#include "fs_errno.h"

#define BOOTLOADER_SIZE 512 
#define BYTES_PER_INODE (1024 * 16)
#define SB_SIZE         sizeof(superblock_t)

static superblock_t *sb_global;

/* start of the disk is divided as following:
 * 1) boot loader  (512 bytes)
 * 2) superblock   (128 bytes)
 * 3) inode bitmap (N bytes)
 * 4) block bitmap (N bytes)
 * 5) inode map    (N bytes)
 * 6) data blocks  (N bytes) */
fs_t *rfs_mkfs(const char *device)
{
    size_t avail_space, disk_size, offset;
    size_t total, bm_size, nwritten, ino_map_size;

    fs_t *fs = malloc(sizeof(fs_t));
    fs->sb   = malloc(sizeof(superblock_t));

    LOG_INFO("superblock size: %u", sizeof(superblock_t));

    fs->sb->magic1 = RFS_SB_MAGIC1;
    fs->sb->magic2 = RFS_SB_MAGIC2;
    fs->sb->flag = RFS_SB_CLEAN;
    fs->sb->dev_block_size = 512;

    if ((fs->fd = open(device, O_RDWR | O_CREAT)) == -1) {
        LOG_EMERG("failed to open disk: %s", strerror(errno));
        return NULL;
    }

    fs->sb->used_blocks = fs->sb->used_inodes = 0;

    fs->sb->dev_block_size = dev_get_block_size();
    fs->sb->num_blocks     = dev_get_num_blocks(fs->fd);

    disk_size   = fs->sb->dev_block_size * fs->sb->num_blocks;
    avail_space = disk_size - BOOTLOADER_SIZE - sizeof(superblock_t);

    LOG_INFO("disk size: %u bytes", disk_size);
    LOG_INFO("available space: %u bytes", avail_space);

    fs->sb->num_inodes = avail_space / (BYTES_PER_INODE + sizeof(inode_t));
    ino_map_size       = fs->sb->num_inodes * sizeof(inode_t);
    fs->sb->num_blocks = (avail_space - ino_map_size - TO_BM_LEN(fs->sb->num_inodes)) / RFS_BLOCK_SIZE;

    /* sanity check, for debugging only */
    total = BOOTLOADER_SIZE + sizeof(superblock_t) + ino_map_size +
            TO_BM_LEN(fs->sb->num_inodes) + TO_BM_LEN(fs->sb->num_blocks) + 
            fs->sb->num_blocks * RFS_BLOCK_SIZE;

    LOG_INFO("total space used: %u bytes", total);

    if (total > disk_size) {
        LOG_EMERG("file system total size exceeds disk size!");
        return NULL;
    }

    bitmap_t *bm_inode = bm_alloc_bitmap(fs->sb->num_inodes);
    bitmap_t *bm_data  = bm_alloc_bitmap(fs->sb->num_blocks);
    inode_t *inode_map = malloc(ino_map_size);

    if ((nwritten = write_blocks(fs, BOOTLOADER_SIZE, fs->sb, SB_SIZE)) != SB_SIZE) {
        LOG_EMERG("failed to write superblock to disk!");
        goto error;
    }

    offset = SB_SIZE + BOOTLOADER_SIZE;
    fs->sb->ino_bm_start = offset;
    bm_size = BM_GET_SIZE(bm_inode);

    LOG_INFO("inode bitmap size and start: %u 0x%x", bm_size, fs->sb->ino_bm_start);

    if ((nwritten = write_blocks(fs, offset, bm_inode, bm_size)) != bm_size) {
        LOG_EMERG("failed to write inode bitmap to disk!");
        goto error;
    }

    offset += bm_size;
    fs->sb->block_bm_start = offset;
    bm_size = BM_GET_SIZE(bm_data);

    LOG_INFO("block bitmap size and start: %u 0x%x", bm_size, fs->sb->block_bm_start);

    if ((nwritten = write_blocks(fs, offset, bm_data, bm_size)) != bm_size) {
        LOG_EMERG("failed to write data block bitmap to disk!");
        goto error;
    }

    offset += bm_size;
    fs->sb->ino_map_start = offset;

    LOG_INFO("inode map size and start: %u 0x%x", ino_map_size, offset);

    if ((nwritten = write_blocks(fs, offset, inode_map, ino_map_size)) != ino_map_size) {
        LOG_EMERG("failed to write inode map to disk!");
        goto error;
    }

    offset += ino_map_size;
    fs->sb->block_map_start = offset;

    LOG_INFO("data block start: 0x%x", offset);

    if ((nwritten = write_blocks(fs, BOOTLOADER_SIZE, fs->sb, SB_SIZE)) != SB_SIZE) {
        LOG_EMERG("failed to write superblock to disk!");
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
    size_t nread;

    fs_t *fs = malloc(sizeof(fs_t));
    fs->sb   = malloc(sizeof(superblock_t));
    fs->fd   = open(device, O_RDWR | O_CREAT);

    if ((nread = read_blocks(fs, BOOTLOADER_SIZE, fs->sb, SB_SIZE)) != SB_SIZE) {
        LOG_EMERG("failed to read superblock!");
        return NULL;
    }

    if (fs->sb->magic1 != RFS_SB_MAGIC1 || fs->sb->magic2 != RFS_SB_MAGIC2) {
        LOG_EMERG("Invalid magic number: 0x%x 0x%x", fs->sb->magic1, fs->sb->magic2);
        return NULL;
    }

    LOG_INFO("superblock:\n"
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
}

fs_status_t rfs_umount(fs_t *fs)
{
}
