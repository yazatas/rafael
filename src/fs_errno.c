#include "fs_errno.h"

// TODO add mutex to guard this
int fs_errno;

const char *fs_strerror(int fs_errno)
{
	switch (fs_errno) {
		case FS_FAIL_INVALID_MAGIC_NUMBER: return "Invalid magic number";
		case FS_FAIL_SUPERBLOCK_NOT_FOUND: return "Superblock not found";
		case FS_FAIL_NOT_ENOUGH_SPACE:     return "Not enough space on the device";
		case FS_FAIL_READ_FAILED:          return "Unable to read from device";
		case FS_FAIL_WRITE_FAILED:         return "Unable to write to device";
		case FS_FAIL_DIRTY_FLAG_SET:       return "Dirty flag set in committed file system";
		default:                           return "Unknown error";
	}
}
