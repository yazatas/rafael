#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "driver.h"
#include "fs.h"
#include "debug.h"

size_t dev_get_block_size(void)
{
    return 512;
}

size_t dev_get_num_blocks(int fd)
{
    return 65536000 / dev_get_block_size();
}

size_t write_blocks(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
	/* LOG_INFO("%u", size); */

    lseek(fs->fd, offset, SEEK_SET);
    return write(fs->fd, buf, size);
}

size_t read_blocks(fs_t *fs, uint32_t offset, void *buf, size_t size)
{
    LOG_INFO("reading %u bytes from disk", size);

    lseek(fs->fd, offset, SEEK_SET);
    return read(fs->fd, buf, size);
}
