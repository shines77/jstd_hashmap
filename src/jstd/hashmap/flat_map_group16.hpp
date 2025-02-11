/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2024-2025 XiongHui Guo (gz_shines at msn.com)

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

#ifndef JSTD_HASHMAP_FLAT_MAP_GROUP16_HPP
#define JSTD_HASHMAP_FLAT_MAP_GROUP16_HPP

#pragma once

#include <cstdint>
#include <cstddef>
#include <assert.h>

#include "jstd/basic/stddef.h"
#include "jstd/traits/type_traits.h"    // For jstd::narrow_cast<T>()
#include "jstd/support/BitVec.h"
#include "jstd/memory/memory_barrier.h"

#define GROUP16_USE_LOOK_UP_TABLE   1

namespace jstd {

class JSTD_DLL group16_meta_ctrl
{
public:
    typedef std::uint8_t value_type;

    static constexpr const value_type kHashMask       = 0b01111111;
    static constexpr const value_type kEmptySlot      = 0b00000000 & kHashMask;
    static constexpr const value_type kOverflowMask   = 0b10000000;

    static constexpr const std::uint32_t kHashMask32  = 0x7F7F7F7Fu;
    static constexpr const std::uint32_t kEmptySlot32 = 0x00000000u;

    static constexpr const value_type kEmptyHash      = 0x08;
    static constexpr const std::uint32_t kEmptyHash32 = 0x08080808u;

    static_assert(((kHashMask & kOverflowMask) == 0), "kHashMask & kOverflowMask must be 0");
    static_assert(((kHashMask | kOverflowMask) == 0b11111111), "kHashMask & kOverflowMask must be 0b11111111");

    group16_meta_ctrl(value_type value = kEmptySlot) : value_(value) {}

