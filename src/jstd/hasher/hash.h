
#ifndef JSTD_HASH_HASH_H
#define JSTD_HASH_HASH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <string>

#include "jstd/string/char_traits.h"

//
// See: https://sourceforge.net/p/predef/wiki/Architectures/
//
// See: https://sourceforge.net/projects/predef/
//
// See: http://stackoverflow.com/questions/735647/ifdef-for-32-bit-platform
//

//
// See: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
//

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* POWER-PC ---------------------------------------------------- */
#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
	defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
	/* POWER-PC 64-bit --------------------------------------------- */
#else
	/* POWER-PC 32-bit --------------------------------------------- */
#endif
#endif // Power PC

#if defined(__sparc)
	/* SPARC ---------------------------------------------------- */
#endif

#if defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
	/* Itanium 64 bit --------------------------------------------- */
#endif // Itanium

#if defined(__x86_64__) || defined(_M_X64)
	/* x86-64, amd64, 64-bit --------------------------------------------- */
#elif defined(__i386) || defined(_M_IX86)
	/* x86, i386, 32-bit --------------------------------------------- */
#endif // x86, amd64

//
// See: http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
//

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* Is POWER-PC ---------------------------------------------------- */
#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) \
 || defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
	/* POWER-PC 64-bit --------------------------------------------- */
    #define _IS_POWER_64_       1
#else
	/* POWER-PC 32-bit --------------------------------------------- */
    #define _IS_POWER_32_       1
#endif
#endif // Power PC

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) \
 || defined(__x86_64) || defined(__amd64) \
 || defined(__ia64) || defined(__itanium__) || defined(_M_IA64) \
 || defined(__ia64__) || defined(__IA64__)
    /* x86-64, amd64, Itanium, 64-bit --------------------------------------------- */
    #define _IS_X86_64_     1
#elif defined(_WIN32) || defined(_M_IX86) || defined(__i386) \
 || defined(__i386__) || defined(_X86_) || defined(__ia86__) || defined(i386)
    /* x86, i386, 32-bit --------------------------------------------- */
    #define _IS_X86_32_     1
#endif // x86, amd64, Itanium

//
// Notice that x32 can be detected by checking if the CPU uses the ILP32 data model.
// See: https://sourceforge.net/p/predef/wiki/DataModels/
//

#include <limits.h> // For __WORDSIZE on Linux

// __LP64__ is for GUN C, __WORDSIZE is for Linux POSIX.
#if defined(__LP64__) || defined(__LP64) || (defined(__WORDSIZE) && (__WORDSIZE == 64))
    #define _IS_64_BIT_     1
#elif defined(__ILP32__) || defined(_ILP32) || (defined(__WORDSIZE) && (__WORDSIZE == 32))
    #define _IS_32_BIT_     1
#elif defined(_WIN64) || defined(_IS_X86_64_)
    #define _IS_64_BIT_     1
#elif defined(_WIN32) || defined(_IS_X86_32_)
    #define _IS_32_BIT_     1
#endif

