
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
#include "jstd/hasher/hash_crc32.h"
#include "jstd/hashmap/map_layout_policy.h"
#include "jstd/hashmap/map_slot_policy.h"
#include "jstd/hashmap/slot_policy_traits.h"
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

#define ROBIN_V2_USE_HASH_POLICY    0

namespace jstd {
namespace v2 {

template <typename Key, typename Value, typename SlotType>
struct robin_hash_map_slot_policy {
    using slot_policy = map_slot_policy<Key, Value, SlotType>;

    using slot_type   = typename slot_policy::slot_type;
    using key_type    = typename slot_policy::key_type;
    using mapped_type = typename slot_policy::mapped_type;
    using init_type   = std::pair</* non const */ key_type, mapped_type>;

    template <typename Allocator, typename ... Args>
    static void construct(Allocator * alloc, slot_type * slot, Args &&... args) {
        slot_policy::construct(alloc, slot, std::forward<Args>(args)...);
    }

    template <typename Allocator>
    static void destroy(Allocator * alloc, slot_type * slot) {
        slot_policy::destroy(alloc, slot);
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, slot_type * src_slot) {
        slot_policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        slot_policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Allocator>
    static void transfer(Allocator * alloc, slot_type * new_slot, slot_type * old_slot) {
        slot_policy::transfer(alloc, new_slot, old_slot);
    }

    template <typename Allocator>
    static void swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        slot_policy::swap(alloc, slot1, slot2, tmp);
    }

    template <typename Allocator>
    static void exchange(Allocator * alloc, slot_type * src, slot_type * dest, slot_type * empty) {
        slot_policy::exchange(alloc, src, dest, empty);
    }

    template <typename Allocator>
    static void move_assign_swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        slot_policy::move_assign_swap(alloc, slot1, slot2, tmp);
    }

    static std::size_t extra_space(const slot_type *) {
        return 0;
    }

    static std::pair<const key_type, mapped_type> & element(slot_type * slot) {
        return slot->value;
    }

    static Value & value(std::pair<const key_type, mapped_type> * kv) {
        return kv->second;
    }

    static const Value & value(const std::pair<const key_type, mapped_type> * kv) {
        return kv->second;
    }
};

template < typename Key, typename Value,
           typename Hash = std::hash<typename std::remove_const<Key>::type>,
           typename KeyEqual = std::equal_to<typename std::remove_const<Key>::type>,
           typename LayoutPolicy = jstd::default_layout_policy<Key, Value>,
           typename Allocator = std::allocator<std::pair<typename std::add_const<typename std::remove_const<Key>::type>::type,
                                                         typename std::remove_const<Value>::type>> >
class robin_hash_map {
public:
    template <typename K, typename V>
    union map_slot_type {
    public:
        using key_type = typename std::remove_const<K>::type;
        using mapped_type = typename std::remove_const<V>::type;
        using value_type = std::pair<const key_type, mapped_type>;
        using mutable_value_type = std::pair<key_type, mapped_type>;

        value_type          value;
        mutable_value_type  mutable_value;
        const key_type      key;
        key_type            mutable_key;

        map_slot_type() {}
        ~map_slot_type() = delete;
    };

    typedef map_slot_type<Key, Value>               slot_type;
    typedef map_slot_type<Key, Value>               node_type;

    typedef v2::robin_hash_map_slot_policy<Key, Value, slot_type>
                                                    slot_policy_t;
    typedef slot_policy_traits<slot_policy_t>       SlotPolicyTraits;

    typedef typename slot_type::key_type            key_type;
    typedef typename slot_type::mapped_type         mapped_type;

    typedef typename slot_type::value_type          value_type;
    typedef typename slot_type::mutable_value_type  mutable_value_type;

    static constexpr bool kIsCompatibleLayout =
            std::is_same<value_type, mutable_value_type>::value ||
            is_compatible_pair_layout<value_type, mutable_value_type>::value;

    typedef typename std::conditional<kIsCompatibleLayout, mutable_value_type, value_type>::type
                                                    actual_value_type;

    typedef Hash                                    hasher;
    typedef KeyEqual                                key_equal;
    typedef Allocator                               allocator_type;
    typedef typename Hash::result_type              hash_result_t;
    typedef typename hash_policy_selector<Hash>::type
                                                    hash_policy_t;
    typedef LayoutPolicy                            layout_policy_t;

    typedef std::size_t                             size_type;
    typedef std::intptr_t                           ssize_type;
    typedef std::size_t                             hash_code_t;
    typedef robin_hash_map<Key, Value, Hash, KeyEqual, LayoutPolicy, Allocator>
                                                    this_type;

    static constexpr bool kUseIndexSalt = false;

    static constexpr size_type npos = size_type(-1);

    static constexpr size_type kCtrlHashMask = 0x000000FFul;
    static constexpr size_type kCtrlShift    = 8;

    static constexpr size_type kGroupBits   = 4;
    static constexpr size_type kGroupWidth  = size_type(1) << kGroupBits;
    static constexpr size_type kGroupMask   = kGroupWidth - 1;
    static constexpr size_type kGroupShift  = kCtrlShift + kGroupBits;

    static constexpr size_type kDefaultCapacity = 0;
    // kMinimumCapacity must be >= 2
    static constexpr size_type kMinimumCapacity = 4;

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
    static constexpr bool isPlainKeyHash = isGccOrClang &&
                                           std::is_same<Hash, std::hash<key_type>>::value &&
                                          (detail::is_plain_type<key_type>::value);

    static constexpr bool kDetectStoreHash = !detail::is_plain_type<key_type>::value;

    static constexpr bool kNeedStoreHash =
        (!layout_policy_t::autoDetectStoreHash && layout_policy_t::needStoreHash) ||
         (layout_policy_t::autoDetectStoreHash && (kDetectStoreHash));

    static constexpr bool kIsPlainKey    = detail::is_plain_type<key_type>::value;
    static constexpr bool kIsPlainMapped = detail::is_plain_type<mapped_type>::value;

    static constexpr bool kIsPlainKV = kIsPlainKey && kIsPlainMapped;

    static constexpr bool is_slot_trivial_copyable =
            (std::is_trivially_copyable<actual_value_type>::value ||
            (std::is_trivially_copyable<key_type>::value &&
             std::is_trivially_copyable<mapped_type>::value) ||
            (std::is_scalar<key_type>::value && std::is_scalar<mapped_type>::value));

    static constexpr bool is_slot_trivial_destructor =
            (std::is_trivially_destructible<actual_value_type>::value ||
            (std::is_trivially_destructible<key_type>::value &&
             std::is_trivially_destructible<mapped_type>::value) ||
            (detail::is_plain_type<key_type>::value &&
             detail::is_plain_type<mapped_type>::value));

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

    enum CompareOp {
        opEQ,   // ==
        opNE,   // !=
        opGT,   // >
        opGE,   // >=
        opLT,   // <
        opLE    // <=
    };

    struct ctrl_data {
        union {
            struct {
                std::uint8_t hash;
                std::int8_t  dist;
            };
            std::int16_t  value;
            std::uint16_t uvalue;
        };

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

        static constexpr std::int16_t make_dist(size_type n) {
            return static_cast<std::int16_t>(n * kDistInc16);
        }

        size_type distance() const {
            return static_cast<size_type>(this->uvalue ^ this->hash);
        }

        bool isEmpty() const {
            return (this->dist == kEmptySlot);
        }

        static bool isEmpty(std::int8_t tag) {
            return (tag == kEmptySlot);
        }

        bool isNonEmpty() const {
            return !(this->isEmpty());
        }

        static bool isNonEmpty(std::int8_t tag) {
            return !ctrl_data::isEmpty(tag);
        }

        bool isEndOf() const {
            return (this->dist == kEndOfMark);
        }

        bool isUsed() const {
            return (this->value >= 0);
        }

        static bool isUsed(std::int8_t tag) {
            return (tag >= 0);
        }

        bool isUnused() const {
            return (this->value < 0);
        }

        static bool isUnused(std::int8_t tag) {
            return (tag < 0);
        }

        bool isEmptyOrZero() const {
            //return (this->dist == 0 || this->dist == kEmptySlot);
            return (std::uint8_t(this->dist + 1) <= 1);
        }

        bool isUnusedOrZero() const {
            // return (this->dist <= 0);
            return (this->value < kDistInc16);
        }

        void setEmpty() {
            this->value = kEmptySlot16;
        }

        void setEndOf() {
            this->value = kEndOfMark16;
        }

        void setUnused() {
            this->value = kEmptySlot16;
        }

        void setDist(std::int8_t dist) {
            this->dist = dist;
        }