    static JSTD_FORCED_INLINE
    int repeated_hash(std::size_t hash) {
        static constexpr std::uint32_t dword_hashs[] = {
           kEmptyHash32, 0x01010101u, 0x02020202u, 0x03030303u,
            0x04040404u, 0x05050505u, 0x06060606u, 0x07070707u,
            0x08080808u, 0x09090909u, 0x0A0A0A0Au, 0x0B0B0B0Bu,
            0x0C0C0C0Cu, 0x0D0D0D0Du, 0x0E0E0E0Eu, 0x0F0F0F0Fu,
            0x10101010u, 0x11111111u, 0x12121212u, 0x13131313u,
            0x14141414u, 0x15151515u, 0x16161616u, 0x17171717u,
            0x18181818u, 0x19191919u, 0x1A1A1A1Au, 0x1B1B1B1Bu,
            0x1C1C1C1Cu, 0x1D1D1D1Du, 0x1E1E1E1Eu, 0x1F1F1F1Fu,
            0x20202020u, 0x21212121u, 0x22222222u, 0x23232323u,
            0x24242424u, 0x25252525u, 0x26262626u, 0x27272727u,
            0x28282828u, 0x29292929u, 0x2A2A2A2Au, 0x2B2B2B2Bu,
            0x2C2C2C2Cu, 0x2D2D2D2Du, 0x2E2E2E2Eu, 0x2F2F2F2Fu,
            0x30303030u, 0x31313131u, 0x32323232u, 0x33333333u,
            0x34343434u, 0x35353535u, 0x36363636u, 0x37373737u,
            0x38383838u, 0x39393939u, 0x3A3A3A3Au, 0x3B3B3B3Bu,
            0x3C3C3C3Cu, 0x3D3D3D3Du, 0x3E3E3E3Eu, 0x3F3F3F3Fu,
            0x40404040u, 0x41414141u, 0x42424242u, 0x43434343u,
            0x44444444u, 0x45454545u, 0x46464646u, 0x47474747u,
            0x48484848u, 0x49494949u, 0x4A4A4A4Au, 0x4B4B4B4Bu,
            0x4C4C4C4Cu, 0x4D4D4D4Du, 0x4E4E4E4Eu, 0x4F4F4F4Fu,
            0x50505050u, 0x51515151u, 0x52525252u, 0x53535353u,
            0x54545454u, 0x55555555u, 0x56565656u, 0x57575757u,
            0x58585858u, 0x59595959u, 0x5A5A5A5Au, 0x5B5B5B5Bu,
            0x5C5C5C5Cu, 0x5D5D5D5Du, 0x5E5E5E5Eu, 0x5F5F5F5Fu,
            0x60606060u, 0x61616161u, 0x62626262u, 0x63636363u,
            0x64646464u, 0x65656565u, 0x66666666u, 0x67676767u,
            0x68686868u, 0x69696969u, 0x6A6A6A6Au, 0x6B6B6B6Bu,
            0x6C6C6C6Cu, 0x6D6D6D6Du, 0x6E6E6E6Eu, 0x6F6F6F6Fu,
            0x70707070u, 0x71717171u, 0x72727272u, 0x73737373u,
            0x74747474u, 0x75757575u, 0x76767676u, 0x77777777u,
            0x78787878u, 0x79797979u, 0x7A7A7A7Au, 0x7B7B7B7Bu,
            0x7C7C7C7Cu, 0x7D7D7D7Du, 0x7E7E7E7Eu, 0x7F7F7F7Fu,

            // This is a mirror of 0 ~ 127
            // Actually, it wasn't used from here on
           kEmptyHash32, 0x01010101u, 0x02020202u, 0x03030303u,
            0x04040404u, 0x05050505u, 0x06060606u, 0x07070707u,
            0x08080808u, 0x09090909u, 0x0A0A0A0Au, 0x0B0B0B0Bu,
            0x0C0C0C0Cu, 0x0D0D0D0Du, 0x0E0E0E0Eu, 0x0F0F0F0Fu,
            0x10101010u, 0x11111111u, 0x12121212u, 0x13131313u,
            0x14141414u, 0x15151515u, 0x16161616u, 0x17171717u,
            0x18181818u, 0x19191919u, 0x1A1A1A1Au, 0x1B1B1B1Bu,
            0x1C1C1C1Cu, 0x1D1D1D1Du, 0x1E1E1E1Eu, 0x1F1F1F1Fu,
            0x20202020u, 0x21212121u, 0x22222222u, 0x23232323u,
            0x24242424u, 0x25252525u, 0x26262626u, 0x27272727u,
            0x28282828u, 0x29292929u, 0x2A2A2A2Au, 0x2B2B2B2Bu,
            0x2C2C2C2Cu, 0x2D2D2D2Du, 0x2E2E2E2Eu, 0x2F2F2F2Fu,
            0x30303030u, 0x31313131u, 0x32323232u, 0x33333333u,
            0x34343434u, 0x35353535u, 0x36363636u, 0x37373737u,
            0x38383838u, 0x39393939u, 0x3A3A3A3Au, 0x3B3B3B3Bu,
            0x3C3C3C3Cu, 0x3D3D3D3Du, 0x3E3E3E3Eu, 0x3F3F3F3Fu,
            0x40404040u, 0x41414141u, 0x42424242u, 0x43434343u,
            0x44444444u, 0x45454545u, 0x46464646u, 0x47474747u,
            0x48484848u, 0x49494949u, 0x4A4A4A4Au, 0x4B4B4B4Bu,
            0x4C4C4C4Cu, 0x4D4D4D4Du, 0x4E4E4E4Eu, 0x4F4F4F4Fu,
            0x50505050u, 0x51515151u, 0x52525252u, 0x53535353u,
            0x54545454u, 0x55555555u, 0x56565656u, 0x57575757u,
            0x58585858u, 0x59595959u, 0x5A5A5A5Au, 0x5B5B5B5Bu,
            0x5C5C5C5Cu, 0x5D5D5D5Du, 0x5E5E5E5Eu, 0x5F5F5F5Fu,
            0x60606060u, 0x61616161u, 0x62626262u, 0x63636363u,
            0x64646464u, 0x65656565u, 0x66666666u, 0x67676767u,
            0x68686868u, 0x69696969u, 0x6A6A6A6Au, 0x6B6B6B6Bu,
            0x6C6C6C6Cu, 0x6D6D6D6Du, 0x6E6E6E6Eu, 0x6F6F6F6Fu,
            0x70707070u, 0x71717171u, 0x72727272u, 0x73737373u,
            0x74747474u, 0x75757575u, 0x76767676u, 0x77777777u,
            0x78787878u, 0x79797979u, 0x7A7A7A7Au, 0x7B7B7B7Bu,
            0x7C7C7C7Cu, 0x7D7D7D7Du, 0x7E7E7E7Eu, 0x7F7F7F7Fu
        };

        return (int)dword_hashs[jstd::narrow_cast<std::uint8_t>(hash)];
    }

    static JSTD_FORCED_INLINE
    std::uint8_t reduced_hash(std::size_t hash) {
        return jstd::narrow_cast<std::uint8_t>(repeated_hash(hash));
    }

    static JSTD_FORCED_INLINE
    value_type hash_bits(value_type hash) {
        return (hash & kHashMask);
    }

    static JSTD_FORCED_INLINE
    std::size_t hash_bits64(std::size_t hash) {
        return (hash & static_cast<std::size_t>(kHashMask));
    }

    static JSTD_FORCED_INLINE
    value_type overflow_bits(value_type hash) {
        return (hash & kOverflowMask);
    }

