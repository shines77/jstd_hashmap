
/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2022 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

*************************************************************************************

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

  You are free to:

    1. Share -- copy and redistribute the material in any medium or format.

    2. Adapt -- remix, transforn, and build upon the material for any purpose,
    even commerically.

    The licensor cannot revoke these freedoms as long as you follow the license terms.

  Under the following terms:

    * Attribution -- You must give appropriate credit, provide a link to the license,
    and indicate if changes were made. You may do so in any reasonable manner,
    but not in any way that suggests the licensor endorses you or your use.

    * ShareAlike -- If you remix, transform, or build upon the material, you must
    distribute your contributions under the same license as the original.

    * No additional restrictions -- You may not apply legal terms or technological
    measures that legally restrict others from doing anything the license permits.

  Notices:

    * You do not have to comply with the license for elements of the material
    in the public domain or where your use is permitted by an applicable exception
    or limitation.

    * No warranties are given. The license may not give you all of the permissions
    necessary for your intended use. For example, other rights such as publicity,
    privacy, or moral rights may limit how you use the material.

************************************************************************************/

#pragma once

#include <memory.h>
#include <math.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::ptrdiff_t, std::size_t
#include <cstdbool>
#include <cassert>
#include <cmath>        // For std::ceil()
#include <memory>       // For std::swap(), std::pointer_traits<T>
#include <limits>       // For std::numeric_limits<T>
#include <cstring>      // For std::memset()
#include <vector>
#include <utility>      // For std::pair<First, Second>, std::integer_sequence<T...>
#include <tuple>        // For std::tuple<Ts...>
#include <initializer_list>
#include <algorithm>    // For std::max(), std::min()
#include <type_traits>
#include <stdexcept>

#include <nmmintrin.h>
#include <immintrin.h>

#include "jstd/type_traits.h"
#include "jstd/utility.h"
#include "jstd/support/BitUtils.h"
#include "jstd/support/Power2.h"
#include "jstd/support/BitVec.h"

#ifdef _MSC_VER
#ifndef __SSE2__
#define __SSE2__
#endif

#ifndef __SSSE3__
#define __SSSE3__
#endif
#endif // _MSC_VER

namespace jstd {
namespace v1 {

static inline
std::size_t align_to(std::size_t size, std::size_t alignment)
{
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0);
    size = (size + alignment - 1) & ~(alignment - 1);
    assert((size / alignment * alignment) == size);
    return size;
}

namespace hashes {

static inline
std::uint32_t Integal_hash1_u32(std::uint32_t value)
{
    std::uint32_t hash = value * 2654435761ul;
    return hash;
}

static inline
std::uint32_t Integal_hash2_u32(std::uint32_t value)
{
    std::uint32_t hash = value * 2654435761ul ^ 2166136261ul;
    return hash;
}

template <typename T>
struct IntegalHash
{
    typedef T           argument_type;
    typedef std::size_t result_type;

    template <typename UInt32, typename std::enable_if<
                                (std::is_integral<UInt32>::value &&
                                (sizeof(UInt32) <= 4))>::type * = nullptr>
    result_type operator () (UInt32 value) const noexcept {
        //std::uint32_t hash = value * 16777619ul ^ 2166136261ul;
        result_type hash = (result_type)((std::uint32_t)value * 2654435761ul + 16777619ul);
        return hash;
    }

    template <typename UInt64, typename std::enable_if<
                                (std::is_integral<UInt64>::value &&
                                (sizeof(UInt64) > 4 && sizeof(UInt64) <= 8))>::type * = nullptr>
    result_type operator () (UInt64 value) const noexcept {
        //std::uint64_t hash = value * 1099511628211ull ^ 14695981039346656037ull;
        result_type hash = (result_type)((std::uint64_t)value * 14695981039346656037ull + 1099511628211ull);
        return hash;
    }

    template <typename Argument, typename std::enable_if<
                                  (!std::is_integral<Argument>::value ||
                                  sizeof(Argument) > 8)>::type * = nullptr>
    result_type operator () (const Argument & value) const noexcept {
        std::hash<Argument> hasher;
        return static_cast<result_type>(hasher(value));
    }
};

} // namespace hashes

template < typename Key, typename Value,
           typename Hash = std::hash<Key>,
           typename KeyEqual = std::equal_to<Key>,
           typename Allocator = std::allocator<std::pair<const Key, Value>> >
class flat16_hash_map {
public:
    typedef Key                             key_type;
    typedef Value                           mapped_type;
    typedef std::pair<const Key, Value>     value_type;
    typedef std::pair<Key, Value>           nc_value_type;

    typedef Hash                            hasher;
    typedef KeyEqual                        key_equal;
    typedef Allocator                       allocator_type;
    typedef typename Hash::result_type      hash_result_t;

    typedef std::size_t                     size_type;
    typedef typename std::make_signed<size_type>::type
                                            ssize_type;
    typedef std::size_t                     index_type;
    typedef std::size_t                     hash_code_t;
    typedef flat16_hash_map<Key, Value, Hash, KeyEqual, Allocator>
                                            this_type;

    static constexpr size_type npos = size_type(-1);

    static constexpr size_type kControlHashMask = 0x0000007Ful;
    static constexpr size_type kControlShift    = 7;

    static constexpr size_type kClusterBits     = 4;
    static constexpr size_type kClusterWidth    = size_type(1) << kClusterBits;
    static constexpr size_type kClusterMask     = kClusterWidth - 1;
    static constexpr size_type kClusterShift    = kControlShift + kClusterBits;

    // kMinimumCapacity must be >= 2
    static constexpr size_type kMinimumCapacity = 4;
    // kDefaultCapacity must be >= kMinimumCapacity
    static constexpr size_type kDefaultCapacity = 4;

    static constexpr float kMinLoadFactor = 0.2f;
    static constexpr float kMaxLoadFactor = 0.8f;
    // Must be kMinLoadFactor <= loadFactor <= kMaxLoadFactor
    static constexpr float kDefaultLoadFactor = 0.5f;

    static constexpr bool entry_is_trivial_destructor =
            (std::is_trivially_destructible<value_type>::value ||
            (std::is_trivially_destructible<key_type>::value &&
             std::is_trivially_destructible<mapped_type>::value) ||
           ((std::is_arithmetic<key_type>::value || std::is_enum<key_type>::value) &&
            (std::is_arithmetic<mapped_type>::value || std::is_enum<mapped_type>::value)));

    static constexpr bool entry_is_trivial_copyable =
            (std::is_trivially_copyable<value_type>::value ||
            (std::is_trivially_copyable<key_type>::value &&
             std::is_trivially_copyable<mapped_type>::value) ||
            (std::is_scalar<key_type>::value && std::is_scalar<mapped_type>::value));

    static constexpr std::uint8_t kEmptyEntry   = 0b10000000;
    static constexpr std::uint8_t kDeletedEntry = 0b11111110;
    static constexpr std::uint8_t kEndOfMark    = 0b11111111;
    static constexpr std::uint8_t kUnusedMask   = 0b10000000;
    static constexpr std::uint8_t kHash2Mask    = 0b01111111;

    static constexpr std::uint32_t kFullMask16  = 0x0000FFFFul;

    static constexpr std::uint64_t kEmptyEntry64   = 0x8080808080808080ull;
    static constexpr std::uint64_t kDeletedEntry64 = 0xFEFEFEFEFEFEFEFEull;
    static constexpr std::uint64_t kEndOfMark64    = 0xFFFFFFFFFFFFFFFFull;
    static constexpr std::uint64_t kUnusedMask64   = 0x8080808080808080ull;

    struct control_byte {
        std::uint8_t value;

        control_byte() noexcept {
        }

        bool isEmpty() const {
            return (this->value == kEmptyEntry);
        }

        bool isDeleted() const {
            return (this->value == kDeletedEntry);
        }

        bool isEndOf() const {
            return (this->value == kEndOfMark);
        }

        bool isEmptyOrDeleted() const {
            return ((std::int8_t)this->value < (std::int8_t)kEndOfMark);
        }

        bool isUsed() const {
            return ((std::int8_t)this->value >= (std::int8_t)0);
        }

        bool isUnused() const {
            return ((std::int8_t)this->value < (std::int8_t)0);
        }

        void setEmpty() {
            this->value = kEmptyEntry;
        }

