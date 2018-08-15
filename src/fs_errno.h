#ifndef __FS_ERRNO_H__
#define __FS_ERRNO_H__

/* TODO: make it static and create fs_set_errno 
 * which uses a mutex to guard the errno */
extern int fs_errno;

typedef enum fs_status {
	FS_OK,
    FS_READ_FAILED,
    FS_WRITE_FAILED,
    FS_NOT_ENOUGH_SPACE,
    FS_SB_READ_FAILED,
    FS_SB_INVALID_MAGIC,

	FS_FAIL_NOT_ENOUGH_SPACE,
	FS_FAIL_READ_FAILED,
	FS_FAIL_WRITE_FAILED,
	FS_FAIL_DIRTY_FLAG_SET,
} fs_status_t;

const char *fs_strerror(fs_status_t status);
void fs_set_errno(fs_status_t status);

#endif /* end of include guard: __FS_ERRNO_H__ */
