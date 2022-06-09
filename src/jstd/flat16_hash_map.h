/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2022 XiongHui Guo (gz_shines@msn.com)

  https://github.com/shines77/jstd_hash_map
  https://gitee.com/shines77/jstd_hash_map

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
#include <memory>       // For std::swap(), std::pointer_traits<T>
#include <limits>       // For std::numeric_limits<T>
#include <cstring>      // For std::memset()
#include <vector>
#include <type_traits>
#include <utility>
#include <algorithm>    // For std::max(), std::min()

#include <nmmintrin.h>
#include <immintrin.h>

#include "jstd/support/BitUtils.h"

#define FLAT16_DEFAULT_LOAD_FACTOR  (0.5)

#ifndef __SSE2__
#define __SSE2__
#endif

namespace jstd {

static inline
std::size_t round_size(std::size_t size, std::size_t alignment)
{
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0);
    size = (size + alignment - 1) / alignment * alignment;
    assert((size / alignment * alignment) == size);
    return size;
}

static inline
std::size_t align_to(std::size_t size, std::size_t alignment)
{
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0);
    size = (size + alignment - 1) & ~(alignment - 1);
    assert((size / alignment * alignment) == size);
    return size;
}

static inline
std::size_t round_up_pow2(std::size_t n)
{
    assert(n > 1);
    if ((n & (n - 1)) == 0)
        return n;
    return n;
}

template <std::uint8_t Value>
struct repeat_8to64 {
    static constexpr std::uint64_t r16 = (std::uint64_t)Value | ((std::uint64_t)Value << 8);
    static constexpr std::uint64_t r32 = r16 | (r16 << 16);
    static constexpr std::uint64_t value = r32 | (r32 << 16);
};

template < typename Key, typename Value,
           typename Hasher = std::hash<Key>,
           typename KeyEqual = std::equal_to<Key>,
           typename Allocator = std::allocator<std::pair<const Key, Value>> >
class flat16_hash_map {
public:
    typedef Key                             key_type;
    typedef Value                           mapped_type;
    typedef std::pair<const Key, Value>     value_type;
    typedef std::pair<Key, Value>           n_value_type;

    typedef Hasher                          hasher_type;
    typedef KeyEqual                        key_equal;
    typedef Allocator                       allocator_type;
    typedef typename Hasher::result_type    hash_result_t;

    typedef std::size_t                     size_type;
    typedef typename std::make_signed<size_type>::type
                                            ssize_type;
    typedef std::size_t                     index_type;
    typedef std::size_t                     hash_code_t;
    typedef flat16_hash_map<Key, Value, Hasher, KeyEqual, Allocator>
                                            this_type;

    static constexpr std::uint8_t kEmptyEntry   = 0b11111111;
    static constexpr std::uint8_t kDeletedEntry = 0b10000000;
    static constexpr std::uint8_t kUnusedMask   = 0b10000000;
    static constexpr std::uint8_t kHash2Mask    = 0b01111111;

    static constexpr std::uint64_t kEmptyEntry64   = 0xFFFFFFFFFFFFFFFFull;
    static constexpr std::uint64_t kDeletedEntry64 = 0x8080808080808080ull;
    static constexpr std::uint64_t kUnusedMask64   = 0x8080808080808080ull;

    static constexpr size_type kControlHashMask = 0x0000007Ful;
    static constexpr size_type kControlShift    = 7;

    static constexpr size_type kClusterBits     = 4;
    static constexpr size_type kClusterEntries  = size_type(1) << kClusterBits;
    static constexpr size_type kClusterMask     = kClusterEntries - 1;
    static constexpr size_type kClusterShift    = kControlShift + kClusterBits;

    static constexpr size_type kDefaultInitialCapacity = kClusterEntries;
    static constexpr size_type kMinimumCapacity = kClusterEntries;

    static constexpr float kDefaultLoadFactor = 0.5;
    static constexpr float kMaxLoadFactor = 1.0;

    struct bitmask128_t {
        std::uint64_t low;
        std::uint64_t high;

        bitmask128_t() noexcept : low(0), high(0) {}
    };

    struct control_byte {
        std::uint8_t value;

        bool isEmpty() const {
            return (this->value = kEmptyEntry);
        }

        bool isDeleted() const {
            return (this->value = kDeletedEntry);
        }

        bool isUsed() const {
            return ((std::int8_t)this->value >= 0);
        }

        bool isEmptyOrDeleted() const {
            return ((std::int8_t)this->value < 0);
        }
    };

    struct alignas(16) cluster {
        union {
            control_byte controls[kClusterEntries];
            bitmask128_t u128;
        };

