#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "dir.h"
#include "file.h"
#include "inode.h"

static bool is_name_valid(const char *name)
{
    if (strcmp(name, ".") == 0)
        return false;
    if (strcmp(name, "..") == 0)
        return false;
    if (strchr(name, '/') != NULL)
        return false;
    if (strlen(name) >= RFS_NAME_MAX_LEN)
        return false;

    return true;
}

int rfs_file_create(fs_t *fs, inode_t *parent, const char *name)
{
    if (!is_name_valid(name)) {
        LOG_ERROR("invalid name '%s'", name);
        return -EINVAL;
    }

    if (rfs_dir_lookup(fs, parent, name) != NULL) {
        LOG_ERROR("file '%s' already exists");
        return -EEXIST;
    }

    int ret;
    inode_t *ino = rfs_inode_alloc(fs);
    ino->flags  |= S_IFREG;

    if ((ret = rfs_dir_insert(fs, parent, ino)) < 0) {
        LOG_ERROR("failed to insert inode to parent directory!");
        return ret;
    }

    if ((ret = rfs_inode_write(fs, ino)) < 0) {
        LOG_ERROR("failed to write inode to disk");
        return ret;
    }

    return 0;
}

ssize_t rfs_file_read(fs_t *fs, inode_t *ino, off_t off, uint8_t *buf, size_t len)
{
}

ssize_t rfs_file_write(fs_t *fs, inode_t *ino, off_t off, uint8_t *buf, size_t len)
{
}

void rfs_file_remove(fs_t *fs)
{
}

void rfs_file_open(fs_t *fs, const char *path)
{
}

void rfs_file_close(fs_t *fs)
{
}

void rfs_file_rename(fs_t *fs)
{
}
