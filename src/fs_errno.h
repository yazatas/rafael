#ifndef __FS_ERRNO_H__
#define __FS_ERRNO_H__

extern int fs_errno;

/* How to use:
 *
 * 1) add new error code to FS_ERRORS
 *   -> style: FS_<ACTION>_FAIL_<CONDITION>
 *      -> FS_MOUNT_FAIL_INVALID_SUPERBLOCK
 *      -> FS_MKFS_FAIL_NOT_ENOUGH_SPACE
 * 2) add explanation of error to fs_strerror
 *    -> only a general explanation, function that failed has
 *       written better explanation to error logs
 */

enum {
	FS_MOUNT_FAIL_INVALID_SUPERBLOCK,
	FS_MKFS_FAIL_NOT_ENOUGH_SPACE,
} FS_ERRORS;

const char *fs_strerror(int fs_errno);

#endif /* end of include guard: __FS_ERRNO_H__ */