        cluster() noexcept {
#if defined(__SSE2__)
            __m128i empty_bits = _mm_set1_epi8(kEmptyEntry);
            _mm_store_si128((__m128i *)&controls[0], empty_bits);
#else
            std::memset((void *)&controls[0], kEmptyEntry, kClusterEntries * sizeof(std::uint8_t));
#endif // __SSE2__
        }

        ~cluster() {
        }

        std::uint32_t getMatchMask(std::uint8_t control_tag) const {
#if defined(__SSE2__)
            __m128i tag_bits = _mm_set1_epi8(control_tag);
            __m128i control_bits = _mm_load_si128((const __m128i *)&this->controls[0]);
            __m128i match_mask = _mm_cmpeq_epi8(control_bits, tag_bits);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(match_mask);
            return mask;
#else
            std::uint32_t mask = 0, bit = 1;
            for (size_type i = 0; i < kClusterEntries; i++) {
                if (this->controls[i].value == control_tag) {
                    mask |= bit;
                }
                bit <<= 1;
            }
            return mask;
#endif // __SSE2__
        }

#if defined(__SSE2__)
        __m128i getMatchMask128(std::uint8_t control_tag) const {
            __m128i tag_bits = _mm_set1_epi8(control_tag);
            __m128i control_bits = _mm_load_si128((const __m128i *)&this->controls[0]);
            __m128i match_mask = _mm_cmpeq_epi8(control_bits, tag_bits);
            return match_mask;
        }
#else
        bitmask128_t getMatchMask128(std::uint8_t control_tag) const {
            std::uint64_t control_tag64;
            control_tag64 = (std::uint64_t)control_tag | ((std::uint64_t)control_tag << 8);
            control_tag64 = control_tag64 | (control_tag64 << 16);
            control_tag64 = control_tag64 | (control_tag64 << 32);

            bitmask128_t mask128;
            mask128.low  = this->u128.low  ^ control_tag64;
            mask128.high = this->u128.high ^ control_tag64;
            return mask128;
        }
#endif // __SSE2__

        std::uint32_t matchHash(std::uint8_t control_hash) const {
            return this->getMatchMask(control_hash);
        }

        std::uint32_t matchEmpty() const {
            return this->getMatchMask(kEmptyEntry);
        }

        std::uint32_t matchDeleted() const {
            return this->getMatchMask(kDeletedEntry);
        }

        bool hasAnyMatch(std::uint8_t control_hash) const {
            return (this->matchHash(control_hash) != 0);
        }

        bool hasAnyEmpty() const {
            return (this->matchEmpty() != 0);
        }

        bool hasAnyDeleted() const {
            return (this->matchDeleted() != 0);
        }

        std::uint32_t matchEmptyOrDeleted() const {
#if defined(__SSE2__)
  #if 1
            __m128i control_bits = _mm_load_si128((const __m128i *)&this->controls[0]);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(control_bits);
            return mask;
  #else
            __m128i zero_bits = _mm_set1_epi8(0);
            __m128i control_bits = _mm_load_si128((const __m128i *)&this->controls[0]);
            __m128i compare_mask = _mm_cmplt_epi8(control_bits, zero_bits);
            std::uint32_t mask = (std::uint32_t)_mm_movemask_epi8(compare_mask);
            return mask;
  #endif
#else
            std::uint32_t mask = 0, bit = 1;
            for (size_type i = 0; i < kClusterEntries; i++) {
                if ((this->controls[i].value & kUnusedMask) != 0) {
                    mask |= bit;
                }
                bit <<= 1;
            }
            return mask;
#endif // __SSE2__       
        }

        bool hasAnyEmptyOrDeleted() const {
            return (this->matchEmptyOrDeleted() != 0);
        }

        bool isUsed() const {
            return (this->matchEmptyOrDeleted() == 0);
        }
    };

    typedef cluster cluster_type;

    struct hash_entry {
        value_type value;
    };

    typedef hash_entry entry_type;

    template <typename ValueType>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using value_type = ValueType;
        using pointer = ValueType *;
        using reference = ValueType &;

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
            return !(lhs == rhs);
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
            return this->entry_->value;
        }

        pointer operator -> () const {
            return std::addressof(this->entry_->value);
        }

        operator basic_iterator<const value_type>() const {
            return { this->entry_ };
        }
    };

    using iterator       = basic_iterator<value_type>;
    using const_iterator = basic_iterator<const value_type>;

private:
    cluster_type *  clusters_;
    size_type       cluster_mask_;

    entry_type *    entries_;
    size_type       entry_size_;
    size_type       entry_mask_;

    size_type       entry_threshold_;
    double          load_factor_;

    hasher_type     hasher_;
    key_equal       key_equal_;