        void setDeleted() {
            this->value = kDeletedEntry;
        }

        void setEndOf() {
            this->value = kEndOfMark;
        }

        void setUsed(std::uint8_t control_hash) {
            assert((control_hash & kUnusedMask) == 0);
            this->value = control_hash;
        }
    };

#ifdef __SSE2__

    template <typename T>
    struct BitMask128_SSE2 {
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef std::uint32_t bitmask_type;

        void clear(pointer data) {
            this->template fillAll8<kEmptyEntry>(data);
        }

        void setAllZeros(pointer data) {
            __m128i tmp;
            __m128i zero_bits = _mm_xor_si128(tmp, tmp);
            _mm_store_si128((__m128i *)data, zero_bits);
        }

        template <std::uint8_t ControlTag>
        void fillAll8(pointer data) {
            __m128i tag_bits = _mm_set1_epi8(ControlTag);
            _mm_store_si128((__m128i *)data, tag_bits);
        }

        __m128i matchControlTag128(const_pointer data, std::uint8_t control_tag) const {
            __m128i tag_bits = _mm_set1_epi8(control_tag);
            __m128i control_bits = _mm_load_si128((const __m128i *)data);
            __m128i match_mask = _mm_cmpeq_epi8(control_bits, tag_bits);
            return match_mask;
        }

