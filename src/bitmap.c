#include "bitmap.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#define BM_GET_MULTIPLE_OF_32(n) (n % 32) ? ((n / 32) + 1) : (n / 32)

bitmap_t *bm_alloc_bitmap(size_t nmemb)
{
    bitmap_t *bm;
    size_t n;

    if ((bm = malloc(sizeof(bitmap_t))) == NULL)
        return NULL;

    n = BM_GET_MULTIPLE_OF_32(nmemb);
    bm->len = n * 32;

    if ((bm->bits = calloc(n, sizeof(uint32_t))) == NULL)
        return NULL;

    return bm;
}

void bm_dealloc_bitmap(bitmap_t *bm)
{
    free(bm->bits);
    free(bm);
}

/* set nth bit */
int bm_set_bit(bitmap_t *bm, uint32_t n)
{
	if (n / 32 > bm->len) {
        LOG_WARN( "bit %u is over range!", n);
		return BM_RANGE_ERROR;
	}

	bm->bits[n / 32] |= 1 << (n % 32);
	return 0;
}

int bm_set_range(bitmap_t *bm, uint32_t n, uint32_t k)
{
    if (n >= bm->len || k >= bm->len) {
        LOG_WARN("argument n(%u) or k(%u) is over range(%u)!", n, k, bm->len);
        return BM_RANGE_ERROR;
    }

    while (n <= k) {
        bm->bits[n / 32] |= 1 << (n % 32);
        n++;
    }
    return 0;
}

/* unset nth bit */
int bm_unset_bit(bitmap_t *bm, uint32_t n)
{
    if (n >= bm->len) {
        LOG_WARN("bit %u is over range!", n);
        return BM_RANGE_ERROR;
    }

    bm->bits[n / 32] &= ~(1 << (n % 32));
    return 0;
}

int bm_unset_range(bitmap_t *bm, uint32_t n, uint32_t k)
{
    if (n >= bm->len || k >= bm->len) {
        LOG_WARN("argument n(%u) or k(%u) is over range(%u)!", n, k, bm->len);
        return BM_RANGE_ERROR;
    }

    while (n <= k) {
        bm->bits[n / 32] &= ~(1 << (n % 32));
        n++;
    }
    return 0;
}

/* return BM_RANGE_ERROR on error and 0/1 on success */
int bm_test_bit(bitmap_t *bm, uint32_t n)
{
    if (n >= bm->len) {
        LOG_WARN("bit %lu is over range!", n);
        return BM_RANGE_ERROR;
    }
    return (bm->bits[n / 32] & (1 << (n % 32))) >> (n % 32);
}

static int bm_find_first(bitmap_t *bm, uint32_t n, uint32_t k, uint8_t bit_status)
{
    if (n >= bm->len || k >= bm->len) {
        LOG_WARN("argument n(%lu) or k(%lu) is over range(%u)!", n, k, bm->len);
        return BM_RANGE_ERROR;
    }

    while (n <= k) {
        if ((bm->bits[n / 32] & (1 << (n % 32))) == bit_status)
            return n;
        n++;
    }

    return BM_NOT_FOUND_ERROR;
}

int bm_find_first_unset(bitmap_t *bm, uint32_t n, uint32_t k)
{
    return bm_find_first(bm, n, k, 0);
}

int bm_find_first_set(bitmap_t *bm, uint32_t n, uint32_t k)
{
    return bm_find_first(bm, n, k, 1);
}
