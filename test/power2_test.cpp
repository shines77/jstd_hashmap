#if 0

#ifndef __SSE4_2__
#define __SSE4_2__
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <string.h>

#include <iostream>
#include <iomanip>  // std::setw(), std::setfill(), std::setprecision().
#include <sstream>
#include <string>
#include <utility>

#ifdef __SSE4_2__

// Support SSE 4.2: _mm_crc32_u32(), _mm_crc32_u64().
#define SUPPORT_SSE42_CRC32C    1

// Support Intel SMID SHA module: sha1 & sha256, it's higher than SSE 4.2 .
// _mm_sha1msg1_epu32(), _mm_sha1msg2_epu32() and so on.
#define SUPPORT_SMID_SHA        0

#endif // __SSE4_2__

#include <jstd/basic/stddef.h>
#include <jstd/basic/inttypes.h>
#include <jstd/type_traits.h>
#include <jstd/support/Power2.h>
#include <jstd/system/Console.h>
#include <jstd/test/Test.h>

//
// About marco # and ##
// See: https://www.cnblogs.com/wb-DarkHorse/p/3588787.html
//

#define ROUND_TO_POW2_TEST(className, funcName, N) \
    printf("Test: [%s<%" PRIuPTR ">] == %" PRIuPTR ", %" PRIuPTR ", ", \
           #className, size_t(N), \
           jstd::compile_time::className<N>::value, \
           jstd::funcName(size_t(N))); \
    JTEST_EXPECT_EQ(jstd::compile_time::className<N>::value, \
                    jstd::funcName(size_t(N)))

void round_down_pow2_test()
{
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 0);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 1);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 2);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 3);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 4);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 5);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 6);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 7);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 8);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 9);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 10);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 11);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 12);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 13);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 14);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 15);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 16);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 17);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 18);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 19);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 20);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 21);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 22);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 23);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 24);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 25);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 26);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 27);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 28);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 29);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 30);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 31);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 32);
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_down_pow2, pow2::round_down, 18446744073709551615ull);
#endif
    printf("\n");
}

void round_down_power2_test()
{
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 0);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 1);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 2);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 3);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 4);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 5);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 6);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 7);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 8);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 9);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 10);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 11);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 12);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 13);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 14);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 15);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 16);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 17);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 18);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 19);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 20);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 21);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 22);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 23);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 24);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 25);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 26);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 27);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 28);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 29);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 30);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 31);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 32);
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_down_power2, pow2::round_down, 18446744073709551615ull);
#endif
    printf("\n");
}

void round_to_pow2_test()
{
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 0);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 1);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 2);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 3);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 4);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 5);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 6);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 7);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 8);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 9);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 10);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 11);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 12);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 13);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 14);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 15);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 16);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 17);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 18);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 19);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 20);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 21);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 22);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 23);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 24);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 25);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 26);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 27);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 28);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 29);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 30);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 31);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 32);
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_to_pow2, pow2::round_to, 18446744073709551615ull);
#endif
    printf("\n");
}

void round_to_power2_test()
{
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 0);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 1);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 2);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 3);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 4);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 5);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 6);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 7);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 8);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 9);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 10);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 11);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 12);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 13);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 14);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 15);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 16);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 17);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 18);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 19);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 20);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 21);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 22);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 23);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 24);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 25);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 26);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 27);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 28);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 29);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 30);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 31);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 32);
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_to_power2, pow2::round_to, 18446744073709551615ull);
#endif
    printf("\n");
}

void round_up_pow2_test()
{
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 0);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 1);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 2);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 3);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 4);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 5);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 6);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 7);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 8);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 9);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 10);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 11);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 12);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 13);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 14);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 15);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 16);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 17);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 18);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 19);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 20);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 21);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 22);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 23);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 24);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 25);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 26);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 27);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 28);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 29);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 30);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 31);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 32);
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_up_pow2, pow2::round_up, 18446744073709551615ull);
#endif
    printf("\n");
}

void round_up_power2_test()
{
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 0);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 1);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 2);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 3);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 4);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 5);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 6);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 7);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 8);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 9);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 10);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 11);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 12);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 13);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 14);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 15);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 16);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 17);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 18);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 19);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 20);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 21);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 22);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 23);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 24);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 25);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 26);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 27);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 28);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 29);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 30);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 31);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 32);
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(round_up_power2, pow2::round_up, 18446744073709551615ull);
#endif
    printf("\n");
}