        void setHash(std::uint8_t ctrl_hash) {
            this->hash = ctrl_hash;
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

        void setValue(const ctrl_data & ctrl) {
            this->value = ctrl.value;
        }

        void incDist() {
            this->uvalue += kDistInc16;
        }

        void decDist() {
            this->uvalue -= kDistInc16;
        }

        void incDist(size_type width) {
            this->uvalue += std::int16_t(kDistInc16 * width);
        }

        void decDist(size_type width) {
            this->uvalue -= std::int16_t(kDistInc16 * width);
        }

        bool hash_equals(std::uint8_t ctrl_hash) const {
            return (this->hash == ctrl_hash);
        }

        bool hash_equals(const ctrl_data & ctrl) const {
            return (this->hash == ctrl.hash);
        }

        bool dist_equals(std::int8_t dist) const {
            return (this->dist == dist);
        }

        bool dist_equals(const ctrl_data & ctrl) const {
            return (this->dist == ctrl.dist);
        }

        bool equals(std::int16_t value) const {
            return (this->value == value);
        }

        bool equals(const ctrl_data & ctrl) const {
            return (this->value == ctrl.value);
        }

        template <CompareOp CmpOp>
        bool cmp_hash(std::uint8_t ctrl_hash) const {
            if (CmpOp == opEQ)
                return (this->hash == ctrl_hash);
            else if (CmpOp == opNE)
                return (this->hash != ctrl_hash);
            else if (CmpOp == opGT)
                return (this->hash >  ctrl_hash);
            else if (CmpOp == opGE)
                return (this->hash >= ctrl_hash);
            else if (CmpOp == opLT)
                return (this->hash <  ctrl_hash);
            else if (CmpOp == opLE)
                return (this->hash <= ctrl_hash);
        }

        template <CompareOp CmpOp>
        bool cmp_hash(const ctrl_data & ctrl) const {
            if (CmpOp == opEQ)
                return (this->hash == ctrl.hash);
            else if (CmpOp == opNE)
                return (this->hash != ctrl.hash);
            else if (CmpOp == opGT)
                return (this->hash >  ctrl.hash);
            else if (CmpOp == opGE)
                return (this->hash >= ctrl.hash);
            else if (CmpOp == opLT)
                return (this->hash <  ctrl.hash);
            else if (CmpOp == opLE)
                return (this->hash <= ctrl.hash);
        }

        template <CompareOp CmpOp>
        bool cmp_dist(std::int8_t dist) const {
            if (CmpOp == opEQ)
                return (this->dist == dist);
            else if (CmpOp == opNE)
                return (this->dist != dist);
            else if (CmpOp == opGT)
                return (this->dist >  dist);
            else if (CmpOp == opGE)
                return (this->dist >= dist);
            else if (CmpOp == opLT)
                return (this->dist <  dist);
            else if (CmpOp == opLE)
                return (this->dist <= dist);
        }

        template <CompareOp CmpOp>
        bool cmp_dist(const ctrl_data & ctrl) const {
            if (CmpOp == opEQ)
                return (this->dist == ctrl.dist);
            else if (CmpOp == opNE)
                return (this->dist != ctrl.dist);
            else if (CmpOp == opGT)
                return (this->dist >  ctrl.dist);
            else if (CmpOp == opGE)
                return (this->dist >= ctrl.dist);
            else if (CmpOp == opLT)
                return (this->dist <  ctrl.dist);
            else if (CmpOp == opLE)
                return (this->dist <= ctrl.dist);
        }

        template <CompareOp CmpOp>
        bool compare(std::int16_t value) const {
            if (CmpOp == opEQ)
                return (this->value == value);
            else if (CmpOp == opNE)
                return (this->value != value);
            else if (CmpOp == opGT)
                return (this->value >  value);
            else if (CmpOp == opGE)
                return (this->value >= value);
            else if (CmpOp == opLT)
                return (this->value <  value);
            else if (CmpOp == opLE)
                return (this->value <= value);
        }

        template <CompareOp CmpOp>
        bool compare(const ctrl_data & ctrl) const {
            if (CmpOp == opEQ)
                return (this->value == ctrl.value);
            else if (CmpOp == opNE)
                return (this->value != ctrl.value);
            else if (CmpOp == opGT)
                return (this->value >  ctrl.value);
            else if (CmpOp == opGE)
                return (this->value >= ctrl.value);
            else if (CmpOp == opLT)
                return (this->value <  ctrl.value);
            else if (CmpOp == opLE)
                return (this->value <= ctrl.value);
        }
    };

    typedef ctrl_data ctrl_type;

    template <typename T>
    struct MatchMask2 {
        typedef T mask32_type;
        mask32_type maskEmpty;
        mask32_type maskHash;

        MatchMask2() noexcept : maskEmpty(0), maskHash(0) {
        }
        MatchMask2(mask32_type maskEmpty, mask32_type maskHash) noexcept
            : maskEmpty(maskEmpty), maskHash(maskHash) {
        }
        MatchMask2(const MatchMask2 & src) noexcept
            : maskEmpty(src.maskEmpty), maskHash(src.maskHash) {
        }
        ~MatchMask2() = default;
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

        pointer ctrl;

        BitMask256_AVX2Ex() noexcept : ctrl(nullptr) {
        }
        explicit BitMask256_AVX2Ex(pointer ctrl) noexcept : ctrl(ctrl) {
        }
        explicit BitMask256_AVX2Ex(const_pointer ctrl) noexcept
            : ctrl(const_cast<pointer>(ctrl)) {
        }
        BitMask256_AVX2Ex(const BitMask256_AVX2Ex & src) noexcept : ctrl(src.ctrl) {
        }
        ~BitMask256_AVX2Ex() = default;

        template <std::int16_t ControlTag>
        void fillAll(pointer ptr) {
            const __m256i tag_bits = _mm256_set1_epi16((short)ControlTag);
            _mm256_storeu_si256((__m256i *)ptr, tag_bits);
        }

        void fillAllZeros() {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)this->ctrl, zero_bits);
        }

        void fillAllEmpty() {
            fillAll<kEmptySlot16>(this->ctrl);
        }

        void fillAllEndOf() {
            fillAll<kEndOfMark16>(this->ctrl);
        }

        std::uint32_t matchTag(std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchLowTag(std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask16 = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask16);
            __m256i match_mask = _mm256_cmpeq_epi16(low_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
        }

        std::uint32_t matchHash(std::uint8_t ctrl_hash) const {
#if 1
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, hash_bits);
                    match_mask = _mm256_slli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return mask;
#else
            return this->matchLowTag(ctrl_hash);
#endif
        }

        std::uint32_t matchEmpty() const {
            return this->matchUnused();
        }

