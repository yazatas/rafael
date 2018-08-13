#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "fs.h"
#include "fs_errno.h"
#include "inode.h"
#include "io.h"


static int get_uid(void)
{
    return 1337;
}

static int get_gid(void)
{
    return 1338;
}

static uint32_t get_ino(fs_t *fs)
{
    int index;

    if ((index = bm_find_first_unset(fs->bm_inode, 0, fs->bm_inode->len - 1)) < 0) {
        LOG_WARN("failed to find free inode");
        return UINT32_MAX;
    }

    bm_set_bit(fs->bm_inode, index);
    return index;
}

/* TODO: refactor this, it's full of
 * idiotic solutions and ugly code */
inode_t *rfs_alloc_inode(fs_t *fs)
{
    LOG_INFO("allocating inode...");

    inode_t *ino = malloc(sizeof(inode_t));

	ino->flags  = 666;
	ino->mode   = 666;
	ino->i_gid  = get_gid();
    ino->i_size = 0;
	ino->i_uid  = get_uid();

	if ((ino->i_ino = get_ino(fs)) == UINT32_MAX) {
        LOG_EMERG("failed to allocate inode number");
        goto error;
    }

    /* TODO: move the code below to block allocator??? */

    /* initially allocate 4 file system blocks for each inode */
    size_t num_blocks = BYTE_TO_BLOCK(BYTES_PER_INODE);

    int index = bm_find_first_unset_range(fs->bm_data, 0, fs->bm_data->len - 1, num_blocks);
    if (index < 0) {
        /* TODO: find longest range suitable for our needs 
         * and then some single blocks until the space requirement is fulfilled */
        LOG_EMERG("no more free blocks"); /* FIXME: */
        goto error;
    }

    LOG_DEBUG("allocated %u blocks for inode %u", num_blocks, ino->i_ino);
    bm_set_range(fs->bm_data, index, index + num_blocks - 1);

    /* TODO: 
     *
     * for now just save the block number to inode blocks arrays
     * and write blocks full of debug bytes
     *
     * in the future the allocation could be delayed until unmount or fsync() */
    static uint8_t buf[RFS_BLOCK_SIZE];
    uint8_t bytes[4] = { 0xab, 0xbc, 0xcd, 0xde };

    size_t block_offset = BYTE_TO_BLOCK(fs->sb->block_map_start) + index;

    for (size_t i = index; i < num_blocks; ++i) {
        LOG_DEBUG("writing %u '0x%x' bytes to block %u",
                RFS_BLOCK_SIZE, bytes[i], block_offset + i);
        memset(buf, bytes[i], RFS_BLOCK_SIZE);
        rfs_write_blocks(fs, block_offset + i, buf, 1);
    }

    return ino;

error:
    free(ino);
    return NULL;
}

int rfs_write_inode(fs_t *fs, inode_t *ino)
{
    /* TODO: update inode bitmap */
    /* TODO: update inode map */
    /* TODO: update inode's blocks */
}

void delete_inode(fs_t *fs, inode_t *ino)
{
    /* TODO: set bitmap entry for this inode to 0 */
    /* TODO: find the inode in the inode map and zero it out */
    /* TODO: set bits in the data bitmap to zero */

    /* TODO: 
     * TODO: SUPERBLOCK/SOMETHING NEEDED!!!
     * TODO: */
}
