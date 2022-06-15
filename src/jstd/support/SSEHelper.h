
#ifndef JSTD_SSE_HELPER_H
#define JSTD_SSE_HELPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <nmmintrin.h>  // For SSE 4.2

namespace jstd {

template <typename CharTy>
struct SSEHelper {
    static constexpr uint8_t _SIDD_CHAR_OPS = _SIDD_UBYTE_OPS;
    static constexpr int kMaxSize = 16;
    static constexpr int kWordSize = 1;
};

template <>
struct SSEHelper<short> {
    static constexpr uint8_t _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static constexpr int kMaxSize = 8;
    static constexpr int kWordSize = 2;
};

template <>
struct SSEHelper<unsigned short> {
    static constexpr uint8_t _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static constexpr int kMaxSize = 8;
    static constexpr int kWordSize = 2;
};

template <>
struct SSEHelper<wchar_t> {
    static constexpr uint8_t _SIDD_CHAR_OPS = _SIDD_UWORD_OPS;
    static constexpr int kMaxSize = 8;
    static constexpr int kWordSize = 2;
};

} // namespace jstd

#endif // JSTD_SSE_HELPER_H
