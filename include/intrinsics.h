#pragma once

#if defined(_WIN32)
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

#if __STDC_VERSION__ >= 202000L && !defined(_MSC_VER)
    #define NATIVE_UINT128 1
typedef unsigned _BitInt(128) uint128_t;
#elif defined(__SIZEOF_INT128__)
    #define NATIVE_UINT128 1
typedef unsigned __int128 uint128_t;
#endif
