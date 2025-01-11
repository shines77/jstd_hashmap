
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
#include "jstd/traits/type_traits.h"
#include "jstd/iterator.h"
#include "jstd/utility/utility.h"
#include "jstd/lang/launder.h"
#include "jstd/hasher/hashes.h"
#include "jstd/hasher/hash_crc32.h"
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

#define ROBIN16_USE_HASH_POLICY     0

namespace jstd {

template < typename Key, typename Value,
           typename Hash = std::hash<typename std::remove_const<Key>::type>,
           typename KeyEqual = std::equal_to<typename std::remove_const<Key>::type>,
           typename Allocator = std::allocator<std::pair<typename std::add_const<typename std::remove_const<Key>::type>::type,
                                                         typename std::remove_const<Value>::type>> >
class JSTD_DLL robin16_hash_map {
public:
    typedef typename std::remove_cv<Key>::type      key_type;
    typedef typename std::remove_cv<Value>::type    mapped_type;

    typedef std::pair<const key_type, mapped_type>  value_type;
    typedef std::pair<key_type, mapped_type>        mutable_value_type;

    static constexpr bool kIsCompatibleLayout =
            std::is_same<value_type, mutable_value_type>::value ||
            is_layout_compatible_pair<value_type, mutable_value_type>::value;

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
    typedef robin16_hash_map<Key, Value, Hash, KeyEqual, Allocator>
                                                    this_type;

    static constexpr bool kUseUnrollLoop = true;
    static constexpr bool kUseIndexSalt = false;

    static constexpr size_type npos = size_type(-1);

    static constexpr size_type kCtrlHashMask = 0x000000FFul;
    static constexpr size_type kCtrlShift    = 8;

    static constexpr size_type kGroupBits   = 4;
    static constexpr size_type kGroupWidth  = size_type(1) << kGroupBits;
    static constexpr size_type kGroupMask   = kGroupWidth - 1;
    static constexpr size_type kGroupShift  = kCtrlShift + kGroupBits;

    // kMinCapacity must be >= 2
    static constexpr size_type kMinCapacity = 4;
    // kDefaultCapacity must be >= kMinCapacity
    static constexpr size_type kDefaultCapacity = 4;

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
    static constexpr bool isPlainKeyHash = isGccOrClang &&
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

    static constexpr std::uint8_t kEmptySlot    = 0b11111111;
    static constexpr std::uint8_t kEndOfMark    = 0b11111110;
    static constexpr std::uint8_t kUnusedMask   = 0b10000000;
    static constexpr std::uint8_t kHash2Mask    = 0b11111111;

    static constexpr std::uint8_t kDistLimit    = 0b10000000;

    static constexpr std::uint32_t kFullMask16  = 0x0000FFFFul;
    static constexpr std::uint32_t kFullMask32  = 0xFFFFFFFFul;

    static constexpr std::uint32_t kFullMask32_Half  = 0x55555555ul;
    static constexpr std::uint32_t kFullMask32_Half2 = 0xAAAAAAAAul;

    static constexpr std::uint64_t kEmptySlot64  = 0xFFFFFFFFFFFFFFFFull;
    static constexpr std::uint64_t kEndOfMark64  = 0xFEFEFEFEFEFEFEFEull;
    static constexpr std::uint64_t kUnusedMask64 = 0x8080808080808080ull;

#if 1
    struct ctrl_data {
        union {
            struct {
                std::uint8_t distance;
                std::uint8_t hash;
            };
            std::uint16_t value;
        };

        static constexpr std::uint8_t kMinUnusedSlot = cmin(kEmptySlot, kEndOfMark);

        ctrl_data() noexcept {
        }

        explicit ctrl_data(std::uint16_t value) noexcept
            : value(value) {
        }

        ctrl_data(std::uint8_t distance, std::uint8_t hash) noexcept
            : distance(distance), hash(hash) {
        }

        ~ctrl_data() = default;


        bool isEmpty() const {
            return (this->distance == kEmptySlot);
        }

        static bool isEmpty(std::uint8_t tag) {
            return (tag == kEmptySlot);
        }

        bool isEmptyOnly() const {
            return (this->distance == kEmptySlot);
        }

        static bool isEmptyOnly(std::uint8_t tag) {
            return (tag == kEmptySlot);
        }

        bool isNonEmpty() const {
            return !(this->isEmpty());
        }

        static bool isNonEmpty(std::uint8_t tag) {
            return !ctrl_data::isEmpty(tag);
        }

        bool isEmptyOrZero() const {
#if 1
            return (std::uint8_t(this->distance + 1) < std::uint8_t(2));
#else
            return (this->distance == kEmptySlot || this->distance == 0);
#endif
        }

        bool isEndOf() const {
            return (this->distance == kEndOfMark);
        }

        bool isUsed() const {
            return (this->distance < kEmptySlot);
        }

        static bool isUsed(std::uint8_t tag) {
            return (tag < kEmptySlot);
        }

        bool isUnused() const {
            return (this->distance >= kEmptySlot);
        }

        static bool isUnused(std::uint8_t tag) {
            return (tag >= kEmptySlot);
        }

        void setHash(std::uint8_t ctrl_hash) {
            this->hash = ctrl_hash;
        }

        void setEmpty() {
            this->distance = kEmptySlot;
        }

        void setEndOf() {
            this->distance = kEndOfMark;
        }

        void setDistance(std::uint8_t distance) {
            this->distance = distance;
        }

        void setUsed(std::uint8_t distance, std::uint8_t ctrl_hash) {
            assert(distance < kEmptySlot);
            this->setDistance(distance);
            this->setHash(ctrl_hash);
        }

        void setValue(std::uint16_t dist_and_hash) {
            this->value = dist_and_hash;
        }
    };
#else
    struct ctrl_data {
        union {
            struct {
                std::uint8_t distance;
                std::uint8_t hash;
            };
            std::uint16_t value;
        };

        static constexpr std::uint8_t kMinUnusedSlot = cmin(kEmptySlot, kEndOfMark);

        ctrl_data() noexcept {
        }

        explicit ctrl_data(std::uint16_t value) noexcept
            : value(value) {
        }

        ctrl_data(std::uint8_t distance, std::uint8_t hash) noexcept
            : distance(distance), hash(hash) {
        }

        ~ctrl_data() = default;

        bool isEmpty() const {
            return (this->distance >= kEndOfMark);
        }

        static bool isEmpty(std::uint8_t tag) {
            return (tag >= kEndOfMark);
        }

        bool isEmptyOnly() const {
            return (this->distance == kEmptySlot);
        }

        static bool isEmptyOnly(std::uint8_t tag) {
            return (tag == kEmptySlot);
        }

        bool isNonEmpty() const {
            return !(this->isEmpty());
        }

        static bool isNonEmpty(std::uint8_t tag) {
            return !ctrl_data::isEmpty(tag);
        }

        bool isEmptyOrZero() const {
#if 1
            return (std::uint8_t(this->distance + 1) < std::uint8_t(2));
#else
            return (this->distance == kEmptySlot || this->distance == 0);
#endif
        }

        bool isEndOf() const {
            return (this->distance == kEndOfMark);
        }

        bool isUsed() const {
            return (this->distance < kEndOfMark);
        }

        static bool isUsed(std::uint8_t tag) {
            return (tag < kEndOfMark);
        }

        bool isUnused() const {
            return (this->distance >= kEndOfMark);
        }

        static bool isUnused(std::uint8_t tag) {
            return (tag >= kEndOfMark);
        }

        void setHash(std::uint8_t ctrl_hash) {
            this->hash = ctrl_hash;
        }

        void setEmpty() {
            this->distance = kEmptySlot;
        }

        void setEndOf() {
            this->distance = kEndOfMark;
        }

        void setDistance(std::uint8_t distance) {
            this->distance = distance;
        }

        void setUsed(std::uint8_t distance, std::uint8_t ctrl_hash) {
            assert(distance < kEmptySlot);
            this->setDistance(distance);
            this->setHash(ctrl_hash);
        }

        void setValue(std::uint16_t dist_and_hash) {
            this->value = dist_and_hash;
        }
    };
#endif

    typedef ctrl_data ctrl_type;

    static constexpr size_type kGroupSize = kGroupWidth * sizeof(ctrl_type);

