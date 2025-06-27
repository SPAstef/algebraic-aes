#include "aes_ni.h"

#define aes_ni_expand_key(k, rc)                                                                   \
    do                                                                                             \
    {                                                                                              \
        __m128i temp2 = _mm_aeskeygenassist_si128(k, rc);                                          \
        __m128i temp3;                                                                             \
                                                                                                   \
        temp2 = _mm_shuffle_epi32(temp2, 0xff);                                                    \
                                                                                                   \
        temp3 = _mm_slli_si128(k, 0x4);                                                            \
        k = _mm_xor_si128(k, temp3);                                                               \
                                                                                                   \
        temp3 = _mm_slli_si128(temp3, 0x4);                                                        \
        k = _mm_xor_si128(k, temp3);                                                               \
                                                                                                   \
        temp3 = _mm_slli_si128(temp3, 0x4);                                                        \
        k = _mm_xor_si128(k, temp3);                                                               \
                                                                                                   \
        k = _mm_xor_si128(k, temp2);                                                               \
    } while (0);

void aesni128_precompute_keys(__m128i *keys, const uint8_t *key)
{
    __m128i k = _mm_loadu_si128((const __m128i *)key);

    _mm_storeu_si128(keys + 0, k);

    aes_ni_expand_key(k, 0x01);
    _mm_storeu_si128(keys + 1, k);

    aes_ni_expand_key(k, 0x02);
    _mm_storeu_si128(keys + 2, k);

    aes_ni_expand_key(k, 0x04);
    _mm_storeu_si128(keys + 3, k);

    aes_ni_expand_key(k, 0x08);
    _mm_storeu_si128(keys + 4, k);

    aes_ni_expand_key(k, 0x10);
    _mm_storeu_si128(keys + 5, k);

    aes_ni_expand_key(k, 0x20);
    _mm_storeu_si128(keys + 6, k);

    aes_ni_expand_key(k, 0x40);
    _mm_storeu_si128(keys + 7, k);

    aes_ni_expand_key(k, 0x80);
    _mm_storeu_si128(keys + 8, k);
    aes_ni_expand_key(k, 0x1B);
    _mm_storeu_si128(keys + 9, k);

    aes_ni_expand_key(k, 0x36);
    _mm_storeu_si128(keys + 10, k);
}

void aesni128_encrypt_block(uint8_t *cip, const uint8_t *msg, const uint8_t *key)
{
    __m128i m = _mm_loadu_si128((__m128i *)msg);
    __m128i k = _mm_loadu_si128((__m128i *)key);

    m = _mm_xor_si128(m, k);

    aes_ni_expand_key(k, 0x01);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x02);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x04);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x08);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x10);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x20);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x40);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x80);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x1B);
    m = _mm_aesenc_si128(m, k);

    aes_ni_expand_key(k, 0x36);
    m = _mm_aesenclast_si128(m, k);

    _mm_storeu_si128((__m128i *)cip, m);
}

void aesni128_encrypt_block_with_keys(uint8_t *cip, const uint8_t *msg, const __m128i *keys)
{
    __m128i m = _mm_loadu_si128((__m128i *)msg);

    m = _mm_xor_si128(m, keys[0]);

    m = _mm_aesenc_si128(m, keys[1]);
    m = _mm_aesenc_si128(m, keys[2]);
    m = _mm_aesenc_si128(m, keys[3]);
    m = _mm_aesenc_si128(m, keys[4]);
    m = _mm_aesenc_si128(m, keys[5]);
    m = _mm_aesenc_si128(m, keys[6]);
    m = _mm_aesenc_si128(m, keys[7]);
    m = _mm_aesenc_si128(m, keys[8]);
    m = _mm_aesenc_si128(m, keys[9]);

    m = _mm_aesenclast_si128(m, keys[10]);

    _mm_storeu_si128((__m128i *)cip, m);
}