        std::uint32_t matchControlTag(const_pointer data, std::uint8_t control_tag) const {
            __m128i tag_bits = _mm_set1_epi8(control_tag);
            __m128i control_bits = _mm_load_si128((const __m128i *)data);
            __m128i match_mask = _mm_cmpeq_epi8(control_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHash(const_pointer data, std::uint8_t control_hash) const {
            return this->matchControlTag(data, control_hash);
        }

        std::uint32_t matchEmpty(const_pointer data) const {
#if defined(__SSSE3__)
            // This only works when kEmptyEntry is 0b10000000.
            __m128i control_bits = _mm_load_si128((const __m128i *)data);
            __m128i empty_mask = _mm_sign_epi8(control_bits, control_bits);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(empty_mask);
            return mask;
#else
            return this->matchControlTag(data, kEmptyEntry);
#endif
        }

        std::uint32_t matchDeleted(const_pointer data) const {
            return this->matchControlTag(data, kDeletedEntry);
        }

        std::uint32_t matchEmptyOrDeleted(const_pointer data) const {
            __m128i tag_bits = _mm_set1_epi8(kEndOfMark);
            __m128i control_bits = _mm_load_si128((const __m128i *)data);
            __m128i match_mask = _mm_cmplt_epi8_fixed(control_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(control_bits);
            return mask;
        }

        std::uint32_t matchUsed(const_pointer data) const {
            std::uint32_t maskUnused = this->matchUnused(data);
            std::uint32_t maskUsed = (maskUnused ^ kFullMask16);
            return maskUsed;
        }

        std::uint32_t matchUnused(const_pointer data) const {
            __m128i control_bits = _mm_load_si128((const __m128i *)data);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(control_bits);
            return mask;
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t control_hash) const {
            return (this->matchHash(data, control_hash) != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            return (this->matchEmpty(data) != 0);
        }

        bool hasAnyDeleted(const_pointer data) const {
            return (this->matchDeleted(data) != 0);
        }

        bool hasAnyEmptyOrDeleted(const_pointer data) const {
            return (this->matchEmptyOrDeleted(data) != 0);
        }

        bool hasAnyUsed(const_pointer data) const {
            return (this->matchUsed(data) != 0);
        }

        bool hasAnyUnused(const_pointer data) const {
            return (this->matchUnused(data) != 0);
        }

        bool isAllEmpty(const_pointer data) const {
            return (this->matchEmpty(data) == kFullMask16);
        }

        bool isAllDeleted(pointer data) const {
            return (this->matchDeleted(data) == kFullMask16);
        }

        bool isAllEmptyOrDeleted(const_pointer data) const {
            return (this->matchEmptyOrDeleted(data) == kFullMask16);
        }

        bool isAllUsed(const_pointer data) const {
            return (this->matchUnused(data) == 0);
        }

        bool isAllUnused(const_pointer data) const {
            return (this->matchUnused(data) == kFullMask16);
        }
    };

    template <typename T>
    using BitMask128 = BitMask128_SSE2<T>;

#else // !__SSE2__

    struct bitmask128_t {
        std::uint64_t low;
        std::uint64_t high;

        static constexpr std::uint64_t kMask8 = 0x8080808080808080ull;

        bitmask128_t() noexcept : low(0), high(0) {}
        bitmask128_t(std::uint64_t low, std::uint64_t high) noexcept
            : low(low), high(high) {}

        std::uint32_t movemask8() const {
            assert((low  & kMask8) == low);
            assert((high & kMask8) == high);

            std::uint32_t mask16_l = 0;
            std::uint64_t _low = low;
            while (_low != 0) {
                std::uint32_t pos = BitUtils::bsf64(_low);
                _low = BitUtils::clearLowBit64(_low);
                mask16_l |= 1u << (pos >> 3u);
            }

            std::uint32_t mask16_h = 0;
            std::uint64_t _high = high;
            while (_high != 0) {
                std::uint32_t pos = BitUtils::bsf64(_high);
                _high = BitUtils::clearLowBit64(_high);
                mask16_h |= 1u << (pos >> 3u);
            }

            std::uint32_t mask16 = (mask16_h << 8u) | mask16_l;
            return mask16;
        }
    };

    //
    // Google - cwisstable: Google's Swiss Table for C
    //
    // See: https://github.com/google/cwisstable/blob/main/cwisstable/internal/control_byte.h
    //
    template <typename T>
    struct BitMask128_u64 {
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef bitmask128_t bitmask_type;

        static constexpr std::size_t kByteWidth = 16;

        static constexpr std::uint64_t kMSBs = 0x8080808080808080ull;
        static constexpr std::uint64_t kLSBs = 0x0101010101010101ull;
        static constexpr std::uint32_t kFullMask64 = 0x8080808080808080ull;

        bitmask128_t * to_bitmask128(pointer data) {
            return (bitmask128_t *)data;
        }

        const bitmask128_t * to_bitmask128(const_pointer data) const {
            return (const bitmask128_t *)data;
        }

        void clear(pointer data) {
            this->template fillAll8<kEmptyEntry64>(data);
        }

        void setAllZeros(pointer data) {
            bitmask128_t * bm128 = this->to_bitmask128(data);
            bm128->low  = 0;
            bm128->high = 0;
        }

        template <std::uint64_t ControlTag>
        void fillAll8(pointer data) {
            bitmask128_t * bm128 = this->to_bitmask128(data);
            bm128->low  = ControlTag;
            bm128->high = ControlTag;
        }

        std::uint32_t matchControlTag16(const_pointer data, std::uint8_t control_tag) const {
#if 1
            assert((std::uint8_t)control_tag >= (std::uint8_t)0);
            return this->matchControlTag16(data, (std::uint64_t)control_tag * kLSBs);
#else
            std::uint8_t * control = (std::uint8_t *)data;
            std::uint32_t mask = 0, bit = 1;
            for (size_type i = 0; i < kClusterWidth; i++) {
                if (control[i] == control_tag) {
                    mask |= bit;
                }
                bit <<= 1;
            }
            return mask;
#endif
        }

        std::uint32_t matchControlTag16(const_pointer data, std::uint64_t control_tag) const {
#if 1
            bitmask128_t bitmask128 = this->matchControlTag(data, control_tag);
            return bitmask128.movemask8();
#else
            std::uint8_t * control = (std::uint8_t *)data;
            std::uint32_t mask = 0, bit = 1;
            for (size_type i = 0; i < kClusterWidth; i++) {
                if (control[i] == (std::uint8_t)control_tag) {
                    mask |= bit;
                }
                bit <<= 1;
            }
            return mask;
#endif
        }

        bitmask128_t matchControlTag(const_pointer data, std::uint8_t control_tag) const {
            assert((std::uint8_t)control_tag >= (std::uint8_t)0);
            return this->matchControlTag(data, (std::uint64_t)control_tag * kLSBs);
        }

        bitmask128_t matchControlTag(const_pointer data, std::uint64_t control_tag) const {
            //
            // For the technique, see:
            // http://graphics.stanford.edu/~seander/bithacks.html##ValueInWord
            // (Determine if a word has a byte equal to n).
            //
            // Caveat: there are false positives but:
            // - they only occur if there is a real match
            // - they never occur on kEmptyEntry, kDeletedEntry, kSentinelEntry
            // - they will be handled gracefully by subsequent checks in code
            //
            // Example:
            //   val = 0x1716151413121110ull
            //   control_tag = 0x12
            //   ret_val = (val - kLSBs) & ~val & kMSBs = 0x0000000080800000ull
            //
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low  ^ control_tag;
            std::uint64_t high = bm128->high ^ control_tag;
            // #define haszero(val) (((val) - 0x01010101ul) & ~(val) & 0x80808080ul)
            low  = (low  - kLSBs) & ~low  & kMSBs;
            high = (high - kLSBs) & ~high & kMSBs;
            return { low, high };
        }

        bitmask128_t matchHash(const_pointer data, std::uint8_t control_hash) const {
            return this->matchControlTag(data, control_hash);
        }

        bitmask128_t matchEmpty(const_pointer data) const {
            // kEmptyEntry = 0b10000000
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low;
            std::uint64_t high = bm128->high;
            low  = low  & (~low  << 6) & kMSBs;
            high = high & (~high << 6) & kMSBs;
            return { low, high };
        }

        bitmask128_t matchDeleted(const_pointer data) const {
#if 1
            // kDeletedEntry = 0b11111110
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low  - kLSBs;
            std::uint64_t high = bm128->high - kLSBs;
            low  = low  & (~low  << 6) & kMSBs;
            high = high & (~high << 6) & kMSBs;
            return { low, high };
#else
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low;
            std::uint64_t high = bm128->high;
            low  = low  & (low  << 6) & (~low  << 7) & kMSBs;
            high = high & (high << 6) & (~high << 7) & kMSBs;
            return { low, high };
#endif
        }

        bitmask128_t matchEmptyOrDeleted(const_pointer data) const {
            // kEmptyOrDeleted < kEndOfMark = 0b11111111
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low;
            std::uint64_t high = bm128->high;
            low  = low  & (~low  << 7) & kMSBs;
            high = high & (~high << 7) & kMSBs;
            return { low, high };
        }

        bitmask128_t matchUsed(const_pointer data) const {
            // kUsedEntry = 0b0xxxxxxx
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low;
            std::uint64_t high = bm128->high;
            low  = ~low  & kUnusedMask64;
            high = ~high & kUnusedMask64;
            return { low, high };
        }

        bitmask128_t matchUnused(const_pointer data) const {
            // kUnusedEntry = 0b1xxxxxxx
            bitmask128_t * bm128 = this->to_bitmask128(data);
            std::uint64_t low  = bm128->low  & kUnusedMask64;
            std::uint64_t high = bm128->high & kUnusedMask64;
            return { low, high };
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t control_hash) const {
            bitmask128_t bitmask = this->matchHash(data, control_hash);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            bitmask128_t bitmask = this->matchEmpty(data);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool hasAnyDeleted(const_pointer data) const {
            bitmask128_t bitmask = this->matchDeleted(data);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool hasAnyEmptyOrDeleted(const_pointer data) const {
            bitmask128_t bitmask = this->matchEmptyOrDeleted(data);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool hasAnyUsed(const_pointer data) const {
            bitmask128_t bitmask = this->matchUsed(data);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool hasAnyUnused(const_pointer data) const {
            bitmask128_t bitmask = this->matchUnused(data);
            return (bitmask.low != 0 && bitmask.high != 0);
        }

        bool isAllEmpty(const_pointer data) const {
            bitmask128_t bitmask = this->matchEmpty(data);
            return (bitmask.low == kFullMask64 && bitmask.high == kFullMask64);
        }

        bool isAllDeleted(pointer data) const {
            bitmask128_t bitmask = this->matchDeleted(data);
            return (bitmask.low == kFullMask64 && bitmask.high == kFullMask64);
        }

        bool isAllEmptyOrDeleted(const_pointer data) const {
            bitmask128_t bitmask = this->matchEmptyOrDeleted(data);
            return (bitmask.low == kFullMask64 && bitmask.high == kFullMask64);
        }

        bool isAllUsed(const_pointer data) const {
            bitmask128_t bitmask = this->matchUnused(data);
            return (bitmask.low == kFullMask64 && bitmask.high == kFullMask64);
        }

        bool isAllUnused(const_pointer data) const {
            bitmask128_t bitmask = this->matchUnused(data);
            return (bitmask.low == kFullMask64 && bitmask.high == kFullMask64);
        }
    };

    template <typename T>
    using BitMask128 = BitMask128_u64<T>;

#endif

    struct alignas(16) hash_cluster {
        typedef BitMask128<control_byte>                        bitmask128_type;
        typedef typename BitMask128<control_byte>::bitmask_type bitmask_type;

        union {
            control_byte controls[kClusterWidth];
            bitmask128_type bitmask;
        };

        hash_cluster() noexcept {
            this->clear();
        }

        ~hash_cluster() = default;

        void clear() {
            bitmask.clear(&this->controls[0]);
        }

        template <std::uint8_t ControlTag>
        void fillAll8() {
            bitmask.template fillAll8<ControlTag>(&this->controls[0]);
        }

        bitmask_type matchControlTag(std::uint8_t control_tag) const {
            return bitmask.matchControlTag(&this->controls[0], control_tag);
        }

        bitmask_type matchHash(std::uint8_t control_hash) const {
            return bitmask.matchHash(&this->controls[0], control_hash);
        }

        bitmask_type matchEmpty() const {
            return bitmask.matchEmpty(&this->controls[0]);
        }

        bitmask_type matchDeleted() const {
            return bitmask.matchDeleted(&this->controls[0]);
        }

        bitmask_type matchEmptyOrDeleted() const {
            return bitmask.matchEmptyOrDeleted(&this->controls[0]);
        }

        bitmask_type matchUsed() const {
            return bitmask.matchUsed(&this->controls[0]);
        }

        bitmask_type matchUnused() const {
            return bitmask.matchUnused(&this->controls[0]);
        }

        bool hasAnyMatch(std::uint8_t control_hash) const {
            return bitmask.hasAnyMatch(&this->controls[0], control_hash);
        }

        bool hasAnyEmpty() const {
            return bitmask.hasAnyEmpty(&this->controls[0]);
        }

        bool hasAnyDeleted() const {
            return bitmask.hasAnyDeleted(&this->controls[0]);
        }

        bool hasAnyEmptyOrDeleted() const {
            return bitmask.hasAnyEmptyOrDeleted(&this->controls[0]);
        }

        bool hasAnyUsed() const {
            return bitmask.hasAnyUsed(&this->controls[0]);
        }

        bool hasAnyUnused() const {
            return bitmask.hasAnyUnused(&this->controls[0]);
        }

        bool isAllEmpty() const {
            return bitmask.isAllEmpty(&this->controls[0]);
        }

        bool isAllDeleted() const {
            return bitmask.isAllDeleted(&this->controls[0]);
        }

        bool isAllEmptyOrDeleted() const {
            return bitmask.isAllEmptyOrDeleted(&this->controls[0]);
        }

        bool isAllUsed() const {
            return bitmask.isAllUsed(&this->controls[0]);
        }

        bool isAllUnused() const {
            return bitmask.isAllUnused(&this->controls[0]);
        }
    };

    typedef hash_cluster cluster_type;

#if 0
#if 1
    struct hash_entry : public value_type {
        hash_entry() : value_type() {
        }
        hash_entry(const hash_entry & src) : value_type(*static_cast<value_type *>(&src)) {
        }
        hash_entry(hash_entry && src) noexcept(std::is_nothrow_move_constructible<value_type>::value)
            : value_type(std::move(*static_cast<value_type *>(&src))) {
        }

        hash_entry(const value_type & val) : value_type(val) {
        }
        hash_entry(value_type && val) noexcept : value_type(std::move(val)) {
        }

        hash_entry(const nc_value_type & val) : value_type(val) {
        }
        hash_entry(nc_value_type && val) noexcept : value_type(std::move(val)) {
        }

        hash_entry(const key_type & key, const mapped_type & value) : value_type(key, value) {
        }
        hash_entry(const key_type & key, mapped_type && value)
            : value_type(key, std::forward<mapped_type>(value)) {
        }
        hash_entry(key_type && key, mapped_type && value) noexcept
            : value_type(std::forward<key_type>(key), std::forward<mapped_type>(value)) {
        }

#if 0
        template <typename ... Args>
        hash_entry(Args && ... args) : value_type(std::forward<Args>(args)...) {
        }
#endif

        virtual ~hash_entry() {
        }

        hash_entry & operator = (const hash_entry & rhs) {
            if (this != std::addressof(rhs)) {
                *this = rhs;
            }
            return *this;
        }

        hash_entry & operator = (hash_entry && rhs)
            noexcept(std::is_nothrow_move_assignable<value_type &&>::value) {
            if (this != std::addressof(rhs)) {
                *this = std::move(rhs);
            }
            return *this;
        }

        void swap(hash_entry & other) noexcept {
            if (this != std::addressof(other)) {
                std::swap(*this, other);
            }
        }
    };
#else
    struct hash_entry {
        value_type value;

        hash_entry() : value() {
        }
        hash_entry(const hash_entry & src) : value(src.value) {
        }
        hash_entry(hash_entry && src) noexcept : value(std::move(src.value)) {
        }

        hash_entry(const value_type & val) : value(val) {
        }
        hash_entry(value_type && val) noexcept : value(std::move(val)) {
        }

        hash_entry(const key_type & key, const mapped_type & value) : value(key, value) {
        }
        hash_entry(const key_type & key, mapped_type && value)
            : value(key, std::forward<mapped_type>(value)) {
        }
        hash_entry(key_type && key, mapped_type && value) noexcept
            : value(std::forward<key_type>(key), std::forward<mapped_type>(value)) {
        }

        template <typename ... Args>
        hash_entry(Args && ... args) : value(std::forward<Args>(args)...) {
        }

        ~hash_entry() {
        }

        hash_entry & operator = (const hash_entry & rhs) {
            if (this != std::addressof(rhs)) {
                this->value = rhs.value;
            }
            return *this;
        }

        hash_entry & operator = (hash_entry && rhs) noexcept {
            if (this != std::addressof(rhs)) {
                this->value = std::move(rhs.value);
            }
            return *this;
        }

        void swap(hash_entry & other) {
            if (this != std::addressof(other)) {
                std::swap(this->value, other.value);
            }
        }
    };
#endif

    inline void swap(hash_entry & lhs, hash_entry & rhs) noexcept {
        lhs.swap(rhs);
    }
#endif

    typedef value_type      entry_type;
    typedef value_type      node_type;

    template <typename ValueType>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = ValueType;
        using pointer = ValueType *;
        using reference = ValueType &;

        using remove_const_value_type = typename std::remove_const<ValueType>::type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

    private:
        control_byte * control_;
        entry_type *   entry_;

    public:
        basic_iterator() noexcept : control_(nullptr), entry_(nullptr) {
        }
        basic_iterator(control_byte * control, entry_type * entry) noexcept
            : control_(control), entry_(entry) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : control_(src.control_), entry_(src.entry_) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) {
            this->control_ = rhs.control_;
            this->entry_ = rhs.entry_;
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.entry_ == rhs.entry_);
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.entry_ != rhs.entry_);
        }

        basic_iterator & operator ++ () {
            do {
                ++(this->control_);
                ++(this->entry_);
            } while (control_->isEmptyOrDeleted());
            return *this;
        }

        basic_iterator operator ++ (int) {
            basic_iterator copy(*this);
            ++*this;
            return copy;
        }

        reference operator * () const {
            return *(this->entry_);
        }

        pointer operator -> () const {
            return std::addressof(*this->entry_);
        }

        operator basic_iterator<const remove_const_value_type>() const {
            return { this->control_, this->entry_ };
        }

        entry_type * value() {
            return this->entry_;
        }

        const entry_type * value() const {
            return this->entry_;
        }
    };

    using iterator       = basic_iterator<value_type>;
    using const_iterator = basic_iterator<const value_type>;

    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<entry_type>
                                        entry_allocator_type;

private:
    cluster_type *  clusters_;
    size_type       cluster_mask_;

    entry_type *    entries_;
    size_type       entry_size_;
    size_type       entry_mask_;

    size_type       entry_threshold_;
    float           load_factor_;

    hasher          hasher_;
    key_equal       key_equal_;

    allocator_type          allocator_;
    entry_allocator_type    entry_allocator_;

public:
    flat16_hash_map() : flat16_hash_map(kDefaultCapacity) {
    }

    explicit flat16_hash_map(size_type init_capacity,
                             const hasher & hash = hasher(),
                             const key_equal & equal = key_equal(),
                             const allocator_type & alloc = allocator_type()) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), entry_allocator_(alloc) {
        this->create_cluster<true>(init_capacity);
    }

    explicit flat16_hash_map(const allocator_type & alloc)
        : flat16_hash_map(kDefaultCapacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    flat16_hash_map(InputIter first, InputIter last,
                    size_type init_capacity = kDefaultCapacity,
                    const hasher & hash = hasher(),
                    const key_equal & equal = key_equal(),
                    const allocator_type & alloc = allocator_type()) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), entry_allocator_(alloc) {
        this->create_cluster<true>(init_capacity);
        this->insert(first, last);
    }

    template <typename InputIter>
    flat16_hash_map(InputIter first, InputIter last,
                    size_type init_capacity,
                    const allocator_type & alloc)
        : flat16_hash_map(first, last, init_capacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    flat16_hash_map(InputIter first, InputIter last,
                    size_type init_capacity,
                    const hasher & hash,
                    const allocator_type & alloc)
        : flat16_hash_map(first, last, init_capacity, hash, key_equal(), alloc) {
    }

    flat16_hash_map(const flat16_hash_map & other)
        : flat16_hash_map(other, std::allocator_traits<allocator_type>::
                                 select_on_container_copy_construction(other.get_allocator())) {
    }

    flat16_hash_map(const flat16_hash_map & other, const Allocator & alloc) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hasher()), key_equal_(key_equal()),
        allocator_(alloc), entry_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type other_size = other.entry_size();
        this->reserve_for_insert(other_size);
        try {
            this->insert_unique(other.begin(), other.end());
        } catch (const std::bad_alloc & ex) {
            this->destroy<true>();
            throw std::bad_alloc();
        } catch (...) {
            this->destroy<true>();
            throw;
        }
    }

    flat16_hash_map(flat16_hash_map && other) noexcept :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(std::move(other.hash_function())),
        key_equal_(std::move(other.key_eq())),
        allocator_(std::move(other.get_allocator())),
        entry_allocator_(std::move(other.get_entry_allocator())) {
        this->swap_content(other);
    }

    flat16_hash_map(flat16_hash_map && other, const Allocator & alloc) noexcept :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(std::move(other.hash_function())),
        key_equal_(std::move(other.key_eq())),
        allocator_(alloc),
        entry_allocator_(std::move(other.get_entry_allocator())) {
        this->swap_content(other);
    }

    flat16_hash_map(std::initializer_list<value_type> init_list,
                    size_type init_capacity = kDefaultCapacity,
                    const hasher & hash = hasher(),
                    const key_equal & equal = key_equal(),
                    const allocator_type & alloc = allocator_type()) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal), allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type new_capacity = (init_capacity >= init_list.size()) ? init_capacity : init_list.size();
        this->reserve_for_insert(new_capacity);
        this->insert(init_list.begin(), init_list.end());
    }

