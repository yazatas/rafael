#ifndef __INODE_H__
#define __INODE_H__

#include <stdint.h>
#include <time.h>

#include "fs.h"

#define RFS_NUM_BLOCKS     12
#define RFS_NAME_MAX_LEN  128
#define BYTES_PER_INODE     (1024 * 16)

typedef struct inode {
	uint32_t i_ino;
    uint32_t i_size;

	/* ownership and access information */
	uint32_t i_uid;
	uint32_t i_gid;
	uint32_t mode;
	uint32_t flags;

    uint32_t blocks[RFS_NUM_BLOCKS];
} inode_t;

inode_t *rfs_alloc_inode(fs_t *fs);
int rfs_write_inode(fs_t *fs, inode_t *ino);
void rfs_delete_inode(fs_t *fs, inode_t *ino);

#endif /* end of include guard: __INODE_H__ */
