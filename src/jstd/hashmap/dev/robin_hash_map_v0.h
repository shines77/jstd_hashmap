
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
#include <string.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::ptrdiff_t, std::size_t
#include <cstdbool>
#include <cassert>
#include <cmath>        // For std::ceil()
#include <memory>       // For std::swap(), std::pointer_traits<T>
#include <limits>       // For std::numeric_limits<T>
#include <cstring>      // For std::memset(), std::memcpy()
#include <vector>
#include <utility>      // For std::pair<First, Second>, std::integer_sequence<T...>
#include <tuple>        // For std::tuple<Ts...>
#include <initializer_list>
#include <algorithm>    // For std::max(), std::min()
#include <type_traits>
#include <stdexcept>

#include "jstd/basic/stddef.h"
#include "jstd/type_traits.h"
#include "jstd/iterator.h"
#include "jstd/utility.h"
#include "jstd/lang/launder.h"
#include "jstd/hasher/hashes.h"
#include "jstd/hasher/hash_crc32c.h"
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

#ifndef __AVX2__
#define __AVX2__
#endif
#endif // _MSC_VER

#define ROBIN_USE_HASH_POLICY     0

namespace jstd {

template < typename Key, typename Value,
           typename Hash = std::hash<typename std::remove_cv<Key>::type>,
           typename KeyEqual = std::equal_to<typename std::remove_cv<Key>::type>,
           typename Allocator = std::allocator<std::pair<typename std::add_const<typename std::remove_cv<Key>::type>::type,
                                                         typename std::remove_cv<Value>::type>> >
class robin_hash_map {
public:
    typedef typename std::remove_cv<Key>::type      key_type;
    typedef typename std::remove_cv<Value>::type    mapped_type;

    typedef std::pair<const key_type, mapped_type>  value_type;
    typedef std::pair<key_type, mapped_type>        mutable_value_type;

    static constexpr bool kIsCompatibleLayout =
            std::is_same<value_type, mutable_value_type>::value ||
            is_compatible_layout<value_type, mutable_value_type>::value;

    typedef typename std::conditional<kIsCompatibleLayout, mutable_value_type, value_type>::type
                                                    actual_value_type;

    typedef Hash                                    hasher;
    typedef KeyEqual                                key_equal;
    typedef Allocator                               allocator_type;
    typedef typename Hash::result_type              hash_result_t;
    typedef typename hash_policy_selector<Hash>::type
                                                    hash_policy_t;

    typedef std::size_t                             size_type;
    typedef std::intptr_t                           ssize_type;
    typedef std::size_t                             hash_code_t;
    typedef robin_hash_map<Key, Value, Hash, KeyEqual, Allocator>
                                                    this_type;

    static constexpr bool kUseIndexSalt = false;

    enum {
        UnrollLoopDisable,
        UnrollMode8,
        UnrollMode16
    };
    static constexpr int kUnrollMode = UnrollMode16;

    static constexpr size_type npos = size_type(-1);

    static constexpr size_type kCtrlHashMask = 0x000000FFul;
    static constexpr size_type kCtrlShift    = 8;

    static constexpr size_type kGroupBits   = 4;
    static constexpr size_type kGroupWidth  = size_type(1) << kGroupBits;
    static constexpr size_type kGroupMask   = kGroupWidth - 1;
    static constexpr size_type kGroupShift  = kCtrlShift + kGroupBits;

    static constexpr size_type kDefaultCapacity = 0;
    // kMinCapacity must be >= 2
    static constexpr size_type kMinCapacity = 4;

    static constexpr size_type kMinLookups = 4;

    static constexpr float kMinLoadFactor = 0.2f;
    static constexpr float kMaxLoadFactor = 0.8f;

    // Must be kMinLoadFactor <= loadFactor <= kMaxLoadFactor
    static constexpr float kDefaultLoadFactor = 0.5f;

    // Don't modify this value at will
    static constexpr size_type kLoadFactorAmplify = 65536;

    static constexpr std::uint32_t kDefaultLoadFactorInt =
            std::uint32_t(kDefaultLoadFactor * kLoadFactorAmplify);
    static constexpr std::uint32_t kDefaultLoadFactorRevInt =
            std::uint32_t(1.0f / kDefaultLoadFactor * kLoadFactorAmplify);

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
    static constexpr bool isGccOrClang = true;
#else
    static constexpr bool isGccOrClang = false;
#endif
    static constexpr bool isPlaneKeyHash = isGccOrClang &&
                                           std::is_same<Hash, std::hash<key_type>>::value &&
                                          (std::is_arithmetic<key_type>::value ||
                                           std::is_enum<key_type>::value);

    static constexpr bool is_slot_trivial_copyable =
            (std::is_trivially_copyable<actual_value_type>::value ||
            (std::is_trivially_copyable<key_type>::value &&
             std::is_trivially_copyable<mapped_type>::value) ||
            (std::is_scalar<key_type>::value && std::is_scalar<mapped_type>::value));

    static constexpr bool is_slot_trivial_destructor =
            (std::is_trivially_destructible<actual_value_type>::value ||
            (std::is_trivially_destructible<key_type>::value &&
             std::is_trivially_destructible<mapped_type>::value) ||
           ((std::is_arithmetic<key_type>::value || std::is_enum<key_type>::value) &&
            (std::is_arithmetic<mapped_type>::value || std::is_enum<mapped_type>::value)));

    static constexpr std::int8_t kEmptySlot     = (std::int8_t)0b11111111;
    static constexpr std::int8_t kEndOfMark     = (std::int8_t)0b11111110;
    static constexpr std::int8_t kUnusedMask    = (std::int8_t)0b10000000;
    static constexpr std::int8_t kHash2Mask     = (std::int8_t)0b11111111;
    static constexpr std::int8_t kMaxDist       = (std::int8_t)0b01111111;

    static constexpr std::int16_t kEmptySlot16  = std::int16_t(std::uint16_t((std::uint8_t)kEmptySlot)  << 8);
    static constexpr std::int16_t kEndOfMark16  = std::int16_t(std::uint16_t((std::uint8_t)kEndOfMark)  << 8);
    static constexpr std::int16_t kUnusedMask16 = std::int16_t(std::uint16_t((std::uint8_t)kUnusedMask) << 8);
    static constexpr std::int16_t kHash2Mask16  = std::int16_t(std::uint16_t((std::uint8_t)kHash2Mask)  << 8);
    static constexpr std::int16_t kMaxDist16    = std::int16_t(std::uint16_t((std::uint8_t)kMaxDist)    << 8);

    static constexpr std::int16_t  kDistInc16   = std::int16_t(0x0100);
    static constexpr std::uint16_t kDistInc16u  = std::uint16_t(0x0100);

    static constexpr std::uint32_t kFullMask16   = 0x0000FFFFul;
    static constexpr std::uint32_t kFullMask32   = 0xFFFFFFFFul;
    static constexpr std::uint32_t kFullMask32_Half  = 0x55555555ul;
    static constexpr std::uint32_t kFullMask32_Half2 = 0xAAAAAAAAul;

    static constexpr std::uint64_t kEmptySlot64  = 0xFFFFFFFFFFFFFFFFull;
    static constexpr std::uint64_t kEndOfMark64  = 0xFEFEFEFEFEFEFEFEull;
    static constexpr std::uint64_t kUnusedMask64 = 0x8080808080808080ull;

    struct ctrl_data {
        union {
            struct {
                std::uint8_t hash;
                std::int8_t  dist;
            };
            std::int16_t  value;
            std::uint16_t uvalue;
        };

        static constexpr std::uint8_t kMinUnusedSlot = cmin(kEmptySlot, kEndOfMark);

        ctrl_data() noexcept {
        }

        explicit ctrl_data(std::int16_t value) noexcept
            : value(value) {
        }

        explicit ctrl_data(std::uint16_t value) noexcept
            : uvalue(value) {
        }

        ctrl_data(std::int8_t dist, std::uint8_t hash) noexcept
            : hash(hash), dist(dist) {
        }

        ctrl_data(const ctrl_data & src) noexcept : value(src.value) {
        }

        ~ctrl_data() = default;

        ctrl_data & operator = (const ctrl_data & rhs) {
            this->value = rhs.value;
            return *this;
        }

        static std::int16_t make(std::int8_t dist, std::uint8_t hash) {
            return std::int16_t((std::uint16_t((std::uint8_t)dist) << 8) | hash);
        }

        static std::uint16_t makeu(std::int8_t dist, std::uint8_t hash) {
            return std::uint16_t((std::uint16_t((std::uint8_t)dist) << 8) | hash);
        }

        bool isEmpty() const {
            return this->isUnused();
        }

        static bool isEmpty(std::int8_t tag) {
            return ctrl_data::isUnused(tag);
        }

        bool isEmptyOnly() const {
            return (this->dist == kEmptySlot);
        }

        static bool isEmptyOnly(std::int8_t tag) {
            return (tag == kEmptySlot);
        }

        bool isNonEmpty() const {
            return !(this->isEmpty());
        }

        static bool isNonEmpty(std::int8_t tag) {
            return !ctrl_data::isEmpty(tag);
        }

        bool isEmptyOrZero() const {
#if 0
            return (std::int8_t(this->dist) <= 0);
#else
            return (this->value < std::int16_t(0x0100));
#endif
        }

        bool isEndOf() const {
            return (this->dist == kEndOfMark);
        }

        bool isUsed() const {
            return (this->value >= 0);
        }

        static bool isUsed(std::int8_t tag) {
            return (std::int8_t(tag) >= 0);
        }

        bool isUnused() const {
            return (this->value < 0);
        }

        static bool isUnused(std::int8_t tag) {
            return (std::int8_t(tag) < 0);
        }

        void setHash(std::uint8_t ctrl_hash) {
            this->hash = ctrl_hash;
        }

        void setEmpty() {
            this->dist = kEmptySlot;
        }

        void setEndOf() {
            this->dist = kEndOfMark;
        }

        void setDist(std::int8_t dist) {
            this->dist = dist;
        }

        void setValue(std::int8_t dist, std::uint8_t hash) {
            assert(dist >= 0 && dist <= kMaxDist);
#if 1
            this->value = ctrl_data::make(dist, hash);
#else
            this->setHash(hash);
            this->setDist(dist);
#endif
        }

        void setValue(std::int16_t dist_and_hash) {
            this->value = dist_and_hash;
        }

        void setValue(std::uint16_t dist_and_hash) {
            this->uvalue = dist_and_hash;
        }

        void incDist() {
            this->value += kDistInc16;
        }

        void decDist() {
            this->value -= kDistInc16;
        }

        void incDist(size_type width) {
            this->value += std::int16_t(kDistInc16 * width);
        }

        void decDist(size_type width) {
            this->value -= std::int16_t(kDistInc16 * width);
        }
    };

    typedef ctrl_data ctrl_type;

    static constexpr size_type kGroupSize = kGroupWidth * sizeof(ctrl_type);

    template <typename T>
    struct MatchMask2 {
        typedef T mask32_type;
        mask32_type maskEmpty;
        mask32_type maskHash;

        MatchMask2(mask32_type maskEmpty, mask32_type maskHash)
            : maskEmpty(maskEmpty), maskHash(maskHash) {
        }
        ~MatchMask2() = default;
    };

    template <typename T>
    struct MatchMask2A {
        typedef T mask32_type;
        mask32_type maskEmpty;
        mask32_type maskDist;

        MatchMask2A(mask32_type maskEmpty, mask32_type maskDist)
            : maskEmpty(maskEmpty), maskDist(maskDist) {
        }
        ~MatchMask2A() = default;
    };

    template <typename T>
    struct MatchMask3 {
        typedef T mask32_type;
        mask32_type maskEmpty;
        mask32_type maskHash;
        mask32_type maskDist;

        MatchMask3(mask32_type maskEmpty, mask32_type maskHash, mask32_type maskDist)
            : maskEmpty(maskEmpty), maskHash(maskHash), maskDist(maskDist) {
        }
        ~MatchMask3() = default;
    };

#if defined(__AVX512BW__) && defined(__AVX512VL__)