    flat16_hash_map(std::initializer_list<value_type> init_list,
                    size_type init_capacity,
                    const allocator_type & alloc)
        : flat16_hash_map(init_list, init_capacity, hasher(), key_equal(), alloc) {
    }

    flat16_hash_map(std::initializer_list<value_type> init_list,
                    size_type init_capacity,
                    const hasher & hash,
                    const allocator_type & alloc)
        : flat16_hash_map(init_list, init_capacity, hash, key_equal(), alloc) {
    }

    ~flat16_hash_map() {
        this->destroy<true>();
    }

    bool is_valid() const { return (this->clusters() != nullptr); }
    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const  { return (this->size() > this->entry_mask()); }

    bool empty() const { return this->is_empty(); }

    size_type size() const { return entry_size_; }
    size_type capacity() const { return (entry_mask_ + 1); }

    control_byte * controls() { return (control_byte *)clusters_; }
    const control_byte * controls() const { return (const control_byte *)clusters_; }

    cluster_type * clusters() { return clusters_; }
    const cluster_type * clusters() const { return clusters_; }

    size_type cluster_mask() const { return cluster_mask_; }
    size_type cluster_count() const { return (cluster_mask_ + 1); }

    entry_type * entries() { return entries_; }
    const entry_type * entries() const { return entries_; }