        std::uint32_t matchEmptyOnly() const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, ones_bits);
                std::uint32_t mask = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return mask;
            } else {
                return this->matchHighTag(static_cast<std::uint8_t>(kEmptySlot));
            }
        }

        std::uint32_t matchNonEmpty() const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ones_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i tag_bits   = _mm256_set1_epi16(kEmptySlot16);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(tag_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchUsed() const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(ctrl_bits, ones_bits);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused() const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i zero_bits  = _mm256_setzero_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movepi16_mask(match_mask);
            return maskUsed;
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(std::int16_t dist_and_hash) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(dist_and_hash <= kMaxDist16);
            __m256i dist_0_hash = _mm256_set1_epi16(dist_and_hash);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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

        std::uint32_t matchEmptyOrZero() const {
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i zero_mask   = _mm256_cmpeq_epi16(zero_bits, ctrl_bits);
            __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movepi16_mask(result_mask);
            return maskEmpty;
        }

        std::uint32_t matchEmptyAndDistance(std::int8_t distance) const {
#if 1
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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

        bool hasAnyMatch(std::uint8_t ctrl_hash) const {
            return (this->matchHash(ctrl_hash) != 0);
        }

        bool hasAnyEmpty() const {
            return (this->matchEmptyOnly() != 0);
        }

        bool hasAnyUsed() const {
            return (this->matchUsed() != 0);
        }

        bool hasAnyUnused() const {
            return (this->matchUnused() != 0);
        }

        bool isAllEmpty() const {
            return (this->matchEmptyOnly() == kFullMask16);
        }

        bool isAllUsed() const {
            return (this->matchUnused() == 0);
        }

        bool isAllUnused() const {
            return (this->matchUsed() == 0);
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

        pointer ctrl;

        BitMask256_AVX2() noexcept : ctrl(nullptr) {
        }
        explicit BitMask256_AVX2(pointer ctrl) noexcept : ctrl(ctrl) {
        }
        explicit BitMask256_AVX2(const_pointer ctrl) noexcept
            : ctrl(const_cast<pointer>(ctrl)) {
        }
        BitMask256_AVX2(const BitMask256_AVX2 & src) noexcept : ctrl(src.ctrl) {
        }
        ~BitMask256_AVX2() = default;

        template <std::int16_t ControlTag>
        void fillAll(pointer ptr) {
            const __m256i tag_bits = _mm256_set1_epi16((short)ControlTag);
            _mm256_storeu_si256((__m256i *)ptr, tag_bits);
        }

        void fillAllZeros() {
            const __m256i zero_bits = _mm256_setzero_si256();
            _mm256_storeu_si256((__m256i *)this->ctrl, zero_bits);
        }

        void fillAllEmpty() {
            fillAll<kEmptySlot16>(this->ctrl);
        }

        void fillAllEndOf() {
            fillAll<kEndOfMark16>(this->ctrl);
        }

        std::uint32_t matchTag(std::int16_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i match_mask = _mm256_cmpeq_epi16(ctrl_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchLowTag(std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i low_mask16 = _mm256_srli_epi16(ones_bits, 8);
            __m256i low_bits   = _mm256_and_si256(ctrl_bits, low_mask16);
            __m256i match_mask = _mm256_cmpeq_epi16(low_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHighTag(std::uint8_t ctrl_tag) const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i tag_bits   = _mm256_set1_epi16(ctrl_tag);
            __m256i high_bits  = _mm256_srli_epi16(ctrl_bits, 8);
            __m256i match_mask = _mm256_cmpeq_epi16(high_bits, tag_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
        }

        std::uint32_t matchHash(std::uint8_t ctrl_hash) const {
#if 1
            __m256i hash_bits  = _mm256_set1_epi16(ctrl_hash);
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, hash_bits);
                    match_mask = _mm256_slli_epi16(match_mask, 8);
            std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return mask;
#else
            return this->matchLowTag(data, ctrl_hash);
#endif
        }

        std::uint32_t matchEmpty() const {
            return this->matchUnused();
        }

        std::uint32_t matchEmptyOnly() const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t mask = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return mask;
            } else {
                return this->matchHighTag(static_cast<std::uint8_t>(kEmptySlot));
            }
        }

        std::uint32_t matchNonEmpty() const {
            if (kEmptySlot == 0b11111111) {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(ones_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            } else {
                __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
                __m256i tag_bits   = _mm256_set1_epi16(kEmptySlot16);
                __m256i ones_bits  = _mm256_setones_si256();
                __m256i match_mask = _mm256_cmpeq_epi8(tag_bits, ctrl_bits);
                        match_mask = _mm256_andnot_si256(match_mask, ones_bits);
                        match_mask = _mm256_srli_epi16(match_mask, 8);
                std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
                return maskUsed;
            }
        }

        std::uint32_t matchUsed() const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i ones_bits  = _mm256_setones_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(ctrl_bits, ones_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return maskUsed;
        }

        std::uint32_t matchUnused() const {
            __m256i ctrl_bits  = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i zero_bits  = _mm256_setzero_si256();
            __m256i match_mask = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
                    match_mask = _mm256_srli_epi16(match_mask, 8);
            std::uint32_t maskUsed = (std::uint32_t)_mm256_movemask_epi8(match_mask);
            return maskUsed;
        }

        MatchMask2<std::uint32_t>
        matchHashAndDistance(std::int16_t dist_and_hash) const {
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(dist_and_hash <= kMaxDist16);
            __m256i dist_0_hash = _mm256_set1_epi16(dist_and_hash);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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

        std::uint32_t matchEmptyOrZero() const {
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
            __m256i zero_bits   = _mm256_setzero_si256();
            __m256i empty_mask  = _mm256_cmpgt_epi16(zero_bits, ctrl_bits);
            __m256i zero_mask   = _mm256_cmpeq_epi16(zero_bits, ctrl_bits);
            __m256i result_mask = _mm256_or_si256(empty_mask, zero_mask);
                    result_mask = _mm256_srli_epi16(result_mask, 8);
            std::uint32_t maskEmpty = (std::uint32_t)_mm256_movemask_epi8(result_mask);
            return maskEmpty;
        }

        std::uint32_t matchEmptyAndDistance(std::int8_t distance) const {
#if 1
            const __m256i kDistanceBase =
                _mm256_setr_epi16(0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
                                  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00);
            assert(distance <= kMaxDist);
            __m256i dist_0      = _mm256_set1_epi16((short)distance);
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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
            __m256i ctrl_bits   = _mm256_loadu_si256((const __m256i *)this->ctrl);
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

        bool hasAnyMatch( std::uint8_t ctrl_hash) const {
            return (this->matchHash(ctrl_hash) != 0);
        }

        bool hasAnyEmpty() const {
            return (this->matchEmptyOnly() != 0);
        }

        bool hasAnyUsed() const {
            return (this->matchUsed() != 0);
        }

        bool hasAnyUnused() const {
            return (this->matchUnused() != 0);
        }

        bool isAllEmpty() const {
            return (this->matchEmptyOnly() == kFullMask32_Half);
        }

        bool isAllUsed() const {
            return (this->matchUnused() == 0);
        }

        bool isAllUnused() const {
            return (this->matchUsed() == 0);
        }

        static inline size_type bitPos(size_type pos) {
            return (pos >> 1);
        }
    };

    template <typename T>
    using BitMask256 = BitMask256_AVX2<T>;

#else

    static_assert(false, "jstd::v2::robin_hash_map<K,V> required Intel AVX2 or heigher intrinsics.")

#endif // __AVX2__

    struct group_mask {
        typedef BitMask256<ctrl_type>                       bitmask256_type;
        typedef typename bitmask256_type::bitmask_type      bitmask_type;
        typedef typename bitmask256_type::pointer           pointer;
        typedef typename bitmask256_type::const_pointer     const_pointer;

        bitmask256_type bitmask;

        group_mask() noexcept : bitmask(nullptr) {
        }
        explicit group_mask(pointer ctrl) noexcept : bitmask(ctrl) {
        }
        explicit group_mask(const_pointer ctrl) noexcept : bitmask(ctrl) {
        }
        group_mask(const group_mask & src) noexcept : bitmask(src.bitmask) {
        }
        ~group_mask() = default;

        group_mask & operator ++ () {
            bitmask.ctrl += kGroupWidth;
            return *this;
        }

        group_mask operator ++ (int) {
            group_mask copy(*this);
            ++*this;
            return copy;
        }

        group_mask & operator -- () {
            bitmask.ctrl -= kGroupWidth;
            return *this;
        }

        group_mask operator -- (int) {
            group_mask copy(*this);
            --*this;
            return copy;
        }

        pointer ctrl() {
            return bitmask.ctrl;
        }

        const_pointer ctrl() const {
            return const_cast<const_pointer>(bitmask.ctrl);
        }

        template <std::int16_t ControlTag>
        void fillAll() {
            bitmask.template fillAll<ControlTag>();
        }

        void fillAllEmpty() {
            bitmask.fillAllEmpty();
        }

        void fillAllEndOf() {
            bitmask.fillAllEndOf();
        }

        bitmask_type matchTag(std::int16_t ctrl_tag) const {
            return bitmask.matchTag(ctrl_tag);
        }

        bitmask_type matchHash(std::uint8_t ctrl_hash) const {
            return bitmask.matchHash(ctrl_hash);
        }

        bitmask_type matchEmpty() const {
            return bitmask.matchEmpty();
        }

        bitmask_type matchEmptyOnly() const {
            return bitmask.matchEmptyOnly();
        }

        bitmask_type matchNonEmpty() const {
            return bitmask.matchNonEmpty();
        }

        bitmask_type matchUsed() const {
            return bitmask.matchUsed();
        }

        bitmask_type matchUnused() const {
            return bitmask.matchUnused();
        }

        MatchMask2<bitmask_type>
        matchHashAndDistance(std::int16_t dist_and_hash) const {
            return bitmask.matchHashAndDistance(dist_and_hash);
        }

        bitmask_type matchEmptyOrZero() const {
            return bitmask.matchEmptyOrZero(this->ctrl);
        }

        bitmask_type matchEmptyAndDistance(std::int8_t distance) const {
            return bitmask.matchEmptyAndDistance(distance);
        }

        bool hasAnyMatch(std::uint8_t ctrl_hash) const {
            return bitmask.hasAnyMatch(ctrl_hash);
        }

        bool hasAnyEmpty() const {
            return bitmask.hasAnyEmpty();
        }

        bool hasAnyUsed() const {
            return bitmask.hasAnyUsed();
        }

        bool hasAnyUnused() const {
            return bitmask.hasAnyUnused();
        }

        bool isAllEmpty() const {
            return bitmask.isAllEmpty();
        }

        bool isAllUsed() const {
            return bitmask.isAllUsed();
        }

        bool isAllUnused() const {
            return bitmask.isAllUnused();
        }

        static inline std::uint8_t pos(size_type position) {
            return (std::uint8_t)bitmask256_type::bitPos(position);
        }

        static inline size_type index(size_type index, size_type position) {
            return (index + group_mask::pos(position));
        }

        friend inline bool operator == (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() == rhs.ctrl());
        }

        friend inline bool operator != (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() != rhs.ctrl());
        }

        friend inline bool operator < (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() < rhs.ctrl());
        }

        friend inline bool operator <= (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() <= rhs.ctrl());
        }

        friend inline bool operator > (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() > rhs.ctrl());
        }

        friend inline bool operator >= (const group_mask & lhs, const group_mask & rhs) noexcept {
            return (lhs.ctrl() >= rhs.ctrl());
        }
    };

    typedef group_mask  group_type;

#if 1
    template <typename ValueType>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = ValueType;
        using pointer = ValueType *;
        using reference = ValueType &;

        using mutable_value_type = typename std::remove_const<ValueType>::type;
        using const_value_type = typename std::add_const<mutable_value_type>::type;

        using opp_value_type = typename std::conditional<std::is_const<ValueType>::value,
                                                         mutable_value_type,
                                                         const_value_type>::type;
        using opp_basic_iterator = basic_iterator<opp_value_type>;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

    private:
        const this_type * owner_;
        size_type index_;

    public:
        basic_iterator() noexcept : owner_(nullptr), index_(0) {
        }
        basic_iterator(this_type * owner, size_type index) noexcept
            : owner_(const_cast<const this_type *>(owner)), index_(index) {
        }
        basic_iterator(const this_type * owner, size_type index) noexcept
            : owner_(owner), index_(index) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : owner_(src.owner_), index_(src.index_) {
        }
        basic_iterator(const opp_basic_iterator & src) noexcept
            : owner_(src.owner()), index_(src.index()) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) noexcept {
            this->owner_ = rhs.owner_;
            this->index_ = rhs.index_;
            return *this;
        }

        basic_iterator & operator = (const opp_basic_iterator & rhs) noexcept {
            this->owner_ = rhs.owner();
            this->index_ = rhs.index();
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) noexcept {
            return (lhs.index_ == rhs.index_) && (lhs.owner_ == rhs.owner_);
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) noexcept {
            return (lhs.index_ != rhs.index_) || (lhs.owner_ != rhs.owner_);
        }

        friend bool operator == (const basic_iterator & lhs, const opp_basic_iterator & rhs) noexcept {
            return (lhs.index() == rhs.index()) && (lhs.owner() == rhs.owner());
        }

        friend bool operator != (const basic_iterator & lhs, const opp_basic_iterator & rhs) noexcept {
            return (lhs.index() != rhs.index()) || (lhs.owner() != rhs.owner());
        }

        basic_iterator & operator ++ () {
            const ctrl_type * ctrl = this->owner_->ctrl_at(this->index_);
            size_type max_index = this->owner_->max_slot_capacity();
            do {
                ++(this->index_);
                ++ctrl;
            } while (ctrl->isEmpty() && (this->index_ < max_index));
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
#if 0
        operator basic_iterator<const mutable_value_type>() const {
            return { this->owner_, this->index_ };
        }
#endif
        this_type * owner() {
            return this->owner_;
        }

        const this_type * owner() const {
            return this->owner_;
        }

        size_type index() {
            return this->index_;
        }

        size_type index() const {
            return this->index_;
        }

        ctrl_type * ctrl() {
            ctrl_type * _ctrl = this->owner_->ctrl_at(this->index);
            return _ctrl;
        }

        const ctrl_type * ctrl() const {
            ctrl_type * _ctrl = this->owner_->ctrl_at(this->index);
            return _ctrl;
        }

        slot_type * slot() {
            slot_type * _slot = this->owner_->slot_at(this->index);
            return _slot;
        }

        const slot_type * slot() const {
            slot_type * _slot = this->owner_->slot_at(this->index);
            return _slot;
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
        using const_value_type = typename std::add_const<mutable_value_type>::type;

        using opp_value_type = typename std::conditional<std::is_const<ValueType>::value,
                                                         mutable_value_type,
                                                         const_value_type>::type;
        using opp_basic_iterator = basic_iterator<opp_value_type>;

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
        basic_iterator(const ctrl_type * ctrl, const slot_type * slot) noexcept
            : ctrl_(const_cast<ctrl_type *>(ctrl)), slot_(const_cast<slot_type *>(slot)) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : ctrl_(src.ctrl_), slot_(src.slot_) {
        }
        basic_iterator(const opp_basic_iterator & src) noexcept
            : owner_(src.ctrl()), index_(src.slot()) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) noexcept {
            this->ctrl_ = rhs.ctrl_;
            this->slot_ = rhs.slot_;
            return *this;
        }

        basic_iterator & operator = (const opp_basic_iterator & rhs) noexcept {
            this->ctrl_ = rhs.ctrl();
            this->slot_ = rhs.slot();
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) noexcept {
            return (lhs.slot_ == rhs.slot_);
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) noexcept {
            return (lhs.slot_ != rhs.slot_);
        }

        friend bool operator == (const basic_iterator & lhs, const opp_basic_iterator & rhs) noexcept {
            return (lhs.slot() == rhs.slot());
        }

        friend bool operator != (const basic_iterator & lhs, const opp_basic_iterator & rhs) noexcept {
            return (lhs.slot() != rhs.slot());
        }

        basic_iterator & operator ++ () {
            do {
                ++(this->ctrl_);
                ++(this->slot_);
            } while (ctrl_->isEmpty());
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

        ctrl_type * ctrl() {
            return this->ctrl_;
        }

        const ctrl_type * ctrl() const {
            return this->ctrl_;
        }

        slot_type * slot() {
            return this->slot_;
        }

        const slot_type * slot() const {
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
#if ROBIN_V2_USE_HASH_POLICY
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
        slots_(nullptr), last_slot_(nullptr),
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
        slots_(nullptr), last_slot_(nullptr),
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
                                select_on_container_copy_construction(other.get_allocator_ref())) {
    }

    robin_hash_map(const robin_hash_map & other, const Allocator & alloc) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(nullptr), last_slot_(nullptr),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_V2_USE_HASH_POLICY
        hash_policy_(other.hash_policy_ref()),
#endif
        hasher_(hasher()), key_equal_(key_equal()),
        allocator_(alloc), mutable_allocator_(alloc),
        ctrl_allocator_(alloc), slot_allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type other_size = other.slot_size();
        this->reserve_for_insert(other_size);
        try {
            this->unique_insert(other.begin(), other.end());
        } catch (const std::bad_alloc & ex) {
            this->destroy();
            throw std::bad_alloc();
        } catch (...) {
            this->destroy();
            throw;
        }
    }

    robin_hash_map(robin_hash_map && other) noexcept(
            std::is_nothrow_copy_constructible<hasher>::value &&
            std::is_nothrow_copy_constructible<key_equal>::value &&
            std::is_nothrow_copy_constructible<allocator_type>::value) :
        ctrls_(jstd_exchange(other.ctrls_, this_type::default_empty_ctrls())),
        slots_(jstd_exchange(other.slots_, nullptr)),
        last_slot_(jstd_exchange(other.last_slot_, nullptr)),
        slot_size_(jstd_exchange(other.slot_size_, 0)),
        slot_mask_(jstd_exchange(other.slot_mask_, 0)),
        max_lookups_(jstd_exchange(other.max_lookups_, kMinLookups)),
        slot_threshold_(jstd_exchange(other.slot_size_, 0)),
        n_mlf_(jstd_exchange(other.n_mlf_, kDefaultLoadFactorInt)),
        n_mlf_rev_(jstd_exchange(other.n_mlf_rev_, kDefaultLoadFactorRevInt)),
#if ROBIN_V2_USE_HASH_POLICY
        hash_policy_(jstd_exchange(other.hash_policy_ref(), hash_policy_t())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(std::move(other.get_allocator_ref())),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        ctrl_allocator_(std::move(other.get_ctrl_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())) {
        // Swap content only
        // this->swap_content(other);
    }

    robin_hash_map(robin_hash_map && other, const Allocator & alloc) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(nullptr), last_slot_(nullptr),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_V2_USE_HASH_POLICY
        hash_policy_(std::move(other.hash_policy_ref())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(alloc),
        mutable_allocator_(std::move(other.get_mutable_allocator_ref())),
        ctrl_allocator_(std::move(other.get_ctrl_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())) {
        if (alloc == other.get_allocator_ref()) {
            // Swap content only
            this->swap_content(other);
        } else {
            // Prepare enough space to ensure that no expansion is required during the insertion process.
            size_type other_size = other.slot_size();
            this->reserve_for_insert(other_size);

            // Note: this will copy elements of dense_set and unordered_set instead of
            // moving them. This can be fixed if it ever becomes an issue.
            try {
                this->unique_insert(other.begin(), other.end());
            } catch (const std::bad_alloc & ex) {
                this->destroy();
                throw std::bad_alloc();
            } catch (...) {
                this->destroy();
                throw;
            }
        }
    }

    robin_hash_map(std::initializer_list<value_type> init_list,
                   size_type init_capacity = kDefaultCapacity,
                   const hasher & hash = hasher(),
                   const key_equal & equal = key_equal(),
                   const allocator_type & alloc = allocator_type()) :
        ctrls_(this_type::default_empty_ctrls()),
        slots_(nullptr), last_slot_(nullptr),
        slot_size_(0), slot_mask_(0), max_lookups_(kMinLookups),
        slot_threshold_(0), n_mlf_(kDefaultLoadFactorInt),
        n_mlf_rev_(kDefaultLoadFactorRevInt),
#if ROBIN_V2_USE_HASH_POLICY
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
        this->destroy();
    }

    robin_hash_map & operator = (const robin_hash_map & other) {
        robin_hash_map tmp(other,
                           std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value
                           ? other.get_allocator_ref()
                           : this->get_allocator_ref());
        this->swap_impl(tmp);
        return *this;
    }

    robin_hash_map & operator = (robin_hash_map && other) noexcept(
        std::allocator_traits<allocator_type>::is_always_equal::value &&
        std::is_nothrow_move_assignable<hasher>::value &&
        std::is_nothrow_move_assignable<key_equal>::value) {
        // TODO: We should only use the operations from the noexcept clause
        // to make sure we actually adhere to other contract.
        return this->move_assign(std::move(other),
                     typename std::allocator_traits<allocator_type>::propagate_on_container_move_assignment());
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
    size_type slot_capacity() const { return (this->slot_mask_ + (this->slot_mask_ != 0)); }
    size_type slot_threshold() const { return this->slot_threshold_; }
    size_type slot_threshold(size_type now_slow_capacity) const {
        return (now_slow_capacity * this->integral_mlf() / kLoadFactorAmplify);
    }

    size_type max_lookups() const { return this->max_lookups_; }
    std::uint16_t max_distance() const {
        return (std::uint16_t)(this->max_lookups() << 8);
    }
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
        ctrl_type * ctrl = this->ctrls();
        ctrl_type * last_ctrl = this->ctrls() + this->group_count() * kGroupWidth;
        group_type group(ctrl), last_group(last_ctrl);
        size_type slot_index = 0;
        for (; group < last_group; ++group) {
            std::uint32_t maskUsed = group.matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = group.index(slot_index, pos);
                return this->iterator_at(index);
            }
            slot_index += kGroupWidth;
        }

        return this->end();
#else
        ctrl_type * ctrl = this->ctrls();
        size_type index;
        for (index = 0; index < this->max_slot_capacity(); index++) {
            if (ctrl->isUsed()) {
                return { this, index };
            }
            ctrl++;
        }
        return { this, index };
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

#if ROBIN_V2_USE_HASH_POLICY
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

#if ROBIN_V2_USE_HASH_POLICY
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
        return "jstd::v2::robin_hash_map<K, V>";
    }

    void clear(bool need_destroy = false) noexcept {
        if (this->slot_capacity() > kDefaultCapacity) {
            if (need_destroy) {
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
            throw std::out_of_range("std::out_of_range exception: jstd::v2::robin_hash_map<K,V>::at(key), "
                                    "the specified key is not exists.");
        }
    }

    const mapped_type & at(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        if (slot != this->last_slot()) {
            return slot->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::v2::robin_hash_map<K,V>::at(key) const, "
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
        return const_cast<const this_type *>(this)->find(key);
    }

    const_iterator find(const key_type & key) const {
        const slot_type * slot = this->find_impl(key);
        return this->iterator_at(this->index_of(slot));
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
        ctrl_type * ctrl = this->ctrl_at(index);
        return this->next_valid_iterator(ctrl, pos);
    }

    const_iterator erase(const_iterator pos) {
        size_type index = this->index_of(pos);
        this->erase_slot(index);
        ctrl_type * ctrl = this->ctrl_at(index);
        return this->next_valid_iterator(ctrl, pos);
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
    static ctrl_type * default_empty_ctrls() {
        static constexpr size_type kMinGroupCount = (kMinLookups + (kGroupWidth - 1)) / kGroupWidth;
        static constexpr size_type kMinCtrlCapacity = (kMinGroupCount + 1) * kGroupWidth;
        alignas(32) static const ctrl_type s_empty_ctrls[kMinCtrlCapacity] = {
            { kEmptySlot, 0 }, { kEmptySlot, 0 }, { kEmptySlot, 0 }, { kEmptySlot, 0,},
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },

            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 },
            { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }, { kEndOfMark, 0 }
        };
        return const_cast<ctrl_type *>(s_empty_ctrls);
    }

    JSTD_FORCED_INLINE
    size_type calc_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = (std::max)(init_capacity, kMinimumCapacity);
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinimumCapacity>(new_capacity);
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

    iterator iterator_at(ctrl_type * ctrl) noexcept {
        return { this, this->index_of(ctrl) };
    }

    const_iterator iterator_at(const ctrl_type * ctrl) const noexcept {
        return { this, this->index_of(ctrl) };
    }

    iterator iterator_at(slot_type * slot) noexcept {
        return { this, this->index_of(slot) };
    }

    const_iterator iterator_at(const slot_type * slot) const noexcept {
        return { this, this->index_of(slot) };
    }

    iterator next_valid_iterator(ctrl_type * ctrl, iterator iter) {
        if (ctrl->isUsed())
            return iter;
        else
            return ++iter;
    }

    const_iterator next_valid_iterator(ctrl_type * ctrl, const_iterator iter) {
        if (ctrl->isUsed())
            return iter;
        else
            return ++iter;
    }

    iterator next_valid_iterator(iterator iter) {
        size_type index = this->index_of(iter);
        ctrl_type * ctrl = this->ctrl_at(index);
        return this->next_valid_iterator(ctrl, iter);
    }

    const_iterator next_valid_iterator(const_iterator iter) {
        size_type index = this->index_of(iter);
        ctrl_type * ctrl = this->ctrl_at(index);
        return this->next_valid_iterator(ctrl, iter);
    }

    inline hash_code_t get_hash(const key_type & key) const
        noexcept(noexcept(this->hasher_(key)))
        /* noexcept(noexcept(std::declval<hasher &>()(key))) */ {
        hash_code_t hash_code = static_cast<hash_code_t>(this->hasher_(key));
        return hash_code;
    }

    inline hash_code_t get_second_hash(hash_code_t value) const noexcept {
#if 1
        return value;
#elif 1
        return (size_type)hashes::mum_hash64((std::uint64_t)value, 11400714819323198485ull);
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
#if ROBIN_V2_USE_HASH_POLICY
        return value;
#elif 0
        return (size_type)hashes::mum_hash64((std::uint64_t)value, 11400714819323198485ull);
#elif 1
        return (size_type)hashes::fibonacci_hash64((size_type)value);
#elif 1
        return (size_type)hashes::simple_int_hash_crc32((size_type)value);
#endif
    }

    inline size_type index_salt() const noexcept {
        return (size_type)((std::uintptr_t)this->ctrls() >> 12);
    }

    inline size_type index_for_hash(hash_code_t hash_code) const noexcept {
#if ROBIN_V2_USE_HASH_POLICY
        if (kUseIndexSalt) {
            hash_code ^= this->index_salt();
        }
        return this->hash_policy_.template index_for_hash<key_type>(hash_code, this->slot_mask());
#else
        if (kUseIndexSalt)
            return ((this->get_second_hash((size_type)hash_code) ^ this->index_salt()) & this->slot_mask());
        else
            return  (this->get_second_hash((size_type)hash_code) & this->slot_mask());
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

    slot_type * slot_at(size_type slot_index) noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->slots() + ssize_type(slot_index));
    }

    const slot_type * slot_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->max_slot_capacity());
        return (this->slots() + ssize_type(slot_index));
    }

    group_type group_at(size_type slot_index) noexcept {
        assert(slot_index < this->max_slot_capacity());
        return { this->ctrl_at(slot_index) };
    }

    const group_type group_at(size_type slot_index) const noexcept {
        assert(slot_index < this->max_slot_capacity());
        return { this->ctrl_at(slot_index) };
    }

    ctrl_type & get_ctrl(size_type slot_index) {
        assert(slot_index < this->max_slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
    }

    const ctrl_type & get_ctrl(size_type slot_index) const {
        assert(slot_index < this->max_slot_capacity());
        ctrl_type * ctrls = this->ctrls();
        return ctrls[slot_index];
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
#if 1
        return pos.index();
#else
        return this->index_of(pos.value());
#endif
    }

    size_type index_of(const_iterator pos) const {
#if 1
        return pos.index();
#else
        return this->index_of(pos.value());
#endif
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

    void destroy() noexcept {
        this->destroy_data();
    }

    void destroy_data() noexcept {
        // Note!!: destroy_slots() need use this->ctrls()
        this->destroy_slots();

        this->destroy_ctrls();
    }

    void destroy_slots() noexcept {
        this->clear_slots();

        if (this->slots_ != nullptr) {
            this->slot_allocator_.deallocate(this->slots_, this->max_slot_capacity());
        }
        this->slots_ = nullptr;
        this->last_slot_ = nullptr;

        this->slot_size_ = 0;
    }

    void destroy_ctrls() noexcept {
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
        ctrl_type * ctrl = ctrls;
        ctrl_type * last_ctrl = ctrls + group_count * kGroupWidth;
        group_type group(ctrl), last_group(last_ctrl);
        for (; group < last_group; ++group) {
            group.fillAllEmpty();
        }

        if (slot_capacity >= kGroupWidth) {
            group.fillAllEmpty();
        } else {
            assert(slot_capacity < kGroupWidth);
            group_type tail_group(ctrls + (slot_capacity + max_lookups));
            tail_group.fillAllEndOf();

            group.fillAllEndOf();
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
            this->slots_ = nullptr;
            this->last_slot_ = nullptr;
            this->slot_size_ = 0;
        }
        this->slot_mask_ = 0;
        this->max_lookups_ = kMinLookups;
        this->slot_threshold_ = 0;
#if ROBIN_V2_USE_HASH_POLICY
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
            assert(new_capacity >= kMinimumCapacity);
        } else {
            new_capacity = init_capacity;
        }

#if ROBIN_V2_USE_HASH_POLICY
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
        assert(new_capacity >= kMinimumCapacity);
        if (AlwaysResize ||
            (!AllowShrink && (new_capacity > this->slot_capacity())) ||
            (AllowShrink && (new_capacity != this->slot_capacity()))) {
            if (!AlwaysResize && !AllowShrink) {
                assert(new_capacity >= this->slot_size());
            }

            ctrl_type * old_ctrls = this->ctrls();
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
                        this->unique_insert_no_grow(old_slot);
                        this->destroy_slot(old_slot);
                    }
                    old_slot++;
                }
            } else {
                if (old_slot_capacity >= kGroupWidth) {
                    ctrl_type * ctrl = old_ctrls;
                    ctrl_type * last_ctrl = old_ctrls + old_group_count * kGroupWidth;
                    group_type group(ctrl), last_group(last_ctrl);
                    size_type start_index = 0;
                    for (; group < last_group; ++group) {
                        std::uint32_t maskUsed = group.matchUsed();
                        while (maskUsed != 0) {
                            size_type pos = BitUtils::bsf32(maskUsed);
                            maskUsed = BitUtils::clearLowBit32(maskUsed);
                            size_type old_index = group.index(start_index, pos);
                            slot_type * old_slot = old_slots + old_index;
                            this->unique_insert_no_grow(old_slot);
                            this->destroy_slot(old_slot);
                        }
                        start_index += kGroupWidth;
                    }
                } else {
                    ctrl_type * last_ctrl = old_ctrls + old_max_slot_capacity;
                    slot_type * old_slot = old_slots;
                    for (ctrl_type * ctrl = old_ctrls; ctrl != last_ctrl; ctrl++) {
                        if (likely(ctrl->isUsed())) {
                            this->unique_insert_no_grow(old_slot);
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
            if (old_slots != nullptr) {
                this->slot_allocator_.deallocate(old_slots, old_max_slot_capacity);
            }
        }
    }

    JSTD_FORCED_INLINE
    void construct_slot(size_type index) {
        slot_type * slot = this->slot_at(index);
        this->construct_slot(slot);
    }

    JSTD_FORCED_INLINE
    void construct_slot(slot_type * slot) {
        SlotPolicyTraits::construct(&this->allocator_, slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot(size_type index) {
        slot_type * slot = this->slot_at(index);
        this->destroy_slot(slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot(slot_type * slot) {
        if (!is_slot_trivial_destructor) {
            SlotPolicyTraits::destroy(&this->allocator_, slot);
        }
    }

    JSTD_FORCED_INLINE
    void destroy_slot_data(ctrl_type * ctrl, slot_type * slot) {
        this->setUnusedCtrl(ctrl);
        this->destroy_slot(slot);
    }

    template <typename T, bool isCompatibleLayout, bool isNoexceptMoveAssign>
    struct exchange_pair;

    template <typename T>
    struct exchange_pair<T, true, true> {
        template <typename Alloc, typename SlotType>
        static void exchange(Alloc * alloc, SlotType * src, SlotType * dest, SlotType * empty)
            noexcept(std::is_nothrow_move_assignable<T>::value)
        {
            empty->mutable_value = std::move(dest->mutable_value);
            dest->mutable_value = std::move(src->mutable_value);
        }
    };

    template <typename T>
    struct exchange_pair<T, true, false> {
        template <typename Alloc, typename SlotType>
        static void exchange(Alloc * alloc, SlotType * src, SlotType * dest, SlotType * empty)
            noexcept(std::is_nothrow_move_constructible<T>::value)
        {
            SlotPolicyTraits::exchange(alloc, src, dest, empty);
        }
    };

    template <typename T>
    struct exchange_pair<T, false, true> {
        typedef typename T::first_type                          first_type;
        typedef typename T::second_type                         second_type;
        typedef typename std::remove_const<first_type>::type    mutable_first_type;

        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }
#if 1
        template <typename Alloc, typename SlotType>
        static void exchange(Alloc * alloc, SlotType * src, SlotType * dest, SlotType * empty)
            noexcept(std::is_nothrow_move_constructible<mutable_first_type>::value &&
                     std::is_nothrow_move_constructible<second_type>::value)
        {
            SlotPolicyTraits::exchange(alloc, src, dest, empty);
        }
#else
        template <typename Alloc, typename SlotType>
        static void exchange(Alloc * alloc, SlotType * src, SlotType * dest, SlotType * empty)
            noexcept(std::is_nothrow_move_assignable<mutable_first_type>::value &&
                     std::is_nothrow_move_assignable<second_type>::value)
        {
            *mutable_key(&empty->value) = std::move(*mutable_key(&dest->value));
            empty->value.second = std::move(dest->value.second);

            *mutable_key(&dest->value) = std::move(*mutable_key(&src->value));
            dest->value.second = std::move(src->value.second);
        }
#endif
    };

    template <typename T>
    struct exchange_pair<T, false, false> {
        typedef typename T::first_type                          first_type;
        typedef typename T::second_type                         second_type;
        typedef typename std::remove_const<first_type>::type    mutable_first_type;

        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        template <typename Alloc, typename SlotType>
        static void exchange(Alloc * alloc, SlotType * src, SlotType * dest, SlotType * empty)
            noexcept(std::is_nothrow_move_constructible<mutable_first_type>::value &&
                     std::is_nothrow_move_constructible<second_type>::value)
        {
            SlotPolicyTraits::exchange(alloc, src, dest, empty);
        }
    };

    template <typename T, bool isCompatibleLayout, bool isNoexceptMoveAssign>
    struct swap_pair;

    template <typename T>
    struct swap_pair<T, true, true> {
        template <typename Alloc, typename SlotType>
        static void swap(Alloc * alloc, SlotType * slot1, SlotType * slot2, SlotType * tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value &&
                     std::is_nothrow_move_assignable<T>::value)
        {
            using std::swap;
            swap(slot1->mutable_value, slot2->mutable_value);
        }
    };

    template <typename T>
    struct swap_pair<T, true, false> {
        template <typename Alloc, typename SlotType>
        static void swap(Alloc * alloc, SlotType * slot1, SlotType * slot2, SlotType * tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value)
        {
            SlotPolicyTraits::construct(alloc, tmp, slot2);
            SlotPolicyTraits::destroy(alloc, slot2);
            SlotPolicyTraits::construct(alloc, slot2, slot1);
            SlotPolicyTraits::destroy(alloc, slot1);
            SlotPolicyTraits::construct(alloc, slot1, tmp);
            SlotPolicyTraits::destroy(alloc, tmp);
        }
    };

    template <typename T>
    struct swap_pair<T, false, true> {
        typedef typename T::first_type                          first_type;
        typedef typename T::second_type                         second_type;
        typedef typename std::remove_const<first_type>::type    mutable_first_type;

        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        template <typename Alloc, typename SlotType>
        static void swap(Alloc * alloc, SlotType * slot1, SlotType * slot2, SlotType * tmp)
            noexcept(std::is_nothrow_move_constructible<mutable_first_type>::value &&
                     std::is_nothrow_move_assignable<mutable_first_type>::value &&
                     std::is_nothrow_move_constructible<second_type>::value &&
                     std::is_nothrow_move_assignable<second_type>::value)
        {
            using std::swap;
            swap(*mutable_key(&slot1->value), *mutable_key(&slot2->value));
            swap(slot1->value.second, slot2->value.second);
        }
    };

    template <typename T>
    struct swap_pair<T, false, false> {
        typedef typename T::first_type                          first_type;
        typedef typename T::second_type                         second_type;
        typedef typename std::remove_const<first_type>::type    mutable_first_type;

        static mutable_first_type * mutable_key(T * value) {
            // Still check for isCompatibleLayout so that we can avoid calling jstd::launder
            // unless necessary because it can interfere with optimizations.
            return launder(const_cast<mutable_first_type *>(std::addressof(value->first)));
        }

        template <typename Alloc, typename SlotType>
        static void swap(Alloc * alloc, SlotType * slot1, SlotType * slot2, SlotType * tmp)
            noexcept(std::is_nothrow_move_constructible<T>::value &&
                     std::is_nothrow_move_constructible<second_type>::value &&
                     std::is_nothrow_move_assignable<second_type>::value)
        {
            typedef typename std::allocator_traits<Allocator>::template rebind_alloc<mutable_first_type>
                                                            FirstTypeAllocator;
            typedef typename std::allocator_traits<Allocator>::template rebind_traits<mutable_first_type>
                                                            FirstTypeTraits;
            FirstTypeAllocator first_alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(*alloc));
#if 1
            FirstTypeTraits::construct(first_alloc, mutable_key(&tmp->value), std::move(*mutable_key(&slot1->value)));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&slot1->value));
            FirstTypeTraits::construct(first_alloc, mutable_key(&slot1->value), std::move(*mutable_key(&slot2->value)));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&slot2->value));
            FirstTypeTraits::construct(first_alloc, mutable_key(&slot2->value), std::move(*mutable_key(&tmp->value)));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&tmp->value));

            using std::swap;
            swap(slot1->value.second, slot2->value.second);