    template <typename T>
    struct BitMask256_AVX2Ex {
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef std::uint32_t bitmask_type;

        void clear(pointer data) {
            this->template fillAll16<kEmptySlot16>(data);
        }

        void setAllZeros(pointer data) {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)data, zero_bits);
        }

        template <std::int16_t ControlTag>
        void fillAll16(pointer data) {
            const __m256i tag_bits = _mm256_set1_epi16((short)ControlTag);
            _mm256_storeu_si256((__m256i *)data, tag_bits);
        }

        __m256i matchTag256(const_pointer data, std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            return match_mask;
        }

        std::uint32_t matchTag(const_pointer data, std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchLowTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask16 = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask16);
            __m256i match_mask = _mm256_cmpeq_epi16(low_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHash(const_pointer data, std::uint8_t ctrl_hash) const {
#if 1
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, hash_bits);
                    match_mask = _mm256_slli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
#else
            return this->matchLowTag(data, ctrl_hash);
#endif
        }

        std::uint32_t matchEmpty(const_pointer data) const {
            return this->matchUnused(data);
        }

        std::uint32_t matchEmptyOnly(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, ones_bits);
                std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return mask;
            } else {
                return this->matchHighTag(data, static_cast<std::uint8_t>(kEmptySlot));
            }
        }

        std::uint32_t matchNonEmpty(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ones_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i tag_bits   = _mm256_set1_epi16(kEmptySlot16);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(tag_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchUsed(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(ctrl_bits, ones_bits);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits  = _mm256_setzero_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return maskUsed;
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(const_pointer data, std::int16_t dist_and_hash) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(dist_and_hash <= kMaxDist16);
            __m256i dist_0_hash = _mm256_set1_epi16(dist_and_hash);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i ones_bits   = _mm256_setones_si256();
            __m256i high_mask   = _mm256_slli_epi16(ones_bits, 8);
            __m256i dist_1_hash = _mm256_adds_epi16(dist_0_hash, kDistanceBase);
            __m256i dist_and_0  = _mm256_and_si256(dist_1_hash, high_mask);
            __m256i ctrl_dist   = _mm256_and_si256(ctrl_bits,   high_mask);
            __m256i match_mask  = _mm256_cmpeq_epi16(dist_1_hash, ctrl_bits);
            __m256i empty_mask  = _mm256_cmpgt_epi16(dist_and_0,  ctrl_dist);
            __m256i result_mask = _mm256_andnot_si256(empty_mask, match_mask);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(empty_mask);
            std::uint32_t maskHash  = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return { maskEmpty, maskHash };
        }

        std::uint32_t matchEmptyOrZero(const_pointer data) const {
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i zero_mask   = _mm256_cmpeq_epi16(zero_bits, ctrl_bits);
            __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return maskEmpty;
        }

        std::uint32_t matchEmptyAndDistance(const_pointer data, std::int8_t distance) const {
#if 1
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i dist_and_0  = _mm256_slli_epi16(dist_0, 8);
            __m256i dist_bits   = _mm256_adds_epi16(dist_and_0, kDistanceBase);
            __m256i result_mask = _mm256_cmpgt_epi16(dist_bits, ctrl_bits);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return maskEmpty;
#else
            const __m256i kDistanceBase2 =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i dist_bits   = _mm256_adds_epi16(dist_0, kDistanceBase2);
            __m256i ctrl_dist   = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i dist_mask   = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
            __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return maskEmpty;
#endif
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t ctrl_hash) const {
            return (this->matchHash(data, ctrl_hash) != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            return (this->matchEmptyOnly(data) != 0);
        }

        bool hasAnyUsed(const_pointer data) const {
            return (this->matchUsed(data) != 0);
        }

        bool hasAnyUnused(const_pointer data) const {
            return (this->matchUnused(data) != 0);
        }

        bool isAllEmpty(const_pointer data) const {
            return (this->matchEmptyOnly(data) == kFullMask16);
        }

        bool isAllUsed(const_pointer data) const {
            return (this->matchUnused(data) == 0);
        }

        bool isAllUnused(const_pointer data) const {
            return (this->matchUsed(data) == 0);
        }

        static inline size_type bitPos(size_type pos) {
            return pos;
        }
    };

    template <typename T>
    using BitMask256 = BitMask256_AVX2Ex<T>;

#elif defined(__AVX2__)

    template <typename T>
    struct BitMask256_AVX2 {
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef std::uint32_t bitmask_type;

        void clear(pointer data) {
            this->template fillAll16<kEmptySlot16>(data);
        }

        void setAllZeros(pointer data) {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)data, zero_bits);
        }

        template <std::int16_t ControlTag>
        void fillAll16(pointer data) {
            const __m256i tag_bits = _mm256_set1_epi16((short)ControlTag);
            _mm256_storeu_si256((__m256i *)data, tag_bits);
        }

        __m256i matchTag256(const_pointer data, std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16((short)ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            return match_mask;
        }

        std::uint32_t matchTag(const_pointer data, std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchLowTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask16 = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask16);
            __m256i match_mask = _mm256_cmpeq_epi16(low_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHash(const_pointer data, std::uint8_t ctrl_hash) const {
#if 1
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, hash_bits);
                    match_mask = _mm256_slli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
#else
            return this->matchLowTag(data, ctrl_hash);
#endif
        }

        std::uint32_t matchEmpty(const_pointer data) const {
            return this->matchUnused(data);
        }

        std::uint32_t matchEmptyOnly(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return mask;
            } else {
                return this->matchHighTag(data, static_cast<std::uint8_t>(kEmptySlot));
            }
        }

        std::uint32_t matchNonEmpty(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ones_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i tag_bits   = _mm256_set1_epi16(kEmptySlot16);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(tag_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchUsed(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(ctrl_bits, ones_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits  = _mm256_setzero_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return maskUsed;
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(const_pointer data, std::int16_t dist_and_hash) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(dist_and_hash <= kMaxDist16);
            __m256i dist_0_hash = _mm256_set1_epi16(dist_and_hash);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i ones_bits   = _mm256_setones_si256();
            __m256i high_mask   = _mm256_slli_epi16(ones_bits, 8);
            __m256i dist_1_hash = _mm256_adds_epi16(dist_0_hash, kDistanceBase);
            __m256i dist_and_0  = _mm256_and_si256(dist_1_hash, high_mask);
            __m256i ctrl_dist   = _mm256_and_si256(ctrl_bits,   high_mask);
            __m256i match_mask  = _mm256_cmpeq_epi16(dist_1_hash, ctrl_bits);
            __m256i empty_mask  = _mm256_cmpgt_epi16(dist_and_0,  ctrl_dist);
            __m256i result_mask = _mm256_andnot_si256(empty_mask, match_mask);
                    empty_mask  = _mm256_srli_epi16(empty_mask, 8);
                    result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(empty_mask);
            std::uint32_t maskHash  = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return { maskEmpty, maskHash };
        }

        std::uint32_t matchEmptyOrZero(const_pointer data) const {
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i zero_mask   = _mm256_cmpeq_epi16(zero_bits, ctrl_bits);
            __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                    result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return maskEmpty;
        }

        std::uint32_t matchEmptyAndDistance(const_pointer data, std::int8_t distance) const {
#if 1
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i dist_and_0  = _mm256_slli_epi16(dist_0, 8);
            __m256i dist_bits   = _mm256_adds_epi16(dist_and_0, kDistanceBase);
            __m256i result_mask = _mm256_cmpgt_epi16(dist_bits, ctrl_bits);
            //      result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return maskEmpty;
#else
            const __m256i kDistanceBase2 =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i dist_bits   = _mm256_adds_epi16(dist_0, kDistanceBase2);
            __m256i ctrl_dist   = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i dist_mask   = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
            __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
            //      result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return maskEmpty;
#endif
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t ctrl_hash) const {
            return (this->matchHash(data, ctrl_hash) != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            return (this->matchEmptyOnly(data) != 0);
        }

        bool hasAnyUsed(const_pointer data) const {
            return (this->matchUsed(data) != 0);
        }

        bool hasAnyUnused(const_pointer data) const {
            return (this->matchUnused(data) != 0);
        }

        bool isAllEmpty(const_pointer data) const {
            return (this->matchEmptyOnly(data) == kFullMask32_Half);
        }

        bool isAllUsed(const_pointer data) const {
            return (this->matchUnused(data) == 0);
        }

        bool isAllUnused(const_pointer data) const {
            return (this->matchUsed(data) == 0);
        }

        static inline size_type bitPos(size_type pos) {
            return (pos >> 1);
        }
    };

    template <typename T>
    using BitMask256 = BitMask256_AVX2<T>;

#else

    static_assert(false, "jstd::robin_hash_map<K,V> required Intel AVX2 or heigher intrinsics.")

#endif // __AVX2__

    struct map_group {
        typedef BitMask256<ctrl_type>                        bitmask256_type;
        typedef typename BitMask256<ctrl_type>::bitmask_type bitmask_type;

        union {
            ctrl_type ctrls[kGroupWidth];
            bitmask256_type bitmask;
        };

        map_group() noexcept {
        }
        ~map_group() noexcept = default;

        void clear() {
            bitmask.clear(&this->ctrls[0]);
        }

        template <std::int16_t ControlTag>
        void fillAll16() {
            bitmask.template fillAll16<ControlTag>(&this->ctrls[0]);
        }

        bitmask_type matchTag(std::int16_t ctrl_tag) const {
            return bitmask.matchTag(&this->ctrls[0], ctrl_tag);
        }

        bitmask_type matchHash(std::uint8_t ctrl_hash) const {
            return bitmask.matchHash(&this->ctrls[0], ctrl_hash);
        }

        bitmask_type matchEmpty() const {
            return bitmask.matchEmpty(&this->ctrls[0]);
        }

        bitmask_type matchEmptyOnly() const {
            return bitmask.matchEmptyOnly(&this->ctrls[0]);
        }

        bitmask_type matchNonEmpty() const {
            return bitmask.matchNonEmpty(&this->ctrls[0]);
        }

        bitmask_type matchUsed() const {
            return bitmask.matchUsed(&this->ctrls[0]);
        }

        bitmask_type matchUnused() const {
            return bitmask.matchUnused(&this->ctrls[0]);
        }

        MatchMask2<bitmask_type>
        matchHashAndDistance(std::int16_t dist_and_hash) const {
            return bitmask.matchHashAndDistance(&this->ctrls[0], dist_and_hash);
        }

        bitmask_type matchEmptyOrZero() const {
            return bitmask.matchEmptyOrZero(&this->ctrls[0]);
        }

        bitmask_type matchEmptyAndDistance(std::int8_t distance) const {
            return bitmask.matchEmptyAndDistance(&this->ctrls[0], distance);
        }

        bool hasAnyMatch(std::uint8_t ctrl_hash) const {
            return bitmask.hasAnyMatch(&this->ctrls[0], ctrl_hash);
        }

        bool hasAnyEmpty() const {
            return bitmask.hasAnyEmpty(&this->ctrls[0]);
        }

        bool hasAnyUsed() const {
            return bitmask.hasAnyUsed(&this->ctrls[0]);
        }

        bool hasAnyUnused() const {
            return bitmask.hasAnyUnused(&this->ctrls[0]);
        }

        bool isAllEmpty() const {
            return bitmask.isAllEmpty(&this->ctrls[0]);
        }

        bool isAllUsed() const {
            return bitmask.isAllUsed(&this->ctrls[0]);
        }

        bool isAllUnused() const {
            return bitmask.isAllUnused(&this->ctrls[0]);
        }

        static inline std::uint8_t pos(size_type position) {
            return (std::uint8_t)bitmask256_type::bitPos(position);
        }

        static inline size_type index(size_type index, size_type position) {
            return (index + map_group::pos(position));
        }
    };

    typedef map_group  group_type;

    class slot_type {
    public:
        union {
            value_type          value;
            mutable_value_type  mutable_value;
            const key_type      key;
            key_type            mutable_key;
        };

        slot_type() {}
        ~slot_type() = default;
    };

    typedef slot_type       mutable_slot_type;
    typedef slot_type       node_type;

    template <typename Alloc, typename T, bool isCompatibleLayout,
              bool is_noexcept_move = is_noexcept_move_assignable<T>::value>
    struct swap_pair {
        static void swap(Alloc & alloc, T & a, T & b, T & tmp) {
            using std::swap;
            swap(a, b);
        }
    };

    template <typename Alloc, typename T>
    struct swap_pair<Alloc, T, true, false> {
        static void swap(Alloc & alloc, T & a, T & b, T & tmp) {
            alloc.construct(&tmp, std::move(a));
            alloc.destroy(&a);
            alloc.construct(&a, std::move(b));
            alloc.destroy(&b);
            alloc.construct(&b, std::move(tmp));
            alloc.destroy(&tmp);
        }
    };

    template <typename Alloc, typename T>
    struct swap_pair<Alloc, T, false, true> {
        typedef typename T::first_type                      first_type;
        typedef typename std::remove_cv<first_type>::type   mutable_first_type;
        typedef typename std::allocator_traits<Alloc>::template rebind_alloc<mutable_first_type>
                                                            mutable_first_allocator_type;
        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        static void swap(Alloc & alloc, T & a, T & b, T & tmp) {
            mutable_first_allocator_type first_allocator;

            first_allocator.construct(mutable_key(&tmp), std::move(*mutable_key(&a)));
            first_allocator.destroy(mutable_key(&a));
            first_allocator.construct(mutable_key(&a), std::move(*mutable_key(&b)));
            first_allocator.destroy(mutable_key(&b));
            first_allocator.construct(mutable_key(&b), std::move(*mutable_key(&tmp)));
            first_allocator.destroy(mutable_key(&tmp));

            using std::swap;
            swap(a.second, b.second);
        }
    };

    template <typename Alloc, typename T>
    struct swap_pair<Alloc, T, false, false> {
        static void swap(Alloc & alloc, T & a, T & b, T & tmp) {
            alloc.construct(&tmp, std::move(a));
            alloc.destroy(&a);
            alloc.construct(&a, std::move(b));
            alloc.destroy(&b);
            alloc.construct(&b, std::move(tmp));
            alloc.destroy(&tmp);
        }
    };

#if 1
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
        const this_type * owner_;
        size_type index_;

    public:
        basic_iterator() noexcept : owner_(nullptr), index_(0) {
        }
        basic_iterator(const this_type * owner, size_type index) noexcept
            : owner_(owner), index_(index) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : owner_(src.owner_), index_(src.index_) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) {
            this->owner_ = rhs.owner_;
            this->index_ = rhs.index_;
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.index_ == rhs.index_) && (lhs.owner_ == rhs.owner_);
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.index_ != rhs.index_) || (lhs.owner_ != rhs.owner_);
        }

        basic_iterator & operator ++ () {
            const ctrl_type * ctrl = this->owner_->ctrl_at(this->index_);
            size_type max_index = this->owner_->max_slot_capacity();
            do {
                ++(this->index_);
                ++ctrl;
            } while (ctrl->isEmptyOnly() && (this->index_ < max_index));
            return *this;
        }

        basic_iterator operator ++ (int) {
            basic_iterator copy(*this);
            ++*this;
            return copy;
        }

        reference operator * () const {
            slot_type * slot = const_cast<this_type *>(this->owner_)->slot_at(this->index_);
            return slot->value;
        }

        pointer operator -> () const {
            slot_type * slot = const_cast<this_type *>(this->owner_)->slot_at(this->index_);
            return std::addressof(slot->value);
        }

        operator basic_iterator<const remove_const_value_type>() const {
            return { this->owner_, this->index_ };
        }

        this_type * onwer() {
            return this->owner_;
        }

        const this_type * onwer() const {
            return this->owner_;
        }

        size_type index() {
            return this->index_;
        }

        size_type index() const {
            return this->index_;
        }

        slot_type * value() {
            slot_type * slot = this->owner_->slot_at(this->index);
            return slot;
        }

        const slot_type * value() const {
            slot_type * slot = this->owner_->slot_at(this->index);
            return slot;
        }
    };
