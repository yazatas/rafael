#include <string.h>
#include <stdio.h>

#include "common.h"
#include "fs.h"
#include "debug.h"
#include "inode.h"
#include "io.h"

void list_inodes(fs_t *fs);

void run_cmd(void *ptr)
{
    puts("\n");
    char command[10];
    fs_t *fs = ptr;
    superblock_t *sb = fs->sb;
    inode_t *ino;

    while (1) {
        printf("> ");
        scanf("%10s", command);

        if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            break;
        } else if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
            printf("\tquit - quit the prompt\n"
                   "\thelp - display this help\n"
                   "\tsb   - dump superblock contents\n"
                   "\talloc_ino - allocate inode\n"
                   "\tlist_ino - list inodes that are on the disk\n"
                   );
        } else if (strcmp(command, "sb") == 0) {
            LOG_INFO("superblock:\n"
                     "\tmagic1:          0x%08x\n"
                     "\tmagic2:          0x%08x\n"
                     "\tnum_blocks:      %10u\n"
                     "\tused_blocks:     %10u\n"
                     "\tdev_block_size:  %10u\n"
                     "\tblock_bm_start:  0x%08x\n"
                     "\tblock_map_start: 0x%08x\n"
                     "\tnum_inodes:      %10u\n"
                     "\tused_inodes:     %10u\n"
                     "\tino_bm_start:    0x%08x\n"
                     "\tino_map_start:   0x%08x\n", fs->sb->magic1, fs->sb->magic2,
                     fs->sb->num_blocks, fs->sb->used_blocks, fs->sb->dev_block_size,
                     fs->sb->block_bm_start, fs->sb->block_map_start, fs->sb->num_inodes,
                     fs->sb->used_inodes, fs->sb->ino_bm_start, fs->sb->ino_map_start);
        } else if (strcmp(command, "alloc_ino") == 0) {
            if ((ino = rfs_alloc_inode(fs)) != NULL) {
                LOG_INFO("allocated inode info:\n"
                     "\ti_ino: %u\n"
                     "\tino->flags: %u\n"
                     "\tino->mode: %u\n"
                     "\tino->i_gid: %u\n"
                     "\tino->i_size: %u\n"
                     "\tino->i_uid: %u\n", ino->i_ino, ino->flags, ino->mode,
                     ino->i_gid, ino->i_size, ino->i_uid);
            }
        }
        else if (strcmp(command, "list_ino") == 0) {
            list_inodes(fs);
        }
        else {
            fprintf(stderr, "unknown command '%s'\n", command);
        }
    }
}

void list_inodes(fs_t *fs)
{
    int start_index = 1, index;
    inode_t ino;

    for (int i = 0; i < 10; ++i) {
        index = bm_find_first_set(fs->bm_inode, start_index, fs->bm_inode->len-1);

        if (index == BM_NOT_FOUND_ERROR) {
            LOG_WARN("no more inodes found, %d %d %d", i, start_index, index);
            break;
        }

        size_t ino_offset = fs->sb->ino_map_start + index * sizeof(inode_t);
        LOG_INFO("ino offset %u", ino_offset);

        if (rfs_read_buf(fs, ino_offset, &ino, sizeof(inode_t)) == 0)
            LOG_WARN("failed to read inode from index %d", index);

        LOG_INFO("inode at %d:\n"
                 "\ti_ino: %u\n"
                 "\tino.flags: %u\n"
                 "\tino.mode: %u\n"
                 "\tino.i_gid: %u\n"
                 "\tino.i_size: %u\n"
                 "\tino.i_uid: %u\n\n", index, ino.i_ino, 
                 ino.flags, ino.mode, ino.i_gid, ino.i_size, ino.i_uid);

        memset(&ino, 0, sizeof(inode_t));

        start_index = index + 1;
    }
}
