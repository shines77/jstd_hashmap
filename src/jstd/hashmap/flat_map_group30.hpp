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

#ifndef JSTD_HASHMAP_FLAT_MAP_GROUP30_HPP
#define JSTD_HASHMAP_FLAT_MAP_GROUP30_HPP

#pragma once

#include <cstdint>
#include <cstddef>
#include <assert.h>

#include "jstd/basic/stddef.h"
#include "jstd/traits/type_traits.h"    // For jstd::narrow_cast<T>()
#include "jstd/support/BitVec.h"
#include "jstd/memory/memory_barrier.h"

#define GROUP30_USE_LOOK_UP_TABLE   1
#define GROUP30_USE_SHIFT_TABLE     1

namespace jstd {

class JSTD_DLL group30_meta_ctrl
{
public:
    using value_type = std::uint8_t;

    static constexpr const value_type kHashMask       = 0b11111111;
    static constexpr const value_type kEmptySlot      = 0b00000000;
    static constexpr const value_type kSentinelSlot   = 0b00000001;

    static constexpr const std::uint32_t kEmptySlot32    = 0x00000000u;

    static constexpr const value_type kEmptyHash         = 0x08;
    static constexpr const value_type kSentinelHash      = 0x09;
    static constexpr const std::uint32_t kEmptyHash32    = 0x08080808u;
    static constexpr const std::uint32_t kSentinelHash32 = 0x09090909u;

    group30_meta_ctrl(value_type value = kEmptySlot) : value_(value) {}

    static JSTD_FORCED_INLINE
    int repeated_hash(std::size_t hash) {
        static constexpr const std::uint32_t dword_hashs[] = {
            // [0, 127]
           kEmptyHash32, kSentinelHash32, 0x02020202u, 0x03030303u,
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

            // [128, 255]
            0x80808080u, 0x81818181u, 0x82828282u, 0x83838383u,
            0x84848484u, 0x85858585u, 0x86868686u, 0x87878787u,
            0x88888888u, 0x89898989u, 0x8A8A8A8Au, 0x8B8B8B8Bu,
            0x8C8C8C8Cu, 0x8D8D8D8Du, 0x8E8E8E8Eu, 0x8F8F8F8Fu,
            0x90909090u, 0x91919191u, 0x92929292u, 0x93939393u,
            0x94949494u, 0x95959595u, 0x96969696u, 0x97979797u,
            0x98989898u, 0x99999999u, 0x9A9A9A9Au, 0x9B9B9B9Bu,
            0x9C9C9C9Cu, 0x9D9D9D9Du, 0x9E9E9E9Eu, 0x9F9F9F9Fu,
            0xA0A0A0A0u, 0xA1A1A1A1u, 0xA2A2A2A2u, 0xA3A3A3A3u,
            0xA4A4A4A4u, 0xA5A5A5A5u, 0xA6A6A6A6u, 0xA7A7A7A7u,
            0xA8A8A8A8u, 0xA9A9A9A9u, 0xAAAAAAAAu, 0xABABABABu,
            0xACACACACu, 0xADADADADu, 0xAEAEAEAEu, 0xAFAFAFAFu,
            0xB0B0B0B0u, 0xB1B1B1B1u, 0xB2B2B2B2u, 0xB3B3B3B3u,
            0xB4B4B4B4u, 0xB5B5B5B5u, 0xB6B6B6B6u, 0xB7B7B7B7u,
            0xB8B8B8B8u, 0xB9B9B9B9u, 0xBABABABAu, 0xBBBBBBBBu,
            0xBCBCBCBCu, 0xBDBDBDBDu, 0xBEBEBEBEu, 0xBFBFBFBFu,
            0xC0C0C0C0u, 0xC1C1C1C1u, 0xC2C2C2C2u, 0xC3C3C3C3u,
            0xC4C4C4C4u, 0xC5C5C5C5u, 0xC6C6C6C6u, 0xC7C7C7C7u,
            0xC8C8C8C8u, 0xC9C9C9C9u, 0xCACACACAu, 0xCBCBCBCBu,
            0xCCCCCCCCu, 0xCDCDCDCDu, 0xCECECECEu, 0xCFCFCFCFu,
            0xD0D0D0D0u, 0xD1D1D1D1u, 0xD2D2D2D2u, 0xD3D3D3D3u,
            0xD4D4D4D4u, 0xD5D5D5D5u, 0xD6D6D6D6u, 0xD7D7D7D7u,
            0xD8D8D8D8u, 0xD9D9D9D9u, 0xDADADADAu, 0xDBDBDBDBu,
            0xDCDCDCDCu, 0xDDDDDDDDu, 0xDEDEDEDEu, 0xDFDFDFDFu,
            0xE0E0E0E0u, 0xE1E1E1E1u, 0xE2E2E2E2u, 0xE3E3E3E3u,
            0xE4E4E4E4u, 0xE5E5E5E5u, 0xE6E6E6E6u, 0xE7E7E7E7u,
            0xE8E8E8E8u, 0xE9E9E9E9u, 0xEAEAEAEAu, 0xEBEBEBEBu,
            0xECECECECu, 0xEDEDEDEDu, 0xEEEEEEEEu, 0xEFEFEFEFu,
            0xF0F0F0F0u, 0xF1F1F1F1u, 0xF2F2F2F2u, 0xF3F3F3F3u,
            0xF4F4F4F4u, 0xF5F5F5F5u, 0xF6F6F6F6u, 0xF7F7F7F7u,
            0xF8F8F8F8u, 0xF9F9F9F9u, 0xFAFAFAFAu, 0xFBFBFBFBu,
            0xFCFCFCFCu, 0xFDFDFDFDu, 0xFEFEFEFEu, 0xFFFFFFFFu,
        };

        return (int)dword_hashs[jstd::narrow_cast<std::uint8_t>(hash)];
    }

