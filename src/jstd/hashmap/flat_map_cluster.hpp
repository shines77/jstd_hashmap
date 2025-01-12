/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2024 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/cluster_flat_map
  https://gitee.com/shines77/cluster_flat_map

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

#ifndef JSTD_HASHMAP_FLAT_MAP_CLUSTER_HPP
#define JSTD_HASHMAP_FLAT_MAP_CLUSTER_HPP

#pragma once

#include <cstdint>
#include <assert.h>

#include "jstd/basic/stddef.h"
#include "jstd/support/BitVec.h"
#include "jstd/memory/memory_barrier.h"

namespace jstd {

class JSTD_DLL cluster_meta_ctrl
{
public:
    static constexpr std::uint8_t kHashMask     = 0b01111111;
    static constexpr std::uint8_t kEmptySlot    = 0b00000000 & kHashMask;
    static constexpr std::uint8_t kOverflowMask = 0b10000000;

    static_assert(((kHashMask & kOverflowMask) == 0), "kHashMask & kOverflowMask must be 0");
    static_assert(((kHashMask | kOverflowMask) == 0b11111111), "kHashMask & kOverflowMask must be 0b11111111");

    typedef std::uint8_t value_type;
    typedef std::uint8_t hash_type;

    cluster_meta_ctrl(value_type value = kEmptySlot) : value(value) {}
    ~cluster_meta_ctrl() {}

    static inline value_type hash_bits(std::size_t hash) {
        return static_cast<value_type>(hash & static_cast<std::size_t>(kHashMask));
    }

    static inline std::size_t hash_bits64(std::size_t hash) {
        return (hash & static_cast<std::size_t>(kHashMask));
    }

    static inline value_type overflow_bits(std::size_t hash) {
        return static_cast<value_type>(hash & static_cast<std::size_t>(kOverflowMask));
    }

    inline bool is_empty() const {
        value_type hash = hash_bits(this->value);
        return (hash == kEmptySlot);
    }

    inline bool is_used() const {
        value_type hash = hash_bits(this->value);
        return (hash != kEmptySlot);
    }

    inline bool is_overflow() const {
        value_type overflow = overflow_bits(this->value);
        return (overflow != 0);
    }

    bool is_equals(hash_type hash) {
        value_type hash8 = hash_bits(this->value);
        return (hash == hash8);
    }

    bool is_equals64(std::size_t hash) {
        std::size_t hash64 = hash_bits64(this->value);
        return (hash == hash64);
    }

    inline value_type get_hash() const {
        return hash_bits(this->value);
    }

    inline value_type get_value() const {
        return this->value;
    }

    inline value_type get_index() const {
        return 0;
    }

    inline void set_empty() {
        this->value = kEmptySlot;
    }

    inline void set_used(hash_type hash) {
        assert(hash_bits(hash) != kEmptySlot);
        this->value = hash;
    }

    inline void set_used64(std::size_t hash) {
        assert(hash_bits(hash) != kEmptySlot);
        this->value = hash_bits(hash);
    }

    inline void set_used_strict(hash_type hash) {
        assert(hash_bits(hash) != kEmptySlot);
        this->value = hash_bits(hash);
    }

    inline void set_overflow() {
        assert(hash_bits(this->value) != kEmptySlot);
        this->value |= kOverflowMask;
    }

    inline void set_value(value_type value) {
        this->value = value;
    }

private:
    value_type value;
};

template <typename T>
class JSTD_DLL flat_map_cluster16
{
public:
    typedef T                       ctrl_type;
    typedef typename T::value_type  value_type;
    typedef typename T::hash_type   hash_type;
    typedef ctrl_type *             pointer;
    typedef const ctrl_type *       const_pointer;
    typedef ctrl_type &             reference;
    typedef const ctrl_type &       const_reference;

    static constexpr std::uint8_t kHashMask     = ctrl_type::kHashMask;
    static constexpr std::uint8_t kEmptySlot    = ctrl_type::kEmptySlot;
    static constexpr std::uint8_t kOverflowMask = ctrl_type::kOverflowMask;

    static constexpr std::size_t kGroupWidth = 16;

    flat_map_cluster16() {}
    ~flat_map_cluster16() {}