    template <typename T>
    struct MatchMask2 {
        typedef T mask32_type;
        mask32_type maskHash;
        mask32_type maskEmpty;

        MatchMask2(mask32_type maskHash, mask32_type maskEmpty)
            : maskHash(maskHash), maskEmpty(maskEmpty) {
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
        mask32_type maskHash;
        mask32_type maskEmpty;
        mask32_type maskDist;

        MatchMask3(mask32_type maskHash, mask32_type maskEmpty, mask32_type maskDist)
            : maskHash(maskHash), maskEmpty(maskEmpty), maskDist(maskDist) {
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
            this->template fillAll<kEmptySlot>(data);
        }

        void setAllZeros(pointer data) {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)data, zero_bits);
        }

        template <std::uint16_t ControlTag>
        void fillAll(pointer data) {
            const __m256i tag_bits = _mm256_set1_epi16(ControlTag);
            _mm256_storeu_si256((__m256i *)data, tag_bits);
        }

        __m256i matchTag256(const_pointer data, std::uint16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            return match_mask;
        }

        std::uint32_t matchTag(const_pointer data, std::uint16_t ctrl_tag) const {
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
            __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i match_mask = _mm256_cmpeq_epi16(tag_bits, low_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(tag_bits, high_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHash(const_pointer data, std::uint8_t ctrl_hash) const {
            return this->matchHighTag(data, ctrl_hash);
        }

        std::uint32_t matchEmpty(const_pointer data) const {
            return this->matchLowTag(data, kEmptySlot);
        }

        std::uint32_t matchUsed(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(kEndOfMark);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i match_mask = _mm256_cmpgt_epi16(tag_bits, low_bits);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused(const_pointer data) const {
            return this->matchEmpty(data);
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(const_pointer data, std::uint8_t ctrl_hash, std::uint8_t distance) const {
            const __m256i kLowMask16  = _mm256_set1_epi16((short)0x00FF);
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
            assert(distance < kEmptySlot);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i dist_value = _mm256_set1_epi16(distance);
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
            __m256i dist_bits  = _mm256_adds_epi16(kDistanceBase, dist_value);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, kLowMask16);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i empty_mask = _mm256_cmpeq_epi16(empty_bits, low_bits);
            __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, low_bits);
                    empty_mask = _mm256_or_si256(empty_mask, dist_mask);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, hash_bits);
            __m256i result_mask = _mm256_andnot_si256(empty_mask, match_mask);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(empty_mask);
            std::uint32_t maskHash  = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return { maskHash, maskEmpty };
        }

        std::uint32_t matchEmptyOrZero(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i zero_bits  = _mm256_setzero_si256();
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(low_bits, empty_bits);
                __m256i zero_mask  = _mm256_cmpeq_epi16(low_bits, zero_bits);
                __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
                return maskEmpty;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i zero_bits  = _mm256_setzero_si256();
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(low_bits, empty_bits);
                __m256i zero_mask  = _mm256_cmpeq_epi16(low_bits, zero_bits);
                __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
                return maskEmpty;
            }
        }

        std::uint32_t matchUsedOrEndOf(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i match_mask = _mm256_cmpgt_epi16(empty_bits, low_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i match_mask = _mm256_cmpgt_epi16(empty_bits, low_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchEmptyAndDistance(const_pointer data, std::uint8_t distance) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
            assert(distance < kEmptySlot);
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i dist_value = _mm256_set1_epi16(distance);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i ctrl_dist  = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i dist_bits  = _mm256_adds_epi16(dist_value, kDistanceBase);
                __m256i empty_mask = _mm256_cmpeq_epi16(ctrl_dist, empty_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
                __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
                return maskEmpty;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i dist_value = _mm256_set1_epi16(distance);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i ctrl_dist  = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i dist_bits  = _mm256_adds_epi16(dist_value, kDistanceBase);
                __m256i empty_mask = _mm256_cmpeq_epi16(ctrl_dist, empty_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
                __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
                return maskEmpty;
            }
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t ctrl_hash) const {
            return (this->matchHash(data, ctrl_hash) != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            return (this->matchEmpty(data) != 0);
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

        bool isAllUsed(const_pointer data) const {
            return (this->matchUnused(data) == 0);
        }

        bool isAllUnused(const_pointer data) const {
            return (this->matchUnused(data) == kFullMask16);
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
            this->template fillAll<kEmptySlot>(data);
        }

        void setAllZeros(pointer data) {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)data, zero_bits);
        }

        template <std::uint16_t ControlTag>
        void fillAll(pointer data) {
            const __m256i tag_bits = _mm256_set1_epi16((short)ControlTag);
            _mm256_storeu_si256((__m256i *)data, tag_bits);
        }

        __m256i matchTag256(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits  = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            return match_mask;
        }

        std::uint32_t matchTag(const_pointer data, std::uint16_t ctrl_tag) const {
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
            __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i match_mask = _mm256_cmpeq_epi16(tag_bits, low_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(const_pointer data, std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(tag_bits, high_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHash(const_pointer data, std::uint8_t ctrl_hash) const {
            return this->matchHighTag(data, ctrl_hash);
        }

        std::uint32_t matchEmpty(const_pointer data) const {
            return this->matchLowTag(data, kEmptySlot);
        }

        std::uint32_t matchUsed(const_pointer data) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i tag_bits   = _mm256_set1_epi16(kEndOfMark);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i match_mask = _mm256_cmpgt_epi16(tag_bits, low_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused(const_pointer data) const {
            return this->matchEmpty(data);
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(const_pointer data, std::uint8_t ctrl_hash, std::uint8_t distance) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
#ifdef _DEBUG
            if (distance > kDistLimit)
                distance = distance;
#endif
            assert(distance < kEmptySlot);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
            __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
            __m256i dist_value = _mm256_set1_epi16(distance);
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
            __m256i dist_bits  = _mm256_adds_epi16(dist_value, kDistanceBase);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i empty_mask = _mm256_cmpeq_epi16(empty_bits, low_bits);
            __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, low_bits);
                    empty_mask = _mm256_or_si256(empty_mask, dist_mask);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, hash_bits);
            __m256i result_mask = _mm256_andnot_si256(empty_mask, match_mask);
                    empty_mask  = _mm256_srli_epi16(empty_mask, 8);
                    result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(empty_mask);
            std::uint32_t maskHash  = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return { maskHash, maskEmpty };
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistFast2(const_pointer data, std::uint8_t ctrl_hash, std::uint8_t distance) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
#ifdef _DEBUG
            if (distance > kDistLimit)
                distance = distance;
#endif
            assert(distance < kEmptySlot);
            std::uint16_t dist_and_hash = (ctrl_hash << 8) | distance;
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)data);
            __m256i empty_bits  = _mm256_set1_epi16(kEmptySlot);
            __m256i dist_0_hash = _mm256_set1_epi16((short)dist_and_hash);
            __m256i ones_bits   = _mm256_setones_si256();
            __m256i low_mask    = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits    = _mm256_and_si256(ctrl_bits, low_mask);
            __m256i dist_1_hash = _mm256_adds_epi16(dist_0_hash, kDistanceBase);
            __m256i match_mask  = _mm256_cmpeq_epi16(dist_1_hash, ctrl_bits);
            if (_mm256_test_all_zeros(match_mask, match_mask) == 0) {
                // match_mask is not all zeros
                __m256i dist_bits  = _mm256_and_si256(dist_1_hash, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(empty_bits, low_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, low_bits);
                        empty_mask = _mm256_or_si256(empty_mask, dist_mask);
                __m256i result_mask = _mm256_andnot_si256(empty_mask, match_mask);
                        result_mask = _mm256_srli_epi16(result_mask, 8);
                std::uint32_t maskHash = (std::uint32_t)_mm256_movemask_epi8(result_mask);
                return { maskHash, 0 };
            } else {
                // match_mask is all zeros
                __m256i dist_bits  = _mm256_and_si256(dist_1_hash, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(empty_bits, low_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, low_bits);
                        empty_mask = _mm256_or_si256(empty_mask, dist_mask);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(empty_mask);
                return { 0, maskEmpty };
            }
        }

        std::uint32_t matchEmptyOrZero(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i zero_bits  = _mm256_setzero_si256();
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(low_bits, empty_bits);
                __m256i zero_mask  = _mm256_cmpeq_epi16(low_bits, zero_bits);
                __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                        result_mask = _mm256_srli_epi16(result_mask, 8);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
                return maskEmpty;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i zero_bits  = _mm256_setzero_si256();
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i empty_mask = _mm256_cmpeq_epi16(low_bits, empty_bits);
                __m256i zero_mask  = _mm256_cmpeq_epi16(low_bits, zero_bits);
                __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                        result_mask = _mm256_srli_epi16(result_mask, 8);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
                return maskEmpty;
            }
        }

        std::uint32_t matchUsedOrEndOf(const_pointer data) const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i match_mask = _mm256_cmpgt_epi16(empty_bits, low_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i match_mask = _mm256_cmpgt_epi16(empty_bits, low_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchEmptyAndDistance(const_pointer data, std::uint8_t distance) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
                                  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F);
            assert(distance < kEmptySlot);
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i dist_value = _mm256_set1_epi16(distance);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i empty_bits = low_mask;
                __m256i ctrl_dist  = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i dist_bits  = _mm256_adds_epi16(dist_value, kDistanceBase);
                __m256i empty_mask = _mm256_cmpeq_epi16(ctrl_dist, empty_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
                __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
                //        result_mask = _mm256_srli_epi16(result_mask, 8);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
                return maskEmpty;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)data);
                __m256i empty_bits = _mm256_set1_epi16(kEmptySlot);
                __m256i dist_value = _mm256_set1_epi16(distance);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i low_mask   = _mm256_srli_epi16(ones_bits, 8);
                __m256i ctrl_dist  = _mm256_and_si256(ctrl_bits, low_mask);
                __m256i dist_bits  = _mm256_adds_epi16(dist_value, kDistanceBase);
                __m256i empty_mask = _mm256_cmpeq_epi16(ctrl_dist, empty_bits);
                __m256i dist_mask  = _mm256_cmpgt_epi16(dist_bits, ctrl_dist);
                __m256i result_mask = _mm256_or_si256(empty_mask, dist_mask);
                //        result_mask = _mm256_srli_epi16(result_mask, 8);
                std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
                return maskEmpty;
            }
        }

        bool hasAnyMatch(const_pointer data, std::uint8_t ctrl_hash) const {
            return (this->matchHash(data, ctrl_hash) != 0);
        }

        bool hasAnyEmpty(const_pointer data) const {
            return (this->matchEmpty(data) != 0);
        }

        bool hasAnyUsed(const_pointer data) const {
            return (this->matchUsed(data) != 0);
        }

        bool hasAnyUnused(const_pointer data) const {
            return (this->matchUnused(data) != 0);
        }

        bool isAllEmpty(const_pointer data) const {
            return (this->matchEmpty(data) == kFullMask32_Half);
        }

        bool isAllUsed(const_pointer data) const {
            return (this->matchUnused(data) == 0);
        }

        bool isAllUnused(const_pointer data) const {
            return (this->matchUnused(data) == kFullMask32_Half);
        }

        static inline size_type bitPos(size_type pos) {
            return (pos >> 1);
        }
    };

    template <typename T>
    using BitMask256 = BitMask256_AVX2<T>;

#else

    static_assert(false, "jstd::robin16_hash_map<K,V> required Intel AVX2 or heigher intrinsics.")

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

        template <std::uint8_t ControlTag>
        void fillAll() {
            bitmask.template fillAll<ControlTag>(&this->ctrls[0]);
        }

        bitmask_type matchTag(std::uint8_t ctrl_tag) const {
            return bitmask.matchTag(&this->ctrls[0], ctrl_tag);
        }

        bitmask_type matchHash(std::uint8_t ctrl_hash) const {
            return bitmask.matchHash(&this->ctrls[0], ctrl_hash);
        }

        MatchMask2<bitmask_type>
        matchHashAndDistance(std::uint8_t ctrl_hash, std::uint8_t distance) const {
            return bitmask.matchHashAndDistance(&this->ctrls[0], ctrl_hash, distance);
        }

        bitmask_type matchEmpty() const {
            return bitmask.matchEmpty(&this->ctrls[0]);
        }

        bitmask_type matchEmptyOrZero() const {
            return bitmask.matchEmptyOrZero(&this->ctrls[0]);
        }

        bitmask_type matchEmptyAndDistance(std::uint8_t distance) const {
            return bitmask.matchEmptyAndDistance(&this->ctrls[0], distance);
        }

        bitmask_type matchUsedOrEndOf() const {
            return bitmask.matchUsedOrEndOf(&this->ctrls[0]);
        }

        bitmask_type matchUsed() const {
            return bitmask.matchUsed(&this->ctrls[0]);
        }

        bitmask_type matchUnused() const {
            return bitmask.matchUnused(&this->ctrls[0]);
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
        ~slot_type() = delete;
    };

    typedef slot_type       mutable_slot_type;
    typedef slot_type       node_type;

#if 1
    template <typename ValueType>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = ValueType;
        using pointer = ValueType *;
        using reference = ValueType &;

        using mutable_value_type = typename std::remove_const<ValueType>::type;

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
            const ctrl_type * ctrl;
            do {
                ++(this->index_);
                ctrl = this->owner_->ctrl_at(this->index_);
            } while (ctrl->isEmptyOnly() && (this->index_ < this->owner_->slot_capacity()));
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

        operator basic_iterator<const mutable_value_type>() const {
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

        using mutable_value_type = typename std::remove_const<ValueType>::type;

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

        operator basic_iterator<const mutable_value_type>() const {
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
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<slot_type>
                                        slot_allocator_type;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<mutable_slot_type>
                                        mutable_slot_allocator_type;

private:
    group_type *    groups_;
    size_type       group_mask_;

    slot_type *     slots_;
    size_type       slot_size_;
    size_type       slot_mask_;

    size_type       slot_threshold_;
    std::uint32_t   n_mlf_;
    std::uint32_t   n_mlf_rev_;
#if ROBIN16_USE_HASH_POLICY
    hash_policy_t   hash_policy_;
#endif

    hasher          hasher_;
    key_equal       key_equal_;

    allocator_type              allocator_;
    mutable_allocator_type      mutable_allocator_;
    slot_allocator_type         slot_allocator_;
    mutable_slot_allocator_type mutable_slot_allocator_;

public:
    robin16_hash_map() : robin16_hash_map(kDefaultCapacity) {
    }

    explicit robin16_hash_map(size_type init_capacity,
                              const hasher & hash = hasher(),
                              const key_equal & equal = key_equal(),
                              const allocator_type & alloc = allocator_type()) :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        slot_allocator_(alloc), mutable_slot_allocator_(alloc) {
        this->create_group<true>(init_capacity);
    }

    explicit robin16_hash_map(const allocator_type & alloc)
        : robin16_hash_map(kDefaultCapacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    robin16_hash_map(InputIter first, InputIter last,
                     size_type init_capacity = kDefaultCapacity,
                     const hasher & hash = hasher(),
                     const key_equal & equal = key_equal(),
                     const allocator_type & alloc = allocator_type()) :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        slot_allocator_(alloc), mutable_slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type input_size = distance(first, last);
        size_type reserve_capacity = (init_capacity >= input_size) ? init_capacity : input_size;
        this->reserve_for_insert(reserve_capacity);
        this->insert(first, last);
    }

    template <typename InputIter>
    robin16_hash_map(InputIter first, InputIter last,
                     size_type init_capacity,
                     const allocator_type & alloc)
        : robin16_hash_map(first, last, init_capacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    robin16_hash_map(InputIter first, InputIter last,
                     size_type init_capacity,
                     const hasher & hash,
                     const allocator_type & alloc)
        : robin16_hash_map(first, last, init_capacity, hash, key_equal(), alloc) {
    }

    robin16_hash_map(const robin16_hash_map & other)
        : robin16_hash_map(other, std::allocator_traits<allocator_type>::
                                  select_on_container_copy_construction(other.get_allocator())) {
    }

    robin16_hash_map(const robin16_hash_map & other, const Allocator & alloc) :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN16_USE_HASH_POLICY
        hash_policy_(other.hash_policy_ref()),
#endif
        hasher_(hasher()), key_equal_(key_equal()),
        allocator_(alloc), mutable_allocator_(alloc),
        slot_allocator_(alloc), mutable_slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type other_size = other.slot_size();
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

    robin16_hash_map(robin16_hash_map && other) noexcept :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN16_USE_HASH_POLICY
        hash_policy_(other.hash_policy_ref()),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(std::move(other.get_allocator_ref())),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())),
        mutable_slot_allocator_(std::move(other.get_mutable_slot_allocator_ref())) {
        // Swap content only
        this->swap_content(other);
    }

    robin16_hash_map(robin16_hash_map && other, const Allocator & alloc) noexcept :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN16_USE_HASH_POLICY
        hash_policy_(other.hash_policy_ref()),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(alloc),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())),
        mutable_slot_allocator_(std::move(other.get_mutable_slot_allocator_ref())) {
        // Swap content only
        this->swap_content(other);
    }

    robin16_hash_map(std::initializer_list<value_type> init_list,
                     size_type init_capacity = kDefaultCapacity,
                     const hasher & hash = hasher(),
                     const key_equal & equal = key_equal(),
                     const allocator_type & alloc = allocator_type()) :
        groups_(nullptr), group_mask_(0),
        slots_(nullptr), slot_size_(0), slot_mask_(0),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN16_USE_HASH_POLICY
        hash_policy_(),
#endif
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), mutable_allocator_(alloc),
        slot_allocator_(alloc), mutable_slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type reserve_capacity = (init_capacity >= init_list.size()) ? init_capacity : init_list.size();
        this->reserve_for_insert(reserve_capacity);
        this->insert(init_list.begin(), init_list.end());
    }