#else
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
        ctrl_type * ctrl_;
        slot_type * slot_;

    public:
        basic_iterator() noexcept : ctrl_(nullptr), slot_(nullptr) {
        }
        basic_iterator(ctrl_type * ctrl, slot_type * slot) noexcept
            : ctrl_(ctrl), slot_(slot) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : ctrl_(src.ctrl_), slot_(src.slot_) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) {
            this->ctrl_ = rhs.ctrl_;
            this->slot_ = rhs.slot_;
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.slot_ == rhs.slot_);
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) {
            return (lhs.slot_ != rhs.slot_);
        }

        basic_iterator & operator ++ () {
            do {
                ++(this->ctrl_);
                ++(this->slot_);
            } while (ctrl_->isEmptyOnly());
            return *this;
        }

        basic_iterator operator ++ (int) {
            basic_iterator copy(*this);
            ++*this;
            return copy;
        }

        reference operator * () const {
            return this->slot_->value;
        }

        pointer operator -> () const {
            return std::addressof(this->slot_->value);
        }

        operator basic_iterator<const remove_const_value_type>() const {
            return { this->ctrl_, this->slot_ };
        }

        slot_type * value() {
            return this->slot_;
        }

        const slot_type * value() const {
            return this->slot_;
        }
    };
#endif

    using iterator       = basic_iterator<value_type>;
    using const_iterator = basic_iterator<const value_type>;

    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<mutable_value_type>
                                        mutable_allocator_type;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<ctrl_type>
                                        ctrl_allocator_type;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<slot_type>
                                        slot_allocator_type;

private:
    ctrl_type *     ctrls_;
    slot_type *     slots_;
    slot_type *     last_slot_;
    size_type       slot_size_;
    size_type       slot_mask_;
    size_type       max_lookups_;

    size_type       slot_threshold_;
    std::uint32_t   n_mlf_;
    std::uint32_t   n_mlf_rev_;
#if ROBIN_USE_HASH_POLICY
    hash_policy_t   hash_policy_;
#endif

    hasher          hasher_;
    key_equal       key_equal_;

    allocator_type              allocator_;
    mutable_allocator_type      mutable_allocator_;
    ctrl_allocator_type         ctrl_allocator_;
    slot_allocator_type         slot_allocator_;

public:
    robin_hash_map() : robin_hash_map(kDefaultCapacity) {
    }

    explicit robin_hash_map(size_type init_capacity,
                            const hasher & hash = hasher(),
                            const key_equal & equal = key_equal(),
                            const allocator_type & alloc = allocator_type()) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        ctrl_allocator_(alloc), slot_allocator_(alloc) {
        if (init_capacity != 0) {
            this->create_slots<true>(init_capacity);
        }
    }

    explicit robin_hash_map(const allocator_type & alloc)
        : robin_hash_map(kDefaultCapacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    robin_hash_map(InputIter first, InputIter last,
                   size_type init_capacity = kDefaultCapacity,
                   const hasher & hash = hasher(),
                   const key_equal & equal = key_equal(),
                   const allocator_type & alloc = allocator_type()) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        ctrl_allocator_(alloc), slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type input_size = distance(first, last);
        size_type reserve_capacity = (init_capacity >= input_size) ? init_capacity : input_size;
        this->reserve_for_insert(reserve_capacity);
        this->insert(first, last);
    }

    template <typename InputIter>
    robin_hash_map(InputIter first, InputIter last,
                   size_type init_capacity,
                   const allocator_type & alloc)
        : robin_hash_map(first, last, init_capacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    robin_hash_map(InputIter first, InputIter last,
                   size_type init_capacity,
                   const hasher & hash,
                   const allocator_type & alloc)
        : robin_hash_map(first, last, init_capacity, hash, key_equal(), alloc) {
    }

    robin_hash_map(const robin_hash_map & other)
        : robin_hash_map(other, std::allocator_traits<allocator_type>::
                                select_on_container_copy_construction(other.get_allocator())) {
    }

    robin_hash_map(const robin_hash_map & other, const Allocator & alloc) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_USE_HASH_POLICY
        hash_policy_(other.hash_policy_ref()),
#endif
        hasher_(hasher()), key_equal_(key_equal()),
        allocator_(alloc), mutable_allocator_(alloc),
        ctrl_allocator_(alloc), slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type other_size = other.slot_size();
        this->reserve_for_insert(other_size);
        try {
            this->insert_unique(other.begin(), other.end());
        } catch (const std::bad_alloc & ex) {
            this->destroy();
            throw std::bad_alloc();
        } catch (...) {
            this->destroy();
            throw;
        }
    }

    robin_hash_map(robin_hash_map && other) noexcept :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_USE_HASH_POLICY
        hash_policy_(std::move(other.hash_policy_ref())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(std::move(other.get_allocator_ref())),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        ctrl_allocator_(std::move(other.get_ctrl_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())) {
        // Swap content only
        this->swap_content(other);
    }

    robin_hash_map(robin_hash_map && other, const Allocator & alloc) noexcept :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_USE_HASH_POLICY
        hash_policy_(std::move(other.hash_policy_ref())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(alloc),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        ctrl_allocator_(std::move(other.get_ctrl_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())) {
        // Swap content only
        this->swap_content(other);
    }

    robin_hash_map(std::initializer_list<value_type> init_list,
                   size_type init_capacity = kDefaultCapacity,
                   const hasher & hash = hasher(),
                   const key_equal & equal = key_equal(),
                   const allocator_type & alloc = allocator_type()) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(this_type::default_empty_slots()),
        last_slot_(this_type::default_last_empty_slot()),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_USE_HASH_POLICY
        hash_policy_(),
#endif
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        ctrl_allocator_(alloc), slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type reserve_capacity = (init_capacity >= init_list.size()) ? init_capacity : init_list.size();
        this->reserve_for_insert(reserve_capacity);
        this->insert(init_list.begin(), init_list.end());
    }

    robin_hash_map(std::initializer_list<value_type> init_list,
                   size_type init_capacity,
                   const allocator_type & alloc)
        : robin_hash_map(init_list, init_capacity, hasher(), key_equal(), alloc) {
    }

    robin_hash_map(std::initializer_list<value_type> init_list,
                   size_type init_capacity,
                   const hasher & hash,
                   const allocator_type & alloc)
        : robin_hash_map(init_list, init_capacity, hash, key_equal(), alloc) {
    }

    ~robin_hash_map() {
        this->destroy_data();
    }

    bool is_valid() const { return (this->ctrls() != nullptr); }
    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const  { return (this->size() > this->slot_mask()); }

    bool empty() const { return this->is_empty(); }

    size_type size() const { return this->slot_size_; }
    size_type capacity() const { return (this->slot_mask_ + 1); }

    ctrl_type * ctrls() { return this->ctrls_; }
    const ctrl_type * ctrls() const {
        return const_cast<const ctrl_type *>(this->ctrls_);
    }

    group_type * groups() {
        return reinterpret_cast<group_type *>(this->ctrls_);
    }
    const group_type * groups() const {
        return const_cast<const group_type *>(reinterpret_cast<group_type *>(this->ctrls_));
    }

    size_type group_mask() const { return (this->slot_mask() / kGroupWidth); }
    size_type group_count() const {
        return ((this->max_slot_capacity() + kGroupWidth - 1) / kGroupWidth);
    }
    size_type group_capacity() const { return (this->group_count() + 1); }

    slot_type * slots() { return this->slots_; }
    const slot_type * slots() const { return this->slots_; }

    slot_type * last_slot() { return this->last_slot_; }
    const slot_type * last_slot() const { return this->last_slot_; }

    size_type slot_size() const { return this->slot_size_; }
    size_type slot_mask() const { return this->slot_mask_; }
    size_type slot_capacity() const { return (this->slot_mask_ + 1); }
    size_type slot_threshold() const { return this->slot_threshold_; }
    size_type slot_threshold(size_type now_slow_capacity) const {
        return (now_slow_capacity * this->integral_mlf() / kLoadFactorAmplify);
    }

    size_type max_lookups() const { return this->max_lookups_; }
    size_type max_slot_capacity() const {
        return (this->slot_capacity() + this->max_lookups());
    }

    constexpr size_type bucket_count() const {
        return this->slot_size_;
    }

    size_type bucket(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        return this->index_of(slot);
    }

    float load_factor() const {
        return ((float)this->slot_size() / this->slot_capacity());
    }

    void max_load_factor(float mlf) {
        if (mlf < kMinLoadFactor)
            mlf = kMinLoadFactor;
        if (mlf > kMaxLoadFactor)
            mlf = kMaxLoadFactor;
        assert(mlf != 0.0f);

        std::uint32_t n_mlf = (std::uint32_t)std::ceil(mlf * kLoadFactorAmplify);
        std::uint32_t n_mlf_rev = (std::uint32_t)std::ceil(1.0f / mlf * kLoadFactorAmplify);
        this->n_mlf_ = n_mlf;
        this->n_mlf_rev_ = n_mlf_rev;
        size_type now_slot_size = this->slot_size();
        size_type new_slot_threshold = this->slot_threshold();
        if (now_slot_size > new_slot_threshold) {
            size_type min_required_capacity = this->min_require_capacity(now_slot_size);
            if (min_required_capacity > this->slot_capacity()) {
                this->rehash(now_slot_size);
            }
        }
    }

    std::uint32_t integral_mlf() const {
        return this->n_mlf_;
    }

    std::uint32_t integral_mlf_rev() const {
        return this->n_mlf_rev_;
    }

    float max_load_factor() const {
        return ((float)this->integral_mlf() / kLoadFactorAmplify);
    }

    float default_mlf() const {
        return kDefaultLoadFactor;
    }

    size_type mul_mlf(size_type capacity) const {
        return (capacity * this->integral_mlf() / kLoadFactorAmplify);
    }

    size_type div_mlf(size_type capacity) const {
        return (capacity * this->integral_mlf_rev() / kLoadFactorAmplify);
    }

    iterator begin() {
#if 1
        group_type * group = this->groups();
        group_type * last_group = this->groups() + this->group_count();
        size_type start_index = 0;
        for (; group != last_group; group++) {
            std::uint32_t maskUsed = group->matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = group->index(start_index, pos);
                return this->iterator_at(index);
            }
            start_index += kGroupWidth;
        }

        return this->end();
#else
        ctrl_type * ctrl = this->ctrls();
        size_type index;
        for (index = 0; index <= this->slot_mask(); index++) {
            if (ctrl->isUsed()) {
                return { ctrl, this->slot_at(index) };
            }
            ctrl++;
        }
        return { ctrl, this->slot_at(index) };
#endif
    }

    const_iterator begin() const {
        return const_cast<this_type *>(this)->begin();
    }

    const_iterator cbegin() const {
        return this->begin();
    }

    iterator end() {
        return this->iterator_at(this->max_slot_capacity());
    }

    const_iterator end() const {
        return this->iterator_at(this->max_slot_capacity());
    }

    const_iterator cend() const {
        return this->end();
    }

    hasher hash_function() const {
        return this->hasher_;
    }

    key_equal key_eq() const {
        return this->key_equal_;
    }

