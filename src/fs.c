#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "fs.h"
#include "fs_errno.h"
#include "debug.h"

#define SB_SIZE sizeof(superblock_t)

superblock_t *rfs_mkfs(const char *device)
{
    /* what to do:
     * 1) allocate space for superblock_t
     * 2) fill it with data you know
     * 3) create hdd_handle and open device 
     * 4) initialize root directory and inode structures 
     * 5) write changes to device */

    superblock_t *sb = malloc(sizeof(superblock_t));

    /* init basic stuff */
    sb->sb_magic1 = RFS_SB_MAGIC1;
    sb->sb_magic2 = RFS_SB_MAGIC2;
    sb->flag      = RFS_SB_DIRTY;

    /* open device and calculate size of inode table etc. */
    sb->fd = open_device(device, MNT_READ_WRITE);

    // TODO remove "/ 32" and add some kind of macro to bitmap.h

    // file system block stuff
    sb->blocks.len  = (sb->fd->size / RFS_BLOCK_SIZE) / 32;
    sb->blocks.bits = malloc(sizeof(uint32_t) * sb->blocks.len);

    debug(LOG_INFO, "bitmap for %zu file system blocks allocated (%zu bytes)!", 
           sb->blocks.len * 32, sb->blocks.len);

    // inode stuff
    sb->num_inodes  = RFS_NUM_INODES;
    sb->inodes.len  = RFS_NUM_INODES / 32;
    sb->inodes.bits = calloc(sb->inodes.len, sizeof(uint32_t));
    bm_set_bit(&sb->inodes, 0); // root node

    debug(LOG_INFO, "bitmap for %zu inodes allocated (%zu bytes)!", 
           sb->inodes.len * 32, sb->inodes.len);

	// TODO mkinode
	// can't be used for this
    sb->root.name[0]   = '/'; 
	sb->root.name[1]   = '\0';
    sb->root.inode_num = 0;
    sb->root.uid  = sb->root.gid   = 0; // FIXME chmod style numbering??
    sb->root.mode = sb->root.flags = 0;
    sb->root.create_time = sb->root.modified_time = time(NULL);

    /* calculate how many file system blocks 
     * boot loader, superblock and inodes consume */
    size_t space = (512 + sizeof(superblock_t) + (RFS_NUM_INODES - 1) 
                        * sizeof(inode_t))     / RFS_BLOCK_SIZE;
    bm_set_range(&sb->blocks, 0, space);
    debug(LOG_INFO, "%zu file system blocks consumed", space);

    debug(LOG_INFO, "writing changes to disk!");

    /* if (commit(sb) != FS_OK) { */
    /*  debug(LOG_EMERG, "failed to write superblock to disk!"); */
    /*  return NULL; */
    /* } */

    return sb;
}

superblock_t *rfs_mount(const char *device, const char *mode)
{
    int nread;
    hdd_handle_t *fd;
    superblock_t *sb = malloc(sizeof(superblock_t));

    fd = sb->fd = open_device(device, mode);

    if ((nread = read_blocks(sb->fd, 1, 0, sb, SB_SIZE)) != SB_SIZE) {
        debug(LOG_EMERG, "read %d bytes", nread);
        fs_errno = FS_FAIL_READ_FAILED;
        return NULL;
    } 

    /* reset superblock's file descriptor as it was overwritten */
    sb->fd = fd;

    debug(LOG_INFO, "superblock found!");
    debug(LOG_INFO, "0x%x (0x%x) and 0x%x (0x%x)", sb->sb_magic1, RFS_SB_MAGIC1,
                                                   sb->sb_magic2, RFS_SB_MAGIC2);

    if (sb->sb_magic1 != RFS_SB_MAGIC1 || sb->sb_magic2 != RFS_SB_MAGIC2) {
        fs_errno = FS_FAIL_INVALID_MAGIC_NUMBER;
        return NULL;
    }

    if (sb->flag != RFS_SB_CLEAN) {
		debug(LOG_WARN, "Dirty flag set when mounting file system!");
		debug(LOG_INFO, "Running file system consistency check!");
        fs_errno = FS_FAIL_DIRTY_FLAG_SET;
        return NULL;
    }

    // TODO: what to do with inodes
    //       and file data that's on the disk??

	// TODO: how to take root file system into account?!?!?!
	// 		 this question must be answered before any file 
	// 		 data block are written, it's absolutely essential

    // TODO root inode is allocated alongsize superblock
    //      read root node (and maybe its immediate children)
    //      to ram memory
    //
    //      TODO how to do that???

    return sb;
}

enum fs_errno_t rfs_umount(superblock_t *sb)
{
    if (!sb) {
        debug(LOG_WARN, "suberblock is NULL!");
		return FS_FAIL_INVALID_SUPERBLOCK;
    }

    debug(LOG_INFO, "unmounting...");
    debug(LOG_INFO, "writing superblock to disk...");

    sb->flag = RFS_SB_CLEAN;

	size_t nwritten;
    if ((nwritten = write_blocks(sb->fd, 1, 0, sb, SB_SIZE)) != SB_SIZE) {
		fprintf(stderr, "%s\n", strerror(errno));
        debug(LOG_EMERG, "failed to write superblock!");
		debug(LOG_EMERG, "wrote %zu bytes!", nwritten);
		return FS_FAIL_WRITE_FAILED;
    }

    close_device(sb->fd);

    free(sb->fd); free(sb);
    debug(LOG_INFO, "file system unmounted successfully!");

	return FS_OK;
}
