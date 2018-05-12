#ifndef __INODE_H__
#define __INODE_H__

#include <stdint.h>
#include <time.h>

#define MAX_INODE_COUNT    4096 /* FIXME: why is this defined   */
#define RFS_NAME_MAX_LEN 128
#define RFS_NUM_INODES   256 /* initial size of inode bitmap */
#define RFS_NUM_BLOCKS   32

typedef struct block {
	uint32_t alloc_group;
	uint16_t start;
	uint16_t len;
} block_t;

/* FIXME: i think this data structure is not correct!??!?!? */
typedef struct data {
	size_t size;
	block_t direct[RFS_NUM_BLOCKS];
	block_t indirect;
	block_t double_indirect;
} data_t;

typedef struct inode {
	char name[RFS_NAME_MAX_LEN];
	uint32_t inode_num;

	/* ownership and access information */
	uint32_t uid;
	uint32_t gid;
	uint32_t mode;
	uint32_t flags;

	time_t create_time;
	time_t modified_time;

	data_t inode_data;
} inode_t;

/* enum mode_bits_t { */
/* }; */

inode_t *mkinode(const char *name,   uint32_t inode_num, uint16_t uid, 
		         uint16_t gid,       uint16_t mode,      uint16_t flags, 
				 time_t create_time, time_t modified_time);

#endif /* end of include guard: __INODE_H__ */