    inline bool is_valid_hash(value_type hash) const {
        return (overflow_bits(hash) == 0);
    }

    JSTD_FORCED_INLINE value_type value() const {
        return this->value_;
    }

    JSTD_FORCED_INLINE value_type index() const {
        return 0;
    }

    JSTD_FORCED_INLINE value_type get_hash() const {
        return hash_bits(this->value_);
    }

    JSTD_FORCED_INLINE bool is_empty() const {
        value_type hash = hash_bits(this->value_);
        return (hash == kEmptySlot);
    }

    JSTD_FORCED_INLINE bool is_used() const {
        value_type hash = hash_bits(this->value_);
        return (hash != kEmptySlot);
    }

    JSTD_FORCED_INLINE bool is_overflow() const {
        value_type overflow = overflow_bits(this->value_);
        return (overflow != 0);
    }

    JSTD_FORCED_INLINE bool is_not_overflow() const {
        value_type overflow = overflow_bits(this->value_);
        return (overflow == 0);
    }

    JSTD_FORCED_INLINE bool is_equals(std::size_t hash) const {
        value_type hash8 = jstd::narrow_cast<value_type>(hash);
        assert(is_valid_hash(hash8));
        assert(hash_bits(hash8) != kEmptySlot);
        value_type value8 = hash_bits(this->value_);
        return (hash8 == value8);
    }

    JSTD_FORCED_INLINE void set_empty() {
        this->value_ = overflow_bits(this->value_) | kEmptySlot;
    }

    JSTD_FORCED_INLINE void set_used(std::size_t hash) {
        value_type hash8 = jstd::narrow_cast<value_type>(hash);
        assert(is_valid_hash(hash8));
        assert(hash_bits(hash8) != kEmptySlot);
        this->value_ = overflow_bits(this->value_) | hash8;
    }

    JSTD_FORCED_INLINE void set_used_strict(std::size_t hash) {
        value_type hash8 = jstd::narrow_cast<value_type>(hash_bits64(hash));
        assert(is_valid_hash(hash8));
        assert(hash_bits(hash8) != kEmptySlot);
        this->value_ = overflow_bits(this->value_) | hash8;
    }

    JSTD_FORCED_INLINE void set_overflow() {
        assert(hash_bits(this->value_) != kEmptySlot);
        this->value_ |= kOverflowMask;
    }

    JSTD_FORCED_INLINE void set_value(value_type value) {
        this->value_ = value;
    }

private:
    value_type value_;
};

template <typename T>
class JSTD_DLL flat_map_group16
{
public:
    typedef T                       ctrl_type;
    typedef typename T::value_type  value_type;
    typedef ctrl_type *             pointer;
    typedef const ctrl_type *       const_pointer;
    typedef ctrl_type &             reference;
    typedef const ctrl_type &       const_reference;

    static constexpr const value_type kHashMask     = ctrl_type::kHashMask;
    static constexpr const value_type kEmptySlot    = ctrl_type::kEmptySlot;
    static constexpr const value_type kOverflowMask = ctrl_type::kOverflowMask;

    static constexpr const std::uint32_t kHashMask32  = ctrl_type::kHashMask32;
    static constexpr const std::uint32_t kEmptySlot32 = ctrl_type::kEmptySlot32;

    static constexpr const std::size_t kGroupWidth = 16;
    static constexpr const bool kIsRegularLayout = true;

    static JSTD_FORCED_INLINE
    __m128i make_empty_bits() noexcept {
        if (kEmptySlot == 0b00000000)
            return _mm_setzero_si128();
        else if (kEmptySlot == 0b11111111)
            return _mm_setones_si128();
        else
            return _mm_set1_epi32((int)kEmptySlot32);
    }