    static JSTD_FORCED_INLINE
    std::uint8_t reduced_hash(std::size_t hash) {
        return jstd::narrow_cast<std::uint8_t>(repeated_hash(hash));
    }

    static JSTD_FORCED_INLINE
    std::size_t hash_bits64(std::size_t hash) {
        return (hash & static_cast<std::size_t>(kHashMask));
    }

    JSTD_FORCED_INLINE value_type value() const {
        return this->value_;
    }

    JSTD_FORCED_INLINE value_type index() const {
        return 0;
    }

    JSTD_FORCED_INLINE value_type get_hash() const {
        return this->value_;
    }

    JSTD_FORCED_INLINE bool is_empty() const {
        value_type hash = this->value_;
        return (hash == kEmptySlot);
    }

    JSTD_FORCED_INLINE bool is_sentinel() const {
        value_type hash = this->value_;
        return (hash == kSentinelSlot);
    }

    JSTD_FORCED_INLINE bool is_used() const {
        value_type hash = this->value_;
        return (hash != kEmptySlot);
    }

    JSTD_FORCED_INLINE bool is_valid() const {
        value_type hash = this->value_;
        return (hash > kSentinelSlot);
    }

    JSTD_FORCED_INLINE bool is_equals(std::size_t hash) const {
        value_type hash8 = static_cast<value_type>(hash);
        return (this->value_ == hash8);
    }

    JSTD_FORCED_INLINE bool is_equals64(std::size_t hash) const {
        std::size_t hash64 = static_cast<std::size_t>(this->value_);
        return (hash == hash64);
    }

    JSTD_FORCED_INLINE void set_empty() {
        this->value_ = kEmptySlot;
    }

    JSTD_FORCED_INLINE void set_sentinel() {
        this->value_ = kSentinelSlot;
    }

    JSTD_FORCED_INLINE void set_used(std::size_t hash) {
        assert(hash > kSentinelSlot);
        this->value_ = static_cast<value_type>(hash);
    }

    JSTD_FORCED_INLINE void set_used64(std::size_t hash) {
        value_type hash8 = static_cast<value_type>(hash_bits64(hash));
        assert(hash8 > kSentinelSlot);
        this->value_ = hash8;
    }

    JSTD_FORCED_INLINE void set_value(value_type value) {
        this->value_ = value;
    }

private:
    value_type value_;
};

template <typename T>
class JSTD_DLL flat_map_group30
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
    static constexpr const value_type kSentinelSlot = ctrl_type::kSentinelSlot;

    static constexpr const std::uint32_t kEmptySlot32 = ctrl_type::kEmptySlot32;

    static constexpr const std::size_t kGroupWidth = 32;
    static constexpr const std::size_t kGroupSize = kGroupWidth - 2;
    static constexpr const bool kIsRegularLayout = true;

    using mask_type = std::uint16_t;

    static constexpr mask_type shift_mask[] = {
        1, 2, 4, 8,
        16, 32, 64, 128,
        256, 512, 1024, 2048,
        4096, 8192, 16384, 32768
    };

