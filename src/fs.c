#include <stdlib.h>

#include "fs.h"

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
	sb->sb_magic = RFS_SB_MAGIC;
	sb->flag     = RFS_SB_DIRTY;

	/* open device and calculate size of inode table etc. */
	sb->fd = open_device(device, MNT_READ_WRITE);

	sb->blocks.len  = (sb->fd->size / RFS_BLOCK_SIZE) / 32;
	sb->blocks.bits = malloc(sizeof(uint32_t) * sb->blocks.len);

	printf("[rfs_mkfs] bitmap for %zu file system blocks allocated!\n", sb->blocks.len * 32);
}

superblock_t *rfs_mount(const char *device)
{
	/* TODO: what am i supposed to do here?? */
	/* what i am supposed to do here:
	 * 1) (try to) read superblock from device 
	 * 2) make sure it's consistent (magic number) 
	 * 3) ...? */
	/* superblock_t *sb = malloc(sizeof(superblock_t)); */
	hdd_handle_t *fd = open_device(device, MNT_READ_WRITE);

	superblock_t *sb = malloc(sizeof(superblock_t));
	sb->fd = fd;

	/* TODO: where does the file system info starts (boot partition!?!?) */
}

void rfs_umount(superblock_t *sb)
{
	/* 1) write superblock to device
	 * 2) close device */
}