    robin16_hash_map(std::initializer_list<value_type> init_list,
                     size_type init_capacity,
                     const allocator_type & alloc)
        : robin16_hash_map(init_list, init_capacity, hasher(), key_equal(), alloc) {
    }

    robin16_hash_map(std::initializer_list<value_type> init_list,
                     size_type init_capacity,
                     const hasher & hash,
                     const allocator_type & alloc)
        : robin16_hash_map(init_list, init_capacity, hash, key_equal(), alloc) {
    }

    ~robin16_hash_map() {
        this->destroy<true>();
    }

    bool is_valid() const { return (this->groups() != nullptr); }
    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const  { return (this->size() > this->slot_mask()); }

    bool empty() const { return this->is_empty(); }

    size_type size() const { return slot_size_; }
    size_type capacity() const { return (slot_mask_ + 1); }

    group_type * groups() { return groups_; }
    const group_type * groups() const { return groups_; }

    ctrl_type * ctrls() { return (ctrl_type *)this->groups(); }
    const ctrl_type * ctrls() const { return (const ctrl_type *)this->groups(); }

    size_type group_mask() const { return group_mask_; }
    size_type group_count() const { return (group_mask_ + 1); }
    size_type group_capacity() const { return (this->group_count() + 1); }

    slot_type * slots() { return slots_; }
    const slot_type * slots() const { return slots_; }

