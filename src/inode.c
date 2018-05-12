#include <string.h>
#include <stdlib.h>

#include "inode.h"

inode_t *mkinode(const char *name, uint32_t inode_num, uint16_t uid, 
		         uint16_t gid, uint16_t mode, uint16_t flags, 
				 time_t create_time, time_t modified_time)
{
	inode_t *inode = malloc(sizeof(inode_t));
	size_t maxlen  = strlen(name);

	// TODO rethink this, one null byte must be written!
	maxlen = (maxlen < RFS_NAME_MAX_LEN) ? maxlen : RFS_NAME_MAX_LEN - 1;
	strncpy(inode->name, name, maxlen);

	inode->uid           = uid;
	inode->gid           = gid;
	inode->mode          = mode;
	inode->flags         = flags;
	inode->inode_num     = inode_num;
	inode->create_time   = create_time;
	inode->modified_time = modified_time;


    /* sb->root.inode_num = 0; */
    /* sb->root.uid  = sb->root.gid   = 0; // FIXME chmod style numbering?? */
    /* sb->root.mode = sb->root.flags = 0; */
    /* sb->root.create_time = sb->root.modified_time = time(NULL); */
}
