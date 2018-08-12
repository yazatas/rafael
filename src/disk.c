#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "debug.h"
#include "disk.h"

#define DISK_BLOCK_SIZE 512

int disk_init(const char *device)
{
    int fd = open(device, O_RDWR | O_CREAT);

    if (fd == -1) {
        LOG_EMERG("failed to open disk");
    }

    return fd;
}

void disk_close(int fd)
{
    close(fd);
}

uint32_t disk_get_size(int fd)
{
    (void)fd;

    return 65536000;
}

uint32_t disk_get_block_size(int fd)
{
    (void)fd;

    return DISK_BLOCK_SIZE;
}

void disk_write(int fd, uint32_t sector, uint8_t *buf)
{
    if (fd == -1) {
        LOG_EMERG("disk is not open!");
        return;
    }

    size_t max_block = disk_get_size(fd) / DISK_BLOCK_SIZE;

    if (sector >= max_block) {
        LOG_EMERG("trying to write block %u but disk has only %u blocks!",
                sector, max_block);
        return;
    }

    lseek(fd, sector * DISK_BLOCK_SIZE, SEEK_SET);
    write(fd, buf, DISK_BLOCK_SIZE);
}

void disk_read(int fd, uint32_t sector, uint8_t *buf)
{
    if (fd == -1) {
        LOG_EMERG("disk is not open!");
        return;
    }

    size_t max_block = disk_get_size(fd) / DISK_BLOCK_SIZE;

    if (sector >= max_block) {
        LOG_EMERG("trying to write block %u but disk has only %u blocks!",
                sector, max_block);
        return;
    }

    lseek(fd, sector * DISK_BLOCK_SIZE, SEEK_SET);
    read(fd, buf, DISK_BLOCK_SIZE);
}