    size_type slot_size() const { return slot_size_; }
    size_type slot_mask() const { return slot_mask_; }
    size_type slot_capacity() const { return (slot_mask_ + 1); }
    size_type slot_threshold() const { return slot_threshold_; }
    size_type slot_threshold(size_type now_slow_capacity) const {
        return (now_slow_capacity * this->integral_mlf() / kLoadFactorAmplify);
    }

    constexpr size_type bucket_count() const {
        return kGroupWidth;
    }

    size_type bucket(const key_type & key) const {
        size_type index = this->find_impl(key);
        return index;
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
        assert(!(this->ctrl_at(this->slot_capacity())->isEmptyOnly()));
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

        return this->iterator_at(this->slot_capacity());
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
        return iterator_at(this->slot_capacity());
    }

    const_iterator end() const {
        return iterator_at(this->slot_capacity());
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

#if ROBIN16_USE_HASH_POLICY
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

    slot_allocator_type get_slot_allocator() const noexcept {
        return this->slot_allocator_;
    }

    mutable_slot_allocator_type get_mutable_slot_allocator() const noexcept {
        return this->mutable_slot_allocator_;
    }

    hasher & hash_function_ref() noexcept {
        return this->hasher_;
    }

    key_equal & key_eq_ref() noexcept {
        return this->key_equal_;
    }

#if ROBIN16_USE_HASH_POLICY
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

    slot_allocator_type & get_slot_allocator_ref() noexcept {
        return this->slot_allocator_;
    }

    mutable_slot_allocator_type & get_mutable_slot_allocator_ref() noexcept {
        return this->mutable_slot_allocator_;
    }

    static const char * name() {
        return "jstd::robin16_hash_map<K, V>";
    }

    void clear(bool need_destroy = false) noexcept {
        if (this->slot_capacity() > kDefaultCapacity) {
            if (need_destroy) {
                this->destroy<true>();
                this->create_group<false>(kDefaultCapacity);
                assert(this->slot_size() == 0);
                return;
            }
        }
        this->destroy<false>();
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

    void swap(robin16_hash_map & other) {
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
        if (index <= this->slot_mask()) {
            slot_type * slot = this->slot_at(index);
            return slot->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::robin16_hash_map<K,V>::at(key), "
                                    "the specified key is not exists.");
        }
    }

    const mapped_type & at(const key_type & key) const {
        size_type index = this->find_impl(key);
        if (index <= this->slot_mask()) {
            const slot_type * slot = this->slot_at(index);
            return slot->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::robin16_hash_map<K,V>::at(key) const, "
                                    "the specified key is not exists.");
        }
    }

    size_type count(const key_type & key) const {
        size_type index = this->find_impl(key);
        return (index <= this->slot_mask()) ? size_type(1) : size_type(0);
    }

    bool contains(const key_type & key) const {
        size_type index = this->find_impl(key);
        return (index <= this->slot_mask());
    }

    iterator find(const key_type & key) {
        size_type index = this->find_impl(key);
        return this->iterator_at(index);
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
        this->erase_slot(index);
        return ++pos;
    }