#else
            typedef typename std::allocator_traits<Allocator>::template rebind_alloc<second_type>
                                                            SecondTypeAllocator;
            typedef typename std::allocator_traits<Allocator>::template rebind_traits<second_type>
                                                            SecondTypeTraits;

            SecondTypeAllocator second_alloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(*alloc));

            FirstTypeTraits::construct(first_alloc, mutable_key(&tmp->value), std::move(*mutable_key(&slot1->value)));
            SecondTypeTraits::construct(second_alloc, &tmp->value.second, std::move(slot1->value.second));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&slot1->value));
            SecondTypeTraits::destroy(second_alloc, &slot1->value.second);

            FirstTypeTraits::construct(first_alloc, mutable_key(&slot1->value), std::move(*mutable_key(&slot2->value)));
            SecondTypeTraits::construct(second_alloc, &slot1->value.second, std::move(slot2->value.second));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&slot2->value));
            SecondTypeTraits::destroy(second_alloc, &slot2->value.second);

            FirstTypeTraits::construct(first_alloc, mutable_key(&slot2->value), std::move(*mutable_key(&tmp->value)));
            SecondTypeTraits::construct(second_alloc, &slot2->value.second, std::move(tmp->value.second));
            FirstTypeTraits::destroy(first_alloc, mutable_key(&tmp->value));
            SecondTypeTraits::destroy(second_alloc, &tmp->value.second);