namespace rocksdb {

namespace port {

static bool kLittleEndian = true;

} // namespace port

namespace hashes {

template <typename CharTy>
static inline
uint32_t DecodeFixed32(const CharTy * ptr) {
    if (port::kLittleEndian) {
        // Load the raw bytes
        uint32_t result;
        // gcc optimizes this to a plain load
        ::memcpy((void *)&result, (const void *)ptr, sizeof(result));
        return result;
    }
    else {
        return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
              | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
              | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
              | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
    }
}

template <typename CharTy>
static inline
uint64_t DecodeFixed64(const CharTy * ptr) {
    if (port::kLittleEndian) {
        // Load the raw bytes
        uint64_t result;
        // gcc optimizes this to a plain load
        ::memcpy((void *)&result, (const void *)ptr, sizeof(result));
        return result;
    }
    else {
        uint64_t lo = DecodeFixed32(ptr);
        uint64_t hi = DecodeFixed32(ptr + 4);
        return (hi << 32) | (lo & 0xFFFFFFFFFULL);
    }
}

template <typename CharTy>
static uint32_t Hash(const CharTy * data, size_t n, uint32_t seed) {
    typedef typename jstd::char_traits<CharTy>::schar_type SCharTy;

    // Similar to murmur hash
    static const uint32_t m = 0xc6a4a793;
    static const uint32_t r = 24;
    const CharTy * limit = data + n;
    uint32_t h = static_cast<uint32_t>(seed ^ (n * m));

    // Pick up four bytes at a time
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    // Pick up remaining bytes
    switch (limit - data) {
        // Note: It would be better if this was cast to unsigned char, but that
        // would be a disk format change since we previously didn't have any cast
        // at all (so gcc used signed char).
        // To understand the difference between shifting unsigned and signed chars,
        // let's use 250 as an example. unsigned char will be 250, while signed char
        // will be -6. Bit-wise, they are equivalent: 11111010. However, when
        // converting negative number (signed char) to int, it will be converted
        // into negative int (of equivalent value, which is -6), while converting
        // positive number (unsigned char) will be converted to 250. Bitwise,
        // this looks like this:
        // signed char 11111010 -> int 11111111111111111111111111111010
        // unsigned char 11111010 -> int 00000000000000000000000011111010
    case 3:
        h += static_cast<uint32_t>(static_cast<SCharTy>(data[2]) << 16);
        // fall through
    case 2:
        h += static_cast<uint32_t>(static_cast<SCharTy>(data[1]) << 8);
        // fall through
    case 1:
        h += static_cast<uint32_t>(static_cast<SCharTy>(data[0]));
        h *= m;
        h ^= (h >> r);
        break;
    }
    return h;
}

} // namespace hashes
} // namespace rocksdb

namespace jstd {

static const std::uint32_t kDefaultHashSeed32 = 0xBC9F1D34UL;
static const std::uint64_t kDefaultHashSeed64 = (std::uint64_t)0x43D1F9CBBC9F1D34ULL;

static const std::size_t   kHashInitValue_M   = (std::size_t)0x397A4A6CC6A4A793ULL;

/**************************************************************************

    About hash algorithm

See:

    BKDRHash, APHash, JSHash, RSHash, SDBMHash, PJWHash, ELFHash
    http://blog.csdn.net/icefireelf/article/details/5796529

    Good: BKDRHash, APHash. Mid: DJBHash, JSHash, RSHash, SDBMHash. Bad: PJWHash, ELFHash.
    http://blog.csdn.net/pingnanlee/article/details/8232372

    Blizzard: "One Way Hash"
    http://blog.chinaunix.net/uid-20775243-id-2554977.html

    Another hash algorithm: hashpjw(PHP), lh_strhash(OpenSSL), calc_hashnr(MySQL)
    http://blog.chinaunix.net/uid-21457204-id-3061239.html
    http://blog.csdn.net/nhczp/article/details/3040546
    http://blog.chinaunix.net/uid-20775243-id-2554977.html

 **************************************************************************/

namespace hashes {

// This string hash function is from OpenSSL.
template <typename CharTy>
static std::uint32_t OpenSSL_Hash(const CharTy * key, std::size_t len)
{
    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;
    
    const UCharTy * limit = src + (len & std::size_t(~(std::size_t)1U));

    std::uint32_t hash = 0;
    std::uint32_t i = 0;
    while (src < limit) {
        hash ^= ((std::uint32_t)(*(unsigned short *)src) << (i & 0x0FU));
        i++;
        src += 2;
    }

    if (src != end) {
        hash ^= ((std::uint32_t)(*src) << (i & 0x0FU));
    }

    return hash;
}

//
// BKDR Hash Function -- Times31, Times33, Times131 ...
// SDBM Hash Function (seed = 65599)
//
//   hash = hash * seed^4 + a * seed^3 + b * seed^2 + c * seed + d;
//
template <typename CharTy>
static std::uint32_t BKDRHash(const CharTy * key, std::size_t len)
{
    static const std::uint32_t seed = 131U;   // 31, 33, 131, 1313, 13131, 131313, etc ...
    static const std::uint32_t seed_2 = seed * seed;
    static const std::uint32_t seed_3 = seed_2 * seed;
    static const std::uint32_t seed_4 = seed_2 * seed_2;

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    len *= sizeof(CharTy);

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;
    std::uint32_t hash = 0;

#if 1
    const UCharTy * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src != limit) {
        hash = hash * seed_4 + (std::uint32_t)src[0] * seed_3 + (std::uint32_t)src[1] * seed_2
                + (std::uint32_t)src[2] * seed + (std::uint32_t)src[3];
        src += 4;
    }
#endif
    while (src != end) {
        hash = hash * seed + (std::uint32_t)(*src);
        src++;
    }

