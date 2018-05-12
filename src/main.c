#include "fs.h"
#include "fs_errno.h"
#include "debug.h"

// TODO create tool which prints hdd layout (setion, empty space)!! extremely useful

int main(void)
{
	puts("\n\n\n---------------------------------------");

	superblock_t *sb;
	enum fs_errno_t err;
	const char *device = "hdd_file";

	if ((sb = rfs_mount(device, MNT_READ_WRITE)) == NULL) {
		debug(LOG_EMERG, "mount failed: %s", fs_strerror(fs_errno));

		/* create file system to device if suberblock is not found
		 *
		 * it either means that file system is corrupt or device is empty */
		if (fs_errno == FS_FAIL_SUPERBLOCK_NOT_FOUND) {
			
			if ((sb = rfs_mkfs(device)) == NULL) {
				debug(LOG_EMERG, "mkfs failed: %s", fs_strerror(fs_errno));
				return -1;
			}
		}
	}

	if ((err = rfs_umount(sb)) != FS_OK) {
		debug(LOG_EMERG, "failed to unmount file system: %s!", fs_strerror(err));
	}
}