#endif
        }
    };

    JSTD_FORCED_INLINE
    void transfer_slot(slot_type * new_slot, slot_type * old_slot) {
        SlotPolicyTraits::transfer(&this->allocator_, new_slot, old_slot);
    }

    JSTD_FORCED_INLINE
    void transfer_slot(slot_type * new_slot, const slot_type * old_slot) {
        SlotPolicyTraits::transfer(this->allocator_, new_slot, old_slot);
    }

    JSTD_FORCED_INLINE
    void exchange_slot(slot_type * src, slot_type * dest, slot_type * empty) {
        if (kIsCompatibleLayout) {
            static constexpr bool isNoexceptMoveAssign = is_noexcept_move_assignable<mutable_value_type>::value;
            exchange_pair<mutable_value_type, true, isNoexceptMoveAssign>::exchange(
                &this->allocator_, src, dest, empty);
        } else {
            static constexpr bool isNoexceptMoveAssign = is_noexcept_move_assignable<value_type>::value;
            exchange_pair<value_type, false, isNoexceptMoveAssign>::exchange(
                &this->allocator_, src, dest, empty);
        }
    }

    JSTD_FORCED_INLINE
    void swap_slot(slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        if (kIsCompatibleLayout) {
            static constexpr bool isNoexceptMoveAssign = is_noexcept_move_assignable<mutable_value_type>::value;
            swap_pair<mutable_value_type, true, isNoexceptMoveAssign>::swap(
                &this->allocator_, slot1, slot2, tmp);
        } else {
            static constexpr bool isNoexceptMoveAssign = is_noexcept_move_assignable<value_type>::value;
            swap_pair<value_type, false, isNoexceptMoveAssign>::swap(
                &this->allocator_, slot1, slot2, tmp);
        }
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::int16_t dist_and_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        this->setUsedCtrl(ctrl, dist_and_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, std::int8_t dist, std::uint8_t ctrl_hash) {
        ctrl_type * ctrl = this->ctrl_at(index);
        this->setUsedCtrl(ctrl, dist, ctrl_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(size_type index, ctrl_type new_ctrl) {
        ctrl_type * ctrl = this->ctrl_at(index);
        this->setUsedCtrl(ctrl, new_ctrl);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(ctrl_type * ctrl, std::int16_t dist_and_hash) {
        ctrl->setValue(dist_and_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(ctrl_type * ctrl, std::int8_t dist, std::uint8_t ctrl_hash) {
        ctrl->setValue(dist, ctrl_hash);
    }

    JSTD_FORCED_INLINE
    void setUsedCtrl(ctrl_type * ctrl, ctrl_type new_ctrl) {
        ctrl->setValue(new_ctrl);
    }

    JSTD_FORCED_INLINE
    void setUnusedCtrl(size_type index) {
        ctrl_type * ctrl = this->ctrl_at(index);
        this->setUnused(ctrl);
    }

    JSTD_FORCED_INLINE
    void setUnusedCtrl(ctrl_type * ctrl) {
        assert(ctrl->isUsed());
        ctrl->setUnused();
    }

    const slot_type * find_impl(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);
        ctrl_type dist_and_hash;

        const ctrl_type * ctrl = this->ctrl_at(slot_index);
        const slot_type * slot = this->slot_at(slot_index);
#if 0
        dist_and_hash.setValue(0, ctrl_hash);

        while (dist_and_hash.value < ctrl->value) {
            dist_and_hash.incDist();
            ctrl++;
        }

        do {
            if (dist_and_hash.value == ctrl->value) {
                const slot_type * target = slot + dist_and_hash.dist;
                if (this->key_equal_(target->value.first, key)) {
                    return target;
                }
            } else if (dist_and_hash.dist > ctrl->dist) {
                break;
            }
            dist_and_hash.incDist();
            ctrl++;
        } while (1);

        return this->last_slot();
#elif 0
        dist_and_hash.setValue(0, ctrl_hash);

        while (dist_and_hash.value < ctrl->value) {
            dist_and_hash.incDist();
            ctrl++;
        }

        do {
            if (dist_and_hash.value == ctrl->value) {
                const slot_type * target = slot + dist_and_hash.dist;
                if (this->key_equal_(target->value.first, key)) {
                    return target;
                }
            }
            dist_and_hash.incDist();
            ctrl++;
        } while (dist_and_hash.dist <= ctrl->dist);

        return this->last_slot();
#elif 1
        ctrl_type dist_and_0(0, 0);

        while (dist_and_0.value <= ctrl->value) {
            if (this->key_equal_(slot->value.first, key)) {
                return slot;
            }
            dist_and_0.incDist();
            ctrl++;
            slot++;
        }

        return this->last_slot();
#elif 0
        ctrl_type dist_and_0(0, 0);

        while (dist_and_0.value <= ctrl->value) {
            if (ctrl->hash_equals(ctrl_hash)) {
                if (this->key_equal_(slot->value.first, key)) {
                    return slot;
                }
            }
            dist_and_0.incDist();
            ctrl++;
            slot++;
        }

        return this->last_slot();
#else
        if (ctrl->value >= std::int16_t(0)) {
            if (ctrl->hash_equals(ctrl_hash)) {
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
            if (ctrl->hash_equals(ctrl_hash)) {
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
        dist_and_hash.setValue(2, ctrl_hash);
        const slot_type * last_slot = this->last_slot();

        while (slot < last_slot) {
            group_type group(ctrl);
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
        }

        return last_slot;
    }

    enum FindResult {
        kNeedGrow = 0,
        kIsNotExists = 1,
        kIsExists = 2
    };

    JSTD_NO_INLINE
    std::pair<slot_type *, FindResult>
    find_or_insert(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);

        ctrl_type * ctrl = this->ctrl_at(slot_index);
        slot_type * slot = this->slot_at(slot_index);
        ctrl_type dist_and_0(0, 0);
        ctrl_type dist_and_hash;
#if 1
        while (dist_and_0.value <= ctrl->value) {
            if (this->key_equal_(slot->value.first, key)) {
                return { slot, kIsExists };
            }

            ctrl++;
            slot++;
            dist_and_0.incDist();
        }

        if (this->need_grow() || (dist_and_0.uvalue >= this->max_distance())) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            auto find_info = this->find_failed(hash_code, dist_and_0);
            ctrl = find_info.first;
            slot = find_info.second;
        }

        dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;
        //return { slot, kIsNotExists };
#elif 0
        while (dist_and_0.value <= ctrl->value) {
            if (ctrl->hash_equals(ctrl_hash)) {
                if (this->key_equal_(slot->value.first, key)) {
                    return { slot, kIsExists };
                }
            }
            ctrl++;
            slot++;
            dist_and_0.incDist();
            assert(slot < this->last_slot());
        }

        if (this->need_grow() || (dist_and_0.uvalue >= this->max_distance())) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            auto find_info = this->find_failed(hash_code, dist_and_0);
            ctrl = find_info.first;
            slot = find_info.second;
        }

        dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;
        //return { slot, kIsNotExists };
#else
        const slot_type * last_slot;

        if (dist_and_0.value <= ctrl->value) {
            if (ctrl->hash_equals(ctrl_hash)) {
                if (this->key_equal_(slot->value.first, key)) {
                    return { slot, kIsExists };
                }
            }
        } else {
            goto InsertOrGrow;
        }

        ctrl++;
        slot++;
        dist_and_0.incDist();

        if (dist_and_0.value <= ctrl->value) {
            if (ctrl->hash_equals(ctrl_hash)) {
                if (this->key_equal_(slot->value.first, key)) {
                    return { slot, kIsExists };
                }
            }
        } else {
            goto InsertOrGrow;
        }

        ctrl++;
        slot++;
        dist_and_hash.setValue(2, ctrl_hash);

        last_slot = this->last_slot();

        while (slot < last_slot) {
            group_type group(ctrl);
            auto mask32 = group.matchHashAndDistance(dist_and_hash.value);
            std::uint32_t maskHash = mask32.maskHash;
            while (maskHash != 0) {
                size_type pos = BitUtils::bsf32(maskHash);
                maskHash = BitUtils::clearLowBit32(maskHash);
                size_type index = group.index(0, pos);
                slot_type * target = slot + index;
                if (this->key_equal_(target->value.first, key)) {
                    dist_and_hash.dist += std::uint8_t(index);
                    return { target, kIsExists };
                }
            }
            std::uint32_t maskEmpty = mask32.maskEmpty;
            if (maskEmpty != 0) {
                // It's a [EmptyEntry], or (distance > ctrl->dist) entry.
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type index = group_type::index(0, pos);
                ctrl = ctrl + index;
                slot = slot + index;
                dist_and_hash.dist += std::uint8_t(index);
                break;
            }
            dist_and_hash.incDist(kGroupWidth);
            ctrl += kGroupWidth;
            slot += kGroupWidth;
        }

        dist_and_0.dist = dist_and_hash.dist;
        goto InsertOrGrow_Start;

InsertOrGrow:
        dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;

InsertOrGrow_Start:
        if (this->need_grow() || (dist_and_0.uvalue >= this->max_distance())) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            auto find_info = this->find_failed(hash_code, dist_and_0);
            ctrl = find_info.first;
            slot = find_info.second;
            dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;
        }
#endif

        if (ctrl->isEmpty()) {
            this->setUsedCtrl(ctrl, dist_and_hash);
            return { slot, kIsNotExists };
        } else {
            bool neednt_grow = this->insert_to_place<false>(ctrl, slot, dist_and_hash);
            return { slot, static_cast<FindResult>(neednt_grow) };
        }
    }

    JSTD_FORCED_INLINE
    std::pair<ctrl_type *, slot_type *>
    find_failed(hash_code_t hash_code, ctrl_type & o_dist_and_0) {
        size_type slot_index = this->index_for_hash(hash_code);
        ctrl_type * ctrl = this->ctrl_at(slot_index);

        ctrl_type dist_and_0(0, 0);
        while (dist_and_0.value <= ctrl->value) {
            dist_and_0.incDist();
            ctrl++;
        }

        slot_index = this->index_of(ctrl);
        slot_type * slot = this->slot_at(slot_index);
        o_dist_and_0 = dist_and_0;
        return { ctrl, slot };
    }

    template <bool isRehashing>
    JSTD_FORCED_INLINE
    bool insert_to_place(ctrl_type * insert_ctrl, slot_type * insert_slot, ctrl_type dist_and_hash) {
        ctrl_type * ctrl = insert_ctrl;
        slot_type * target = insert_slot;
        ctrl_type to_insert(dist_and_hash);
        assert(!ctrl->isEmpty());
        assert(to_insert.dist > ctrl->dist);
        std::swap(to_insert.value, ctrl->value);
        ctrl++;
        to_insert.incDist();
#if 1
        alignas(slot_type) unsigned char slot_raw[sizeof(slot_type)];

        slot_type * empty  = reinterpret_cast<slot_type *>(&slot_raw);
        slot_type * insert = insert_slot;
#else
        alignas(slot_type) unsigned char slot_raw1[sizeof(slot_type)];
        alignas(slot_type) unsigned char slot_raw2[sizeof(slot_type)];

        slot_type * insert = reinterpret_cast<slot_type *>(&slot_raw1);
        slot_type * empty  = reinterpret_cast<slot_type *>(&slot_raw2);

        this->transfer_slot(insert, target);
#endif
        target++;

        // Initialize the empty slot use default constructor if necessary
        this->construct_empty_slot(empty);

        slot_type * last_slot = this->last_slot();
        while (target < last_slot) {
            if (ctrl->isEmpty()) {
                this->emplace_rich_slot(ctrl, target, insert, to_insert.value);
                this->destroy_empty_slot(empty);
                return true;
            } else if (to_insert.dist > ctrl->dist) {
                std::swap(to_insert.value, ctrl->value);
                if (kIsPlainKV) {
                    this->swap_slot(insert, target, empty);
                } else {
                    this->exchange_slot(insert, target, empty);
                    std::swap(insert, empty);
                }
            }

            ctrl++;
            target++;
            to_insert.incDist();
            assert(to_insert.dist <= kMaxDist);

            if (isRehashing) {
                assert(to_insert.uvalue < this->max_distance());
            } else {
                if (to_insert.uvalue >= this->max_distance()) {
                    this->emplace_rich_slot(insert_ctrl, insert_slot, insert, to_insert.value);
                    this->destroy_empty_slot(empty);
                    return false;
                }
            }
        }

        this->emplace_rich_slot(insert_ctrl, insert_slot, insert, to_insert.value);
        this->destroy_empty_slot(empty);
        return false;
    }

    JSTD_FORCED_INLINE
    void emplace_rich_slot(ctrl_type * ctrl, slot_type * slot, slot_type * insert,
                           std::int16_t dist_and_hash) {
        this->setUsedCtrl(ctrl, dist_and_hash);
        this->transfer_slot(slot, insert);
    }

    JSTD_FORCED_INLINE
    void construct_empty_slot(slot_type * empty) {
        static constexpr bool isNoexceptMoveAssignable = is_noexcept_move_assignable<value_type>::value;
        static constexpr bool isMutableNoexceptMoveAssignable = is_noexcept_move_assignable<mutable_value_type>::value;

        if ((!is_slot_trivial_destructor) && (!kIsPlainKV)) {
            if (kIsCompatibleLayout) {
                if (isMutableNoexceptMoveAssignable) {
                    this->construct_slot(empty);
                    return;
                }
            } else {
                if (isNoexceptMoveAssignable) {
                    this->construct_slot(empty);
                    return;
                }
            }
        }
        this->placement_new_slot(empty);
    }

    JSTD_FORCED_INLINE
    void destroy_empty_slot(slot_type * empty) {
        static constexpr bool isNoexceptMoveAssignable = is_noexcept_move_assignable<value_type>::value;
        static constexpr bool isMutableNoexceptMoveAssignable = is_noexcept_move_assignable<mutable_value_type>::value;

        if ((!is_slot_trivial_destructor) && (!kIsPlainKV)) {
            if (kIsCompatibleLayout) {
                if (isMutableNoexceptMoveAssignable) {
                    this->destroy_slot(empty);
                }
            } else {
                if (isNoexceptMoveAssignable) {
                    this->destroy_slot(empty);
                }
            }
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(const value_type & value) {
        auto find_info = this->find_or_insert(value.first);
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace_impl<AlwaysUpdate>(value);
            }

            this->placement_new_slot(slot);
            if (kIsCompatibleLayout)
                this->mutable_allocator_.construct(&slot->mutable_value, value);
            else
                this->allocator_.construct(&slot->value, value);
            this->slot_size_++;
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot->value.second = value.second;
            }
            return { this->iterator_at(slot), false };
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> emplace_impl(value_type && value) {
        auto find_info = this->find_or_insert(value.first);
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace_impl<AlwaysUpdate>(std::forward<value_type>(value));
            }

            if (kIsCompatibleLayout)
                this->mutable_allocator_.construct(&slot->mutable_value, std::forward<value_type>(value));
            else
                this->allocator_.construct(&slot->value, std::forward<value_type>(value));
            this->slot_size_++;
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                static constexpr bool is_rvalue_ref = std::is_rvalue_reference<decltype(value)>::value;
                if (is_rvalue_ref)
                    slot->value.second = std::move(value.second);
                else
                    slot->value.second = value.second;
            }
            return { this->iterator_at(slot), false };
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
        auto find_info = this->find_or_insert(key);
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace_impl<AlwaysUpdate, KeyT, MappedT>(
                        std::forward<KeyT>(key), std::forward<MappedT>(value)
                    );
            }

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
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            static constexpr bool isMappedType = jstd::is_same_ex<MappedT, mapped_type>::value;
            if (AlwaysUpdate) {
                if (isMappedType) {
                    slot->value.second = std::forward<MappedT>(value);
                } else {
                    mapped_type mapped_value(std::forward<MappedT>(value));
                    slot->value.second = std::move(mapped_value);
                }
            }
            return { this->iterator_at(slot), false };
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
        auto find_info = this->find_or_insert(key);
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace(std::piecewise_construct,
                                        std::forward_as_tuple(std::forward<KeyT>(key)),
                                        std::forward_as_tuple(std::forward<Args>(args)...));
            }

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
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                mapped_type mapped_value(std::forward<Args>(args)...);
                slot->value.second = std::move(mapped_value);
            }
            return { this->iterator_at(slot), false };
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
        tuple_wrapper2<key_type> key_wrapper(first);
        auto find_info = this->find_or_insert(key_wrapper.value());
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace(std::piecewise_construct,
                                        std::forward<std::tuple<Ts1...>>(first),
                                        std::forward<std::tuple<Ts2...>>(second));
            }

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
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                tuple_wrapper2<mapped_type> mapped_wrapper(std::move(second));
                slot->value.second = std::move(mapped_wrapper.value());
            }
            return { this->iterator_at(slot), false };
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
        value_type value(std::forward<First>(first), std::forward<Args>(args)...);
        auto find_info = this->find_or_insert(value.first);
        slot_type * slot = find_info.first;
        size_type is_exists = find_info.second;
        if (is_exists != kIsExists) {
            // The key to be inserted is not exists.
            assert(slot != nullptr);
            if (is_exists == kNeedGrow) {
                this->grow_if_necessary();
                return this->emplace(std::move(value));
            }

            this->placement_new_slot(slot);
            if (kIsCompatibleLayout) {
                this->mutable_allocator_.construct(&slot->mutable_value, std::move(value));
            } else {
                this->allocator_.construct(&slot->value, std::move(value));
            }
            this->slot_size_++;
            return { this->iterator_at(slot), true };
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot->value.second = std::move(value.second);
            }
            return { this->iterator_at(slot), false };
        }
    }

    JSTD_FORCED_INLINE
    std::pair<slot_type *, bool>
    unique_find_or_insert(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);

        ctrl_type * ctrl = this->ctrl_at(slot_index);
        slot_type * slot = this->slot_at(slot_index);
        ctrl_type dist_and_0(0, 0);

        while (dist_and_0.value <= ctrl->value) {
            dist_and_0.incDist();
            ctrl++;
        }

        if (this->need_grow() || (dist_and_0.uvalue >= this->max_distance())) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            auto find_info = this->find_failed(hash_code, dist_and_0);
            ctrl = find_info.first;
            slot = find_info.second;
        } else {
            slot += dist_and_0.dist;
        }

        ctrl_type dist_and_hash;
        dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;