    JSTD_FORCED_INLINE
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
            __m128i empty_bits = make_empty_bits();
            _mm_store_si128(reinterpret_cast<__m128i *>(ctrls), empty_bits);
        }
    }

    JSTD_FORCED_INLINE
    __m128i load_metadata() const {
        return _mm_load_si128(reinterpret_cast<const __m128i *>(ctrls));
    }

    JSTD_FORCED_INLINE value_type value(std::size_t pos) const {
        const ctrl_type & ctrl = at(pos);
        return ctrl.value();
    }

    JSTD_FORCED_INLINE bool is_empty(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_empty();
    }

    JSTD_FORCED_INLINE bool is_used(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_used();
    }

    JSTD_FORCED_INLINE bool is_overflow(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_overflow();
    }

    JSTD_FORCED_INLINE bool is_not_overflow(std::size_t pos) const {
        assert(pos < kGroupWidth);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_not_overflow();
    }

    JSTD_FORCED_INLINE bool is_equals(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupWidth);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_equals(hash);
    }

    JSTD_FORCED_INLINE void set_empty(std::size_t pos) {
        assert(pos < kGroupWidth);
        ctrl_type & ctrl = at(pos);
        ctrl.set_empty();
    }

    JSTD_FORCED_INLINE void set_used(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupWidth);
        ctrl_type & ctrl = at(pos);
        ctrl.set_used(hash);
    }

    JSTD_FORCED_INLINE void set_used_strict(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupWidth);
        ctrl_type & ctrl = at(pos);
        ctrl.set_used_strict(hash);
    }

    JSTD_FORCED_INLINE void set_overflow(std::size_t pos) {
        ctrl_type & ctrl = at(pos);
        ctrl.set_overflow();
    }

    static JSTD_FORCED_INLINE
    __m128i make_mask_bits() noexcept {
#if 1
        return _mm_set1_epi32((int)kHashMask32);
#else
        return _mm_set1_epi8((char)kHashMask);
#endif
    }

    static JSTD_FORCED_INLINE
    __m128i make_hash_bits(std::size_t hash) noexcept {
#if GROUP16_USE_LOOK_UP_TABLE
        // Use lookup table
        int hash32 = ctrl_type::repeated_hash(hash);
        __m128i hash_bits = _mm_set1_epi32(hash32);
#else
        __m128i hash_bits = _mm_set1_epi8(static_cast<char>(hash));
#endif
        return hash_bits;
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_empty(__m128i mask_bits) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();

        __m128i empty_bits;
        if (kEmptySlot == 0b00000000)
            empty_bits = _mm_setzero_si128();
        else if (kEmptySlot == 0b11111111)
            empty_bits = _mm_setones_si128();
        else if (kEmptySlot == kHashMask)
            empty_bits = mask_bits;
        else
            empty_bits = make_empty_bits();

        __m128i match_mask;;
        if ((kEmptySlot & kHashMask) == kEmptySlot)
            match_mask = _mm_cmpeq_epi8(empty_bits, _mm_and_si128(ctrl_bits, mask_bits));
        else
            match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits),
                                        _mm_and_si128(empty_bits, mask_bits));
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_empty() const noexcept {
        return this->match_empty(make_mask_bits());
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_used(__m128i mask_bits) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();

        __m128i empty_bits, match_mask;
        if (kEmptySlot == 0b00000000) {
            empty_bits = _mm_setzero_si128();
            match_mask = _mm_cmplt_epi8(empty_bits, _mm_and_si128(ctrl_bits, mask_bits));
        }
        else if (kEmptySlot == 0b11111111) {
            empty_bits = _mm_setones_si128();
            match_mask = _mm_cmplt_epi8(_mm_and_si128(ctrl_bits, mask_bits),
                                        _mm_and_si128(empty_bits, mask_bits));
        }
        else if (kEmptySlot == kHashMask) {
            empty_bits = mask_bits;
            match_mask = _mm_cmplt_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        }
        else if ((kEmptySlot & kHashMask) == kEmptySlot) {
            empty_bits = make_empty_bits();
            match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), empty_bits);
        }
        else {
            empty_bits = make_empty_bits();
            match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits),
                                        _mm_and_si128(empty_bits, mask_bits));
        }

        int mask = _mm_movemask_epi8(match_mask);
        if (kEmptySlot != 0b00000000 && kEmptySlot != 0b11111111 &&
            kEmptySlot != kHashMask) {
            mask = (~mask) & 0xFFFFU;
        }
        return static_cast<std::uint32_t>(mask);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_used() const noexcept {
        return this->match_used(make_mask_bits());
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(std::size_t hash, __m128i mask_bits) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();
        __m128i hash_bits = make_hash_bits(hash);
        //__COMPILER_BARRIER();

        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), hash_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(__m128i hash_bits, __m128i mask_bits) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();

        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), hash_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(std::size_t hash) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();
        __m128i mask_bits = make_mask_bits();
        __m128i hash_bits = make_hash_bits(hash);
        //__COMPILER_BARRIER();
        
        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), hash_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(__m128i hash_bits) const noexcept {
        // Latency = 6
        __m128i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();
        __m128i mask_bits = make_mask_bits();
        //__COMPILER_BARRIER();
        
        __m128i match_mask = _mm_cmpeq_epi8(_mm_and_si128(ctrl_bits, mask_bits), hash_bits);
        int mask = _mm_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask);
    }

private:
    JSTD_FORCED_INLINE ctrl_type & at(std::size_t pos) {
        return ctrls[pos];
    }

    JSTD_FORCED_INLINE const ctrl_type & at(std::size_t pos) const {
        return ctrls[pos];
    }

private:
    alignas(16) ctrl_type ctrls[kGroupWidth];
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_GROUP16_HPP
