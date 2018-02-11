#include <stdlib.h>
#include <stdint.h>

#include "driver.h"
#include "fs.h"

static size_t get_device_size(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	size_t size = ftell(fp); /* FIXME: this is bad, muh sign */
	fseek(fp, 0L, SEEK_SET);

	return size;
}

hdd_handle_t *open_device(const char *device, const char *mode)
{
	hdd_handle_t *fd = malloc(sizeof(hdd_handle_t));
	fd->dev_bsize = DEV_BLOCK_SIZE;
	fd->fp = fopen(device, mode); /* this is kind of ironic ":D" */
	fd->size = get_device_size(fd->fp);

	fprintf(stderr, "[open_device] device '%s' of size %zu opened!\n", device, fd->size);
	return fd;
}

int close_device(hdd_handle_t *fd)
{
	fclose(fd->fp);
	return 0;
}

int write_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size)
{
	/* TODO: what am i supposed to do here */
}

int read_blocks(hdd_handle_t *fd, uint32_t block, uint32_t offset, void *buf, size_t size)
{

}