    return hash;
}

//
// BKDR Hash Function (seed = 31) -- Times31, use on Java string hashCode.
//
//   hash = hash * seed^4 + a * seed^3 + b * seed^2 + c * seed + d;
//
template <typename CharTy>
static std::uint32_t BKDRHash_31(const CharTy * key, std::size_t len)
{
    static const std::uint32_t seed = 31U;   // 31, 33, 131, 1313, 13131, 131313, etc ...
    static const std::uint32_t seed_2 = seed * seed;
    static const std::uint32_t seed_3 = seed_2 * seed;
    static const std::uint32_t seed_4 = seed_2 * seed_2;

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    len *= sizeof(CharTy);

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;
    std::uint32_t hash = 0;

#if 1
    const UCharTy * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src < limit) {
        hash = hash * seed_4 + (std::uint32_t)src[0] * seed_3 + (std::uint32_t)src[1] * seed_2
                + (std::uint32_t)src[2] * seed + (std::uint32_t)src[3];
        src += 4;
    }
#endif
    while (src != end) {
        hash = hash * seed + (std::uint32_t)(*src);
        src++;
    }

    return hash;
}

//
// BKDR Hash Function (seed = 31) -- Times31, use on Java string hashCode.
//
//   hash = hash * seed^4 + a * seed^3 + b * seed^2 + c * seed + d;
//
template <typename CharTy>
static std::uint32_t BKDRHash_31_std(const CharTy * key, std::size_t len)
{
    static const std::uint32_t seed = 31U;   // 31, 33, 131, 1313, 13131, 131313, etc ...

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    len *= sizeof(CharTy);

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;
    std::uint32_t hash = 0;

    while (src != end) {
        hash = hash * seed + (std::uint32_t)(*src);
        src++;
    }

    return hash;
}

//
// Times31, BKDR Hash Function (seed = 31) -- Often use in Java string hash.
//
template <typename CharTy>
static inline
std::uint32_t Times31(const CharTy * key, std::size_t len)
{
    return BKDRHash_31(key, len);
}

//
// Times31, BKDR Hash Function (seed = 31) -- Often use in Java string hash.
//
template <typename CharTy>
static inline
std::uint32_t Times31Std(const CharTy * key, std::size_t len)
{
    return BKDRHash_31_std(key, len);
}

//
// APHash Hash Function
//
template <typename CharTy>
static std::uint32_t APHash(const CharTy * key, std::size_t len)
{
    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;

    std::uint32_t hash = 0;

#if 1
    const UCharTy * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src != limit) {
        //if (*src == '\0')
        //    break;
        hash ^=   ((hash <<  7U) ^ ((std::uint32_t)src[0]) ^ (hash >> 3U));
        hash ^= (~((hash << 11U) ^ ((std::uint32_t)src[1]) ^ (hash >> 5U)));
        hash ^=   ((hash <<  7U) ^ ((std::uint32_t)src[2]) ^ (hash >> 3U));
        hash ^= (~((hash << 11U) ^ ((std::uint32_t)src[3]) ^ (hash >> 5U)));
        src += 4;
    }
#endif
    std::uint32_t i = 0;
    while (src != end) {
        //if (*src == CharTy('\0'))
        //    break;
        if ((i & 1) == 0)
            hash ^=   ((hash <<  7U) ^ ((std::uint32_t)(*src)) ^ (hash >> 3U));
        else
            hash ^= (~((hash << 11U) ^ ((std::uint32_t)(*src)) ^ (hash >> 5U)));
        i++;
        src++;
    }
    return hash;
}

//
// DJB Hash Function
//
template <typename CharTy>
static std::uint32_t DJBHash(const CharTy * key, std::size_t len)
{
    std::uint32_t hash = 5381U;

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    const UCharTy * src = (const UCharTy *)key;
    const UCharTy * end = src + len;

    while (src != end) {
        //if (*src == UCharTy('\0'))
        //    break;
        hash += (hash << 5U) + (std::uint32_t)(*src);
        src++;
    }

    return hash;
}

} // namespace hashes

