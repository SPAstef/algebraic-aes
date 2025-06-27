#include "rand.h"

#include <stdlib.h>

// This is neither random nor uniform!!! Use only for testing purposes
uint64_t prand64(uint64_t lo, uint64_t hi)
{
    uint64_t r = 0;

    for (int i = 0; i < 8; ++i)
        r = (r << 8) | (rand() & 0xFF);

    return lo + (r % (hi - lo + 1));
}