    static constexpr const std::size_t kOverflowBits = CHAR_BIT * sizeof(mask_type);

    static JSTD_FORCED_INLINE
    __m256i make_empty_bits() noexcept {
        if (kEmptySlot == 0b00000000)
            return _mm256_setzero_si256();
        else if (kEmptySlot == 0b11111111)
            return _mm256_setones_si256();
        else
            return _mm256_set1_epi32((int)kEmptySlot32);
    }

    JSTD_FORCED_INLINE
    void init() {
        if (kEmptySlot == 0b00000000) {
            __m256i zeros = _mm256_setzero_si256();
            _mm256_store_si256(reinterpret_cast<__m256i *>(ctrls), zeros);
        }
        else if (kEmptySlot == 0b11111111) {
            __m256i ones = _mm256_setones_si256();
            _mm256_store_si256(reinterpret_cast<__m256i *>(ctrls), ones);
            clear_overflow();
        }
        else {
            __m256i empty_bits = make_empty_bits();
            _mm256_store_si256(reinterpret_cast<__m256i *>(ctrls), empty_bits);
            clear_overflow();
        }
    }

    JSTD_FORCED_INLINE
    __m256i load_metadata() const {
#if defined(JSTD_THREAD_SANITIZER)
        /*
         * ThreadSanitizer complains on 1-byte atomic writes combined with
         * 32-byte atomic reads.
         */
        return _mm256_set_epi8(
            (char)ctrls[31], (char)ctrls[30], (char)ctrls[29], (char)ctrls[28],
            (char)ctrls[27], (char)ctrls[26], (char)ctrls[25], (char)ctrls[24],
            (char)ctrls[23], (char)ctrls[22], (char)ctrls[21], (char)ctrls[20],
            (char)ctrls[19], (char)ctrls[18], (char)ctrls[17], (char)ctrls[16]
            (char)ctrls[15], (char)ctrls[14], (char)ctrls[13], (char)ctrls[12],
            (char)ctrls[11], (char)ctrls[10], (char)ctrls[9],  (char)ctrls[8],
            (char)ctrls[7],  (char)ctrls[6],  (char)ctrls[5],  (char)ctrls[4],
            (char)ctrls[3],  (char)ctrls[2],  (char)ctrls[1],  (char)ctrls[0]
        );
#else
        return _mm256_load_si256(reinterpret_cast<const __m256i *>(ctrls));
#endif
    }

    JSTD_FORCED_INLINE value_type value(std::size_t pos) const {
        const ctrl_type & ctrl = at(pos);
        return ctrl.value();
    }

    JSTD_FORCED_INLINE bool is_empty(std::size_t pos) const {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_empty();
    }

    JSTD_FORCED_INLINE bool is_sentinel(std::size_t pos) const {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_sentinel();
    }

    JSTD_FORCED_INLINE bool is_used(std::size_t pos) const {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_used();
    }

    JSTD_FORCED_INLINE bool is_valid(std::size_t pos) const {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_valid();
    }

    JSTD_FORCED_INLINE bool is_equals(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_equals(hash);
    }

