#include "bitmap.h"
#include <stdio.h>

/* these functions return 0 on success and -1 on error */

int bm_set_bit(bitmap_t *bm, uint32_t n)
{
	if (n / 32 > bm->len) {
		fprintf(stderr, "bit %u is over range!\n", n);
		return -1;
	}

	bm->bits[n / 32] |= 1 << (n % 32);
	return 0;
}

int bm_set_range(bitmap_t *bm, uint32_t n, uint32_t k)
{
	if (n / 32 > bm->len || k / 32 > bm->len) {
		fprintf(stderr, "argument is over range!");
		return -1;
	}

	while (n <= k) {
		bm->bits[n / 32] |= 1 << (n % 32);
		n++;
	}
	return 0;
}

int bm_unset_bit(bitmap_t *bm, uint32_t n)
{
	if (n / 32 > bm->len) {
		fprintf(stderr, "bit %u is over range!\n", n);
		return -1;
	}

	bm->bits[n / 32] &= ~(1 << (n % 32));
	return 0;
}

int bm_unset_range(bitmap_t *bm, uint32_t n, uint32_t k)
{
	if (n / 32 > bm->len || k / 32 > bm->len) {
		fprintf(stderr, "argument is over range!");
		return -1;
	}

	while (n <= k) {
		bm->bits[n / 32] &= ~(1 << (n % 32));
		n++;
	}
	return 0;
}

/* return -1 on error and 0/1 on success */
int test_bit(bitmap_t *bm, uint32_t n)
{
	if (n / 32 > bm->len) {
		fprintf(stderr, "bit %u is over range!\n", n);
		return -1;
	}
	return (bm->bits[n / 32] & (1 << (n % 32)));
}
