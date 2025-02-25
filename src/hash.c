#include "hash.h"

uint32_t default_murmurhash3_32(const void *key, size_t len, uint32_t seed) {
    const uint8_t *data = (const uint8_t *) key;
    const uint32_t c1 = 0xCC9E2D51;
    const uint32_t c2 = 0x1B873593;
    uint32_t h1 = seed;
    uint32_t k1 = 0;

    // 分块处理每 4 字节
    const size_t n_blocks = len / 4;
    for (size_t i = 0; i < n_blocks; i++) {
        k1 = data[i * 4] | (data[i * 4 + 1] << 8) | (data[i * 4 + 2] << 16) | (data[i * 4 + 3] << 24);
        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xE6546B64;
    }

    // 处理剩余字节
    k1 = 0;
    const uint8_t *tail = data + n_blocks * 4;
    switch (len & 3) {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    // 混合最终结果
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85EBCA6B;
    h1 ^= h1 >> 13;
    h1 *= 0xC2B2AE35;
    h1 ^= h1 >> 16;

    return h1;
}

uint32_t murmurhash3_32(const void *key, size_t len) {
    return default_murmurhash3_32(key, len, 0);
}

uint64_t default_murmurhash3_64(const void *key, size_t len, uint64_t seed) {
    const uint8_t *data = (const uint8_t *) key;
    const uint64_t c1 = 0x87C37B91114253D5ULL;
    const uint64_t c2 = 0x4CF5AD432745937FULL;
    uint64_t h1 = seed;
    uint64_t h2 = seed;

    // 分块处理每 16 字节
    const size_t n_blocks = len / 16;
    for (size_t i = 0; i < n_blocks; i++) {
        uint64_t k1 = data[i * 16] | ((uint64_t) data[i * 16 + 1] << 8) |
                      ((uint64_t) data[i * 16 + 2] << 16) | ((uint64_t) data[i * 16 + 3] << 24) |
                      ((uint64_t) data[i * 16 + 4] << 32) | ((uint64_t) data[i * 16 + 5] << 40) |
                      ((uint64_t) data[i * 16 + 6] << 48) | ((uint64_t) data[i * 16 + 7] << 56);

        uint64_t k2 = data[i * 16 + 8] | ((uint64_t) data[i * 16 + 9] << 8) |
                      ((uint64_t) data[i * 16 + 10] << 16) | ((uint64_t) data[i * 16 + 11] << 24) |
                      ((uint64_t) data[i * 16 + 12] << 32) | ((uint64_t) data[i * 16 + 13] << 40) |
                      ((uint64_t) data[i * 16 + 14] << 48) | ((uint64_t) data[i * 16 + 15] << 56);

        // 混合 k1
        k1 *= c1;
        k1 = ROTL64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1 = ROTL64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52DCE729;

        // 混合 k2
        k2 *= c2;
        k2 = ROTL64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2 = ROTL64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495AB5;
    }

    // 处理剩余字节
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    const uint8_t *tail = data + n_blocks * 16;
    switch (len & 15) {
        case 15:
            k2 ^= (uint64_t) tail[14] << 48;
        case 14:
            k2 ^= (uint64_t) tail[13] << 40;
        case 13:
            k2 ^= (uint64_t) tail[12] << 32;
        case 12:
            k2 ^= (uint64_t) tail[11] << 24;
        case 11:
            k2 ^= (uint64_t) tail[10] << 16;
        case 10:
            k2 ^= (uint64_t) tail[9] << 8;
        case 9:
            k2 ^= (uint64_t) tail[8];
            k2 *= c2;
            k2 = ROTL64(k2, 33);
            k2 *= c1;
            h2 ^= k2;

        case 8:
            k1 ^= (uint64_t) tail[7] << 56;
        case 7:
            k1 ^= (uint64_t) tail[6] << 48;
        case 6:
            k1 ^= (uint64_t) tail[5] << 40;
        case 5:
            k1 ^= (uint64_t) tail[4] << 32;
        case 4:
            k1 ^= (uint64_t) tail[3] << 24;
        case 3:
            k1 ^= (uint64_t) tail[2] << 16;
        case 2:
            k1 ^= (uint64_t) tail[1] << 8;
        case 1:
            k1 ^= (uint64_t) tail[0];
            k1 *= c1;
            k1 = ROTL64(k1, 31);
            k1 *= c2;
            h1 ^= k1;
    }

    // 混合最终结果
    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 33;
    h1 *= 0xFF51AFD7ED558CCDULL;
    h1 ^= h1 >> 33;
    h1 *= 0xC4CEB9FE1A85EC53ULL;
    h1 ^= h1 >> 33;

    h2 ^= h2 >> 33;
    h2 *= 0xFF51AFD7ED558CCDULL;
    h2 ^= h2 >> 33;
    h2 *= 0xC4CEB9FE1A85EC53ULL;
    h2 ^= h2 >> 33;

    h1 += h2;
    return h1;
}

uint64_t murmurhash3_64(const void *key, size_t len) {
    return default_murmurhash3_64(key, len, 0);
}