void next_pow2_test()
{
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 0);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 1);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 2);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 3);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 4);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 5);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 6);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 7);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 8);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 9);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 10);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 11);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 12);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 13);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 14);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 15);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 16);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 17);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 18);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 19);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 20);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 21);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 22);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 23);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 24);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 25);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 26);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 27);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 28);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 29);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 30);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 31);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 32);
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(next_pow2, pow2::next_pow2, 18446744073709551615ull);
#endif
    printf("\n");
}

void next_power2_test()
{
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 0);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 1);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 2);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 3);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 4);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 5);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 6);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 7);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 8);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 9);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 10);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 11);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 12);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 13);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 14);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 15);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 16);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 17);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 18);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 19);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 20);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 21);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 22);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 23);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 24);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 25);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 26);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 27);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 28);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 29);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 30);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 31);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 32);
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 33);
    // 0x7FFFFFFF
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 2147483647);
    // 0x80000000
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 2147483648);
    // 0x80000001
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 2147483649);
#if (JSTD_WORD_LEN == 64)
    // 0x7FFFFFFFFFFFFFFEFULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 9223372036854775806ull);
    // 0x7FFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 9223372036854775807ull);
    // 0x8000000000000000ULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 9223372036854775808ull);
    // 0x8000000000000001ULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 9223372036854775809ull);
    // 0xFFFFFFFFFFFFFFFEULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 18446744073709551614ull);
    // 0xFFFFFFFFFFFFFFFFULL
    ROUND_TO_POW2_TEST(next_power2, pow2::next_pow2, 18446744073709551615ull);
#endif
    printf("\n");
}

void integral_traits_test()
{
    printf("jstd::integral_traits<size_t>::bits       = %" PRIuSIZE "\n", jstd::integral_traits<size_t>::bits);
    printf("jstd::integral_traits<size_t>::max_shift  = %" PRIuSIZE "\n", jstd::integral_traits<size_t>::max_shift);
    printf("jstd::integral_traits<size_t>::max_power2 = %" PRIuSIZE "\n", jstd::integral_traits<size_t>::max_power2);
    printf("jstd::integral_traits<size_t>::max_num    = %" PRIuSIZE "\n", jstd::integral_traits<size_t>::max_num);
    printf("\n");
}

int main(int argc, char * argv[])
{
    integral_traits_test();

    round_down_pow2_test();
    round_down_power2_test();

    round_to_pow2_test();
    round_to_power2_test();

    round_up_pow2_test();
    round_up_power2_test();

    next_pow2_test();
    next_power2_test();

    jstd::Console::ReadKey();
    return 0;
}

#else

#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <string>
#include <cstring>
#include <memory>
#include <vector>

#define USE_CACHE_OVERWRITE     1

void cpu_warm_up(int delayMillsecs)
{
    using namespace std::chrono;
    double delayTimeLimit = (double)delayMillsecs / 1.0;
    volatile intptr_t  sum = 0;

    printf("------------------------------------------\n\n");
    printf("CPU warm-up begin ...\n");

    high_resolution_clock::time_point startTime, endTime;
    duration<double, std::ratio<1, 1000>> elapsedTime;
    startTime = high_resolution_clock::now();
    do {
        for (intptr_t i = 0; i < 500; ++i) {
            sum += i;
            for (intptr_t j = 5000; j >= 0; --j) {
                sum -= j;
            }
        }
        endTime = high_resolution_clock::now();
        elapsedTime = endTime - startTime;
    } while (elapsedTime.count() < delayTimeLimit);

    printf("sum = %d, time: %0.3f ms\n", (int)sum, elapsedTime.count());
    printf("CPU warm-up end   ... \n\n");
    printf("------------------------------------------\n\n");
}

//
// See: https://www.zhihu.com/question/550456179
//
struct ListNode
{
    size_t val;
    ListNode * next;

    ListNode() : val(0), next(nullptr) {}
    explicit ListNode(size_t val) : val(val), next(nullptr) {}
};

