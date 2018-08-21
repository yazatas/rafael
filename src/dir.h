#ifndef __DIR_H__
#define __DIR_H__

#include <stdint.h>

#include "inode.h" 

#define RFS_NAME_MAX_LEN 32

typedef struct dentry {
    uint32_t d_flags;
    uint32_t d_inode;
    char d_name[RFS_NAME_MAX_LEN];
} dentry_t;

/* TODO: dir or dentry???*/

dentry_t *rfs_dir_lookup(fs_t *fs, inode_t *haystack, const char *needle);

int rfs_dir_insert(fs_t *fs, inode_t *parent, inode_t *child);

#endif /* end of include guard: __DIR_H__ */