//
// class PrimaryHash
//
template <typename T, std::uint32_t N>
class PrimaryHash {
public:
    typedef T hash_type;

private:
    template <std::uint32_t MissAlign>
    inline hash_type decode_value(const char * data) {
        // Maybe got a error
        static_assert(((N == 4) || (N == 8)), "PrimaryHash::decode_value(), MissAlign maybe overflow.");
        return 0;
    }

public:
    template <std::uint32_t MissAlign>
    hash_type value(const char * key, std::size_t len, std::size_t seed) {
        static_assert(((N == 4) || (N == 8)), "PrimaryHash::value(), N maybe overflow.");
        return 0;
    }
};

template <typename T>
class PrimaryHash<T, 4U> {
public:
    typedef T hash_type;
    static const std::uint32_t N = sizeof(T);

private:
    static inline hash_type decode_value_generic(const char * data) {
        return ((hash_type)data[0] << 24) | ((hash_type)data[1] << 16)
             | ((hash_type)data[2] <<  8) | ((hash_type)data[3]);
    }

    template <std::uint32_t MissAlign>
    inline hash_type decode_value(const char * data) {
        return PrimaryHash<T, 4U>::decode_value_generic(data);
    }

public:
    template <std::uint32_t MissAlign>
    hash_type value(const char * key, std::size_t len, std::size_t seed) {
        // Similar to murmur hash
        static const std::size_t _m = kHashInitValue_M;
        static const std::uint32_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;
        static const hash_type hash_mask = static_cast<hash_type>(-1);

        register const char * data = key;
        const char * end = data + len;

        static const hash_type m = static_cast<hash_type>(_m & hash_mask);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));

        // The data address is aligned to sizeof(hash_type) bytes.
        register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
        while (data < limit) {
            hash_type val = PrimaryHash<T, 4U>::template decode_value<MissAlign>(data);
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            data += sizeof(hash_type);
        }
        hash_type remain = (hash_type)(end - data);
        if (remain == 0)
            return hash;
        // Filter the extra bits
        hash_type val = PrimaryHash<T, 4U>::template decode_value<MissAlign>(data);
        static const hash_type val_mask = (hash_type)(-1);
        val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
        hash += val;
        hash *= m;
        hash ^= (hash >> half_bits);
        return hash;
    }
};

//
// GCC error: explicit specialization in non-namespace scope (desperate for help)
// See: http://stackoverflow.com/questions/5777236/gcc-error-explicit-specialization-in-non-namespace-scope-desperate-for-help
//
// Why is partial specialziation of a nested class template allowed, while complete isn't?
// See: http://stackoverflow.com/questions/2537716/why-is-partial-specialziation-of-a-nested-class-template-allowed-while-complete
//
template <>
template <>
inline
typename PrimaryHash<std::uint32_t, 4U>::hash_type
PrimaryHash<std::uint32_t, 4U>::decode_value<0U>(const char * data) {
    assert(data != nullptr);
    hash_type value = *(hash_type *)(data);
    return value;
}

template <>
template <>
inline
typename PrimaryHash<std::uint32_t, 4U>::hash_type
PrimaryHash<std::uint32_t, 4U>::decode_value<1U>(const char * data) {
    static const std::uint32_t M = 1;
    static const hash_type mask = (hash_type)(-1);
    assert(data != nullptr);
    hash_type value = ((*(hash_type *)(data - M) & (mask >> (M * 8U))) << (M * 8U))
                     | (*(hash_type *)(data + (N - M)) & (mask >> ((N - M) * 8U)));
    return value;
}

template <>
template <>
inline
typename PrimaryHash<std::uint32_t, 4U>::hash_type
PrimaryHash<std::uint32_t, 4U>::decode_value<2U>(const char * data) {
    static const std::uint32_t M = 2;
    static const hash_type mask = (hash_type)(-1);
    assert(data != nullptr);
    hash_type value = ((*(hash_type *)(data - M) & (mask >> (M * 8U))) << (M * 8U))
                     | (*(hash_type *)(data + (N - M)) & (mask >> ((N - M) * 8U)));
    return value;
}

template <>
template <>
inline
typename PrimaryHash<std::uint32_t, 4U>::hash_type
PrimaryHash<std::uint32_t, 4U>::decode_value<3U>(const char * data) {
    static const std::uint32_t M = 3;
    static const hash_type mask = (hash_type)(-1);
    assert(data != nullptr);
    hash_type value = ((*(hash_type *)(data - M) & (mask >> (M * 8U))) << (M * 8U))
                     | (*(hash_type *)(data + (N - M)) & (mask >> ((N - M) * 8U)));
    return value;
}

