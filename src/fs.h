#ifndef __FS_H__
#define __FS_H__

#include <stdint.h>
#include <time.h>
#include <stdio.h>

#include "bitmap.h"
#include "driver.h"

#define RFS_BLOCK_SIZE   4096
#define MAX_INODE_COUNT    4096
#define RFS_NAME_MAX_LEN 128
#define RFS_NUM_BLOCKS   32

#define RFS_SB_MAGIC     0xDEADBEEF
#define RFS_SB_CLEAN     0xCAFEBABE
#define RFS_SB_DIRTY     0xBAAAAAAD

#define MNT_READ       "r"
#define MNT_WRITE      "w"
#define MNT_READ_WRITE "r+"

typedef struct block {
	uint32_t alloc_group;
	uint16_t start;
	uint16_t len;
} block_t;

/* FIXME: i think this data structure is not correct!??!?!? */
typedef struct data {
	size_t size;
	block_t direct[RFS_NUM_BLOCKS];
	block_t indirect;
	block_t double_indirect;
} data_t;

typedef struct inode {
	char name[RFS_NAME_MAX_LEN];
	uint32_t inode_num;

	/* ownership and access information */
	uint32_t uid;
	uint32_t gid;
	uint32_t mode;
	uint32_t flags;

	time_t create_time;
	time_t modified_time;

	data_t inode_data;
} inode_t;

typedef struct superblock {
	uint32_t sb_magic;
	uint32_t flag; /* clean/dirty */

	size_t num_blocks;
	size_t used_blocks;

	bitmap_t inodes;
	bitmap_t blocks;

	size_t num_inodes;
	inode_t *root; /* this is a directory */

	hdd_handle_t *fd;

	uint8_t padding[48];
} superblock_t;

superblock_t *rfs_mkfs(const char *device);
superblock_t *rfs_mount(const char *device);
void rfs_umount(superblock_t *sb);

#endif /* end of include guard: __FS_H__ */