    JSTD_FORCED_INLINE bool is_equals64(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupSize);
        const ctrl_type & ctrl = at(pos);
        return ctrl.is_equals64(hash);
    }

    JSTD_FORCED_INLINE void set_empty(std::size_t pos) {
        assert(pos < kGroupSize);
        ctrl_type & ctrl = at(pos);
        ctrl.set_empty();
    }

    JSTD_FORCED_INLINE void set_sentinel() {
        ctrl_type & ctrl = at(kGroupSize - 1);
        ctrl.set_sentinel();
    }

    JSTD_FORCED_INLINE void set_used(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupSize);
        ctrl_type & ctrl = at(pos);
        ctrl.set_used(hash);
    }

    JSTD_FORCED_INLINE void set_used64(std::size_t pos, std::size_t hash) {
        assert(pos < kGroupSize);
        ctrl_type & ctrl = at(pos);
        ctrl.set_used64(hash);
    }

    JSTD_FORCED_INLINE bool is_overflow(std::size_t hash) const {
        return !this->is_not_overflow(hash);
    }

    JSTD_FORCED_INLINE bool is_not_overflow(std::size_t hash) const {
        std::size_t pos = hash % kOverflowBits;
        JSTD_ASSUME(pos < kOverflowBits);
#if GROUP30_USE_SHIFT_TABLE
        mask_type mask = shift_mask[pos];
        const mask_type & overflow = overflow_masks();
        return ((overflow & mask) == 0);
#else
        std::size_t mask = std::size_t(1) << pos;
        JSTD_ASSUME(mask < (std::size_t(1) << kOverflowBits));
        const mask_type & overflow = overflow_masks();
        return ((overflow & jstd::narrow_cast<mask_type>(mask)) == 0);
        // return ((static_cast<std::size_t>(overflow) & mask) == 0);
#endif
    }

    JSTD_FORCED_INLINE bool has_any_overflow() const {
        const mask_type & mask = overflow_masks();
        return (mask != 0);
    }

    JSTD_FORCED_INLINE void set_overflow(std::size_t hash) {
        std::size_t pos = hash % kOverflowBits;
        JSTD_ASSUME(pos < kOverflowBits);
#if GROUP30_USE_SHIFT_TABLE
        mask_type mask = shift_mask[pos];
        mask_type & overflow = overflow_masks();
        overflow |= mask;
#else
        std::uint32_t mask = 1 << static_cast<std::uint32_t>(pos);
        JSTD_ASSUME(mask < (1 << kOverflowBits));
        mask_type & overflow = overflow_masks();
        overflow |= jstd::narrow_cast<mask_type>(mask);
#endif
    }

    JSTD_FORCED_INLINE void clear_overflow() {
        mask_type & overflow = overflow_masks();
        overflow = 0;
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_empty() const noexcept {
        // Latency = 6
        __m256i ctrl_bits = load_metadata();
        //__COMPILER_BARRIER();

        __m256i empty_bits;
        if (kEmptySlot == 0b00000000)
            empty_bits = _mm256_setzero_si256();
        else if (kEmptySlot == 0b11111111)
            empty_bits = _mm256_setones_si256();
        else
            empty_bits = make_empty_bits();

        __m256i match_mask = _mm256_cmpeq_epi8(empty_bits, ctrl_bits);
        int mask = _mm256_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask & 0x3FFFFFFFU);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_used() const noexcept {
        std::uint32_t mask = this->match_empty();
        return static_cast<std::uint32_t>((~mask) & 0x3FFFFFFFU);
    }

    static JSTD_FORCED_INLINE
    __m256i make_hash_bits(std::size_t hash) noexcept {
#if GROUP30_USE_LOOK_UP_TABLE
        // Use lookup table
        int hash32 = ctrl_type::repeated_hash(hash);
        __m256i hash_bits = _mm256_set1_epi32(hash32);
#else
        __m256i hash_bits = _mm256_set1_epi8(static_cast<char>(hash));
#endif
        return hash_bits;
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(std::size_t hash) const noexcept {
        // Latency = 6
        __m256i ctrl_bits  = load_metadata();
        //__COMPILER_BARRIER();
        __m256i hash_bits = make_hash_bits(hash);
        //__COMPILER_BARRIER();

        __m256i match_mask = _mm256_cmpeq_epi8(ctrl_bits, hash_bits);
        int mask = _mm256_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask & 0x3FFFFFFFU);
    }

    JSTD_FORCED_INLINE
    std::uint32_t match_hash(const __m256i & hash_bits) const noexcept {
        // Latency = 6
        __m256i ctrl_bits  = load_metadata();
        //__COMPILER_BARRIER();

        __m256i match_mask = _mm256_cmpeq_epi8(hash_bits, ctrl_bits);
        int mask = _mm256_movemask_epi8(match_mask);
        return static_cast<std::uint32_t>(mask & 0x3FFFFFFFU);
    }

private:
    JSTD_FORCED_INLINE ctrl_type & at(std::size_t pos) {
        return ctrls[pos];
    }

    JSTD_FORCED_INLINE const ctrl_type & at(std::size_t pos) const {
        return ctrls[pos];
    }

    JSTD_FORCED_INLINE mask_type & overflow_masks() {
        ctrl_type * mask_ctrl = &ctrls[kGroupSize];
        return *reinterpret_cast<mask_type *>(mask_ctrl);
    }

    JSTD_FORCED_INLINE const mask_type & overflow_masks() const {
        ctrl_type * mask_ctrl = &ctrls[kGroupSize];
        return *reinterpret_cast<const mask_type *>(mask_ctrl);
    }

private:
    alignas(32) ctrl_type ctrls[kGroupWidth];
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_GROUP30_HPP