int main(int argc, char * argv[])
{
    using namespace std::chrono;

    static constexpr size_t Iters = 6;
    static constexpr intptr_t kMaxCount = 10000000;
    static constexpr size_t kBufSize = 128 * 1024 * 1024;   // 128 MB

    cpu_warm_up(1000);

    std::unique_ptr<char> src(new char[kBufSize]);
    std::unique_ptr<char> dest(new char[kBufSize]);

    double time1 = 0.0, time2 = 0.0, time3 = 0.0, time4 = 0.0;
    std::vector<std::string> results;

    *src.get() = 'A';

    ListNode * head = nullptr;
    for (intptr_t i = kMaxCount - 1; i >= 0; i--) {
        ListNode * cur = new ListNode(i);
        cur->next = head;
        head = cur;
    }

    for (size_t i = 0; i < Iters; i++) {
#if USE_CACHE_OVERWRITE
        // Force invalid the caches
        std::memcpy((void *)dest.get(), (const void *)src.get(), kBufSize * sizeof(char));
#endif

        if (1) {
            auto begin = high_resolution_clock::now();
            ListNode * cur = head;
            while (cur->next != nullptr) {
                cur = cur->next;
            }
            auto end = high_resolution_clock::now();

            duration<double> elapsed = duration_cast<duration<double>>(end - begin);
            printf("cur = %p, dest[0] = %c, time1: %0.3f ms\n", cur, *dest.get(), elapsed.count() * 1000);
            time1 = elapsed.count() * 1000;
        }

#if USE_CACHE_OVERWRITE
        // Force invalid the caches
        std::memcpy((void *)dest.get(), (const void *)src.get(), kBufSize * sizeof(char));
#endif

        if (1) {
            auto begin = high_resolution_clock::now();
            ListNode * cur = head;
            while (cur->next != nullptr && cur->next->val != kMaxCount) {
                cur = cur->next;
            }
            auto end = high_resolution_clock::now();

            duration<double> elapsed = duration_cast<duration<double>>(end - begin);
            printf("cur = %p, dest[0] = %c, time2: %0.3f ms\n", cur, *dest.get(), elapsed.count() * 1000);
            time2 = elapsed.count() * 1000;
        }

#if USE_CACHE_OVERWRITE
        // Force invalid the caches
        std::memcpy((void *)dest.get(), (const void *)src.get(), kBufSize * sizeof(char));
#endif

        if (1) {
            auto begin = high_resolution_clock::now();
            ListNode * cur = head;
            while (cur->next != nullptr) {
                cur = cur->next;
            }
            auto end = high_resolution_clock::now();

            duration<double> elapsed = duration_cast<duration<double>>(end - begin);
            printf("cur = %p, dest[0] = %c, time3: %0.3f ms\n", cur, *dest.get(), elapsed.count() * 1000);
            time3 = elapsed.count() * 1000;
        }

#if USE_CACHE_OVERWRITE
        // Force invalid the caches
        std::memcpy((void *)dest.get(), (const void *)src.get(), kBufSize * sizeof(char));
#endif

        if (1) {
            auto begin = high_resolution_clock::now();
            ListNode * cur = head;
            while (cur->next != nullptr && cur->next->val != kMaxCount) {
                cur = cur->next;
            }
            auto end = high_resolution_clock::now();

            duration<double> elapsed = duration_cast<duration<double>>(end - begin);
            printf("cur = %p, dest[0] = %c, time4: %0.3f ms\n", cur, *dest.get(), elapsed.count() * 1000);
            time4 = elapsed.count() * 1000;
        }

        printf("\n");

        char text[512];
        snprintf(text, sizeof(text) - 1,
                 "t1: %0.3f ms, t2: %0.3f ms, t3: %0.3f ms, t4: %0.3f ms, (%s) %s - (%s) %s",
                 time1, time2, time3, time4,
                 (time1 > time2) ? "t1 > t2" : "t1 < t2",
                 (time1 > time2) ? "*" : " ",
                 (time3 > time4) ? "t3 > t4" : "t3 < t4",
                 (time3 > time4) ? "*" : "");

        results.push_back(text);
    }

    // Print all test results
    for (auto const & result : results) {
        printf("%s\n", result.c_str());
    }
    printf("\n");

    return 0;
}

#endif
