#include <stdlib.h>
#include <stdint.h>

#include "driver.h"
#include "fs.h"
#include "debug.h"

static size_t get_device_size(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	size_t size = ftell(fp); /* FIXME: muh sign */
	fseek(fp, 0L, SEEK_SET);

	return size;
}

hdd_handle_t *open_device(const char *device, const char *mode)
{
	hdd_handle_t *fd = malloc(sizeof(hdd_handle_t));
	fd->dev_bsize = DEV_BLOCK_SIZE;
	fd->fp = fopen(device, mode);
	fd->size = get_device_size(fd->fp);

	debug(LOG_INFO, "device '%s' opened!", device);
	debug(LOG_INFO, "%zu bytes %zu kilobytes %zu megabytes", fd->size, fd->size / 1000, fd->size / 1000000);
	return fd;
}

int close_device(hdd_handle_t *fd)
{
	fclose(fd->fp);
	return 0;
}

size_t write_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size)
{
	debug(LOG_INFO, "%u", size);
	fseek(fd->fp, DEV_BLOCK_SIZE * block + offset, SEEK_SET);
	return fwrite(buf, 1, size, fd->fp);
}

size_t read_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size)
{
	fseek(fd->fp, DEV_BLOCK_SIZE * block + offset, SEEK_SET);
	return fread(buf, 1, size, fd->fp);
}
