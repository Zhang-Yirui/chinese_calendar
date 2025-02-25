#ifndef CALENDAR_HASH_H
#define CALENDAR_HASH_H

#include <stdint.h>
#include <stddef.h>

#define ROTL32(x, r) (((x) << (r)) | ((x) >> (32 - (r)))) // 左旋转宏
#define ROTL64(x, r) (((x) << (r)) | ((x) >> (64 - (r)))) // 左旋转宏

uint32_t default_murmurhash3_32(const void *key, size_t len, uint32_t seed);

uint64_t default_murmurhash3_64(const void *key, size_t len, uint64_t seed);

uint32_t murmurhash3_32(const void *key, size_t len);

uint64_t murmurhash3_64(const void *key, size_t len);

#endif //CALENDAR_HASH_H
