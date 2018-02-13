#include "fs_errno.h"

int fs_errno;

const char *fs_strerror(int fs_errno)
{
	switch (fs_errno) {
		case FS_MOUNT_FAIL_INVALID_SUPERBLOCK:
			return "Invalid superblock, mount failed\n";
		case FS_MKFS_FAIL_NOT_ENOUGH_SPACE:
			return "Not enough space on the device, mkfs failed\n";
		default:
			return "Unknown error";
	}
}