    const_iterator erase(const_iterator pos) {
        size_type index = this->index_of(pos);
        this->erase_slot(index);
        return ++pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        // TODO: There is a bug, which needs to be traversed from the last to the first.
        for (; first != last; ++first) {
            size_type index = this->index_of(first);
            this->erase_slot(index);
        }
        return { first };
    }

private:
    JSTD_FORCED_INLINE
    size_type calc_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = (std::max)(init_capacity, kMinCapacity);
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinCapacity>(new_capacity);
        }
        return new_capacity;
    }

    JSTD_FORCED_INLINE
    size_type min_require_capacity(size_type init_capacity) {
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
        return (size_type)hashes::fibonacci_hash64((size_type)value);
#elif 1
        return (size_type)hashes::int_hash_crc32((size_type)value);
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
#if ROBIN16_USE_HASH_POLICY
        return value;
#elif 1
        return (size_type)hashes::fibonacci_hash64((size_type)value);
#elif 1
        return (size_type)hashes::simple_int_hash_crc32((size_type)value);
#endif
    }

    inline size_type index_salt() const noexcept {
        return (size_type)((std::uintptr_t)this->groups() >> 12);
    }

    inline size_type index_for_hash(hash_code_t hash_code) const noexcept {
#if ROBIN16_USE_HASH_POLICY
        return this->hash_policy_.template index_for_hash<key_type>(hash_code, this->slot_mask());
#else
        if (kUseIndexSalt)
            return ((this->get_second_hash((size_type)hash_code) ^ this->index_salt()) & this->slot_mask());
        else
            return  (this->get_second_hash((size_type)hash_code) & this->slot_mask());
#endif
    }

    inline size_type next_index(size_type index) const noexcept {
        return ((index + 1) & this->slot_mask());
    }

    inline size_type next_index(size_type index, size_type slot_mask) const noexcept {
        return ((index + 1) & slot_mask);
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

    std::uint8_t round_dist(size_type last, size_type first) const {
        size_type distance = (last + this->slot_capacity() - first) & this->slot_mask();
        assert(distance < (size_type)kEndOfMark);
#if 1
        return (std::uint8_t)distance;
#else
        return ((distance < (size_type)kEndOfMark) ? std::uint8_t(distance) : (kEndOfMark - 1));
#endif
    }

    inline size_type prev_group(size_type group_index) const noexcept {
        return ((group_index + this->group_mask()) & this->group_mask());
    }

    inline size_type next_group(size_type group_index) const noexcept {
        return ((group_index + 1) & this->group_mask());
    }

    inline size_type slot_prev_group(size_type slot_index) const noexcept {
        return ((slot_index + this->slot_mask()) & this->slot_mask());
    }

    inline size_type slot_next_group(size_type slot_index) const noexcept {
        return ((slot_index + kGroupWidth) & this->slot_mask());
    }

    ctrl_type * ctrl_at(size_type slot_index) noexcept {
        assert(slot_index <= this->slot_capacity());
        return (this->ctrls() + slot_index);
    }

    const ctrl_type * ctrl_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->slot_capacity());
        return (this->ctrls() + slot_index);
    }

    ctrl_type * ctrl_at_ex(size_type slot_index) noexcept {
        assert(slot_index <= (this->slot_capacity() + kGroupWidth));
        return (this->ctrls() + slot_index);
    }

    const ctrl_type * ctrl_at_ex(size_type slot_index) const noexcept {
        assert(slot_index <= (this->slot_capacity() + kGroupWidth));
        return (this->ctrls() + slot_index);
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
        assert(slot_index < this->slot_capacity());
        return (group_type *)(this->ctrl_at(slot_index));
    }

    const group_type * group_at(size_type slot_index) const noexcept {
        assert(slot_index < this->slot_capacity());
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
        assert(slot_index <= this->slot_capacity());
        return (this->slots() + slot_index);
    }

    const slot_type * slot_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->slot_capacity());
        return (this->slots() + slot_index);
    }

    ctrl_type & get_ctrl(size_type slot_index) {
        assert(slot_index < this->slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
    }

    const ctrl_type & get_ctrl(size_type slot_index) const {
        assert(slot_index < this->slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
    }

    group_type & get_group(size_type slot_index) {
        assert(slot_index < this->slot_capacity());
        return *this->group_at(slot_index);
    }

    const group_type & get_group(size_type slot_index) const {
        assert(slot_index < this->slot_capacity());
        return *this->group_at(slot_index);
    }

    group_type & get_physical_group(size_type group_index) {
        assert(group_index < this->group_count());
        return this->groups_[group_index];
    }

    const group_type & get_physical_group(size_type group_index) const {
        assert(group_index < this->group_count());
        return this->groups_[group_index];
    }

    slot_type & get_slot(size_type slot_index) {
        assert(slot_index < this->slot_capacity());
        return this->slots_[slot_index];
    }

    const slot_type & get_slot(size_type slot_index) const {
        assert(slot_index < this->slot_capacity());
        return this->slots_[slot_index];
    }

    size_type index_of(iterator pos) const {
        return this->index_of(pos.value());
    }

    size_type index_of(const_iterator pos) const {
        return this->index_of(pos.value());
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

    static void placement_new_slot(slot_type * slot) {
        // The construction of union doesn't do anything at runtime but it allows us
        // to access its members without violating aliasing rules.
        new (slot) slot_type;
    }

    template <bool finitial>
    void destroy() noexcept {
        this->destroy_slots<finitial>();

        // Note!!: destroy_slots() need use this->groups()
        this->destroy_group<finitial>();
    }

    template <bool finitial>
    void destroy_group() noexcept {
        if (this->groups_ != nullptr) {
            if (!finitial) {
                for (size_type group_index = 0; group_index <= this->group_mask(); group_index++) {
                    group_type * group = this->physical_group_at(group_index);
                    group->clear();
                }
            }
            if (finitial) {
                delete[] this->groups_;
                this->groups_ = nullptr;
            }
        }
    }

    template <bool finitial>
    void destroy_slots() noexcept(is_slot_trivial_destructor) {
        // Destroy all slots.
        if (this->slots_ != nullptr) {
            if (!is_slot_trivial_destructor) {
                ctrl_type * ctrl = this->ctrls();
                assert(ctrl != nullptr);
                for (size_type index = 0; index <= this->slot_mask(); index++) {
                    if (ctrl->isUsed()) {
                        this->destroy_slot(index);
                    }
                    ctrl++;
                }
            }
            if (finitial) {
                this->slot_allocator_.deallocate(this->slots_, this->slot_capacity());
                this->slots_ = nullptr;
            }
        }

        this->slot_size_ = 0;
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

    template <bool WriteEndofMark>
    JSTD_FORCED_INLINE
    void copy_and_mirror_ctrls() {
        // Copy and mirror the beginning ctrl bytes.
        size_type copy_size = (std::min)(this->slot_capacity(), kGroupWidth);
        ctrl_type * ctrls = this->ctrls();
        std::memcpy((void *)&ctrls[this->slot_capacity()], (const void *)&ctrls[0],
                    sizeof(ctrl_type) * copy_size);

        if (WriteEndofMark) {
            // Set the end of mark
            ctrl_type * ctrl_0 = ctrls;
            if (ctrl_0->isEmpty()) {
                ctrl_type * ctrl_mirror = this->ctrl_at(this->slot_capacity());
                ctrl_mirror->setEndOf();
            }
        }
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::uint8_t distance, std::uint8_t ctrl_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        ctrl->setUsed(distance, ctrl_hash);
        this->setUsedMirrorCtrl(index, distance, ctrl_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::uint16_t dist_and_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        ctrl->setValue(dist_and_hash);
        this->setUsedMirrorCtrl(index, dist_and_hash);
    }

    JSTD_FORCED_INLINE
    void setUnusedCtrl(size_type index, std::uint8_t tag) {
        ctrl_type * ctrl = this->ctrl_at(index);
        assert(ctrl->isUsed());
        ctrl->setDistance(tag);
        this->setUnusedMirrorCtrl(index, tag);
    }

    JSTD_FORCED_INLINE
    void setUsedMirrorCtrl(size_type index, std::uint8_t distance, std::uint8_t ctrl_hash) {
        assert(ctrl_type::isUsed(distance));
        if (index < kGroupWidth) {
            ctrl_type * ctrl_mirror = this->ctrl_at_ex(index + this->slot_capacity());
            ctrl_mirror->setUsed(distance, ctrl_hash);
        }
    }

    JSTD_FORCED_INLINE
    void setUsedMirrorCtrl(size_type index, std::uint16_t dist_and_hash) {
        if (index < kGroupWidth) {
            ctrl_type * ctrl_mirror = this->ctrl_at_ex(index + this->slot_capacity());
            ctrl_mirror->setValue(dist_and_hash);
        }
    }

    JSTD_FORCED_INLINE
    void setUnusedMirrorCtrl(size_type index, std::uint8_t tag) {
        if (index < kGroupWidth) {
            ctrl_type * ctrl_mirror = this->ctrl_at_ex(index + this->slot_capacity());
            ctrl_mirror->setDistance(tag);
#if 0
            if (index == 0) {
                assert(ctrl_type::isEmpty(tag));
                ctrl_mirror->setEndOf();
            }
#endif
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
        this->create_group<true>(new_capacity);
    }

    template <bool initialize = false>
    void create_group(size_type init_capacity) {
        size_type new_capacity;
        if (initialize)
            new_capacity = this->calc_capacity(init_capacity);
        else
            new_capacity = init_capacity;
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);

#if ROBIN16_USE_HASH_POLICY
        auto hash_policy_setting = this->hash_policy_.calc_next_capacity(new_capacity);
        this->hash_policy_.commit(hash_policy_setting);
#endif

        size_type group_count = (new_capacity + (kGroupWidth - 1)) / kGroupWidth;
        assert(group_count > 0);
        group_type * new_groups = new group_type[group_count + 1];
        groups_ = new_groups;
        group_mask_ = group_count - 1;

        for (size_type index = 0; index < group_count; index++) {
            new_groups[index].template fillAll<kEmptySlot>();
        }
        if (new_capacity >= kGroupWidth) {
            new_groups[group_count].template fillAll<kEmptySlot>();
        } else {
            assert(new_capacity < kGroupWidth);
            group_type * tail_group = (group_type *)((char *)new_groups + new_capacity * 2 * sizeof(ctrl_type));
            (*tail_group).template fillAll<kEndOfMark>();

            new_groups[group_count].template fillAll<kEndOfMark>();
        }

        slot_type * slots = slot_allocator_.allocate(new_capacity);
        slots_ = slots;
        if (initialize) {
            assert(slot_size_ == 0);
        } else {
            slot_size_ = 0;
        }
        slot_mask_ = new_capacity - 1;
        slot_threshold_ = this->slot_threshold(new_capacity);
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

            group_type * old_groups = this->groups();
            ctrl_type * old_ctrls = this->ctrls();
            size_type old_group_count = this->group_count();
            size_type old_group_capacity = this->group_capacity();

            slot_type * old_slots = this->slots();
            size_type old_slot_size = this->slot_size();
            size_type old_slot_mask = this->slot_mask();
            size_type old_slot_capacity = this->slot_capacity();

            this->create_group<false>(new_capacity);

            if ((this->max_load_factor() < 0.5f) && false) {
                ctrl_type * last_ctrl = old_ctrls + old_slot_capacity;
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
#if 1
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
#else
                    group_type * last_group = old_groups + old_group_count - 1;
                    size_type start_index = old_slot_capacity - kGroupWidth;
                    for (group_type * group = last_group; group >= old_groups; group--) {
                        std::uint32_t maskUsed = group->matchUsed();
                        while (maskUsed != 0) {
                            size_type pos = BitUtils::bsf32(maskUsed);
                            maskUsed = BitUtils::clearLowBit32(maskUsed);
                            size_type old_index = group->index(start_index, pos);
                            slot_type * old_slot = old_slots + old_index;
                            this->move_insert_unique(old_slot);
                            this->destroy_slot(old_slot);
                        }
                        start_index -= kGroupWidth;
                    }
#endif
                } else {
                    ctrl_type * last_ctrl = old_ctrls + old_slot_capacity;
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

            this->slot_allocator_.deallocate(old_slots, old_slot_capacity);

            if (old_groups != nullptr) {
                delete[] old_groups;
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

    template <typename Alloc, typename T, bool isCompatibleLayout,
              bool is_noexcept_move = is_noexcept_move_assignable<T>::value>
    struct swap_pair {
        static void swap(Alloc & alloc, T & a, T & b, T & tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value &&
                     std::is_nothrow_move_assignable<T>::value)
        {
            using std::swap;
            swap(a, b);
        }
    };

    template <typename Alloc, typename T>
    struct swap_pair<Alloc, T, true, false> {
        static void swap(Alloc & alloc, T & a, T & b, T & tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value)
        {
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
        typedef typename T::second_type                     second_type;
        typedef typename std::remove_cv<first_type>::type   mutable_first_type;
        typedef typename std::allocator_traits<Alloc>::template rebind_alloc<mutable_first_type>
                                                            mutable_first_allocator_type;
        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        static void swap(Alloc & alloc, T & a, T & b, T & tmp)
            noexcept(std::is_nothrow_move_constructible<mutable_first_type>::value &&
                     std::is_nothrow_move_assignable<mutable_first_type>::value &&
                     std::is_nothrow_move_constructible<second_type>::value &&
                     std::is_nothrow_move_assignable<second_type>::value)
        {
            using std::swap;
            swap(*mutable_key(&a), *mutable_key(&b));
            swap(a.second, b.second);
        }
    };

    template <typename Alloc, typename T>
    struct swap_pair<Alloc, T, false, false> {
        typedef typename T::first_type                      first_type;
        typedef typename T::second_type                     second_type;
        typedef typename std::remove_cv<first_type>::type   mutable_first_type;
        typedef typename std::allocator_traits<Alloc>::template rebind_alloc<mutable_first_type>
                                                            mutable_first_allocator_type;
        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        static void swap(Alloc & alloc, T & a, T & b, T & tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value &&
                     std::is_nothrow_move_constructible<second_type>::value &&
                     std::is_nothrow_move_assignable<second_type>::value)
        {
#if 1
            mutable_first_allocator_type first_allocator;

            first_allocator.construct(mutable_key(&tmp), std::move(*mutable_key(&a)));
            first_allocator.destroy(mutable_key(&a));
            first_allocator.construct(mutable_key(&a), std::move(*mutable_key(&b)));
            first_allocator.destroy(mutable_key(&b));
            first_allocator.construct(mutable_key(&b), std::move(*mutable_key(&tmp)));
            first_allocator.destroy(mutable_key(&tmp));

            using std::swap;
            swap(a.second, b.second);
#else
            mutable_first_allocator_type first_allocator;
            second_allocator_type second_allocator;

            first_allocator.construct(mutable_key(&tmp), std::move(*mutable_key(&a)));
            second_allocator.construct(&tmp.second, std::move(a.second));
            first_allocator.destroy(mutable_key(&a));
            second_allocator.destroy((&a.second);

            first_allocator.construct(mutable_key(&a), std::move(*mutable_key(&b)));
            second_allocator.construct(&a.second, std::move(b.second));
            first_allocator.destroy(mutable_key(&b));
            second_allocator.destroy((&b.second);

            first_allocator.construct(mutable_key(&b), std::move(*mutable_key(&tmp)));
            second_allocator.construct(&b.second, std::move(tmp.second));
            first_allocator.destroy(mutable_key(&tmp));
            second_allocator.destroy((&tmp.second);
#endif
        }
    };

    JSTD_FORCED_INLINE
    void swap_slot(size_type index1, size_type index2, slot_type * tmp) {
        slot_type * slot1 = this->slot_at(index1);
        slot_type * slot2 = this->slot_at(index2);
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

    size_type find_impl(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type slot_index = this->index_for_hash(hash_code);
        size_type start_slot = slot_index;
        std::uint8_t distance = 0;
        if (kUseUnrollLoop) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);

            // Optimize from: (ctrl->isUsed() && (ctrl->distance >= 0))
            if (likely(ctrl->isUsed())) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot_index;
                    }
                }
            } else {
                return this->slot_capacity();
            }

            ctrl = this->next_ctrl(slot_index);

            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->distance + 1) > 1)) {
            //if (likely(ctrl->isUsed() && (ctrl->distance >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        return slot_index;
                    }
                }
            } else {
                return this->slot_capacity();
            }

            distance = 2;
            ctrl = this->next_ctrl(slot_index);
#if 0
            do {
                // Optimization: merging two comparisons
                if (likely(std::uint8_t(ctrl->distance + 1) > distance)) {
                //if (likely(ctrl->isUsed() && (ctrl->distance >= distance))) {
                    if (ctrl->hash == ctrl_hash) {
                        const slot_type * slot = this->slot_at(slot_index);
                        if (this->key_equal_(slot->value.first, key)) {
                            return slot_index;
                        }
                    }
                    ctrl = this->next_ctrl(slot_index);
                    distance++;
                    if (distance >= 4)
                        break;
                } else {
                    return this->slot_capacity();
                }
            } while (1);
#endif
        }

        do {
            const group_type & group = this->get_group(slot_index);
            auto mask32 = group.matchHashAndDistance(ctrl_hash, distance);
            std::uint32_t maskHash = mask32.maskHash;
            size_type start_index = slot_index;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(start_index, pos);
                index = this->round_index(index);
                const slot_type * target = this->slot_at(index);
                if (this->key_equal_(target->value.first, key)) {
                    return index;
                }
            }
            if (mask32.maskEmpty != 0) {
                break;
            }
            distance += kGroupWidth;
            slot_index = this->slot_next_group(slot_index);
        } while (slot_index != start_slot);

        return this->slot_capacity();
    }

#if 0
    JSTD_FORCED_INLINE
    size_type find_impl(const key_type & key, size_type & first_slot, size_type & last_slot,
                        std::uint8_t & o_distance, std::uint8_t & o_ctrl_hash) const {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type start_slot = slot_index;
        std::uint8_t distance = 0;
        first_slot = start_slot;
        o_ctrl_hash = ctrl_hash;

        if (kUseUnrollLoop) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);
            if (likely(ctrl->isUsed() && (ctrl->distance >= 0))) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_distance = 0;
                        last_slot = slot_index;
                        return slot_index;
                    }
                }
            } else {
                return npos;
            }

            ctrl = this->next_ctrl(slot_index);
            if (likely(ctrl->isUsed() && (ctrl->distance >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    const slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_distance = 1;
                        last_slot = slot_index;
                        return slot_index;
                    }
                }
            } else {
                return npos;
            }
            distance = 2;
            slot_index = this->next_index(slot_index);
        }

        do {
            const group_type & group = this->get_group(slot_index);
            auto mask32 = group.matchHashAndDistance(ctrl_hash, distance);
            std::uint32_t maskHash = mask32.maskHash;
            size_type start_index = slot_index;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(start_index, pos);
                index = this->round_index(index);
                const slot_type * target = this->slot_at(index);
                if (this->key_equal_(target->value.first, key)) {
                    o_distance = this->round_dist(index, start_slot);
                    last_slot = index;
                    return index;
                }
            }
            if (mask32.maskEmpty != 0) {
                size_type pos = BitUtils::bsf32(mask32.maskEmpty);
                size_type index = group.index(start_index, pos);
                index = this->round_index(index);
                o_distance = this->round_dist(index, start_slot);
                last_slot = index;
                return npos;
            }
            distance += kGroupWidth;
            slot_index = this->slot_next_group(slot_index);
        } while (slot_index != start_slot);

        o_distance = kEndOfMark;
        last_slot = npos;
        return npos;
    }

    JSTD_FORCED_INLINE
    size_type find_first_empty_slot(const key_type & key, std::uint8_t & o_distance,
                                                          std::uint8_t & o_ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type first_slot = slot_index;
        o_ctrl_hash = ctrl_hash;

        if (1) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);
            if (likely(ctrl->isEmpty())) {
                o_distance = 0;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            if (likely(ctrl->isEmpty())) {
                o_distance = 1;
                return slot_index;
            }

            slot_index = this->next_index(slot_index);
        }

        // Find the first empty slot and insert
        do {
            const group_type & group = this->get_group(slot_index);
            std::uint32_t maskEmpty = group.matchEmpty();
            if (maskEmpty != 0) {
                // Found a [EmptyEntry] to insert
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type index = group.index(slot_index, pos);
                o_distance = this->round_dist(index, first_slot);
                return this->round_index(index);
            }
            slot_index = this->slot_next_group(slot_index);
            assert(slot_index != first_slot);
        } while (1);

        o_distance = kEndOfMark;
        return npos;
    }