template <typename T>
class PrimaryHash<T, 8U> {
public:
    typedef T hash_type;
    static const std::uint32_t N = sizeof(T);

private:
    static inline hash_type decode_value_generic(const char * data) {
        hash_type value = ((hash_type)data[0] << 56) | ((hash_type)data[1] << 48)
                        | ((hash_type)data[2] << 40) | ((hash_type)data[3] << 32)
                        | ((hash_type)data[4] << 24) | ((hash_type)data[5] << 16)
                        | ((hash_type)data[6] <<  8) | ((hash_type)data[7]);
        return value;
    }

    template <std::uint32_t MissAlign>
    inline hash_type decode_value(const char * data) {
        static const std::uint32_t M = MissAlign % N;
        static const hash_type mask = (hash_type)(-1);
        hash_type value;
        assert(MissAlign < N);
        assert(data != nullptr);
        value = ((*(hash_type *)(data - M) & (mask >> (M * 8U))) << (M * 8U))
               | (*(hash_type *)(data + (N - M)) & (mask >> ((N - M) * 8U)));
        return value;
    }

public:
    template <std::uint32_t MissAlign>
    hash_type value(const char * key, std::size_t len, std::size_t seed) {
        // Similar to murmur hash
        static const std::size_t _m = kHashInitValue_M;
        static const std::uint32_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;
        static const hash_type hash_mask = static_cast<hash_type>(-1);

        register const char * data = key;
        const char * end = data + len;

        static const hash_type m = static_cast<hash_type>(_m & hash_mask);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));

        // The data address is aligned to sizeof(hash_type) bytes.
        register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
        while (data < limit) {
            hash_type val = PrimaryHash<T, 8U>::template decode_value<MissAlign>(data);
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            data += sizeof(hash_type);
        }
        hash_type remain = (hash_type)(end - data);
        if (remain == 0)
            return hash;
        // Filter the extra bits
        hash_type val = PrimaryHash<T, 8U>::template decode_value<MissAlign>(data);
        static const hash_type val_mask = (hash_type)(-1);
        val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
        hash += val;
        hash *= m;
        hash ^= (hash >> half_bits);
        return hash;
    }
};

//
// GCC error: explicit specialization in non-namespace scope (desperate for help)
// See: http://stackoverflow.com/questions/5777236/gcc-error-explicit-specialization-in-non-namespace-scope-desperate-for-help
//
template <>
template <>
inline
typename PrimaryHash<std::uint32_t, 8U>::hash_type
PrimaryHash<std::uint32_t, 8U>::decode_value<0U>(char const * data) {
    hash_type value;
    assert(data != nullptr);
    value = *(hash_type *)(data);
    return value;
}

template <>
template <>
inline
typename PrimaryHash<std::uint64_t, 8U>::hash_type
PrimaryHash<std::uint64_t, 8U>::decode_value<0U>(char const * data) {
    hash_type value;
    assert(data != nullptr);
    value = *(hash_type *)(data);
    return value;
}

//
// class HashUtils
//
template <typename T = std::uint32_t>
class HashUtils {
public:
    typedef T hash_type;

    HashUtils() {}
    ~HashUtils() {}

    template <std::uint32_t N>
    inline hash_type decodeValue(const char * data, std::uint32_t missalign) const {
        assert(missalign < N);
        assert(data != nullptr);
        return 0;
    }

    template <std::uint32_t N, std::uint32_t MissAlign>
    inline hash_type decode_value(const char * data) const {
        //static const std::uint32_t N = sizeof(hash_type);
        static const std::uint32_t M = MissAlign % N;
        static const hash_type mask = (hash_type)(-1);
        hash_type value;
        static_assert((N == 4) || (N == 8 && MissAlign < N), "MissAlign >= N");
        assert(MissAlign < N);
        assert(data != nullptr);
        /*
        static_assert(M != 0, "MissAlign == 0");
        static_assert(M != 1, "MissAlign == 1");
        static_assert(M != 2, "MissAlign == 2");
        static_assert(M != 3, "MissAlign == 3");
        //*/
#if 1
        if (M != 0) {
            value = ((*(hash_type *)(data - M) & (mask >> (M * 8))) << (M * 8))
                   | (*(hash_type *)(data + (N - M)) & (mask >> (((N - M) % N) * 8)));
        }
        else {
            value = *(hash_type *)(data);
        }
#else
        if (N == 4) {
            value = ((hash_type)data[0] << 24) | ((hash_type)data[1] << 16) | ((hash_type)data[2] << 8) | ((hash_type)data[3]);
        }
        else if (N == 8) {
            value = ((hash_type)data[0] << (56 % (N*8))) | ((hash_type)data[1] << (48 % (N*8)))
                  | ((hash_type)data[2] << (40 % (N*8))) | ((hash_type)data[3] << (32 % (N*8)))
                  | ((hash_type)data[4] << (24 % (N*8))) | ((hash_type)data[5] << (16 % (N*8)))
                  | ((hash_type)data[6] << ( 8 % (N*8))) | ((hash_type)data[7]);
        }
        else {
            // Maybe got a error
            static_assert(((N == 4) || (N == 8)), "HashUtils::decode_value(), N maybe overflow.");
        }
#endif
        return value;
    }

