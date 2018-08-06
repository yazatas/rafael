#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#include "disk.h"

#define DISK_BLOCK_SIZE 512

static int fd = -1;

void disk_init(const char *device)
{
    fd = open(device, O_RDWR | O_CREAT);

    if (fd == -1) {
        LOG_EMERG("failed to open disk");
    }
}

void disk_close(void)
{
    close(fd);
}

uint32_t disk_size(void)
{
    return 65536000;
}

uint32_t disk_block_size(void)
{
    return DISK_BLOCK_SIZE;
}

void disk_write(uint32_t blocknum, uint8_t *buf)
{
    if (fd == -1) {
        LOG_EMERG("disk is not open!");
        return;
    }

    size_t max_block = disk_size() / DISK_BLOCK_SIZE;

    if (blocknum >= max_block) {
        LOG_EMERG("trying to write block %u but disk has only %u blocks!",
                blocknum, max_block);
        return;
    }

    lseek(fd, blocknum * DISK_BLOCK_SIZE, SEEK_SET);
    write(fd, buf, DISK_BLOCK_SIZE);
}

void disk_read(uint32_t blocknum, uint8_t *buf)
{
    if (fd == -1) {
        LOG_EMERG("disk is not open!");
        return;
    }

    size_t max_block = disk_size() / DISK_BLOCK_SIZE;

    if (blocknum >= max_block) {
        LOG_EMERG("trying to write block %u but disk has only %u blocks!",
                blocknum, max_block);
        return;
    }

    lseek(fd, blocknum * DISK_BLOCK_SIZE, SEEK_SET);
    read(fd, buf, DISK_BLOCK_SIZE);
}
