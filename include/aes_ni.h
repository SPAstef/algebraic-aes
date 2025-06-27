#pragma once

#include "intrinsics.h"

#include <stdint.h>

enum
{
    AES_NI_BLOCK_SIZE = 16U
};
enum
{
    AES_NI_ROUNDS = 10U
};

void aesni128_precompute_keys(__m128i *keys, const uint8_t *key);

void aesni128_encrypt_block(uint8_t *cip, const uint8_t *key, const uint8_t *msg);

void aesni128_encrypt_block_with_keys(uint8_t *cip, const uint8_t *msg, const __m128i *keys);
