#include "fs.h"

int main(void)
{
	puts("\n\n\n---------------------------------------");
	const char *device = "hdd_file";
	superblock_t *sb;

	if ((sb = rfs_mkfs(device)) == NULL) {
		fprintf(stderr, "[debug][error] mkfs failed!\n");
		return -1;
	}

	puts("niilo tventtituu hiör");
}