#endif

    JSTD_NO_INLINE
    std::pair<size_type, bool>
    find_or_prepare_insert(const key_type & key, std::uint8_t & o_distance,
                                                 std::uint8_t & o_ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type first_slot = slot_index;
        size_type last_slot = npos;
        std::uint8_t distance = 0;
        std::uint32_t maskEmpty;
        o_ctrl_hash = ctrl_hash;

        if (kUseUnrollLoop) {
            ctrl_type * ctrl = this->ctrl_at(slot_index);
            // Optimize from: (ctrl->isUsed() && (ctrl->distance >= 0))
            if (likely(ctrl->isUsed())) {
                if (ctrl->hash == ctrl_hash) {
                    slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_distance = 0;
                        return { slot_index, true };
                    }
                }
            } else {
                if (this->need_grow()) {
                    // The size of slot reach the slot threshold or hashmap is full.
                    this->grow_if_necessary();

                    return this->find_or_prepare_insert(key, o_distance, o_ctrl_hash);
                }
                o_distance = 0;
                return { slot_index, false };
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->distance + 1) > 1)) {
            //if (likely(ctrl->isUsed() && (ctrl->distance >= 1))) {
                if (ctrl->hash == ctrl_hash) {
                    slot_type * slot = this->slot_at(slot_index);
                    if (this->key_equal_(slot->value.first, key)) {
                        o_distance = 1;
                        return { slot_index, true };
                    }
                }
            } else {
                if (this->need_grow()) {
                    // The size of slot reach the slot threshold or hashmap is full.
                    this->grow_if_necessary();

                    return this->find_or_prepare_insert(key, o_distance, o_ctrl_hash);
                }
                o_distance = 1;
                return { slot_index, false };
            }

            distance = 2;
            do {
                ctrl = this->next_ctrl(slot_index);
                // Optimization: merging two comparisons
                if (likely(std::uint8_t(ctrl->distance + 1) > distance)) {
                //if (likely(ctrl->isUsed() && (ctrl->distance >= distance))) {
                    if (ctrl->hash == ctrl_hash) {
                        slot_type * slot = this->slot_at(slot_index);
                        if (this->key_equal_(slot->value.first, key)) {
                            o_distance = distance;
                            return { slot_index, true };
                        }
                    }
                    distance++;
                    if (distance >= 4)
                        break;
                } else {
                    if (this->need_grow()) {
                        // The size of slot reach the slot threshold or hashmap is full.
                        this->grow_if_necessary();

                        return this->find_or_prepare_insert(key, o_distance, o_ctrl_hash);
                    }
                    o_distance = distance;
                    return { slot_index, false };
                }
            } while (1);

            slot_index = this->next_index(slot_index);
        }

        do {
            const group_type & group = this->get_group(slot_index);
            auto mask32 = group.matchHashAndDistance(ctrl_hash, distance);
            std::uint32_t maskHash = mask32.maskHash;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(slot_index, pos);
                index = this->round_index(index);
                const slot_type * target = this->slot_at(index);
                if (this->key_equal_(target->value.first, key)) {
                    o_distance = this->round_dist(index, first_slot);
                    assert(o_distance == (distance + group_type::pos(pos)));
                    return { index, true };
                }
            }
            maskEmpty = mask32.maskEmpty;
            if (maskEmpty != 0) {
                last_slot = slot_index;
                break;
            }
            distance += kGroupWidth;