#if 0
        slot_type * last_slot = this->last_slot();

        // Find the first empty slot and insert
        while (slot < last_slot) {
            group_type group(ctrl);
            std::uint32_t maskEmpty = group.matchEmptyAndDistance(dist_and_hash.dist);
            if (maskEmpty != 0) {
                // Found a [EmptyEntry] to insert
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type index = group.index(0, pos);
                ctrl += index;
                slot += index;
                dist_and_hash.dist += std::uint8_t(index);
                goto Insert_To_Slot;
            }
            dist_and_hash.incDist(kGroupWidth);
            ctrl += kGroupWidth;
            slot += kGroupWidth;            
            assert(dist_and_hash.value >= 0 && dist_and_hash.uvalue < this->max_distance());
        }

        return { nullptr, false };

Insert_To_Slot:
#endif
        if (ctrl->isEmpty()) {
            this->setUsedCtrl(ctrl, dist_and_hash);
            return { slot, false };
        } else {
            bool neednt_grow = this->insert_to_place<true>(ctrl, slot, dist_and_hash);
            return { slot, !neednt_grow };
        }
    }

    JSTD_FORCED_INLINE
    std::pair<slot_type *, bool>
    unique_find_or_insert_no_grow(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        size_type slot_index = this->index_for_hash(hash_code);
        std::uint8_t ctrl_hash = this->get_ctrl_hash(hash_code);

        ctrl_type * ctrl = this->ctrl_at(slot_index);
        slot_type * slot = this->slot_at(slot_index);
        ctrl_type dist_and_0(0, 0);

        while (dist_and_0.value <= ctrl->value) {
            dist_and_0.incDist();
            ctrl++;
        }

        slot += dist_and_0.dist;

        ctrl_type dist_and_hash;
        dist_and_hash.uvalue = dist_and_0.uvalue | ctrl_hash;

        if (ctrl->isEmpty()) {
            this->setUsedCtrl(ctrl, dist_and_hash);
            return { slot, false };
        } else {
            bool neednt_grow = this->insert_to_place<true>(ctrl, slot, dist_and_hash);
            return { slot, !neednt_grow };
        }
    }

    // Use in rehash_impl()
    bool unique_insert_no_grow(slot_type * slot) {
        ctrl_type dist_and_hash;
        auto find_info = this->unique_find_or_insert_no_grow(slot->value.first);
        slot_type * new_slot = find_info.first;
        bool need_grow = find_info.second;

        this->placement_new_slot(new_slot);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&new_slot->mutable_value,
                                               std::move(slot->mutable_value));
        } else {
            this->allocator_.construct(&new_slot->value, std::move(slot->value));
        }
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
        return need_grow;
    }

    void unique_insert(const value_type & value) {
        auto find_info = this->unique_find_or_insert(value.first);
        slot_type * slot = find_info.first;
        bool need_grow = find_info.second;

        if (need_grow) {
            this->grow_if_necessary();
            this->unique_insert(value);
            return;
        }

        this->placement_new_slot(slot);
        if (kIsCompatibleLayout) {
            this->mutable_allocator_.construct(&slot->mutable_value, value);
        } else {
            this->allocator_.construct(&slot->value, value);
        }
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    void unique_insert(value_type && value) {
        auto find_info = this->unique_find_or_insert(value.first);
        slot_type * slot = find_info.first;
        bool need_grow = find_info.second;

        if (need_grow) {
            this->grow_if_necessary();
            this->unique_insert(std::forward<value_type>(value));
            return;
        }

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
    void unique_insert(InputIter first, InputIter last) {
        for (InputIter iter = first; iter != last; ++iter) {
            this->unique_insert(static_cast<value_type>(*iter));
        }
    }

    JSTD_FORCED_INLINE
    size_type find_and_erase(const key_type & key) {
        const slot_type * slot = this->find_impl(key);
        if (slot != this->last_slot()) {
            size_type to_erase = this->index_of(slot);
            this->erase_slot(to_erase);
            return 1;
        } else {
            return 0;
        }
    }

    JSTD_NO_INLINE
    void erase_slot(size_type to_erase) {
        assert(to_erase < this->max_slot_capacity());

        ctrl_type * curr_ctrl = this->ctrl_at(to_erase);
        slot_type * curr_slot = this->slot_at(to_erase);
        assert(curr_ctrl->isUsed());

        this->destroy_slot(curr_slot);

        assert(this->slot_size_ > 0);
        this->slot_size_--;

        ctrl_type * next_ctrl = curr_ctrl + std::ptrdiff_t(1);
        slot_type * next_slot = curr_slot + std::ptrdiff_t(1);

        while (!next_ctrl->isUnusedOrZero()) {
            ctrl_type dist_and_hash(*next_ctrl);
            assert(dist_and_hash.dist > 0);
            dist_and_hash.decDist();
            this->setUsedCtrl(curr_ctrl, dist_and_hash);
            this->transfer_slot(curr_slot, next_slot);

            curr_ctrl++;
            curr_slot++;

            next_ctrl++;
            next_slot++;
        }

        this->setUnusedCtrl(curr_ctrl);
    }

    // TODO: Optimize this assuming *this and other don't overlap.
    this_type & move_assign(this_type && other, std::true_type) {
        if (std::addressof(other) != this) {
            this_type tmp(std::move(other));
            this->swap_impl(tmp);
        }
        return *this;
    }

    this_type & move_assign(this_type && other, std::false_type) {
        if (std::addressof(other) != this) {
            this_type tmp(std::move(other), this->get_allocator_ref());
            this->swap_impl(tmp);
        }
        return *this;
    }

    void swap_content(this_type & other) {
        using std::swap;
        swap(this->ctrls_, other.ctrls_);
        swap(this->slots_, other.slots_);
        swap(this->slot_size_, other.slot_size_);
        swap(this->slot_mask_, other.slot_mask_);
        swap(this->max_lookups_, other.max_lookups_);
        swap(this->slot_threshold_, other.slot_threshold_);
        swap(this->n_mlf_, other.n_mlf_);
        swap(this->n_mlf_rev_, other.n_mlf_rev_);
#if ROBIN_V2_USE_HASH_POLICY
        swap(this->hash_policy_, other.hash_policy_ref());
#endif
    }

    void swap_policy(this_type & other) {
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

    void swap_impl(this_type & other) {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

template <class Key, class Value, class Hash, class KeyEqual, class LayoutPolicy, class Alloc, class Pred>
typename robin_hash_map<Key, Value, Hash, KeyEqual, LayoutPolicy, Alloc>::size_type
inline
erase_if(robin_hash_map<Key, Value, Hash, KeyEqual, LayoutPolicy, Alloc> & hash_map, Pred pred)
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

} // namespace v2
} // namespace jstd