#if ROBIN_USE_HASH_POLICY
    hash_policy_t hash_policy() const {
        return this->hash_policy_;
    }
#endif

    allocator_type get_allocator() const noexcept {
        return this->allocator_;
    }

    mutable_allocator_type get_mutable_allocator() const noexcept {
        return this->mutable_allocator_;
    }

    ctrl_allocator_type get_ctrl_allocator() const noexcept {
        return this->ctrl_allocator_;
    }

    slot_allocator_type get_slot_allocator() const noexcept {
        return this->slot_allocator_;
    }

    hasher & hash_function_ref() noexcept {
        return this->hasher_;
    }

    key_equal & key_eq_ref() noexcept {
        return this->key_equal_;
    }

#if ROBIN_USE_HASH_POLICY
    hash_policy_t & hash_policy_ref() noexcept {
        return this->hash_policy_;
    }
#endif

    allocator_type & get_allocator_ref() noexcept {
        return this->allocator_;
    }

    mutable_allocator_type & get_mutable_allocator_ref() noexcept {
        return this->mutable_allocator_;
    }

    ctrl_allocator_type & get_ctrl_allocator_ref() noexcept {
        return this->ctrl_allocator_;
    }

    slot_allocator_type & get_slot_allocator_ref() noexcept {
        return this->slot_allocator_;
    }

    static const char * name() {
        return "jstd::robin_hash_map<K, V>";
    }

    void clear(bool need_destory = false) noexcept {
        if (this->slot_capacity() > kDefaultCapacity) {
            if (need_destory) {
                this->destroy_data();
                this->create_slots<false>(kDefaultCapacity);
                assert(this->slot_size() == 0);
                return;
            }
        }
        this->clear_data();
        assert(this->slot_size() == 0);
    }

    void reserve(size_type new_capacity, bool read_only = false) {
        this->rehash(new_capacity, read_only);
    }

    void resize(size_type new_capacity, bool read_only = false) {
        this->rehash(new_capacity, read_only);
    }

    void rehash(size_type new_capacity, bool read_only = false) {
        if (!read_only)
            new_capacity = (std::max)(this->min_require_capacity(new_capacity), this->slot_size());
        else
            new_capacity = (std::max)(new_capacity, this->slot_size());
        this->rehash_impl<true, false>(new_capacity);
    }

    void shrink_to_fit(bool read_only = false) {
        size_type new_capacity;
        if (!read_only)
            new_capacity = this->min_require_capacity(this->slot_size());
        else
            new_capacity = this->slot_size();
        this->rehash_impl<true, false>(new_capacity);
    }

    void swap(robin_hash_map & other) {
        if (std::addressof(other) != this) {
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
        slot_type * slot = const_cast<slot_type *>(this->find_impl(key));
        if (slot != this->last_slot()) {
            return slot->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::robin_hash_map<K,V>::at(key), "
                                    "the specified key is not exists.");
        }
    }

    const mapped_type & at(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        if (slot != this->last_slot()) {
            return slot->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::robin_hash_map<K,V>::at(key) const, "
                                    "the specified key is not exists.");
        }
    }

    size_type count(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        return size_type(slot != this->last_slot());
    }

    bool contains(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        return (slot != this->last_slot());
    }

    iterator find(const key_type & key) {
        const slot_type * slot = this->find_impl(key);
        return this->iterator_at(this->index_of(slot));
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
              (!jstd::is_same_ex<P, mutable_value_type>::value) &&
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
              (!jstd::is_same_ex<P, mutable_value_type>::value) &&
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
              (!jstd::is_same_ex<P, mutable_value_type>::value) &&
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
        this->erase_slot(pos);
        return ++pos;
    }

    const_iterator erase(const_iterator pos) {
        size_type index = this->index_of(pos);
        this->erase_slot(index);
        return ++pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        for (; first != last; ++first) {
            size_type index = this->index_of(first);
            this->erase_slot(index);
        }
        return { first };
    }

private:
    static ctrl_type * default_empty_ctrls() {
        static constexpr size_type kMinGroupCount = (kMinLookups + (kGroupWidth - 1)) / kGroupWidth;
        static constexpr size_type kMinCtrlCapacity = (kMinGroupCount + 1) * kGroupWidth;
        static ctrl_type s_empty_ctrls[kMinCtrlCapacity] = {
            { kEmptySlot, 0 }, { kEmptySlot, 0 }, { kEmptySlot, 0 }, { kEmptySlot, 0,},
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },

            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }
        };
        return s_empty_ctrls;
    }

    static slot_type * default_empty_slots() {
        static slot_type s_empty_slots[kMinLookups] = {
            {}, {}, {}, {}
        };
        return s_empty_slots;
    }

    static slot_type * default_last_empty_slot() {
        return (default_empty_slots() + kMinLookups);
    }

    JSTD_FORCED_INLINE
    size_type calc_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = (std::max)(init_capacity, kMinCapacity);
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinCapacity>(new_capacity);
        }
        return new_capacity;
    }

    size_type calc_max_lookups(size_type new_capacity) const {
        assert(new_capacity > 1);
        assert(pow2::is_pow2(new_capacity));
#if 1
        // Fast to get log2_int, if the new_size is power of 2.
        // Use bsf(n) has the same effect.
        size_type max_lookups = size_type(BitUtils::bsr(new_capacity));
#else
        size_type max_lookups = size_type(pow2::log2_int<size_type, size_type(2)>(new_capacity));
#endif
        return max_lookups;
    }

    size_type min_require_capacity(size_type init_capacity) const {
        size_type new_capacity = init_capacity * this->integral_mlf_rev() / kLoadFactorAmplify;
        return new_capacity;
    }

    bool is_positive(size_type value) const {
        return (std::intptr_t(value) >= 0);
    }

#if 1
    iterator iterator_at(size_type index) noexcept {
        return { this, index };
    }

    const_iterator iterator_at(size_type index) const noexcept {
        return { this, index };
    }
#else
    iterator iterator_at(size_type index) noexcept {
        return { this->ctrl_at(index), this->slot_at(index) };
    }

    const_iterator iterator_at(size_type index) const noexcept {
        return { this->ctrl_at(index), this->slot_at(index) };
    }