#ifdef _DEBUG
            if (distance >= (kEmptySlot - kGroupWidth - 1))
                distance = distance;
#endif
            slot_index = this->slot_next_group(slot_index);
        } while (slot_index != first_slot);

        if (this->need_grow() || (last_slot == npos)) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            return this->find_or_prepare_insert(key, o_distance, o_ctrl_hash);
        }

        // It's a [EmptyEntry], or (distance > ctrl->distance) entry.
        assert(maskEmpty != 0);
        size_type pos = BitUtils::bsf32(maskEmpty);
        size_type index = group_type::index(last_slot, pos);
        o_distance = this->round_dist(index, first_slot);
        assert(o_distance == (distance + group_type::pos(pos)));
        index = this->round_index(index);
        return { index, false };
    }

    template <bool isRehashing>
    JSTD_NO_INLINE
    bool insert_to_place(size_type target, std::uint8_t distance, std::uint8_t ctrl_hash) {
        ctrl_type insert_ctrl(distance, ctrl_hash);
        ctrl_type * ctrl = this->ctrl_at(target);
        assert(!ctrl->isEmpty());
        assert(distance > ctrl->distance);
        this->setUsedMirrorCtrl(target, insert_ctrl.value);
        std::swap(insert_ctrl.value, ctrl->value);
        insert_ctrl.distance++;

        alignas(slot_type) unsigned char slot_raw1[sizeof(slot_type)];
        alignas(slot_type) unsigned char slot_raw2[sizeof(slot_type)];

        slot_type * to_insert = reinterpret_cast<slot_type *>(&slot_raw1);
        slot_type * tmp_slot  = reinterpret_cast<slot_type *>(&slot_raw2);
        this->placement_new_slot(to_insert);
        this->placement_new_slot(tmp_slot);
        slot_type * target_slot = this->slot_at(target);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&to_insert->mutable_value, std::move(target_slot->mutable_value));
        } else {
            this->allocator_.construct(&to_insert->value, std::move(target_slot->value));
        }
        this->destroy_slot(target_slot);

        size_type slot_index = this->next_index(target);
        do {
            ctrl = this->ctrl_at(slot_index);
            if (ctrl->isEmpty()) {
                this->emplace_rich_slot(to_insert, slot_index, insert_ctrl.value);
                return false;
            } else if ((insert_ctrl.distance > ctrl->distance) /* || (insert_ctrl.distance == (kEndOfMark - 1)) */) {
                this->setUsedMirrorCtrl(slot_index, insert_ctrl.value);
                std::swap(insert_ctrl.value, ctrl->value);

                slot_type * slot = this->slot_at(slot_index);
                this->swap_slot(to_insert, slot, tmp_slot);
            }

            assert(insert_ctrl.distance < kEmptySlot);
            slot_index = this->next_index(slot_index);

            if (isRehashing) {
                insert_ctrl.distance++;
#ifdef _DEBUG
                if (insert_ctrl.distance >= kEndOfMark)
                    distance = distance;
#endif
                //insert_ctrl.distance = (insert_ctrl.distance < kEndOfMark) ? insert_ctrl.distance : (kEndOfMark - 1);
            } else {
                insert_ctrl.distance++;
                if (insert_ctrl.distance > kDistLimit) {
                    this->emplace_rich_slot(to_insert, target, insert_ctrl.value);
                    return true;
                }
            }
        } while (slot_index != target);

        this->emplace_rich_slot(to_insert, target, insert_ctrl.value);
        return true;
    }

    JSTD_FORCED_INLINE
    void emplace_rich_slot(slot_type * to_insert, size_type target,
                               std::uint16_t dist_and_hash) {
        this->setUsedCtrl(target, dist_and_hash);

        slot_type * slot = this->slot_at(target);
        this->placement_new_slot(slot);
        if (kIsCompatibleLayout)
            this->mutable_allocator_.construct(&slot->mutable_value, std::move(to_insert->mutable_value));
        else
            this->allocator_.construct(&slot->value, std::move(to_insert->value));

        this->destroy_slot(to_insert);
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(const value_type & value) {
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        auto find_info = this->find_or_prepare_insert(value.first, distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        auto find_info = this->find_or_prepare_insert(value.first, distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        auto find_info = this->find_or_prepare_insert(key, distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        auto find_info = this->find_or_prepare_insert(key, distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        tuple_wrapper2<key_type> key_wrapper(first);
        auto find_info = this->find_or_prepare_insert(key_wrapper.value(), distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        value_type value(std::forward<First>(first), std::forward<Args>(args)...);
        auto find_info = this->find_or_prepare_insert(value.first, distance, ctrl_hash);
        size_type target = find_info.first;
        bool is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            assert(target != npos);

            ctrl_type * ctrl = this->ctrl_at(target);
            if (ctrl->isEmpty()) {
                // Found [EmptyEntry] to insert
                assert(ctrl->isEmpty());
                this->setUsedCtrl(target, distance, ctrl_hash);
            } else {
                // Insert to target place
                bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
    size_type unique_prepare_insert(const key_type & key, std::uint8_t & o_distance,
                                                          std::uint8_t & o_ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        size_type first_slot = slot_index;
        std::uint8_t distance = 0;
        o_ctrl_hash = ctrl_hash;

        if (kUseUnrollLoop) {
            const ctrl_type * ctrl = this->ctrl_at(slot_index);
            // Optimize from: (ctrl->isEmpty() || (ctrl->distance < 0))
            if (likely(ctrl->isEmpty())) {
                o_distance = 0;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->distance + 1) < 2)) {
            //if (likely(ctrl->isEmpty() || (ctrl->distance < 1))) {
                o_distance = 1;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->distance + 1) < 3)) {
            //if (likely(ctrl->isEmpty() || (ctrl->distance < 2))) {
                o_distance = 2;
                return slot_index;
            }

            ctrl = this->next_ctrl(slot_index);
            // Optimization: merging two comparisons
            if (likely(std::uint8_t(ctrl->distance + 1) < 4)) {
            //if (likely(ctrl->isEmpty() || (ctrl->distance < 3))) {
                o_distance = 3;
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
                o_distance = this->round_dist(index, first_slot);
                assert(o_distance == (distance + group_type::pos(pos)));
                return this->round_index(index);
            }
            distance += kGroupWidth;
#ifdef _DEBUG
            if (distance >= (kEmptySlot - kGroupWidth - 1))
                distance = distance;
#endif
            slot_index = this->slot_next_group(slot_index);
            assert(slot_index != first_slot);
        } while (1);

        o_distance = kEndOfMark;
        return npos;
    }

    // Use in rehash_impl()
    bool move_insert_unique(slot_type * slot) {
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        size_type target = this->unique_prepare_insert(slot->value.first,
                                                       distance, ctrl_hash);
        assert(target != npos);
        bool need_grow = false;

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, distance, ctrl_hash);
        } else {
            // Insert to target place
            need_grow = this->insert_to_place<true>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        size_type target = this->unique_prepare_insert(value.first, distance, ctrl_hash);
        assert(target != npos);

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, distance, ctrl_hash);
        } else {
            // Insert to target place
            bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        std::uint8_t distance;
        std::uint8_t ctrl_hash;
        size_type target = this->unique_prepare_insert(value.first, distance, ctrl_hash);
        assert(target != npos);

        ctrl_type * ctrl = this->ctrl_at(target);
        if (ctrl->isEmpty()) {
            // Found [EmptyEntry] to insert
            assert(ctrl->isEmpty());
            this->setUsedCtrl(target, distance, ctrl_hash);
        } else {
            // Insert to target place
            bool need_grow = this->insert_to_place<false>(target, distance, ctrl_hash);
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
        size_type to_erase = this->find_impl(key);
        if (to_erase != this->slot_capacity()) {
            this->erase_slot(to_erase);
            return 1;
        } else {
            return 0;
        }
    }

    JSTD_NO_INLINE
    void erase_slot(size_type to_erase) {
        assert(to_erase < this->slot_capacity());

        ctrl_type * curr_ctrl = this->ctrl_at(to_erase);
        slot_type * curr_slot = this->slot_at(to_erase);
        assert(curr_ctrl->isUsed());

        this->destroy_slot(curr_slot);

        assert(this->slot_size_ > 0);
        this->slot_size_--;

        size_type curr_index;
        ctrl_type * next_ctrl = curr_ctrl + std::ptrdiff_t(1);
        slot_type * next_slot = curr_slot + std::ptrdiff_t(1);

        while (!next_ctrl->isEmptyOrZero()) {
            ctrl_type dist_and_hash(*next_ctrl);
            assert(dist_and_hash.distance > 0);
            dist_and_hash.distance--;
            curr_index = this->index_of(curr_ctrl);
            this->setUsedCtrl(curr_index, dist_and_hash.value);
            this->transfer_slot(curr_slot, next_slot);

            curr_ctrl++;
            curr_slot++;

            next_ctrl++;
            next_slot++;
        }

        curr_index = this->index_of(curr_ctrl);
        this->setUnusedCtrl(curr_index, kEmptySlot);
    }

    void swap_content(robin16_hash_map & other) {
        using std::swap;
        swap(this->groups_, other.groups_);
        swap(this->group_mask_, other.group_mask_);
        swap(this->slots_, other.slots_);
        swap(this->slot_size_, other.slot_size_);
        swap(this->slot_mask_, other.slot_mask_);
        swap(this->slot_threshold_, other.slot_threshold_);
        swap(this->n_mlf_, other.n_mlf_);
        swap(this->n_mlf_rev_, other.n_mlf_rev_);
#if ROBIN16_USE_HASH_POLICY
        swap(this->hash_policy_, other.hash_policy_ref());
#endif
    }

    void swap_policy(robin16_hash_map & other) {
        using std::swap;
        swap(this->hasher_, other.hash_function_ref());
        swap(this->key_equal_, other.key_eq_ref());
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
            swap(this->allocator_, other.get_allocator_ref());
        }
        if (std::allocator_traits<mutable_allocator_type>::propagate_on_container_swap::value) {
            swap(this->mutable_allocator_, other.get_mutable_allocator_ref());
        }
        if (std::allocator_traits<slot_allocator_type>::propagate_on_container_swap::value) {
            swap(this->slot_allocator_, other.get_slot_allocator_ref());
        }
        if (std::allocator_traits<mutable_slot_allocator_type>::propagate_on_container_swap::value) {
            swap(this->mutable_slot_allocator_, other.get_mutable_slot_allocator_ref());
        }
    }

    void swap_impl(robin16_hash_map & other) {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

template <class Key, class Value, class Hash, class KeyEqual, class Alloc, class Pred>
typename robin16_hash_map<Key, Value, Hash, KeyEqual, Alloc>::size_type
inline
erase_if(robin16_hash_map<Key, Value, Hash, KeyEqual, Alloc> & hash_map, Pred pred)
{
    auto old_size = hash_map.size();
    for (auto it = hash_map.begin(), last = hash_map.end(); it != last; ) {
        if (pred(*it)) {
            it = hash_map.erase(it);
        } else {
            ++it;
        }
    }
    return (old_size - hash_map.size());
}

} // namespace jstd
