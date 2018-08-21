#ifndef __FILE_H__
#define __FILE_H__

#include "fs.h"

/* return 0 on success and negated error number on error */
int rfs_file_create(fs_t *fs, inode_t *parent, const char *name);

void rfs_file_remove(fs_t *fs);

void rfs_file_open(fs_t *fs, const char *path);

void rfs_file_rename(fs_t *fs);

/* returns bytes read on success and errno on error */
ssize_t rfs_file_read(fs_t *fs, inode_t *ino, off_t off, uint8_t *buf, size_t len);

/* returns bytes written on success and errno on error */
ssize_t rfs_file_write(fs_t *fs, inode_t *ino, off_t off, uint8_t *buf, size_t len);

#endif /* end of include guard: __FILE_H__ */
