#ifndef __INODE_H__
#define __INODE_H__

#include <stdint.h>
#include <time.h>

#include "fs.h"

#define RFS_NUM_BLOCKS      4
#define BYTES_PER_INODE     (1024 * 16)

/* has to be either file or dir */
enum {
    S_IFDIR = 1 << 0,
    S_IFREG = 1 << 1,
};

typedef struct inode {
	uint32_t i_ino;
    uint32_t i_size;

	/* ownership and access information */
	uint32_t i_uid;
	uint32_t i_gid;
	uint32_t mode;
	uint32_t flags;

    /* TODO: rethink this */
    uint32_t blocks[RFS_NUM_BLOCKS];

    uint8_t unused[24];
} inode_t;

inode_t *rfs_inode_alloc(fs_t *fs);
int rfs_inode_write(fs_t *fs, inode_t *ino);
void rfs_inode_delete(fs_t *fs, inode_t *ino);

#endif /* end of include guard: __INODE_H__ */