    hash_type primaryHash_aligned(const char * key, std::size_t len, std::size_t seed) const {
        // Similar to murmur hash
        static const std::size_t _m = kHashInitValue_M;
        static const std::uint32_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;
        static const hash_type hash_mask = static_cast<hash_type>(-1);

        register const char * data = key;
        const char * end = data + len;

        static const hash_type m = static_cast<hash_type>(_m & hash_mask);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));

        // The data address is aligned to sizeof(hash_type) bytes.
        register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
        while (data < limit) {
            hash_type val = *(hash_type *)data;
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            data += sizeof(hash_type);
        }
        hash_type remain = (hash_type)(end - data);
        if (remain == 0)
            return hash;
        // Filter the extra bits
        hash_type val = *(hash_type *)data;
        static const hash_type val_mask = (hash_type)(-1);
        val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
        hash += val;
        hash *= m;
        hash ^= (hash >> half_bits);
        return hash;
    }

    template <std::uint32_t MissAlign>
    hash_type primaryHash_unaligned(const char * key, std::size_t len, std::size_t seed) const {
        // Similar to murmur hash
        static const std::size_t _m = kHashInitValue_M;
        static const std::uint32_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;
        static const hash_type hash_mask = static_cast<hash_type>(-1);

        register const char * data = key;
        const char * end = data + len;

        static const hash_type m = static_cast<hash_type>(_m & hash_mask);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));

        // The data address is not aligned to sizeof(hash_type) bytes.
        register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
        while (data < limit) {
            hash_type val = decode_value<sizeof(hash_type), MissAlign>(data);
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            data += sizeof(hash_type);
        }
        hash_type remain = (hash_type)(end - data);
        if (remain == 0)
            return hash;
        // Filter the extra bits
        hash_type val = decode_value<sizeof(hash_type), MissAlign>(data);
        static const hash_type val_mask = (hash_type)(-1);
        val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
        hash += val;
        hash *= m;
        hash ^= (hash >> half_bits);
        return hash;
    }

