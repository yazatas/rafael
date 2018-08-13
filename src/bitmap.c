#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "common.h"
#include "debug.h"
#include "fs.h"
#include "io.h"

#define BM_GET_MULTIPLE_OF_32(n) (n % 32) ? ((n / 32) + 1) : (n / 32)

bitmap_t *bm_alloc_bitmap(size_t nmemb)
{
    bitmap_t *bm;
    size_t num_bits;

    if ((bm = malloc(sizeof(bitmap_t))) == NULL) {
        return NULL;
    }

    if (nmemb == 0) {
        LOG_WARN("empty bitmap allocated");

        bm->bits = NULL;
        bm->len  = 0;
        return bm;
    }

    num_bits = BM_GET_MULTIPLE_OF_32(nmemb); /* round it up to nearest multiple of 32 */
    bm->len = num_bits * 32;

    if ((bm->bits = calloc(num_bits, sizeof(uint32_t))) == NULL) {
        return NULL;
    }

    LOG_DEBUG("allocated %u bytes for bitmap", num_bits * sizeof(uint32_t));

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

static int bm_find_first_range(bitmap_t *bm, uint32_t n, uint32_t k, size_t len, uint8_t bit_status)
{
    if (n >= bm->len || k >= bm->len) {
        LOG_WARN("argument n(%lu) or k(%lu) is over range(%u)!", n, k, bm->len);
        return BM_RANGE_ERROR;
    }

    int start = BM_NOT_FOUND_ERROR;
    size_t cur_len = 0;

    while (n <= k) {
        if ((bm->bits[n / 32] & (1 << (n % 32))) == bit_status) {

            LOG_INFO("bit at index %u matches %u", n, bit_status); 

            if (start == BM_NOT_FOUND_ERROR)
                start = n;

            if (++cur_len == len)
                return start;

        } else {
            start   = BM_NOT_FOUND_ERROR;
            cur_len = 0;
        }
        n++;
    }

    return BM_NOT_FOUND_ERROR;
}

int bm_find_first_set_range(bitmap_t *bm, uint32_t n, uint32_t k, size_t len)
{
    return bm_find_first_range(bm, n, k, len, 1);
}

int bm_find_first_unset_range(bitmap_t *bm, uint32_t n, uint32_t k, size_t len)
{
    return bm_find_first_range(bm, n, k, len, 0);
}

size_t bm_write_to_disk(fs_t *fs, off_t offset, bitmap_t *bm)
{
    size_t size  = sizeof(size_t) + (bm->len / 32) * sizeof(uint32_t);
    uint8_t *buf = malloc(size);

    memcpy(buf, &bm->len, sizeof(size_t));
    memcpy(buf + sizeof(size_t), bm->bits, size - sizeof(size_t));

    LOG_DEBUG("writing %u bytes to disk at offset %u", size, offset);

    size_t nwritten = rfs_write_buf(fs, offset, buf, size);
    free(buf);

    return nwritten;
}

/* read initially 4KB block of data from disk and hope that 
 * that's all we need. If it is, just copy the length and 
 * bits from temporary buffer to the bitmap.
 *
 * If, however, the bitmap is larger than 4KB (very likely with
 * large disks) just perform another disk read but this time use 
 * the bm->bits as the destination buffer as we already know 
 * the amount of bytes needed */
size_t bm_read_from_disk(fs_t *fs, off_t offset, bitmap_t *bm)
{
    if (bm->bits != NULL) {
        free(bm->bits);
        bm->len = 0;
    }

    size_t ret = RFS_BLOCK_SIZE, size;
    uint8_t *buf = malloc(RFS_BLOCK_SIZE);

    if (rfs_read_blocks(fs, BYTE_TO_BLOCK(offset), buf, 1) == 0) {
        LOG_EMERG("read from disk failed!");
        ret = 0;
        goto end;
    }

    memcpy(&bm->len, buf, sizeof(size_t));
    size = (bm->len / 32) * sizeof(uint32_t);

    if (size + sizeof(size_t) > RFS_BLOCK_SIZE) {
        LOG_WARN("bitmap takes too much space, copying only part of it");
        ret = 0;
        goto end;
    }

    bm->bits = malloc((bm->len / 32) * sizeof(uint32_t));
    memcpy(bm->bits, buf + sizeof(size_t), size);

    LOG_INFO("bitmap can hold up to %u elements", bm->len);

end:
    free(buf);
    return ret;
}
