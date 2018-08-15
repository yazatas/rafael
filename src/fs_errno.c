#include "fs_errno.h"

// TODO add mutex to guard this
int fs_errno;

static int fs_errno_new;

void fs_set_errno(fs_status_t status)
{
    /* TODO: use pthread for now?? */
    fs_errno_new = status;
}

const char *fs_strerror(fs_status_t fs_errno)
{
	switch (fs_errno_new) {

        case FS_OK:                        return "Success";
        case FS_READ_FAILED:               return "Read failed";
        case FS_WRITE_FAILED:              return "Write failed";
        case FS_NOT_ENOUGH_SPACE:          return "Not enough space";
        case FS_SB_READ_FAILED:            return "Failed to read Superblock";
        case FS_SB_INVALID_MAGIC:          return "Invalid magic number";

        /* FIXME: deprecated, remove */
		case FS_FAIL_NOT_ENOUGH_SPACE:     return "Not enough space on the device";
		case FS_FAIL_READ_FAILED:          return "Unable to read from device";
		case FS_FAIL_WRITE_FAILED:         return "Unable to write to device";
		case FS_FAIL_DIRTY_FLAG_SET:       return "Dirty flag set in committed file system";
		default:                           return "Unknown error";
	}
}