//#undef _IS_X86_64_
//#undef _IS_X86_32_

    hash_type primaryHash(const char * key, std::size_t len, std::size_t seed) const {
        static const std::uint32_t N = sizeof(hash_type);
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;

        //return hashes::Times31(key, len);

#if defined(_IS_X86_64_) || defined(_IS_X86_32_)
        return primaryHash_aligned(key, len, seed);
#else
        std::uint32_t missalign =
            static_cast<std::uint32_t>(reinterpret_cast<std::size_t>(key)) & align_mask;

        PrimaryHash<T, sizeof(hash_type)> primaryHash_;
        if (missalign == 0) {
            return primaryHash_.template value<0U>(key, len, seed);
        }
        else if (missalign == 1) {
            return primaryHash_.template value<1U>(key, len, seed);
        }
        else if (missalign == 2) {
            return primaryHash_.template value<2U>(key, len, seed);
        }
        else if (missalign == 3) {
            return primaryHash_.template value<3U>(key, len, seed);
        }
        else {
            if (N > 4) {
                if (missalign == 4) {
                    return primaryHash_.template value<4U>(key, len, seed);
                }
                else if (missalign == 5) {
                    return primaryHash_.template value<5U>(key, len, seed);
                }
                else if (missalign == 6) {
                    return primaryHash_.template value<6U>(key, len, seed);
                }
                else if (missalign == 7) {
                    return primaryHash_.template value<7U>(key, len, seed);
                }
            }
            else {
                // Maybe got a error
                static_assert((N == 4) || (N == 8), "HashUtils::primaryHash(), missalign maybe overflow.");
                assert((N == 4 && missalign >= 4) || (N == 8 && missalign >= 8));
            }
        }
        return 0;
#endif
    }

    hash_type primaryHash_new(const char * key, std::size_t len, std::size_t seed) const {
        static const std::uint32_t N = sizeof(hash_type);
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;

#if defined(_IS_X86_64_) || defined(_IS_X86_32_)
        return primaryHash_aligned(key, len, seed);
#else
        std::uint32_t missalign = static_cast<std::uint32_t>(reinterpret_cast<std::size_t>(key)) & align_mask;
        if (missalign == 0) {
            return primaryHash_aligned(key, len, seed);
        }
        else if (missalign == 1) {
            return primaryHash_unaligned<1>(key, len, seed);
        }
        else if (missalign == 2) {
            return primaryHash_unaligned<2>(key, len, seed);
        }
        else if (missalign == 3) {
            return primaryHash_unaligned<3>(key, len, seed);
        }
        else {
            if (N > 4) {
                if (missalign == 4) {
                    return primaryHash_unaligned<4>(key, len, seed);
                }
                else if (missalign == 5) {
                    return primaryHash_unaligned<5>(key, len, seed);
                }
                else if (missalign == 6) {
                    return primaryHash_unaligned<6>(key, len, seed);
                }
                else if (missalign == 7) {
                    return primaryHash_unaligned<7>(key, len, seed);
                }
            }
            else {
                // Maybe got a error
                static_assert((N == 4) || (N == 8), "HashUtils::primaryHash(), missalign maybe overflow.");
                assert((N == 4 && missalign >= 4) || (N == 8 && missalign >= 8));
            }
        }
        return 0;
#endif
    }

    hash_type primaryHash_old(const char * key, std::size_t len, std::size_t seed) const {
        // Similar to murmur hash
        static const std::size_t _m = kHashInitValue_M;
        static const std::uint32_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::uint32_t align_mask = sizeof(hash_type) - 1;
        static const hash_type hash_mask = static_cast<hash_type>(-1);

        register const char * data = key;
        const char * end = data + len;

        static const hash_type m = static_cast<hash_type>(_m & hash_mask);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));

#if !(defined(_IS_X86_64_) || defined(_IS_X86_32_))
        std::uint32_t missalign = static_cast<std::uint32_t>(reinterpret_cast<std::size_t>(data)) & align_mask;
        if (missalign == 0) {
#endif
            // The data address is aligned to sizeof(hash_type) bytes.
            // But it don't need care on x86 or amd64 archs.
            register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
            while (data < limit) {
                hash_type val = *(hash_type *)data;
                hash += val;
                hash *= m;
                hash ^= (hash >> half_bits);
                data += sizeof(hash_type);
            }
            hash_type remain = (hash_type)(end - data);
            if (remain == 0)
                return hash;
            // Filter the extra bits
            hash_type val = *(hash_type *)data;
            static const hash_type val_mask = (hash_type)(-1);
            val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            return hash;
#if !(defined(_IS_X86_64_) || defined(_IS_X86_32_))
        }
        else {
            // The data address is not aligned to sizeof(hash_type) bytes.
            register const char * limit = (const char *)(data + (len & ~(std::size_t)align_mask));
            while (data < limit) {
                hash_type val = decodeValue<sizeof(hash_type)>(data, missalign);
                hash += val;
                hash *= m;
                hash ^= (hash >> half_bits);
                data += sizeof(hash_type);
            }
            hash_type remain = (hash_type)(end - data);
            if (remain == 0)
                return hash;
            // Filter the extra bits
            hash_type val = decodeValue<sizeof(hash_type)>(data, missalign);
            static const hash_type val_mask = (hash_type)(-1);
            val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            return hash;
        }
#endif
    }

    /*
    hash_type primaryHash(const Slice & key, std::size_t seed) {
        return primaryHash(key.data(), key.size(), seed);
    }

    template <std::size_t N>
    hash_type primaryHash(const char (&key)[N], std::size_t seed) {
        return primaryHash(key, N, seed);
    }
    //*/

    hash_type secondaryHash(const char * key, std::size_t len) const {
        //return static_cast<hash_type>(hashes::OpenSSL_Hash(key, len));
        return static_cast<hash_type>(hashes::Times31(key, len));
    }

    /*
    hash_type secondaryHash(const Slice & key) const {
        return secondaryHash(key.data(), key.size());
    }
    //*/

    template <std::size_t N>
    hash_type secondaryHash(const char (&key)[N]) const {
        return secondaryHash(key, N);
    }


    hash_type OpenSSLHash(const char * key, std::size_t len) const {
        return static_cast<hash_type>(hashes::OpenSSL_Hash(key, len));
    }
};