    void init() {
        if (kEmptySlot == 0b00000000) {
            __m128i zeros = _mm_setzero_si128();
            _mm_store_si128(reinterpret_cast<__m128i *>(ctrls), zeros);
        }
        else if (kEmptySlot == 0b11111111) {
            __m128i ones = _mm_setones_si128();
            _mm_store_si128(reinterpret_cast<__m128i *>(ctrls), ones);
        }
        else {
            __m128i empty_bits = _mm_set1_epi8(kEmptySlot);
            _mm_store_si128(reinterpret_cast<__m128i *>(ctrls), empty_bits);
        }
    }

    inline __m128i _load_data() const {
        return _mm_load_si128(reinterpret_cast<const __m128i *>(ctrls));
    }

    inline bool is_empty(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type * ctrl = &ctrls[pos];
        return ctrl->is_empty();
    }

    inline bool is_used(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type * ctrl = &ctrls[pos];
        return ctrl->is_used();
    }

    inline bool is_overflow(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type * ctrl = &ctrls[pos];
        return ctrl->is_overflow();
    }

    bool is_equals(std::size_t pos, hash_type hash) {
        assert(pos < kGroupWidth);
        const ctrl_type * ctrl = &ctrls[pos];
        return ctrl->is_equals(hash);
    }

    bool is_equals64(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupWidth);
        const ctrl_type * ctrl = &ctrls[pos];
        return ctrl->is_equals64(hash);
    }

    inline void set_empty(std::size_t pos) {
        assert(pos < kGroupWidth);
        ctrl_type * ctrl = &ctrls[pos];
        ctrl->set_empty();
    }

    inline void set_used(std::size_t pos, hash_type hash) {
        assert(pos < kGroupWidth);
        ctrl_type * ctrl = &ctrls[pos];
        ctrl->set_used(hash);
    }

    inline void set_used64(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupWidth);
        ctrl_type * ctrl = &ctrls[pos];
        ctrl->set_used64(hash);
    }

    inline void set_used_strict(std::size_t pos, hash_type hash) {
        assert(pos < kGroupWidth);
        ctrl_type * ctrl = &ctrls[pos];
        ctrl->set_used_strict(hash);
    }

    inline void set_overflow(std::size_t pos) {
        ctrl_type * ctrl = &ctrls[pos];
        ctrl->set_overflow();
    }

    inline std::uint32_t match_empty() const {
        // Latency = 6
        __m128i ctrl_bits = _load_data();
        __COMPILER_BARRIER();
        __m128i mask_bits = _mm_set1_epi8(kHashMask);
        __COMPILER_BARRIER();

        __m128i empty_bits;
        if (kEmptySlot == 0b00000000)
            empty_bits = _mm_setzero_si128();
        else if (kEmptySlot == 0b11111111)
            empty_bits = _mm_setones_si128();
        else
            empty_bits = _mm_set1_epi8(kEmptySlot);

        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

    inline std::uint32_t match_used() const {
        // Latency = 6
        __m128i ctrl_bits = _load_data();
        __COMPILER_BARRIER();
        __m128i mask_bits = _mm_set1_epi8(kHashMask);
        __COMPILER_BARRIER();

        __m128i empty_bits, match_mask;
        if (kEmptySlot == 0b00000000) {
            empty_bits = _mm_setzero_si128();
            match_mask = _mm_cmpgt_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        }
        else if (kEmptySlot == 0b11111111) {
            empty_bits = _mm_setones_si128();
            match_mask = _mm_cmplt_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        }
        else {
            empty_bits = _mm_set1_epi8(kEmptySlot);
            match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        }

        int mask = _mm_movemask_epi8(match_mask);
        if (kEmptySlot != 0b00000000 && kEmptySlot != 0b11111111) {
            mask = ~mask & 0xFFFF;
        }
        return static_cast<std::uint32_t>(mask);
    }

    inline std::uint32_t match_hash(hash_type hash) const {
        // Latency = 6
        __m128i ctrl_bits  = _load_data();
        __COMPILER_BARRIER();
        __m128i mask_bits  = _mm_set1_epi8(kHashMask);
        __COMPILER_BARRIER();
        __m128i hash_bits  = _mm_set1_epi8(hash);
        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), hash_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

private:
    alignas(16) ctrl_type ctrls[kGroupWidth];
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_CLUSTER_HPP