#endif

    inline hash_code_t get_hash(const key_type & key) const noexcept {
        hash_code_t hash_code = static_cast<hash_code_t>(this->hasher_(key));
        return hash_code;
    }

    inline hash_code_t get_second_hash(hash_code_t value) const noexcept {
#if 1
        return value;
#elif 1
        return (size_type)hashes::mum_hash64((std::uint64_t)value, 11400714819323198485ull);
#elif 1
        return (size_type)hashes::fibonacci_hash((size_type)value);
#elif 1
        return (size_type)hashes::int_hash_crc32c((size_type)value);
#else
        hash_code_t hash_code;
        if (sizeof(size_type) == 4)
            hash_code = (hash_code_t)(((std::uint64_t)value * 2654435761ul) >> 12);
        else
            hash_code = (hash_code_t)(((std::uint64_t)value * 14695981039346656037ull) >> 28);
        return hash_code;
#endif
    }

    inline hash_code_t get_third_hash(hash_code_t value) const noexcept {
#if ROBIN_USE_HASH_POLICY
        return value;
#elif 0
        return (size_type)hashes::mum_hash64((std::uint64_t)value, 11400714819323198485ull);
#elif 1
        return (size_type)hashes::fibonacci_hash((size_type)value);
#elif 1
        return (size_type)hashes::simple_int_hash_crc32c((size_type)value);
#endif
    }

    inline size_type index_salt() const noexcept {
        return (size_type)((std::uintptr_t)this->ctrls() >> 12);
    }

    inline size_type index_for_hash(hash_code_t hash_code) const noexcept {
#if ROBIN_USE_HASH_POLICY
        return this->hash_policy_.index_for_hash(hash_code, this->slot_mask());
#else
        if (kUseIndexSalt)
            return ((this->get_second_hash((size_type)hash_code) ^ this->index_salt()) & this->slot_mask());
        else
            return  (this->get_second_hash((size_type)hash_code) & this->slot_mask());
#endif
    }

    inline size_type index_for_hash(hash_code_t hash_code, size_type slot_mask) const noexcept {
        assert(pow2::is_pow2(slot_mask + 1));
#if ROBIN_USE_HASH_POLICY
        return this->hash_policy_.index_for_hash(hash_code, slot_mask);
#else
        if (kUseIndexSalt)
            return ((this->get_second_hash((size_type)hash_code) ^ this->index_salt()) & slot_mask);
        else
            return  (this->get_second_hash((size_type)hash_code) & slot_mask);
#endif
    }

    inline size_type next_index(size_type index) const noexcept {
        assert(index < this->max_slot_capacity());
        return (index + 1);
    }

    inline size_type next_index(size_type index, size_type slot_mask) const noexcept {
        assert(index < this->max_slot_capacity());
        return (index + 1);
    }

    inline std::uint8_t get_ctrl_hash(hash_code_t hash_code) const noexcept {
        std::uint8_t ctrl_hash = static_cast<std::uint8_t>(
                this->get_third_hash((size_type)hash_code) & kCtrlHashMask);
        return ctrl_hash;
    }

    size_type round_index(size_type index) const {
        return (index & this->slot_mask());
    }

    size_type round_index(size_type index, size_type slot_mask) const {
        assert(pow2::is_pow2(slot_mask + 1));
        return (index & slot_mask);
    }

    std::int8_t round_dist(size_type last, size_type first) const {
        assert(last >= first);
        size_type distance = last - first;
        assert(distance < (size_type)kMaxDist);
#if 1
        return (std::int8_t)distance;
#else
        return ((distance <= (size_type)kMaxDist) ? std::int8_t(distance) : kMaxDist);
#endif
    }

    inline size_type prev_group(size_type group_index) const noexcept {
        return ((group_index + this->group_mask()) & this->group_mask());
    }

    inline size_type next_group(size_type group_index) const noexcept {
        return ((group_index + 1) & this->group_mask());
    }

    inline size_type slot_prev_group(size_type slot_index) const noexcept {
        return (slot_index - kGroupWidth);
    }

    inline size_type slot_next_group(size_type slot_index) const noexcept {
        return (slot_index + kGroupWidth);
    }

    ctrl_type * ctrl_at(size_type slot_index) noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->ctrls() + ssize_type(slot_index));
    }

    const ctrl_type * ctrl_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->ctrls() + ssize_type(slot_index));
    }

    inline ctrl_type * next_ctrl(size_type & slot_index) noexcept {
        slot_index = this->next_index(slot_index);
        return this->ctrl_at(slot_index);
    }

    inline const ctrl_type * next_ctrl(size_type & slot_index) const noexcept {
        slot_index = this->next_index(slot_index);
        return this->ctrl_at(slot_index);
    }

    group_type * group_at(size_type slot_index) noexcept {
        assert(slot_index < this->max_slot_capacity());
        return (group_type *)(this->ctrl_at(slot_index));
    }

    const group_type * group_at(size_type slot_index) const noexcept {
        assert(slot_index < this->max_slot_capacity());
        return (const group_type *)(this->ctrl_at(slot_index));
    }

    group_type * physical_group_at(size_type group_index) noexcept {
        assert(group_index < this->group_count());
        return (this->groups() + group_index);
    }

    const group_type * physical_group_at(size_type group_index) const noexcept {
        assert(group_index < this->group_count());
        return (this->groups() + group_index);
    }

    slot_type * slot_at(size_type slot_index) noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->slots() + ssize_type(slot_index));
    }

    const slot_type * slot_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->slots() + ssize_type(slot_index));
    }

    ctrl_type & get_control(size_type slot_index) {
        assert(slot_index < this->max_slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
    }

    const ctrl_type & get_control(size_type slot_index) const {
        assert(slot_index < this->max_slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
    }

    group_type & get_group(size_type slot_index) {
        assert(slot_index < this->max_slot_capacity());
        return *this->group_at(slot_index);
    }

    const group_type & get_group(size_type slot_index) const {
        assert(slot_index < this->max_slot_capacity());
        return *this->group_at(slot_index);
    }

    group_type & get_group(ctrl_type * ctrl) {
        size_type slot_index = this->index_of(ctrl);
        assert(slot_index < this->max_slot_capacity());
        group_type * group = reinterpret_cast<group_type *>(ctrl);
        return *group;
    }

    const group_type & get_group(const ctrl_type * ctrl) const {
        size_type slot_index = this->index_of(ctrl);
        assert(slot_index < this->max_slot_capacity());
        const group_type * group = const_cast<const group_type *>(
                reinterpret_cast<group_type *>(const_cast<ctrl_type *>(ctrl))
            );
        return *group;
    }

    group_type & get_physical_group(size_type group_index) {
        assert(group_index < this->group_count());
        return this->ctrls_[group_index];
    }

    const group_type & get_physical_group(size_type group_index) const {
        assert(group_index < this->group_count());
        return this->ctrls_[group_index];
    }

    slot_type & get_slot(size_type slot_index) {
        assert(slot_index < this->max_slot_capacity());
        return this->slots_[slot_index];
    }

    const slot_type & get_slot(size_type slot_index) const {
        assert(slot_index < this->max_slot_capacity());
        return this->slots_[slot_index];
    }

    size_type index_of(iterator pos) const {
        return this->index_of(pos.value());
    }

    size_type index_of(const_iterator pos) const {
        return this->index_of(pos.value());
    }

    size_type index_of(slot_type * slot) const {
        assert(slot != nullptr);
        assert(slot >= this->slots());
        size_type index = (size_type)(slot - this->slots());
        assert(is_positive(index));
        return index;
    }

    size_type index_of(const slot_type * slot) const {
        return this->index_of((slot_type *)slot);
    }

    size_type index_of(ctrl_type * ctrl) const {
        assert(ctrl != nullptr);
        assert(ctrl >= this->ctrls());
        size_type index = (size_type)(ctrl - this->ctrls());
        assert(is_positive(index));
        return index;
    }

    size_type index_of(const ctrl_type * ctrl) const {
        return this->index_of((ctrl_type *)ctrl);
    }

    static void placement_new_slot(slot_type * slot) {
        // The construction of union doesn't do anything at runtime but it allows us
        // to access its members without violating aliasing rules.
        new (slot) slot_type;
    }

    void destroy_data() noexcept {
        // Note!!: destory_slots() need use this->ctrls()
        this->destory_slots();
        
        this->destory_ctrls();
    }

    void destory_slots() noexcept {
        this->clear_slots();

        if (this->slots_ != this_type::default_empty_slots()) {
            this->slot_allocator_.deallocate(this->slots_, this->max_slot_capacity());
        }
        this->slots_ = this_type::default_empty_slots();
        this->last_slot_ = this_type::default_last_empty_slot();

        this->slot_size_ = 0;
    }

    void destory_ctrls() noexcept {
        if (this->ctrls_ != this_type::default_empty_ctrls()) {
            size_type max_ctrl_capacity = (this->group_count() + 1) * kGroupWidth;
            this->ctrl_allocator_.deallocate(this->ctrls_, max_ctrl_capacity);
        }
        this->ctrls_ = this_type::default_empty_ctrls();
    }

    void clear_data() noexcept {
        // Note!!: clear_slots() need use this->ctrls()
        this->clear_slots();

        this->clear_ctrls(this->ctrls(), this->slot_capacity(),
                          this->max_lookups(), this->group_count());
    }

    JSTD_FORCED_INLINE
    void clear_slots() noexcept {
        if (!is_slot_trivial_destructor) {
            ctrl_type * ctrl = this->ctrls();
            assert(ctrl != nullptr);
            for (size_type index = 0; index < this->max_slot_capacity(); index++) {
                if (ctrl->isUsed()) {
                    this->destroy_slot(index);
                }
                ctrl++;
            }
        }

        this->slot_size_ = 0;
    }

    JSTD_FORCED_INLINE
    void clear_ctrls(ctrl_type * ctrls, size_type slot_capacity,
                     size_type max_lookups, size_type group_count) noexcept {
        group_type * group = reinterpret_cast<group_type *>(ctrls);
        group_type * last_group = group + group_count;
        for (; group < last_group; ++group) {
            group->template fillAll16<kEmptySlot16>();
        }
        if (slot_capacity >= kGroupWidth) {
            group->template fillAll16<kEmptySlot16>();
        } else {
            assert(slot_capacity < kGroupWidth);
            group_type * tail_group = reinterpret_cast<group_type *>(ctrls + (slot_capacity + max_lookups));
            tail_group->template fillAll16<kEndOfMark16>();

            group->template fillAll16<kEndOfMark16>();
        }
    }

    inline bool need_grow() const {
        return (this->slot_size() >= this->slot_threshold());
    }

    void grow_if_necessary() {
        size_type new_capacity = (this->slot_mask_ + 1) * 2;
        this->rehash_impl<false, true>(new_capacity);
    }

    JSTD_FORCED_INLINE
    void reserve_for_insert(size_type init_capacity) {
        size_type new_capacity = this->min_require_capacity(init_capacity);
        this->create_slots<true>(new_capacity);
    }

    template <bool initialize = false>
    void reset() noexcept {
        if (initialize) {
            this->ctrls_ = this_type::default_empty_ctrls();
            this->slots_ = this_type::default_empty_slots();
            this->last_slot_ = this_type::default_last_empty_slot();
            this->slot_size_ = 0;
        }        
        this->slot_mask_ = 0;
        this->max_lookups_ = kMinLookups;
        this->slot_threshold_ = 0;
#if ROBIN_USE_HASH_POLICY
        this->hash_policy_.reset();
#endif
    }

    template <bool initialize = false>
    void create_slots(size_type init_capacity) {
        if (init_capacity == 0) {
            if (!initialize) {
                this->destroy_data();
            }
            this->reset<initialize>();
            return;
        }

        size_type new_capacity;
        if (initialize) {
            new_capacity = this->calc_capacity(init_capacity);
            assert(new_capacity > 0);
            assert(new_capacity >= kMinCapacity);
        } else {
            new_capacity = init_capacity;
        }

#if ROBIN_USE_HASH_POLICY
        auto hash_policy_setting = this->hash_policy_.calc_next_capacity(new_capacity);
        this->hash_policy_.commit(hash_policy_setting);        
#endif
        size_type new_max_lookups = this->calc_max_lookups(new_capacity);
        if (new_max_lookups < 16)
            new_max_lookups = (std::max)(new_max_lookups * 2, kMinLookups);
        else
            new_max_lookups = (std::min)(new_max_lookups * 4, size_type(std::uint8_t(kMaxDist) + 1));
        this->max_lookups_ = new_max_lookups;

        size_type new_ctrl_capacity = new_capacity + new_max_lookups;
        size_type new_group_count = (new_ctrl_capacity + (kGroupWidth - 1)) / kGroupWidth;
        assert(new_group_count > 0);
        size_type ctrl_alloc_size = (new_group_count + 1) * kGroupWidth;

        ctrl_type * new_ctrls = this->ctrl_allocator_.allocate(ctrl_alloc_size);
        this->ctrls_ = new_ctrls;
        this->max_lookups_ = new_max_lookups;

        // Reset ctrls to default state
        this->clear_ctrls(new_ctrls, new_capacity, new_max_lookups, new_group_count);

        slot_type * new_slots = this->slot_allocator_.allocate(new_ctrl_capacity);
        this->slots_ = new_slots;
        this->last_slot_ = new_slots + new_ctrl_capacity;
        if (initialize) {
            assert(this->slot_size_ == 0);
        } else {
            this->slot_size_ = 0;
        }
        this->slot_mask_ = new_capacity - 1;
        this->slot_threshold_ = this->slot_threshold(new_capacity);
    }

    template <bool AllowShrink, bool AlwaysResize>
    JSTD_NO_INLINE
    void rehash_impl(size_type new_capacity) {
        new_capacity = this->calc_capacity(new_capacity);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);
        if (AlwaysResize ||
            (!AllowShrink && (new_capacity > this->slot_capacity())) ||
            (AllowShrink && (new_capacity != this->slot_capacity()))) {
            if (!AlwaysResize && !AllowShrink) {
                assert(new_capacity >= this->slot_size());
            }

            ctrl_type * old_ctrls = this->ctrls();
            group_type * old_groups = this->groups();
            size_type old_group_count = this->group_count();
            size_type old_group_capacity = this->group_capacity();

            slot_type * old_slots = this->slots();
            size_type old_slot_size = this->slot_size();
            size_type old_slot_mask = this->slot_mask();
            size_type old_slot_capacity = this->slot_capacity();
            size_type old_max_slot_capacity = this->max_slot_capacity();

            this->create_slots<false>(new_capacity);

            if ((this->max_load_factor() < 0.5f) && false) {
                ctrl_type * last_ctrl = old_ctrls + old_max_slot_capacity;
                slot_type * old_slot = old_slots;
                for (ctrl_type * ctrl = old_ctrls; ctrl != last_ctrl; ctrl++) {
                    if (likely(ctrl->isUsed())) {
                        this->move_insert_unique(old_slot);
                        this->destroy_slot(old_slot);
                    }
                    old_slot++;
                }
            } else {
                if (old_slot_capacity >= kGroupWidth) {
                    group_type * last_group = old_groups + old_group_count;
                    size_type start_index = 0;
                    for (group_type * group = old_groups; group != last_group; group++) {
                        std::uint32_t maskUsed = group->matchUsed();
                        while (maskUsed != 0) {
                            size_type pos = BitUtils::bsf32(maskUsed);
                            maskUsed = BitUtils::clearLowBit32(maskUsed);
                            size_type old_index = group->index(start_index, pos);
                            slot_type * old_slot = old_slots + old_index;
                            this->move_insert_unique(old_slot);
                            this->destroy_slot(old_slot);
                        }
                        start_index += kGroupWidth;
                    }
                } else {
                    ctrl_type * last_ctrl = old_ctrls + old_max_slot_capacity;
                    slot_type * old_slot = old_slots;
                    for (ctrl_type * ctrl = old_ctrls; ctrl != last_ctrl; ctrl++) {
                        if (likely(ctrl->isUsed())) {
                            this->move_insert_unique(old_slot);
                            this->destroy_slot(old_slot);
                        }
                        old_slot++;
                    }
                }
            }

            assert(this->slot_size() == old_slot_size);

            if (old_ctrls != this->default_empty_ctrls()) {
                size_type old_max_ctrl_capacity = (old_group_count + 1) * kGroupWidth;
                this->ctrl_allocator_.deallocate(old_ctrls, old_max_ctrl_capacity);
            }
            if (old_slots != this->default_empty_slots()) {
                this->slot_allocator_.deallocate(old_slots, old_max_slot_capacity);
            }
        }
    }

    JSTD_FORCED_INLINE
    void destroy_slot(size_type index) {
        slot_type * slot = this->slot_at(index);
        this->destroy_slot(slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot(slot_type * slot) {
        if (!is_slot_trivial_destructor) {
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.destroy(&slot->mutable_value);
            } else {
                this->allocator_.destroy(&slot->value);
            }
        }
    }

    JSTD_FORCED_INLINE
    void transfer_slot(size_type dest_index, size_type src_index) {
        slot_type * dest_slot = this->slot_at(dest_index);
        slot_type * src_slot  = this->slot_at(src_index);
        this->transfer_slot(dest_slot, src_slot);
    }

    JSTD_FORCED_INLINE
    void transfer_slot(slot_type * dest_slot, slot_type * src_slot) {
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&dest_slot->mutable_value,
                                               std::move(src_slot->mutable_value));
        } else {
            this->allocator_.construct(&dest_slot->value, std::move(src_slot->value));
        }
        this->destroy_slot(src_slot);
    }

    JSTD_FORCED_INLINE
    void transfer_slot(slot_type * dest_slot, const slot_type * src_slot) {
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&dest_slot->mutable_value,
                                               src_slot->mutable_value);
        } else {
            this->allocator_.construct(&dest_slot->value, src_slot->value);
        }
        this->destroy_slot(src_slot);
    }

    JSTD_FORCED_INLINE
    void swap_slot(size_type slot_index1, size_type slot_index2, slot_type * tmp) {
        slot_type * slot1 = this->slot_at(slot_index1);
        slot_type * slot2 = this->slot_at(slot_index2);
        this->swap_slot(slot1, slot2, tmp);
    }

    JSTD_FORCED_INLINE
    void swap_slot(slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        if (kIsCompatibleLayout) {
            swap_pair<mutable_allocator_type, mutable_value_type, true>::swap(
                this->mutable_allocator_,
                slot1->mutable_value, slot2->mutable_value, tmp->mutable_value);
        } else {
            swap_pair<allocator_type, value_type, false>::swap(
                this->allocator_, slot1->value, slot2->value, tmp->value);
        }
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::int16_t dist_and_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        ctrl->setValue(dist_and_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::int8_t dist, std::uint8_t ctrl_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        ctrl->setValue(dist, ctrl_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, ctrl_type dist_and_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        ctrl->setValue(dist_and_hash.value);
    }

    JSTD_FORCED_INLINE
    void setUnusedCtrl(size_type index, std::int8_t tag) {
        ctrl_type * ctrl = this->ctrl_at(index);
        assert(ctrl->isUsed());
        ctrl->setDist(tag);
    }

    const slot_type * find_impl(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type start_slot = slot_index;
        std::int8_t distance = 0;
        ctrl_type dist_and_hash(2, ctrl_hash);

        const ctrl_type * ctrl = this->ctrl_at(slot_index);
        const slot_type * slot = this->slot_at(slot_index);

        if (kUnrollMode == UnrollMode16) {
#if 1
            ctrl_type dist_and_0(0, 0);

            while (ctrl->value >= dist_and_0.value) {
                if (this->key_equal_(slot->value.first, key)) {
                    return slot;
                }
                slot++;
                ctrl++;
                dist_and_0.incDist();
            }

            return this->last_slot();
#elif 0
            ctrl_type dist_and_0(0, 0);

            while (ctrl->value >= dist_and_0.value) {
                if (ctrl->hash == ctrl_hash) {
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot;
                    }
                }
                slot++;
                ctrl++;
                dist_and_0.incDist();
            }

            return this->last_slot();
#else
            if (ctrl->value >= std::int16_t(0)) {
                if (ctrl->hash == ctrl_hash) {
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot;
                    }
                }
            } else {
                return this->last_slot();
            }

            ctrl++;
            slot++;

            if (ctrl->value >= kDistInc16) {
                if (ctrl->hash == ctrl_hash) {
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot;
                    }
                }
            } else {
                return this->last_slot();
            }
            
            ctrl++;
            slot++;
#endif
        } else if (kUnrollMode == UnrollMode8) {
            // Optimize from: (ctrl->isUsed() && (ctrl->dist >= 0))
            if (likely(ctrl->isUsed())) {
                if (ctrl->hash == ctrl_hash) {
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot;
                    }
                }
            } else {
                return this->last_slot();
            }

            ctrl++;
            slot++;

            // Optimization: merging two comparisons
            if (likely(ctrl->dist > 0)) {
            //if (likely(ctrl->isUsed() && (ctrl->dist >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot;
                    }
                }
            } else {
                return this->last_slot();
            }

            ctrl++;
            slot++;
            distance = 2;

            do {
                // Optimization: merging two comparisons
                if (likely(ctrl->dist >= distance)) {
                //if (likely(ctrl->isUsed() && (ctrl->dist >= distance))) {
                    if (ctrl->hash == ctrl_hash) {
                        if (this->key_equal_(slot->value.first, key)) {
                            return slot;
                        }
                    }

                    slot++;
                    ctrl++;
                    distance++;
                } else {
                    break;
                }
            } while (1);

            return this->last_slot();
        }

        const slot_type * last_slot = this->last_slot();

        do {
            const group_type & group = this->get_group(ctrl);
            auto mask32 = group.matchHashAndDistance(dist_and_hash.value);
            std::uint32_t maskHash = mask32.maskHash;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(0, pos);
                const slot_type * target = slot + index;
                if (this->key_equal_(target->value.first, key)) {
                    return target;
                }
            }
            if (mask32.maskEmpty != 0) {
                break;
            }
            dist_and_hash.incDist(kGroupWidth);
            ctrl += kGroupWidth;
            slot += kGroupWidth;
        } while (slot < last_slot);

        return last_slot;
    }

    JSTD_NO_INLINE
    std::pair<size_type, bool>
    find_and_prepare_insert(const key_type & key, ctrl_type & o_dist_and_hash) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type first_slot = slot_index;
        size_type last_slot = npos;
        std::int8_t distance = 0;
        ctrl_type dist_and_hash(0, ctrl_hash);
        std::uint32_t maskEmpty;

        if (kUnrollMode == UnrollMode16) {
            std::uint16_t max_dist = (std::uint16_t)(this->max_lookups_ << 8);
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            while (ctrl->dist >= dist_and_hash.dist) {
                if (likely(ctrl->value == dist_and_hash.value)) {
                    slot_index = this->index_of(ctrl);
                    slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_dist_and_hash = dist_and_hash;
                        return { slot_index, true };
                    }
                }

                ctrl++;
                dist_and_hash.incDist();
                if (dist_and_hash.uvalue >= max_dist)
                    break;
            }

            if (this->need_grow() || (dist_and_hash.uvalue >= max_dist)) {
                // The size of slot reach the slot threshold or hashmap is full.
                this->grow_if_necessary();

                return this->find_and_prepare_insert(key, o_dist_and_hash);
            }

            o_dist_and_hash = dist_and_hash;
            slot_index = this->index_of(ctrl);
            return { slot_index, false };
        } else if (kUnrollMode == UnrollMode8) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            // Optimize from: (ctrl->isUsed() && (ctrl->dist >= 0))
            if (likely(ctrl->isUsed())) {
                if (ctrl->hash == ctrl_hash) {
                    slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_dist_and_hash.setValue(0, ctrl_hash);
                        return { slot_index, true };
                    }
                }
            } else {
                if (this->need_grow()) {
                    // The size of slot reach the slot threshold or hashmap is full.
                    this->grow_if_necessary();

                    return this->find_and_prepare_insert(key, o_dist_and_hash);
                }
                o_dist_and_hash.setValue(0, ctrl_hash);
                return { slot_index, false };
            }

            ctrl++;

            // Optimization: merging two comparisons
            if (likely(ctrl->dist > 0)) {
            //if (likely(ctrl->isUsed() && (ctrl->dist >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_dist_and_hash.setValue(1, ctrl_hash);
                        return { (slot_index + 1), true };
                    }
                }
            } else {
                if (this->need_grow()) {
                    // The size of slot reach the slot threshold or hashmap is full.
                    this->grow_if_necessary();

                    return this->find_and_prepare_insert(key, o_dist_and_hash);
                }
                o_dist_and_hash.setValue(1, ctrl_hash);
                return { (slot_index + 1), false };
            }

            ctrl++;
            distance = 2;

            do {
                // Optimization: merging two comparisons
                if (likely(ctrl->dist >= distance)) {
                //if (likely(ctrl->isUsed() && (ctrl->dist >= distance))) {
                    if (ctrl->hash == ctrl_hash) {
                        slot_index = this->index_of(ctrl);
                        slot_type * slot = this->slot_at(slot_index);
                        if (this->key_equal_(slot->value.first, key)) {
                            o_dist_and_hash.setValue(distance, ctrl_hash);
                            return { slot_index, true };
                        }
                    }
                    ctrl++;
                    distance++;
                    if (ctrl->dist < distance)
                        break;
                } else {
                    if (this->need_grow()) {
                        // The size of slot reach the slot threshold or hashmap is full.
                        this->grow_if_necessary();

                        return this->find_and_prepare_insert(key, o_dist_and_hash);
                    }
                    o_dist_and_hash.setValue(distance, ctrl_hash);
                    slot_index = this->index_of(ctrl);
                    return { slot_index, false };
                }
            } while (1);

            o_dist_and_hash.setValue(distance, ctrl_hash);
            slot_index = this->index_of(ctrl);
            return { slot_index, false };
        }

        size_type max_slot_index = this->max_slot_capacity();

        do {
            const group_type & group = this->get_group(slot_index);
            auto mask32 = group.matchHashAndDistance(dist_and_hash.value);
            std::uint32_t maskHash = mask32.maskHash;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(slot_index, pos);
                const slot_type * target = this->slot_at(index);
                if (this->key_equal_(target->value.first, key)) {
                    distance = this->round_dist(index, first_slot);
                    assert(distance == (dist_and_hash.dist + group_type::pos(pos)));
                    o_dist_and_hash.setValue(distance, ctrl_hash);
                    return { index, true };
                }
            }
            maskEmpty = mask32.maskEmpty;
            if (maskEmpty != 0) {
                last_slot = slot_index;
                break;
            }
            dist_and_hash.incDist(kGroupWidth);
            slot_index = this->slot_next_group(slot_index);
        } while (slot_index < max_slot_index);

        if (this->need_grow() || (last_slot == npos)) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            return this->find_and_prepare_insert(key, o_dist_and_hash);
        }

        // It's a [EmptyEntry], or (distance > ctrl->dist) entry.
        assert(maskEmpty != 0);
        size_type pos = BitUtils::bsf32(maskEmpty);
        size_type index = group_type::index(last_slot, pos);
        std::int8_t o_distance = this->round_dist(index, first_slot);
        assert(o_distance == (distance + group_type::pos(pos)));
        o_dist_and_hash.setValue(o_distance, ctrl_hash);
        return { index, false };
    }

    template <bool isRehashing>
    JSTD_NO_INLINE
    bool insert_to_place(size_type target, ctrl_type dist_and_hash) {
        ctrl_type insert_ctrl(dist_and_hash);
        ctrl_type * ctrl = this->ctrl_at(target);
        assert(!ctrl->isEmpty());
        assert(dist_and_hash.dist > ctrl->dist);
        std::swap(insert_ctrl.value, ctrl->value);
        insert_ctrl.incDist();
        ctrl++;

        alignas(slot_type) unsigned char raw[sizeof(slot_type)];
        alignas(slot_type) unsigned char tmp_raw[sizeof(slot_type)];

        slot_type * to_insert = reinterpret_cast<slot_type *>(&raw);
        slot_type * tmp_slot  = reinterpret_cast<slot_type *>(&tmp_raw);
        this->placement_new_slot(to_insert);
        this->placement_new_slot(tmp_slot);
        slot_type * target_slot = this->slot_at(target);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&to_insert->mutable_value, std::move(target_slot->mutable_value));
        } else {
            this->allocator_.construct(&to_insert->value, std::move(target_slot->value));
        }
        if (is_slot_trivial_destructor) {
            this->destroy_slot(target_slot);
        }

        size_type max_index = this->max_slot_capacity();
        size_type slot_index = this->next_index(target);
        while (slot_index < max_index) {
            if (ctrl->isEmptyOnly()) {
                this->emplace_tmp_rich_slot(to_insert, slot_index, insert_ctrl.value);
                return false;
            } else if ((insert_ctrl.dist > ctrl->dist) /* || (insert_ctrl.distance == (kEndOfMark - 1)) */) {
                std::swap(insert_ctrl.value, ctrl->value);

                slot_type * slot = this->slot_at(slot_index);
                this->swap_slot(to_insert, slot, tmp_slot);
            }

            insert_ctrl.incDist();
            assert(insert_ctrl.dist <= kMaxDist);

            if (isRehashing) {
                //insert_ctrl.distance = (insert_ctrl.distance < kEndOfMark) ? insert_ctrl.distance : (kEndOfMark - 1);
                assert(size_type(std::uint8_t(insert_ctrl.dist)) < this->max_lookups());
            } else {
                if (size_type(std::uint8_t(insert_ctrl.dist)) >= this->max_lookups()) {
                    this->emplace_tmp_rich_slot(to_insert, target, insert_ctrl.value);
                    return true;
                }
            }
            ctrl++;
            slot_index++;
        }

        this->emplace_tmp_rich_slot(to_insert, target, insert_ctrl.value);
        return true;
    }

    JSTD_FORCED_INLINE
    void emplace_tmp_rich_slot(slot_type * to_insert, size_type target,
                               std::int16_t dist_and_hash) {
        this->setUsedCtrl(target, dist_and_hash);

        slot_type * slot = this->slot_at(target);
        this->placement_new_slot(slot);
        if (kIsCompatibleLayout)
            this->mutable_allocator_.construct(&slot->mutable_value, std::move(to_insert->mutable_value));
        else
            this->allocator_.construct(&slot->value, std::move(to_insert->value));

        if (is_slot_trivial_destructor) {
            this->destroy_slot(to_insert);
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(const value_type & value) {
        ctrl_type dist_and_hash;
        auto find_info = this->find_and_prepare_insert(value.first, dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace_impl<AlwaysUpdate>(value);
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout)
                this->mutable_allocator_.construct(&slot->mutable_value, value);
            else
                this->allocator_.construct(&slot->value, value);
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = this->slot_at(target);
                slot->value.second = value.second;
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(value_type && value) {
        ctrl_type dist_and_hash;
        auto find_info = this->find_and_prepare_insert(value.first, dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace_impl<AlwaysUpdate>(std::forward<value_type>(value));
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout)
                this->mutable_allocator_.construct(&slot->mutable_value, std::forward<value_type>(value));
            else
                this->allocator_.construct(&slot->value, std::forward<value_type>(value));
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                static constexpr bool is_rvalue_ref = std::is_rvalue_reference<decltype(value)>::value;
                slot_type * slot = this->slot_at(target);
                if (is_rvalue_ref)
                    slot->value.second = std::move(value.second);
                else
                    slot->value.second = value.second;
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename KeyT, typename MappedT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, mutable_value_type>::value &&
               !std::is_constructible<mutable_value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
              (jstd::is_same_ex<KeyT, key_type>::value ||
               std::is_constructible<key_type, KeyT &&>::value) &&
              (jstd::is_same_ex<MappedT, mapped_type>::value ||
               std::is_constructible<mapped_type, MappedT &&>::value)>::type * = nullptr>
    std::pair<iterator, bool> emplace_impl(KeyT && key, MappedT && value) {
        ctrl_type dist_and_hash;
        auto find_info = this->find_and_prepare_insert(key, dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace_impl<AlwaysUpdate, KeyT, MappedT>(
                            std::forward<KeyT>(key), std::forward<MappedT>(value)
                        );
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.construct(&slot->mutable_value,
                                                   std::forward<KeyT>(key),
                                                   std::forward<MappedT>(value));
            } else {
                this->allocator_.construct(&slot->value, std::forward<KeyT>(key),
                                                         std::forward<MappedT>(value));
            }
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            static constexpr bool isMappedType = jstd::is_same_ex<MappedT, mapped_type>::value;
            if (AlwaysUpdate) {
                if (isMappedType) {
                    slot_type * slot = this->slot_at(target);
                    slot->value.second = std::forward<MappedT>(value);
                } else {
                    mapped_type mapped_value(std::forward<MappedT>(value));
                    slot_type * slot = this->slot_at(target);
                    slot->value.second = std::move(mapped_value);
                }
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, mutable_value_type>::value &&
               !std::is_constructible<mutable_value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
              (jstd::is_same_ex<KeyT, key_type>::value ||
               std::is_constructible<key_type, KeyT &&>::value)>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(KeyT && key, Args && ... args) {
        ctrl_type dist_and_hash;
        auto find_info = this->find_and_prepare_insert(key, dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace(std::piecewise_construct,
                                         std::forward_as_tuple(std::forward<KeyT>(key)),
                                         std::forward_as_tuple(std::forward<Args>(args)...));
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.construct(&slot->mutable_value,
                                                   std::piecewise_construct,
                                                   std::forward_as_tuple(std::forward<KeyT>(key)),
                                                   std::forward_as_tuple(std::forward<Args>(args)...));
            } else {
                this->allocator_.construct(&slot->value,
                                           std::piecewise_construct,
                                           std::forward_as_tuple(std::forward<KeyT>(key)),
                                           std::forward_as_tuple(std::forward<Args>(args)...));
            }
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                mapped_type mapped_value(std::forward<Args>(args)...);
                slot_type * slot = this->slot_at(target);
                slot->value.second = std::move(mapped_value);
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename PieceWise, typename std::enable_if<
              (!jstd::is_same_ex<PieceWise, value_type>::value &&
               !std::is_constructible<value_type, PieceWise &&>::value) &&
              (!jstd::is_same_ex<PieceWise, mutable_value_type>::value &&
               !std::is_constructible<mutable_value_type, PieceWise &&>::value) &&
              jstd::is_same_ex<PieceWise, std::piecewise_construct_t>::value &&
              (!jstd::is_same_ex<PieceWise, key_type>::value &&
               !std::is_constructible<key_type, PieceWise &&>::value)>::type * = nullptr,
              typename ... Ts1, typename ... Ts2>
    std::pair<iterator, bool> emplace_impl(PieceWise && hint,
                                           std::tuple<Ts1...> && first,
                                           std::tuple<Ts2...> && second) {
        ctrl_type dist_and_hash;
        tuple_wrapper2<key_type> key_wrapper(first);
        auto find_info = this->find_and_prepare_insert(key_wrapper.value(), dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace(std::piecewise_construct,
                                         std::forward<std::tuple<Ts1...>>(first),
                                         std::forward<std::tuple<Ts2...>>(second));
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.construct(&slot->mutable_value,
                                                   std::piecewise_construct,
                                                   std::forward<std::tuple<Ts1...>>(first),
                                                   std::forward<std::tuple<Ts2...>>(second));
            } else {
                this->allocator_.construct(&slot->value,
                                           std::piecewise_construct,
                                           std::forward<std::tuple<Ts1...>>(first),
                                           std::forward<std::tuple<Ts2...>>(second));
            }
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                tuple_wrapper2<mapped_type> mapped_wrapper(std::move(second));
                slot_type * slot = this->slot_at(target);
                slot->value.second = std::move(mapped_wrapper.value());
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool AlwaysUpdate, typename First, typename std::enable_if<
              (!jstd::is_same_ex<First, value_type>::value &&
               !std::is_constructible<value_type, First &&>::value) &&
              (!jstd::is_same_ex<First, mutable_value_type>::value &&
               !std::is_constructible<mutable_value_type, First &&>::value) &&
              (!jstd::is_same_ex<First, std::piecewise_construct_t>::value) &&
              (!jstd::is_same_ex<First, key_type>::value &&
               !std::is_constructible<key_type, First &&>::value)>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(First && first, Args && ... args) {
        ctrl_type dist_and_hash;
        value_type value(std::forward<First>(first), std::forward<Args>(args)...);
        auto find_info = this->find_and_prepare_insert(value.first, dist_and_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, dist_and_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
                if (need_grow) {
                    this->grow_if_necessary();
                    return this->emplace(std::move(value));
                }
            }

            slot_type * slot = this->slot_at(target);
            this->placement_new_slot(slot);
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.construct(&slot->mutable_value, std::move(value));
            } else {
                this->allocator_.construct(&slot->value, std::move(value));
            }
            this->slot_size_++;
            return { this->iterator_at(target), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = this->slot_at(target);
                slot->value.second = std::move(value.second);
            }
            return { this->iterator_at(target), false };
        }
    }

    JSTD_FORCED_INLINE
    size_type unique_prepare_insert(const key_type & key, ctrl_type & o_ctrl) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type first_slot = slot_index;
        std::int8_t distance = 0;
        o_ctrl.hash = ctrl_hash;

        if (kUnrollMode == UnrollMode16) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);
            if (likely(ctrl->dist < 0)) {
                o_ctrl.dist = 0;
                return slot_index;
            }

            ctrl++;

            if (likely(ctrl->dist < 1)) {
                o_ctrl.dist = 1;
                return (slot_index + 1);
            }

            ctrl++;

            if (likely(ctrl->dist < 2)) {
                o_ctrl.dist = 2;
                return (slot_index + 2);
            }

            ctrl++;

            if (likely(ctrl->dist < 3)) {
                o_ctrl.dist = 3;
                return (slot_index + 3);
            }

            ctrl++;
            distance = 4;

            ctrl_type * last_ctrl = this->ctrl_at(this->max_slot_capacity());

            while (ctrl < last_ctrl) {
                if (likely(ctrl->dist < distance)) {
                    slot_index = this->index_of(ctrl);
                    assert(slot_index < this->max_slot_capacity());                    
                    std::int8_t o_dist = this->round_dist(slot_index, first_slot);
                    assert(o_dist == distance);
                    o_ctrl.dist = distance;
                    assert(o_ctrl.dist >= 0 && o_ctrl.dist < (std::int8_t)this->max_lookups());
                    assert(o_ctrl.dist <= kMaxDist);
                    return slot_index;
                }
                distance++;
                ctrl++;
            }

            slot_index = this->index_of(ctrl);
            assert(slot_index < this->max_slot_capacity());
            std::int8_t o_dist = this->round_dist(slot_index, first_slot);
            assert(o_dist == distance);
            o_ctrl.dist = distance;
            assert(o_ctrl.dist >= 0 && o_ctrl.dist < (std::int8_t)this->max_lookups());
            assert(o_ctrl.dist <= kMaxDist);
            return slot_index;
        } else if (kUnrollMode == UnrollMode8) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);
            // Optimize from: (ctrl->isEmpty() || (ctrl->dist < 0))
            if (likely(ctrl->isEmpty())) {
                o_ctrl.dist = 0;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(ctrl->dist < 1)) {
            //if (likely(ctrl->isEmpty() || (ctrl->dist < 1))) {
                o_ctrl.dist = 1;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(ctrl->dist < 2)) {
            //if (likely(ctrl->isEmpty() || (ctrl->dist < 2))) {
                o_ctrl.dist = 2;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(ctrl->dist < 3)) {
            //if (likely(ctrl->isEmpty() || (ctrl->dist < 3))) {
                o_ctrl.dist = 3;
                return slot_index;
            }

            distance = 4;
            slot_index = this->next_index(slot_index);
        }

        // Find the first empty slot and insert
        do {
            const group_type & group = this->get_group(slot_index);
            std::uint32_t maskEmpty = group.matchEmptyAndDistance(distance);
            if (maskEmpty != 0) {
                // Found a [EmptyEntry] to insert
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type index = group.index(slot_index, pos);
                o_ctrl.dist = this->round_dist(index, first_slot);
                assert(o_ctrl.dist == (distance + group_type::pos(pos)));
                return index;
            }
            distance += kGroupWidth;
            slot_index = this->slot_next_group(slot_index);
            assert(slot_index != first_slot);
        } while (1);

        o_ctrl.dist = kEndOfMark;
        return npos;
    }

    // Use in rehash_impl()
    bool move_insert_unique(slot_type * slot) {
        ctrl_type dist_and_hash;
        size_type target = this->unique_prepare_insert(slot->value.first, dist_and_hash);
        assert(target != npos);
        bool need_grow = false;

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, dist_and_hash);
        } else {
            // Insert to target place
            need_grow = this->insert_to_place<true>(target, dist_and_hash);
            assert(need_grow == false);
        }

        slot_type * new_slot = this->slot_at(target);
        this->placement_new_slot(new_slot);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&new_slot->mutable_value,
                                               std::move(*static_cast<mutable_value_type *>(&slot->mutable_value)));
        } else {
            this->allocator_.construct(&new_slot->value, std::move(slot->value));
        }
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
        return need_grow;
    }

    void insert_unique(const value_type & value) {
        ctrl_type dist_and_hash;
        size_type target = this->unique_prepare_insert(value.first, dist_and_hash);
        assert(target != npos);

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, dist_and_hash);
        } else {
            // Insert to target place
            bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
            if (need_grow) {
                this->grow_if_necessary();
                this->insert_unique(value);
                return;
            }
        }

        slot_type * slot = this->slot_at(target);
        this->placement_new_slot(slot);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&slot->mutable_value, value);
        } else {
            this->allocator_.construct(&slot->value, value);
        }
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    void insert_unique(value_type && value) {
        ctrl_type dist_and_hash;
        size_type target = this->unique_prepare_insert(value.first, dist_and_hash);
        assert(target != npos);

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, dist_and_hash);
        } else {
            // Insert to target place
            bool need_grow = this->insert_to_place<false>(target, dist_and_hash);
            if (need_grow) {
                this->grow_if_necessary();
                this->insert_unique(std::forward<value_type>(value));
                return;
            }
        }

        slot_type * slot = this->slot_at(target);
        this->placement_new_slot(slot);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&slot->mutable_value,
                                               std::move(*static_cast<mutable_value_type *>(&value)));
        } else {
            this->allocator_.construct(&slot->value, std::move(value));
        }
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    // Use in constructor
    template <typename InputIter>
    void insert_unique(InputIter first, InputIter last) {
        for (InputIter iter = first; iter != last; ++iter) {
            this->insert_unique(static_cast<value_type>(*iter));
        }
    }

    JSTD_FORCED_INLINE
    size_type find_and_erase(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type start_slot = slot_index;
        std::int8_t distance = 0;
        ctrl_data dist_and_hash(0, ctrl_hash);

        if (kUnrollMode == UnrollMode16) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            if (likely(ctrl->value == dist_and_hash.value)) {
                const slot_type * slot = this->slot_at(slot_index);
                if (this->key_equal_(slot->value.first, key)) {
                    this->erase_slot(slot_index);
                    return 1;
                }
            }

            ctrl++;
            dist_and_hash.incDist();

            if (likely(ctrl->value == dist_and_hash.value)) {
                slot_index++;
                const slot_type * slot = this->slot_at(slot_index);
                if (this->key_equal_(slot->value.first, key)) {
                    this->erase_slot(slot_index);
                    return 1;
                }
            }

            ctrl++;
            dist_and_hash.incDist();

            do {
                if (likely(ctrl->value == dist_and_hash.value)) {
                    slot_index = this->index_of(ctrl);
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        this->erase_slot(slot_index);
                        return 1;
                    }
                }
                ctrl++;
                dist_and_hash.incDist();
            } while (ctrl->dist >= dist_and_hash.dist);

            return 0;
        } else if (kUnrollMode == UnrollMode8) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);

            // Optimize from: (ctrl->isUsed() && (ctrl->dist >= 0))
            if (likely(ctrl->isUsed())) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        this->erase_slot(slot_index);
                        return 1;
                    }
                }
            } else {
                return 0;
            }

            ctrl++;
            slot_index++;

            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->dist + 1) > 1)) {
            //if (likely(ctrl->isUsed() && (ctrl->dist >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        this->erase_slot(slot_index);
                        return 1;
                    }
                }
            } else {
                return 0;
            }

            ctrl++;
            slot_index++;
            distance = 2;

            do {
                // Optimization: merging two comparisons
                if (likely(std::uint8_t(ctrl->dist + 1) > distance)) {
                //if (likely(ctrl->isUsed() && (ctrl->dist >= distance))) {
                    if (ctrl->hash == ctrl_hash) {
                        const slot_type * slot = this->slot_at(slot_index);
                        if (this->key_equal_(slot->value.first, key)) {
                            this->erase_slot(slot_index);
                            return 1;
                        }
                    }

                    slot_index++;
                    ctrl++;
                    distance++;
                    if (ctrl->dist < distance)
                        break;
                } else {
                    return 0;
                }
            } while (1);

            return 0;
        }

        size_type max_slot_index = this->max_slot_capacity();

        do {
            const group_type & group = this->get_group(slot_index);
            auto mask32 = group.matchHashAndDistance(dist_and_hash.value);
            std::uint32_t maskHash = mask32.maskHash;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(slot_index, pos);
                const slot_type * target = this->slot_at(index);
                if (this->key_equal_(target->value.first, key)) {
                    this->erase_slot(index);
                    return 1;
                }
            }
            if (mask32.maskEmpty != 0) {
                break;
            }
            dist_and_hash.incDist(kGroupWidth);
            slot_index = this->slot_next_group(slot_index);
        } while (slot_index < max_slot_index);

        return 0;
    }

    JSTD_NO_INLINE
    void erase_slot(size_type to_erase) {
        assert(to_erase <= this->slot_capacity());
        assert(this->ctrl_at(to_erase)->isUsed());
        size_type prev_index;
        size_type last_index = npos;
        size_type first_index = this->next_index(to_erase);
        size_type slot_index = first_index;

        if (1) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            if (kUnrollMode == UnrollMode16) {
                if (ctrl->isEmptyOrZero()) {
                    prev_index = to_erase;
                    goto ClearSlot;
                }

                ctrl++;
                slot_index++;

                if (ctrl->isEmptyOrZero()) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;

                if (ctrl->isEmptyOrZero()) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;

                if (ctrl->isEmptyOrZero()) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;
            } else if (kUnrollMode == UnrollMode8) {
                if (std::uint8_t(ctrl->dist + 1) < 2) {
                    prev_index = to_erase;
                    goto ClearSlot;
                }

                ctrl++;
                slot_index++;

                if (std::uint8_t(ctrl->dist + 1) < 2) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;

                if (std::uint8_t(ctrl->dist + 1) < 2) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;

                if (std::uint8_t(ctrl->dist + 1) < 2) {
                    last_index = slot_index;
                    goto TransferSlots;
                }

                ctrl++;
                slot_index++;
            }

            size_type max_slot_index = this->max_slot_capacity();

            if (this->slot_capacity() >= kGroupWidth) {
                do {
                    const group_type & group = this->get_group(slot_index);
                    auto maskEmpty = group.matchEmptyOrZero();
                    if (maskEmpty != 0) {
                        size_type pos = BitUtils::bsf32(maskEmpty);
                        maskEmpty = BitUtils::clearLowBit32(maskEmpty);
                        size_type index = group.index(slot_index, pos);
                        last_index = index;
                        break;
                    }
                    slot_index = this->slot_next_group(slot_index);
                } while (slot_index < max_slot_index);
            } else {
                while (slot_index < max_slot_index) {
                    ctrl++;
                    slot_index++;
                    if (ctrl->isEmptyOrZero()) {
                        last_index = slot_index;
                        break;
                    }
                }
            }

            goto TransferSlots;
        }