template <>
template <>
inline typename HashUtils<std::uint32_t>::hash_type
HashUtils<std::uint32_t>::decodeValue<4U>(const char * data, std::uint32_t missalign) const {
    static const std::size_t N = 4;
    static const hash_type mask = (hash_type)(-1);
    hash_type value;
    assert(missalign < N);
    assert(data != nullptr);
#if 1
    value = ((hash_type)data[0] << 24) | ((hash_type)data[1] << 16) | ((hash_type)data[2] << 8) | ((hash_type)data[3]);
#else
    if (missalign == 1) {
        value = ((*(hash_type *)(data - 1) & (mask >> (1 * 8U))) << 8U) | (*(hash_type *)(data + (N - 1)) & (mask >> ((N - 1) * 8U)));
    }
    else if (missalign == 2) {
        value = ((*(hash_type *)(data - 2) & (mask >> (2 * 8U))) << 16U) | (*(hash_type *)(data + (N - 2)) & (mask >> ((N - 2) * 8U)));
    }
    else if (missalign == 3) {
        value = ((*(hash_type *)(data - 3) & (mask >> (3 * 8U))) << 24U) | (*(hash_type *)(data + (N - 3)) & (mask >> ((N - 3) * 8U)));
    }
    else {
        // TODO: Error?
        value = 0;
    }
#endif
    return value;
}

template <>
template <>
inline typename HashUtils<std::uint64_t>::hash_type
HashUtils<std::uint64_t>::decodeValue<8U>(const char * data, std::uint32_t missalign) const {
    static const std::size_t N = 8;
    static const hash_type mask = (hash_type)(-1);
    hash_type value;
    assert(missalign < N);
    assert(data != nullptr);
#if 1
    value = ((hash_type)data[0] << 56) | ((hash_type)data[1] << 48) | ((hash_type)data[2] << 40) | ((hash_type)data[3] << 32)
          | ((hash_type)data[4] << 24) | ((hash_type)data[5] << 16) | ((hash_type)data[6] <<  8) | ((hash_type)data[7]);
#else
    if (missalign == 1) {
        value = ((*(hash_type *)(data - 1) & (mask >> (1 * 8U))) <<  8U) | (*(hash_type *)(data + (N - 1)) & (mask >> ((N - 1) * 8U)));
    }
    else if (missalign == 2) {
        value = ((*(hash_type *)(data - 2) & (mask >> (2 * 8U))) << 16U) | (*(hash_type *)(data + (N - 2)) & (mask >> ((N - 2) * 8U)));
    }
    else if (missalign == 3) {
        value = ((*(hash_type *)(data - 3) & (mask >> (3 * 8U))) << 24U) | (*(hash_type *)(data + (N - 3)) & (mask >> ((N - 3) * 8U)));
    }
    else if (missalign == 4) {
        value = ((*(hash_type *)(data - 4) & (mask >> (4 * 8U))) << 32U) | (*(hash_type *)(data + (N - 4)) & (mask >> ((N - 4) * 8U)));
    }
    else if (missalign == 5) {
        value = ((*(hash_type *)(data - 5) & (mask >> (5 * 8U))) << 40U) | (*(hash_type *)(data + (N - 5)) & (mask >> ((N - 5) * 8U)));
    }
    else if (missalign == 6) {
        value = ((*(hash_type *)(data - 6) & (mask >> (6 * 8U))) << 48U) | (*(hash_type *)(data + (N - 6)) & (mask >> ((N - 6) * 8U)));
    }
    else if (missalign == 7) {
        value = ((*(hash_type *)(data - 7) & (mask >> (7 * 8U))) << 56U) | (*(hash_type *)(data + (N - 7)) & (mask >> ((N - 7) * 8U)));
    }
    else {
        // TODO: Error?
        value = 0;
    }
#endif
    return value;
}

} // namespace kvdb

#endif // JSTD_HASH_HASH_H
