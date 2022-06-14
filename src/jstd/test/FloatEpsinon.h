
#ifndef JSTD_TEST_FLOAT_EPSINON_H
#define JSTD_TEST_FLOAT_EPSINON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <float.h>

//
// Doug Gwyn: if (RelDif(a, b) <= TOLERANCE) ...
//
// See: https://www.jianshu.com/p/b4f0dc31fd4e
//

// FLT_EPSILON = 1.192092896e-07F
#define FLOAT_EPSINON               FLT_EPSILON

// DBL_EPSILON = 2.2204460492503131e-016
#define DOUBLE_EPSINON              DBL_EPSILON

#define FLOAT_POSITIVE_EPSINON      ( 1e-6)
#define FLOAT_NEGATIVE_EPSINON      (-1e-6)

#define DOUBLE_POSITIVE_EPSINON     ( 1e-12)
#define DOUBLE_NEGATIVE_EPSINON     (-1e-12)

namespace jstd {

//
// See: https://my.oschina.net/2bit/blog/3065096
//

static inline int float32_is_equal(float x, float y) {
    union float32_t {
        float    f32;
        uint32_t u32;
    };
    float32_t val_x, val_y;
    val_x.f32 = x;
    val_y.f32 = y;
    return ((x == y) || (
        ((val_x.u32 ^ val_y.u32) <= 2) &&
        ((val_x.u32 & 0x7F800000UL) != 0x7F800000UL)));
}

static inline int float64_is_equal(double x, double y) {
    union float64_t {
        double   f64;
        uint64_t u64;
    };
    float64_t val_x, val_y;
    val_x.f64 = x;
    val_y.f64 = y;
    return ((x == y) || (
        ((val_x.u64 ^ val_y.u64) <= 2) &&
        ((val_x.u64 & 0x7FF0000000000000ULL) != 0x7FF0000000000000ULL)));
}

} // namespace jstd

#endif // JSTD_TEST_FLOAT_EPSINON_H