TransferSlots:
        assert(last_index != npos);
        prev_index = to_erase;
        slot_index = first_index;
        while (slot_index != last_index) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            assert(ctrl->dist > 0);
            std::int16_t dist_and_hash = ctrl->value;
            dist_and_hash--;
            this->setUsedCtrl(prev_index, dist_and_hash);

            slot_type * prev_slot = this->slot_at(prev_index);
            slot_type * slot = this->slot_at(slot_index);
            this->transfer_slot(prev_slot, slot);

            prev_index = slot_index;
            slot_index = this->next_index(slot_index);
        }

ClearSlot:
        // Setting to empty ctrl
        ctrl_type * prev_ctrl = this->ctrl_at(prev_index);
        assert(prev_ctrl->isUsed());
        this->setUnusedCtrl(prev_index, kEmptySlot);
        // Destroy slot
        this->destroy_slot(prev_index);

        assert(this->slot_size_ > 0);
        this->slot_size_--;
    }

    void swap_content(robin_hash_map & other) {
        using std::swap;
        swap(this->ctrls_, other.ctrls());
        swap(this->slots_, other.slots());
        swap(this->slot_size_, other.slot_size());
        swap(this->slot_mask_, other.slot_mask());
        swap(this->max_lookups_, other.max_lookups());
        swap(this->slot_threshold_, other.slot_threshold());
        swap(this->n_mlf_, other.integral_mlf());
        swap(this->n_mlf_rev_, other.integral_mlf_rev());
#if ROBIN_USE_HASH_POLICY
        swap(this->hash_policy_, other.hash_policy_ref());
#endif
    }

    void swap_policy(robin_hash_map & other) {
        using std::swap;
        swap(this->hasher_, other.hash_function_ref());
        swap(this->key_equal_, other.key_eq_ref());
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
            swap(this->allocator_, other.get_allocator_ref());
        }
        if (std::allocator_traits<mutable_allocator_type>::propagate_on_container_swap::value) {
            swap(this->mutable_allocator_, other.get_mutable_allocator_ref());
        }
        if (std::allocator_traits<ctrl_allocator_type>::propagate_on_container_swap::value) {
            swap(this->ctrl_allocator_, other.get_ctrl_allocator_ref());
        }
        if (std::allocator_traits<slot_allocator_type>::propagate_on_container_swap::value) {
            swap(this->slot_allocator_, other.get_slot_allocator_ref());
        }
    }

    void swap_impl(robin_hash_map & other) {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

} // namespace jstd
