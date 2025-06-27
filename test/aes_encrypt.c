#include "aes.h"
#include "string_utils.h"
#include "intrinsics.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

#define aes_ni_expand_key_partial(k, rc)                                                           \
    do                                                                                             \
    {                                                                                              \
        k = _mm_aeskeygenassist_si128(k, rc);                                                      \
    } while (0);

void swap_bytes_16(uint8_t *buf)
{
    for (int i = 0; i < 8; ++i)
    {
        uint8_t tmp = buf[i];
        buf[i] = buf[15 - i];
        buf[15 - i] = tmp;
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <hex_key> <hex_msg>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint8_t key[AES_BLOCK_SIZE] = {0}; // 2b7e151628aed2a6abf7158809cf4f3c (test vector)
    uint8_t msg[AES_BLOCK_SIZE] = {0}; // 6bc1bee22e409f96e93d7e117393172a (test vector)
    uint8_t cip[AES_BLOCK_SIZE] = {0}; // 3ad77bb40d7a3660a89ecaf32466ef97 (test vector)

    hexload(key, AES_BLOCK_SIZE, argv[1]);
    hexload(msg, AES_BLOCK_SIZE, argv[2]);


    printf("Key: ");
    hexprint(key, AES_BLOCK_SIZE);
    putchar('\n');

    printf("Msg: ");
    hexprint(msg, AES_BLOCK_SIZE);
    putchar('\n');

    aes128_encrypt_block(cip, key, msg);

    printf("Cip: ");
    hexprint(cip, AES_BLOCK_SIZE);
    putchar('\n');

    // Swap bytes of key (reverse order)

    printf("KeyStream:\n");
    printf("    Reference                           keygenassist (AES-NI)               full "
           "expansion (AES-NI)\n");

    fq_mat_t rk = aes128_frombytes(key);
    __m128i ni_rk = _mm_loadu_si128((const void *)key);

    // Notice how we need to flip the byte order to match Intel's whitepaper (we'll need to flip it again to print it)
    swap_bytes_16(key);
    __m128i ni_rk_partial = _mm_loadu_si128((const void *)key);

    // Need a macro since _mm_aeskeygenassist_si128 wants a constant
#define SCHED_ROUND(i, rc)                                                                         \
    do                                                                                             \
    {                                                                                              \
        uint8_t buf[16] = {0};                                                                     \
        uint8_t buf_ni[16] = {0};                                                                  \
        uint8_t buf_ni_partial[16] = {0};                                                          \
                                                                                                   \
        aes128_tobytes(buf, rk);                                                                   \
        _mm_store_si128((void *)buf_ni, ni_rk);                                                    \
        _mm_store_si128((void *)buf_ni_partial, ni_rk_partial);                                    \
                                                                                                   \
        swap_bytes_16(buf_ni_partial);                                                             \
        printf("%2d: ", i);                                                                        \
        hexprint(buf, AES_BLOCK_SIZE);                                                             \
        printf("    ");                                                                            \
        hexprint(buf_ni_partial, AES_BLOCK_SIZE);                                                  \
        printf("    ");                                                                            \
        hexprint(buf_ni, AES_BLOCK_SIZE);                                                          \
        putchar('\n');                                                                             \
                                                                                                   \
        rk = aes128_schedule(rk, i);                                                               \
        aes_ni_expand_key(ni_rk, rc);                                                              \
        aes_ni_expand_key_partial(ni_rk_partial, rc);                                              \
    } while (0)

    SCHED_ROUND(0, 0x01);
    SCHED_ROUND(1, 0x02);
    SCHED_ROUND(2, 0x04);
    SCHED_ROUND(3, 0x08);
    SCHED_ROUND(4, 0x10);
    SCHED_ROUND(5, 0x20);
    SCHED_ROUND(6, 0x40);
    SCHED_ROUND(7, 0x80);
    SCHED_ROUND(8, 0x1B);
    SCHED_ROUND(9, 0x36);
    SCHED_ROUND(10, 0x00);

    return 0;
}