    size_type entry_size() const { return entry_size_; }
    size_type entry_mask() const { return entry_mask_; }
    size_type entry_capacity() const { return (entry_mask_ + 1); }
    size_type entry_threshold() const { return entry_threshold_; }

    constexpr size_type bucket_count() const {
        return kClusterWidth;
    }

    constexpr size_type bucket(const key_type & key) const {
        size_type index = this->find_impl(key);
        return ((index != npos) ? (index / kClusterWidth) : npos);
    }

    float load_factor() const {
        return ((float)this->entry_size() / this->entry_capacity());
    }

    void max_load_factor(float load_factor) {
        if (load_factor < kMinLoadFactor)
            load_factor = kMinLoadFactor;
        if (load_factor > kMaxLoadFactor)
            load_factor = kMaxLoadFactor;
        this->load_factor_ = load_factor;
        this->entry_threshold_ = (size_type)((float)this->entry_capacity() * load_factor);
    }

    float max_load_factor() const {
        return this->load_factor_;
    }

    float default_load_factor() const {
        return kDefaultLoadFactor;
    }

    iterator begin() {
#if 1
        cluster_type * cluster = this->clusters();
        cluster_type * last_cluster = this->clusters() + this->cluster_count();
        size_type start_index = 0;
        for (; cluster != last_cluster; cluster++) {
            std::uint32_t maskUsed = cluster->matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = start_index + pos;
                return this->iterator_at(index);
            }
            start_index += kClusterWidth;
        }

