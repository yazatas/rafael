#ifndef __FS_H__
#define __FS_H__

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>

#include "bitmap.h"
#include "fs_errno.h"

#define RFS_BLOCK_SIZE   4096
#define RFS_SB_MAGIC1    0xDEADBEEF
#define RFS_SB_MAGIC2    0x13371338
#define RFS_SB_CLEAN     0xCAFEBABE
#define RFS_SB_DIRTY     0xBAAAAAAD

#define MNT_READ       "r"
#define MNT_WRITE      "w"
#define MNT_READ_WRITE "r+"

typedef struct inode inode_t;

/* two magic numbers at the beginning and end to make sure at least
 * superblock's integrity can be verified 100% */
typedef struct superblock {
	uint32_t magic1;
	uint32_t flag; /* clean/dirty */

	size_t num_blocks;
	size_t used_blocks;
    size_t dev_block_size;
    off_t  block_bm_start;
    off_t  block_map_start;

	size_t num_inodes;
    size_t used_inodes;
    off_t  ino_bm_start;
    off_t  ino_map_start;

	uint32_t magic2;

    uint8_t unused[42];
} superblock_t;

/* TODO: this can be removed?? */
typedef struct mount {
    superblock_t *sb;
    bitmap_t *bm_inode;
    bitmap_t *bm_data;
    inode_t *inode_map;
} mount_t;

typedef struct fs {
    superblock_t *sb;
    int fd;

    bitmap_t *bm_inode;
    bitmap_t *bm_data;

    /* TODO: this is just a temporary solution */
    inode_t **inode_map;
    size_t ino_map_len;
} fs_t;

/* open device  */
fs_t *rfs_mkfs(const char *device);

/* open device (if it's not open yet), allocate space for
 * superblock and other file system data structures
 * initialize inode structure and load root node
 * from disk to memory
 *
 * return pointer to mount_t structure
 * return NULL on error and set fs_errno */
fs_t *rfs_mount(const char *device);

/* write all changes to device and close device file descriptor */
fs_status_t rfs_umount(fs_t *fs);

#endif /* end of include guard: __FS_H__ */
