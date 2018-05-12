#ifndef __FS_H__
#define __FS_H__

#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "bitmap.h"
#include "driver.h"
#include "fs_errno.h"
#include "inode.h"

#define RFS_BLOCK_SIZE   4096

#define RFS_SB_MAGIC1    0xDEADBEEF
#define RFS_SB_MAGIC2    0x13371338
#define RFS_SB_CLEAN     0xCAFEBABE
#define RFS_SB_DIRTY     0xBAAAAAAD

#define MNT_READ       "r"
#define MNT_WRITE      "w"
#define MNT_READ_WRITE "r+"

/*
 * two magic numbers at the beginning and end to make sure at least
 * superblock's integrity can be verified 100%
 *
 * TODO: align this to some boundary
 */
typedef struct superblock {
	uint32_t sb_magic1;
	uint32_t flag; /* clean/dirty */

	size_t num_blocks;
	size_t used_blocks;

	bitmap_t inodes;
	bitmap_t blocks;

	size_t num_inodes;
	inode_t root; /* this is a directory */

	hdd_handle_t *fd;

	uint32_t sb_magic2;
} superblock_t;


superblock_t *rfs_mkfs(const char *device);

/* open device (if it's not open yet), allocate space for
 * superblock and other file system data structures 
 * initialize inode structure and load root node 
 * from disk to memory 
 *
 * return pointer to superblock on success
 * return NULL on error and set fs_errno */
superblock_t *rfs_mount(const char *device, const char *mode);

// write all changes to device and close device file descriptor
enum fs_errno_t rfs_umount(superblock_t *sb);

#endif /* end of include guard: __FS_H__ */