        return this->iterator_at(this->entry_capacity());
#else
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_mask(); index++) {
            if (control->isUsed()) {
                return { control, this->entry_at(index) };
            }
            control++;
        }
        return { control, this->entry_at(index) };
#endif
    }

    const_iterator begin() const {
        cluster_type * cluster = this->clusters();
        cluster_type * last_cluster = this->clusters() + this->cluster_count();
        size_type start_index = 0;
        for (; cluster != last_cluster; cluster++) {
            std::uint32_t maskUsed = cluster->matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = start_index + pos;
                return this->iterator_at(index);
            }
            start_index += kClusterWidth;
        }

        return this->iterator_at(this->entry_capacity());
    }

    const_iterator cbegin() const {
        return this->begin();
    }

    iterator end() {
        return iterator_at(this->entry_capacity());
    }

    const_iterator end() const {
        return iterator_at(this->entry_capacity());
    }

    const_iterator cend() const {
        return this->end();
    }

    allocator_type get_allocator() const noexcept {
        return this->allocator_;
    }

    entry_allocator_type get_entry_allocator() const noexcept {
        return this->entry_allocator_;
    }

    hasher hash_function() const {
        return this->hasher_;
    }

    key_equal key_eq() const {
        return this->key_equal_;
    }

    static const char * name() {
        return "jstd::v1::flat16_hash_map<K, V>";
    }

    void clear(bool need_destory = false) noexcept {
        if (this->entry_capacity() > kDefaultCapacity) {
            if (need_destory) {
                this->destroy<true>();
                this->create_cluster<false>(kDefaultCapacity);
                assert(this->entry_size() == 0);
                return;
            }
        }
        this->destroy<false>();
        assert(this->entry_size() == 0);
    }

    void reserve(size_type new_capacity, bool read_only = false) {
        this->rehash(new_capacity, read_only);
    }

    void resize(size_type new_capacity, bool read_only = false) {
        this->rehash(new_capacity, read_only);
    }

    void rehash(size_type new_capacity, bool read_only = false) {
        if (!read_only)
            new_capacity = (std::max)(this->capacity_for_reserve(new_capacity), this->entry_size());
        else
            new_capacity = (std::max)(new_capacity, this->entry_size());
        this->rehash_impl<true, false>(new_capacity);
    }

    void shrink_to_fit(bool read_only = false) {
        size_type new_capacity;
        if (!read_only)
            new_capacity = this->capacity_for_reserve(this->entry_size());
        else
            new_capacity = this->entry_size();
        this->rehash_impl<true, false>(new_capacity);
    }

    void swap(flat16_hash_map & other) {
        if (&other != this) {
            this->swap_impl(other);
        }
    }

    mapped_type & operator [] (const key_type & key) {
        return this->try_emplace(key).first->second;
    }

    mapped_type & operator [] (key_type && key) {
        return this->try_emplace(std::move(key)).first->second;
    }

    mapped_type & at(const key_type & key) {
        size_type index = this->find_impl(key);
        if (index != npos) {
            entry_type * entry = this->entry_at(index);
            return entry->second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::flat16_hash_map<K,V>::at(key), "
                                    "the specified key is not exists.");
        }
    }

    const mapped_type & at(const key_type & key) const {
        size_type index = this->find_impl(key);
        if (index != npos) {
            entry_type * entry = this->entry_at(index);
            return entry->second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::flat16_hash_map<K,V>::at(key) const, "
                                    "the specified key is not exists.");
        }
    }

    size_type count(const key_type & key) const {
        size_type index = this->find_impl(key);
        return (index != npos) ? size_type(1) : size_type(0);
    }

    bool contains(const key_type & key) const {
        size_type index = this->find_impl(key);
        return (index != npos);
    }

    iterator find(const key_type & key) {
        size_type index = this->find_impl(key);
        if (index != npos)
            return this->iterator_at(index);
        else
            return this->end();
    }

    const_iterator find(const key_type & key) const {
        return const_cast<this_type *>(this)->find(key);
    }

    std::pair<iterator, iterator> equal_range(const key_type & key) {
        iterator iter = this->find(key);
        if (iter != this->end())
            return { iter, std::next(iter) };
        else
            return { iter, iter };
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type & key) const {
        const_iterator iter = this->find(key);
        if (iter != this->end())
            return { iter, std::next(iter) };
        else
            return { iter, iter };
    }

    std::pair<iterator, bool> insert(const value_type & value) {
        return this->emplace_impl<false>(value);
    }

    std::pair<iterator, bool> insert(value_type && value) {
        return this->emplace_impl<false>(std::move(value));
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value) &&
              (!jstd::is_same_ex<P, nc_value_type>::value) &&
              std::is_constructible<value_type, P &&>::value>::type * = nullptr>
    std::pair<iterator, bool> insert(P && value) {
        return this->emplace_impl<false>(std::forward<P>(value));
    }

    iterator insert(const_iterator hint, const value_type & value) {
        return this->emplace_impl<false>(value).first;
    }

    iterator insert(const_iterator hint, value_type && value) {
        return this->emplace_impl<false>(std::move(value)).first;
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value) &&
              (!jstd::is_same_ex<P, nc_value_type>::value) &&
              std::is_constructible<value_type, P &&>::value>::type * = nullptr>
    std::pair<iterator, bool> insert(const_iterator hint, P && value) {
        return this->emplace_impl<false>(std::forward<P>(value));
    }

    template <typename InputIter>
    void insert(InputIter first, InputIter last) {
        for (; first != last; ++first) {
            this->emplace_impl<false>(*first);
        }
    }

    void insert(std::initializer_list<value_type> ilist) {
        this->insert(ilist.begin(), ilist.end());
    }

    template <typename KeyType, typename MappedType>
    std::pair<iterator, bool> insert_or_assign(const KeyType & key, MappedType && value) {
        return this->emplace_impl<true>(key, std::forward<MappedType>(value));
    }

    template <typename KeyType, typename MappedType>
    std::pair<iterator, bool> insert_or_assign(KeyType && key, MappedType && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedType>(value));
    }

    template <typename KeyType, typename MappedType>
    iterator insert_or_assign(const_iterator hint, const KeyType & key, MappedType && value) {
        return this->emplace_impl<true>(key, std::forward<MappedType>(value))->first;
    }

    template <typename KeyType, typename MappedType>
    iterator insert_or_assign(const_iterator hint, KeyType && key, MappedType && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedType>(value))->first;
    }

    std::pair<iterator, bool> insert_always(const value_type & value) {
        return this->emplace_impl<true>(value);
    }

    std::pair<iterator, bool> insert_always(value_type && value) {
        return this->emplace_impl<true>(std::move(value));
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value) &&
              (!jstd::is_same_ex<P, nc_value_type>::value) &&
              std::is_constructible<value_type, P &&>::value>::type * = nullptr>
    std::pair<iterator, bool> insert_always(P && value) {
        return this->emplace_impl<true>(std::forward<P>(value));
    }

    template <typename ... Args>
    std::pair<iterator, bool> emplace(Args && ... args) {
        return this->emplace_impl<false>(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    iterator emplace_hint(const_iterator hint, Args && ... args) {
        return this->emplace_impl<false>(std::forward<Args>(args)...).first;
    }

    template <typename ... Args>
    std::pair<iterator, bool> try_emplace(const key_type & key, Args && ... args) {
        return this->emplace_impl<false>(key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    std::pair<iterator, bool> try_emplace(key_type && key, Args && ... args) {
        return this->emplace_impl<false>(std::forward<key_type>(key), std::forward<Args>(args)...);
    }

    template <typename ... Args>
    std::pair<iterator, bool> try_emplace(const_iterator hint, const key_type & key, Args && ... args) {
        return this->emplace_impl<false>(key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    std::pair<iterator, bool> try_emplace(const_iterator hint, key_type && key, Args && ... args) {
        return this->emplace_impl<false>(std::forward<key_type>(key), std::forward<Args>(args)...);
    }

    size_type erase(const key_type & key) {
        size_type num_deleted = this->find_and_erase(key);
        return num_deleted;
    }

    iterator erase(iterator pos) {
        size_type index = this->index_of(pos);
        this->erase_entry(pos);
        return ++pos;
    }

    const_iterator erase(const_iterator pos) {
        size_type index = this->index_of(pos);
        this->erase_entry(index);
        return ++pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        for (; first != last; ++first) {
            size_type index = this->index_of(first);
            this->erase_entry(index);
        }
        return { first };
    }

private:
    JSTD_FORCED_INLINE
    size_type calc_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = (std::max)(init_capacity, kMinimumCapacity);
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinimumCapacity>(new_capacity);
        }
        return new_capacity;
    }

    JSTD_FORCED_INLINE
    size_type capacity_for_reserve(size_type init_capacity) {
        size_type new_capacity = (size_type)std::ceil((float)init_capacity / this->max_load_factor());
        return new_capacity;
    }

    bool is_positive(size_type value) const {
        return (std::intptr_t(value) >= 0);
    }

    iterator iterator_at(size_type index) noexcept {
        return { this->control_at(index), this->entry_at(index) };
    }

    const_iterator iterator_at(size_type index) const noexcept {
        return { this->control_at(index), this->entry_at(index) };
    }

    inline hash_code_t get_hash(const key_type & key) const noexcept {
        hash_code_t hash_code = static_cast<hash_code_t>(this->hasher_(key));
        return hash_code;
    }

    inline hash_code_t get_second_hash(hash_code_t value) const noexcept {
        hash_code_t hash_code;
        if (sizeof(size_type) == 4)
            hash_code = (hash_code_t)((size_type)value * 2654435761ul);
        else
            hash_code = (hash_code_t)((size_type)value * 14695981039346656037ull);
        return hash_code;
    }

    inline std::uint8_t get_control_hash(hash_code_t hash_code) const noexcept {
        return static_cast<std::uint8_t>(hash_code & kControlHashMask);
    }

    inline index_type index_for(hash_code_t hash_code) const noexcept {
        return (index_type)((size_type)hash_code & this->cluster_mask());
    }

    inline index_type index_for(hash_code_t hash_code, size_type cluster_mask) const noexcept {
        return (index_type)((size_type)hash_code & cluster_mask);
    }

    inline index_type prev_cluster(index_type cluster_index) const noexcept {
        return (index_type)(((size_type)cluster_index + this->cluster_mask()) & this->cluster_mask());
    }

    inline index_type next_cluster(index_type cluster_index) const noexcept {
        return (index_type)((size_type)(cluster_index + 1) & this->cluster_mask());
    }

    control_byte * control_at(size_type index) noexcept {
        assert(index <= this->entry_capacity());
        return (this->controls() + index);
    }

    const control_byte * control_at(size_type index) const noexcept {
        assert(index <= this->entry_capacity());
        return (this->controls() + index);
    }

    cluster_type * cluster_at(size_type cluster_index) noexcept {
        assert(cluster_index < this->cluster_count());
        return (this->clusters() + cluster_index);
    }

    const cluster_type * cluster_at(size_type cluster_index) const noexcept {
        assert(cluster_index < this->cluster_count());
        return (this->clusters() + cluster_index);
    }

    entry_type * entry_at(size_type index) noexcept {
        assert(index <= this->entry_capacity());
        return (this->entries() + index);
    }

    const entry_type * entry_at(size_type index) const noexcept {
        assert(index <= this->entry_capacity());
        return (this->entries() + index);
    }

    control_byte & get_control(size_type index) {
        assert(index < this->entry_capacity());
        control_byte * controls = this->controls();
        return controls[index];
    }

    const control_byte & get_control(size_type index) const {
        assert(index < this->entry_capacity());
        control_byte * controls = this->controls();
        return controls[index];
    }

    cluster_type & get_cluster(size_type cluster_index) {
        assert(cluster_index < this->cluster_count());
        return this->clusters_[cluster_index];
    }

    const cluster_type & get_cluster(size_type cluster_index) const {
        assert(cluster_index < this->cluster_count());
        return this->clusters_[cluster_index];
    }

    entry_type & get_entry(size_type index) {
        assert(index < this->entry_capacity());
        return this->entries_[index];
    }

    const entry_type & get_entry(size_type index) const {
        assert(index < this->entry_capacity());
        return this->entries_[index];
    }

    size_type index_of(iterator pos) const {
        return this->index_of(pos.value());
    }

    size_type index_of(const_iterator pos) const {
        return this->index_of(pos.value());
    }

    index_type index_of(entry_type * entry) const {
        assert(entry != nullptr);
        assert(entry >= this->entries());
        index_type index = (index_type)(entry - this->entries());
        assert(is_positive(index));
        return index;
    }

    index_type index_of(const entry_type * entry) const {
        return this->index_of((entry_type *)entry);
    }

    bool entry_is_used(entry_type * entry) const {
        index_type index = this->index_of(entry);
        return this->control_is_used(index);
    }

    bool control_is_used(size_type index) const {
        control_byte * control = this->control_at(index);
        return control->isUsed();
    }

    template <bool finitial>
    void destroy() noexcept {
        this->destory_entries<finitial>();

        // Note!!: destory_entries() need use this->clusters()
        this->destory_cluster<finitial>();
    }

    template <bool finitial>
    void destory_cluster() noexcept {
        if (this->clusters_ != nullptr) {
            if (!finitial) {
                for (size_type index = 0; index <= this->cluster_mask(); index++) {
                    cluster_type * cluster = this->cluster_at(index);
                    cluster->clear();
                }
            }
            if (finitial) {
                delete[] this->clusters_;
                this->clusters_ = nullptr;
            }
        }
    }

    template <bool finitial>
    void destory_entries() noexcept {
        // Destroy all entries.
        if (this->entries_ != nullptr) {
            if (!entry_is_trivial_destructor) {
                control_byte * control = this->controls();
                for (size_type index = 0; index <= this->entry_mask(); index++) {
                    if (control->isUsed()) {
                        entry_type * entry = this->entry_at(index);
                        this->entry_allocator_.destroy(entry);
                    }
                    control++;
                }
            }
            if (finitial) {
                this->entry_allocator_.deallocate(this->entries_, this->entry_capacity());
                this->entries_ = nullptr;
            }
        }

        this->entry_size_ = 0;
    }

    template <bool initialize = false>
    void create_cluster(size_type init_capacity) {
        size_type new_capacity;
        if (initialize)
            new_capacity = calc_capacity(init_capacity);
        else
            new_capacity = init_capacity;
        assert(new_capacity > 0);
        assert(new_capacity >= kMinimumCapacity);

        size_type cluster_count = (new_capacity + (kClusterWidth - 1)) / kClusterWidth;
        assert(cluster_count > 0);
        cluster_type * clusters = new cluster_type[cluster_count + 1];
        clusters_ = clusters;
        cluster_mask_ = cluster_count - 1;

        if (cluster_count == 1) {
            cluster_type * tail_group = (cluster_type *)((char *)clusters + kClusterWidth + new_capacity);
            (*clusters).template fillAll8<kEndOfMark>();
        }
        clusters[cluster_count].template fillAll8<kEndOfMark>();

        entry_type * entries = entry_allocator_.allocate(new_capacity);
        entries_ = entries;
        if (initialize) {
            assert(entry_size_ == 0);
        } else {
            entry_size_ = 0;
        }
        entry_mask_ = new_capacity - 1;
        entry_threshold_ = (size_type)((float)new_capacity * this->max_load_factor());
    }

    inline bool need_grow() const {
        return (this->entry_size_ > this->entry_threshold_);
    }

    void grow_if_necessary() {
        size_type new_capacity = (this->entry_mask_ + 1) * 2;
        this->rehash_impl<false, true>(new_capacity);
    }

    JSTD_FORCED_INLINE
    void reserve_for_insert(size_type init_capacity) {
        size_type new_capacity = this->capacity_for_reserve(init_capacity);
        this->create_cluster<true>(new_capacity);
    }

    template <bool AllowShrink, bool AlwaysResize>
    void rehash_impl(size_type new_capacity) {
        new_capacity = this->calc_capacity(new_capacity);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinimumCapacity);
        if (AlwaysResize ||
            (!AllowShrink && (new_capacity > this->entry_capacity())) ||
            (AllowShrink && (new_capacity != this->entry_capacity()))) {
            if (!AlwaysResize && !AllowShrink) {
                assert(new_capacity >= this->entry_size());
            }

            cluster_type * old_clusters = this->clusters();
            control_byte * old_controls = this->controls();
            size_type old_cluster_count = this->cluster_count();

            entry_type * old_entries = this->entries();
            size_type old_entry_size = this->entry_size();
            size_type old_entry_capacity = this->entry_capacity();

            this->create_cluster<false>(new_capacity);

            //if (old_entry_capacity >= kClusterWidth)
            {
                cluster_type * last_cluster = old_clusters + old_cluster_count;
                entry_type * entry_start = old_entries;
                for (cluster_type * cluster = old_clusters; cluster != last_cluster; cluster++) {
                    std::uint32_t maskUsed = cluster->matchUsed();
                    while (maskUsed != 0) {
                        size_type pos = BitUtils::bsf32(maskUsed);
                        maskUsed = BitUtils::clearLowBit32(maskUsed);
                        entry_type * entry = entry_start + pos;
                        this->move_insert_unique(entry);
                        this->entry_allocator_.destroy(entry);
                    }
                    entry_start += kClusterWidth;
                }
            }
            assert(this->entry_size() == old_entry_size);

            this->entry_allocator_.deallocate(old_entries, old_entry_capacity);

            if (old_clusters != nullptr) {
                delete[] old_clusters;
            }
        }
    }

    size_type find_impl(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type first_cluster = cluster_index;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterWidth;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    return index;
                }
            }
            if (cluster.hasAnyEmpty()) {
                return npos;
            }
            cluster_index = this->next_cluster(cluster_index);
        } while (cluster_index != first_cluster);

        return npos;
    }

    JSTD_FORCED_INLINE
    size_type find_impl(const key_type & key, index_type & first_cluster,
                        index_type & last_cluster, std::uint8_t & ctrl_hash) const {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_cluster = cluster_index;
        first_cluster = start_cluster;
        ctrl_hash = control_hash;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterWidth;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    last_cluster = cluster_index;
                    return index;
                }
            }
            if (cluster.hasAnyEmpty()) {
                last_cluster = cluster_index;
                return npos;
            }
            cluster_index = this->next_cluster(cluster_index);
        } while (cluster_index != start_cluster);

        last_cluster = npos;
        return npos;
    }

    JSTD_FORCED_INLINE
    size_type find_first_unused_entry(const key_type & key, std::uint8_t & ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type first_cluster = cluster_index;
        ctrl_hash = control_hash;

        // Find the first unused entry and insert
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t maskUnused = cluster.matchEmptyOrDeleted();
            if (maskUnused != 0) {
                // Found a [EmptyEntry] or [DeletedEntry] to insert
                size_type pos = BitUtils::bsf32(maskUnused);
                size_type start_index = cluster_index * kClusterWidth;
                return (start_index + pos);
            }
            cluster_index = this->next_cluster(cluster_index);
            assert(cluster_index != first_cluster);
        } while (1);

        return npos;
    }

    JSTD_FORCED_INLINE
    size_type find_first_empty_entry(const key_type & key, std::uint8_t & ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type first_cluster = cluster_index;
        ctrl_hash = control_hash;

        // Find the first empty entry and insert
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t maskEmpty = cluster.matchEmpty();
            if (maskEmpty != 0) {
                // Found a [EmptyEntry] to insert
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type start_index = cluster_index * kClusterWidth;
                return (start_index + pos);
            }
            cluster_index = this->next_cluster(cluster_index);
            assert(cluster_index != first_cluster);
        } while (1);

        return npos;
    }

    // Use in rehash_impl()
    void move_insert_unique(entry_type * value) {
        std::uint8_t ctrl_hash;
        size_type target = this->find_first_empty_entry(value->first, ctrl_hash);
        assert(target != npos);

        // Found a [DeletedEntry] or [EmptyEntry] to insert
        control_byte * control = this->control_at(target);
        assert(control->isEmpty());
        control->setUsed(ctrl_hash);
        entry_type * entry = this->entry_at(target);
        this->entry_allocator_.construct(entry,
              std::move(*static_cast<value_type *>(value)));
        this->entry_size_++;
        assert(this->entry_size() <= this->entry_capacity());
    }

    void insert_unique(const value_type & value) {
        std::uint8_t ctrl_hash;
        size_type target = this->find_first_empty_entry(value.first, ctrl_hash);
        assert(target != npos);

        // Found a [DeletedEntry] or [EmptyEntry] to insert
        control_byte * control = this->control_at(target);
        assert(control->isEmpty());
        control->setUsed(ctrl_hash);
        entry_type * entry = this->entry_at(target);
        this->entry_allocator_.construct(entry, value);
        this->entry_size_++;
        assert(this->entry_size() <= this->entry_capacity());
    }

    void insert_unique(value_type && value) {
        std::uint8_t ctrl_hash;
        size_type target = this->find_first_empty_entry(value.first, ctrl_hash);
        assert(target != npos);

        // Found a [DeletedEntry] or [EmptyEntry] to insert
        control_byte * control = this->control_at(target);
        assert(control->isEmpty());
        control->setUsed(ctrl_hash);
        entry_type * entry = this->entry_at(target);
        this->entry_allocator_.construct(entry, std::move(value));
        this->entry_size_++;
        assert(this->entry_size() <= this->entry_capacity());
    }

    // Use in constructor
    template <typename InputIter>
    void insert_unique(InputIter first, InputIter last) {
        for (InputIter iter = first; iter != last; ++iter) {
            this->insert_unique(static_cast<value_type>(*iter));
        }
    }

    std::pair<size_type, bool>
    find_and_prepare_insert(const key_type & key, std::uint8_t & ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type first_cluster = cluster_index;
        index_type last_cluster = npos;
        std::uint32_t maskEmpty;
        ctrl_hash = control_hash;

        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterWidth;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    return { index, true };
                }
            }
            maskEmpty = cluster.matchEmpty();
            if (maskEmpty != 0) {
                last_cluster = cluster_index;
                break;
            }
            cluster_index = this->next_cluster(cluster_index);
        } while (cluster_index != first_cluster);

        if (this->need_grow() || (last_cluster == npos)) {
            // The size of entry reach the entry threshold or hashmap is full.
            this->grow_if_necessary();

            return this->find_and_prepare_insert(key, ctrl_hash);
        }

        // Find the first deleted or empty entry and insert
        if (cluster_index != first_cluster) {
            // Find the first [DeletedEntry] from first_cluster to last_cluster.
            cluster_index = first_cluster;
            do {
                const cluster_type & cluster = this->get_cluster(cluster_index);
                std::uint32_t maskDeleted = cluster.matchDeleted();
                if (maskDeleted != 0) {
                    // Found a [DeletedEntry] to insert
                    size_type pos = BitUtils::bsf32(maskDeleted);
                    size_type start_index = cluster_index * kClusterWidth;
                    return { (start_index + pos), false };
                }
                if (cluster_index == last_cluster)
                    break;
                cluster_index = this->next_cluster(cluster_index);
            } while (cluster_index != first_cluster);

            // Not found any [DeletedEntry], so we use [EmptyEntry] to insert
            // Skip to final processing
        } else {
            assert(last_cluster != npos);
            const cluster_type & cluster = this->get_cluster(last_cluster);
            std::uint32_t maskDeleted = cluster.matchDeleted();
            if (maskDeleted != 0) {
                // Found a [DeletedEntry] to insert
                maskEmpty = maskDeleted;
            } else {
                // Found a [EmptyEntry] to insert
                assert(maskEmpty != 0);
            }
        }

        // It's a [EmptyEntry] or [DeletedEntry] to insert
        size_type pos = BitUtils::bsf32(maskEmpty);
        size_type start_index = last_cluster * kClusterWidth;
        return { (start_index + pos), false };
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(const value_type & value) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(value.first, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, value);
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                entry_type * entry = this->entry_at(target);
                entry->second = value.second;
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(value_type && value) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(value.first, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, std::forward<value_type>(value));
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                static constexpr bool is_rvalue_ref = std::is_rvalue_reference<decltype(value)>::value;
                entry_type * entry = this->entry_at(target);
                if (is_rvalue_ref)
                    entry->second = std::move(value.second);
                else
                    entry->second = value.second;
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename KeyT, typename MappedT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, nc_value_type>::value &&
               !std::is_constructible<nc_value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
              (jstd::is_same_ex<KeyT, key_type>::value ||
               std::is_constructible<key_type, KeyT &&>::value) &&
              (jstd::is_same_ex<MappedT, mapped_type>::value ||
               std::is_constructible<mapped_type, MappedT &&>::value)>::type * = nullptr>
    std::pair<iterator, bool> emplace_impl(KeyT && key, MappedT && value) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(key, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, std::forward<KeyT>(key),
                                                    std::forward<MappedT>(value));
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            static constexpr bool isMappedType = jstd::is_same_ex<MappedT, mapped_type>::value;
            if (AlwaysUpdate) {
                if (isMappedType) {
                    entry_type * entry = this->entry_at(target);
                    entry->second = std::forward<MappedT>(value);
                } else {
                    mapped_type mapped_value(std::forward<MappedT>(value));
                    entry_type * entry = this->entry_at(target);
                    entry->second = std::move(mapped_value);
                }
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, nc_value_type>::value &&
               !std::is_constructible<nc_value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
              (jstd::is_same_ex<KeyT, key_type>::value ||
               std::is_constructible<key_type, KeyT &&>::value)>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(KeyT && key, Args && ... args) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(key, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, std::piecewise_construct,
                                                    std::forward_as_tuple(std::forward<KeyT>(key)),
                                                    std::forward_as_tuple(std::forward<Args>(args)...));
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                mapped_type mapped_value(std::forward<Args>(args)...);
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(mapped_value);
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename PieceWise, typename std::enable_if<
              (!jstd::is_same_ex<PieceWise, value_type>::value &&
               !std::is_constructible<value_type, PieceWise &&>::value) &&
              (!jstd::is_same_ex<PieceWise, nc_value_type>::value &&
               !std::is_constructible<nc_value_type, PieceWise &&>::value) &&
              jstd::is_same_ex<PieceWise, std::piecewise_construct_t>::value &&
              (!jstd::is_same_ex<PieceWise, key_type>::value &&
               !std::is_constructible<key_type, PieceWise &&>::value)>::type * = nullptr,
              typename ... Ts1, typename ... Ts2>
    std::pair<iterator, bool> emplace_impl(PieceWise && hint,
                                           std::tuple<Ts1...> && first,
                                           std::tuple<Ts2...> && second) {
        std::uint8_t ctrl_hash;
        tuple_wrapper2<key_type> key_wrapper(first);
        //break_from_tuple(first);
        //break_from_tuple(second);
        auto find_info = this->find_and_prepare_insert(key_wrapper.value(), ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, std::piecewise_construct,
                                                    std::forward<std::tuple<Ts1...>>(first),
                                                    std::forward<std::tuple<Ts2...>>(second));
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                tuple_wrapper2<mapped_type> mapped_wrapper(std::move(second));
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(mapped_wrapper.value());
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename First, typename std::enable_if<
              (!jstd::is_same_ex<First, value_type>::value &&
               !std::is_constructible<value_type, First &&>::value) &&
              (!jstd::is_same_ex<First, nc_value_type>::value &&
               !std::is_constructible<nc_value_type, First &&>::value) &&
              (!jstd::is_same_ex<First, std::piecewise_construct_t>::value) &&
              (!jstd::is_same_ex<First, key_type>::value &&
               !std::is_constructible<key_type, First &&>::value)>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(First && first, Args && ... args) {
        std::uint8_t ctrl_hash;
        value_type value(std::forward<First>(first), std::forward<Args>(args)...);
        auto find_info = this->find_and_prepare_insert(value.first, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            // Found a [DeletedEntry] or [EmptyEntry] to insert
            control_byte * control = this->control_at(target);
            assert(control->isEmptyOrDeleted());
            control->setUsed(ctrl_hash);
            entry_type * entry = this->entry_at(target);
            this->entry_allocator_.construct(entry, std::move(value));
            this->entry_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(value.second);
            }
            return { this->iterator_at(target), false };
        }
    }

    JSTD_FORCED_INLINE
    size_type find_and_erase(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        hash_code_t hash_code_2nd = this->get_second_hash(hash_code);
        std::uint8_t control_hash = this->get_control_hash(hash_code_2nd);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_cluster = cluster_index;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterWidth;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    this->erase_entry(index);
                    return 1;
                }
            }
            if (cluster.hasAnyEmpty()) {
                return 0;
            }
            cluster_index = this->next_cluster(cluster_index);
        } while (cluster_index != start_cluster);

        return 0;
    }

    JSTD_FORCED_INLINE
    void erase_entry(size_type index) {
        assert(index <= this->entry_capacity());
        control_byte & control = this->get_control(index);
        assert(control.isUsed());
        index_type start_cluster = (index_type)(index / kClusterWidth);
        const cluster_type & cluster = this->get_cluster(start_cluster);
        if (cluster.hasAnyEmpty()) {
            control.setEmpty();
        } else {
            index_type cluster_index = this->next_cluster(start_cluster);
            if (cluster_index != start_cluster) {
                const cluster_type & cluster = this->get_cluster(cluster_index);
                if (!cluster.isAllEmpty())
                    control.setDeleted();
                else
                    control.setEmpty();
            } else {
                control.setEmpty();
            }
        }
        // Destroy entry
        this->entry_allocator_.destroy(this->entry_at(index));
        assert(this->entry_size_ > 0);
        this->entry_size_--;
    }

    void swap_content(flat16_hash_map & other) {
        std::swap(this->clusters_, other.clusters());
        std::swap(this->cluster_mask_, other.cluster_mask());
        std::swap(this->entries_, other.entries());
        std::swap(this->entry_size_, other.entry_size());
        std::swap(this->entry_mask_, other.entry_mask());
        std::swap(this->entry_threshold_, other.entry_threshold());
        std::swap(this->load_factor_, other.max_load_factor());
    }

    void swap_policy(flat16_hash_map & other) {
        std::swap(this->hasher_, other.hash_function());
        std::swap(this->key_equal_, other.key_eq());
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
            std::swap(this->allocator_, other.get_allocator());
        }
        if (std::allocator_traits<entry_allocator_type>::propagate_on_container_swap::value) {
            std::swap(this->entry_allocator_, other.get_entry_allocator());
        }
    }

    void swap_impl(flat16_hash_map & other) {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

} // namespace v1
} // namespace jstd