public:
    explicit flat16_hash_map(size_type initialCapacity = kDefaultInitialCapacity) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_((double)kDefaultLoadFactor) {
        init_cluster(initialCapacity);
    }

    ~flat16_hash_map() {
        this->destroy();
    }

    void destroy() {
        if (clusters_ != nullptr) {
            delete[] clusters_;
            clusters_ = nullptr;
        }
        if (entries_ != nullptr) {
            delete[] entries_;
            entries_ = nullptr;
        }
    }

    bool is_valid() const { return (this->clusters() != nullptr); }
    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const  { return (this->size() == this->entry_capacity()); }

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

    double load_factor() const {
        return load_factor_;
    }

    double max_load_factor() const {
        return static_cast<double>(kMaxLoadFactor);
    }

    iterator iterator_at(size_type index) {
        return { (this->controls() + index), (this->entries() + index) };
    }

    const_iterator iterator_at(size_type index) const {
        return { (this->controls() + index), (this->entries() + index) };
    }

    iterator begin() {
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_mask(); index++) {
            if (control->isUsed()) {
                return { control, (this->entries() + index) };
            }
            control++;
        }
        return { control, (this->entries() + index) };
    }

    const_iterator begin() const {
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_mask(); index++) {
            if (control->isUsed()) {
                return { control, (this->entries() + index) };
            }
            control++;
        }
        return { control, (this->entries() + index) };
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

    iterator find(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t control_hash = this->get_control_hash(hash_code);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_index = cluster_index;
        do {
            cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type entry_start = cluster_index * kClusterEntries;
            while (mask16 != 0) {
                std::uint32_t bit = BitUtils::ls1b32(mask16);
                size_type pos = BitUtils::bsr32(bit);
                const key_type & target_key = this->get_entry(entry_start + pos).value.first;
                if (this->key_equal_(target_key, key)) {
                    return iterator_at(entry_start + pos);
                }
            }
            if (cluster.hasAnyEmpty()) {
                return this->end();
            }
            cluster_index = (cluster_index + 1) & cluster_mask_;
        } while (cluster_index != start_index);

        return this->end();
    }

    std::pair<iterator, bool> insert(const value_type & value) {
        return this->emplace(value);
    }

    std::pair<iterator, bool> insert(value_type && value) {
        return this->emplace(std::forward<value_type>(value));
    }

    std::pair<iterator, bool> emplace(const value_type & value) {
        iterator iter = this->find(value.first);
        if (iter == this->end()) {
            //
        }
        return { iter, false };
    }

    std::pair<iterator, bool> emplace(value_type && value) {
        iterator iter = this->find(value.first);
        if (iter == this->end()) {
            //
        }
        return { iter, false };
    }

private:
    inline size_type calc_capacity(size_type capacity) const {
        size_type new_capacity = (capacity >= kMinimumCapacity) ? capacity : kMinimumCapacity;
        new_capacity = round_up_pow2(new_capacity);
        return new_capacity;
    }

    inline hash_code_t get_hash(const key_type & key) const {
        hash_code_t hash_code = static_cast<hash_code_t>(this->hasher_(key));
        return hash_code;
    }

    inline std::uint8_t get_control_hash(hash_code_t hash_code) const {
        return static_cast<std::uint8_t>(hash_code & kControlHashMask);
    }

    inline index_type index_for(hash_code_t hash_code) const {
        return (index_type)(((size_type)hash_code >> kControlShift) & cluster_mask_);
    }

    inline index_type index_for(hash_code_t hash_code, size_type cluster_mask) const {
        return (index_type)(((size_type)hash_code >> kControlShift) & cluster_mask);
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

    index_type index_of(entry_type * entry) const {
        assert(entry >= this->entries_);
        index_type index = (index_type)(entry - this->entries());
        return index;
    }

    bool cluster_has_value(size_type index) const {
        std::uint8_t * controls = (std::uint8_t *)this->clusters();
        return ((controls[index] & kUnusedMask) == 0);
    }

    bool has_value(entry_type * entry) const {
        index_type entry_index = this->index_of(entry);
        return this->cluster_has_value(entry_index);
    }

    void init_cluster(size_type init_capacity) {
        size_type new_capacity = align_to(init_capacity, kClusterEntries);
        assert(new_capacity > 1);
        assert(new_capacity >= kMinimumCapacity);

        size_type cluster_count = new_capacity / kClusterEntries;
        assert(cluster_count > 0);
        cluster_type * clusters = new cluster_type[cluster_count];
        clusters_ = clusters;
        cluster_mask_ = cluster_count - 1;

        entry_type * entries = new entry_type[new_capacity];
        entries_ = entries;
        assert(entry_size_ == 0);
        entry_mask_ = new_capacity - 1;
        entry_threshold_ = (size_type)(new_capacity * FLAT16_DEFAULT_LOAD_FACTOR);
    }

    cluster_type * create_cluster(size_type cluster_count) {
        cluster_type * clusters = new cluster_type[cluster_count];
        return clusters;
    }

};

} // namespace jstd
