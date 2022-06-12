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
#include <utility>      // For std::pair<First, Second>, std::integer_sequence<T...>
#include <tuple>        // For std::tuple<Ts...>
#include <algorithm>    // For std::max(), std::min()
#include <type_traits>

#include <nmmintrin.h>
#include <immintrin.h>

#include "jstd/type_traits.h"
#include "jstd/utility.h"
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

namespace hash {

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

struct IntegalHash
{
    std::uint32_t operator () (std::uint32_t value) const {
        //std::uint32_t hash = value * 16777619ul ^ 2166136261ul;
        std::uint32_t hash = value * 2654435761ul + 16777619ul;
        return hash;
    }

    std::uint64_t operator () (std::uint64_t value) const {
        //std::uint64_t hash = value * 1099511628211ull ^ 14695981039346656037ull;
        std::uint64_t hash = value * 14695981039346656037ull + 1099511628211ull;
        return hash;
    }
};

} // namespace hash

template <std::uint8_t Value>
struct repeat_u8x4 {
    static constexpr std::uint32_t value = (std::uint32_t)Value * 0x01010101ul;
};

template <std::uint8_t Value>
struct repeat_u8x8 {
    static constexpr std::uint64_t value = (std::uint64_t)Value * 0x0101010101010101ull;
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
    typedef std::pair<Key, Value>           nc_value_type;

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

    static constexpr size_type npos = size_type(-1);

    static constexpr std::uint8_t kEmptyEntry   = 0b11111111;
    static constexpr std::uint8_t kDeletedEntry = 0b10000000;
    static constexpr std::uint8_t kUnusedMask   = 0b10000000;
    static constexpr std::uint8_t kHash2Mask    = 0b01111111;

    static constexpr std::uint64_t kEmptyEntry64   = 0xFFFFFFFFFFFFFFFFull;
    static constexpr std::uint64_t kDeletedEntry64 = 0x8080808080808080ull;
    static constexpr std::uint64_t kUnusedMask64   = 0x8080808080808080ull;

    static constexpr std::uint32_t kFullMask16  = 0x0000FFFFul;
    static constexpr size_type kControlHashMask = 0x0000007Ful;
    static constexpr size_type kControlShift    = 7;

    static constexpr size_type kClusterBits     = 4;
    static constexpr size_type kClusterEntries  = size_type(1) << kClusterBits;
    static constexpr size_type kClusterMask     = kClusterEntries - 1;
    static constexpr size_type kClusterShift    = kControlShift + kClusterBits;

    // Must be kDefaultCapacity >= kMinimumCapacity
    static constexpr size_type kMinimumCapacity = 4;
    static constexpr size_type kDefaultCapacity = 4;

    static constexpr float kDefaultLoadFactor = 0.5;
    static constexpr float kMaxLoadFactor = 1.0;

    struct bitmask128_t {
        std::uint64_t low;
        std::uint64_t high;

        bitmask128_t() noexcept : low(0), high(0) {}
    };

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

        bool isUsed() const {
            return ((std::int8_t)this->value >= 0);
        }

        bool isEmptyOrDeleted() const {
            return ((std::int8_t)this->value < 0);
        }

        std::uint8_t getValue() const {
            return this->value;
        }

        void setEmpty() {
            this->value = kEmptyEntry;
        }

        void setDeleted() {
            this->value = kDeletedEntry;
        }

        void setUsed(std::int8_t control_hash) {
            assert((control_hash & kUnusedMask) == 0);
            this->value = control_hash;
        }
    };

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

    typedef value_type entry_type;
    typedef std::allocator<entry_type> entry_allocator_type;

    struct alignas(16) hash_cluster {
        union {
            control_byte controls[kClusterEntries];
            bitmask128_t u128;
        };

        hash_cluster() noexcept {
            this->clear();
        }

        ~hash_cluster() {
        }

        void clear() {
#if defined(__SSE2__)
            __m128i empty_bits = _mm_set1_epi8(kEmptyEntry);
            _mm_store_si128((__m128i *)&controls[0], empty_bits);
#else
            std::memset((void *)&controls[0], kEmptyEntry, kClusterEntries * sizeof(std::uint8_t));
#endif // __SSE2__
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

        bool isFull() const {
            return (this->matchEmptyOrDeleted() == 0);
        }

        bool isAllEmpty() const {
            return (this->matchEmpty() == kFullMask16);
        }
    };

    typedef hash_cluster cluster_type;

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
            return *(this->entry_);
        }

        pointer operator -> () const {
            return std::addressof(*this->entry_);
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

    allocator_type          value_allocator_;
    entry_allocator_type    entry_allocator_;

public:
    explicit flat16_hash_map(size_type initialCapacity = kDefaultCapacity) :
        clusters_(nullptr), cluster_mask_(0),
        entries_(nullptr), entry_size_(0), entry_mask_(0),
        entry_threshold_(0), load_factor_((double)kDefaultLoadFactor) {
        this->create_cluster<true>(initialCapacity);
    }

    ~flat16_hash_map() {
        this->destroy<true>();
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

    iterator begin() {
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_mask(); index++) {
            if (control->isUsed()) {
                return { control, this->entry_at(index) };
            }
            control++;
        }
        return { control, this->entry_at(index) };
    }

    const_iterator begin() const {
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_mask(); index++) {
            if (control->isUsed()) {
                return { control, this->entry_at(index) };
            }
            control++;
        }
        return { control, this->entry_at(index) };
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

    void clear(bool need_destory = true) noexcept {
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

    iterator find(const key_type & key) {
        size_type index = this->find_impl(key);
        if (index != npos)
            return this->iterator_at(index);
        else
            return this->end();
    }

    const_iterator find(const key_type & key) const {
        size_type index = this->find_impl(key);
        if (index != npos)
            return this->iterator_at(index);
        else
            return this->end();
    }

    std::pair<iterator, bool> insert(const value_type & value) {
        return this->emplace_impl<true>(value);
    }

    std::pair<iterator, bool> insert(value_type && value) {
        return this->emplace_impl<true>(std::move(value));
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value) &&
              (!jstd::is_same_ex<P, nc_value_type>::value) &&
              std::is_constructible<value_type, P &&>::value>::type * = nullptr>
    std::pair<iterator, bool> insert(P && value) {
        return this->emplace_impl<true>(std::forward<P>(value));
    }

    iterator insert(const_iterator hint, const value_type & value) {
        return this->emplace_impl<true>(value).first;
    }

    iterator insert(const_iterator hint, value_type && value) {
        return this->emplace_impl<true>(std::move(value)).first;
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value) &&
              (!jstd::is_same_ex<P, nc_value_type>::value) &&
              std::is_constructible<value_type, P &&>::value>::type * = nullptr>
    std::pair<iterator, bool> insert(const_iterator hint, P && value) {
        return this->emplace_impl<true>(std::forward<P>(value));
    }

    template <typename ... Args>
    std::pair<iterator, bool> emplace(Args && ... args) {
        return this->emplace_impl<false>(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    iterator emplace_hint(const_iterator hint, Args && ... args) {
        return this->emplace(std::forward<Args>(args)...).first;
    }

    size_type erase(const key_type & key) {
        size_type deleted = this->find_and_erase(key);
        return deleted;
    }

    size_type erase_at(size_type index) {
        return 1;
    }

    inline void grow() {
        size_type new_capacity = (this->entry_mask_ + 1) * 2;
        this->rehash(new_capacity);
    }

    void rehash(size_type new_capacity) {
        //
    }

private:
    inline size_type calc_capacity(size_type capacity) const noexcept {
        size_type new_capacity = (std::max)(capacity, kMinimumCapacity);
        new_capacity = round_up_pow2(new_capacity);
        return new_capacity;
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

    inline std::uint8_t get_control_hash(hash_code_t hash_code) const noexcept {
        return static_cast<std::uint8_t>(hash_code & kControlHashMask);
    }

    inline index_type index_for(hash_code_t hash_code) const noexcept {
        return (index_type)(((size_type)hash_code >> kControlShift) & this->cluster_mask());
    }

    inline index_type index_for(hash_code_t hash_code, size_type cluster_mask) const noexcept {
        return (index_type)(((size_type)hash_code >> kControlShift) & cluster_mask);
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

    index_type index_of(entry_type * entry) const noexcept {
        assert(entry >= this->entries_);
        index_type index = (index_type)(entry - this->entries());
        return index;
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
        if (finitial) {
            if (this->clusters_ != nullptr) {
                delete[] this->clusters_;
                this->clusters_ = nullptr;
            }
        } else {
            for (size_type index = 0; index <= this->cluster_mask(); index++) {
                cluster_type * cluster = this->cluster_at(index);
                cluster->clear();
            }
        }
    }

    template <bool finitial>
    void destory_entries() noexcept {
        // Destroy all entries.
        if (this->entries_ != nullptr) {
            control_byte * control = this->controls();
            for (size_type index = 0; index <= this->entry_mask(); index++) {
                if (control->isUsed()) {
                    entry_type * entry = this->entry_at(index);
                    this->entry_allocator_.destroy(entry);
                }
                control++;
            }
            if (finitial) {
                this->entry_allocator_.deallocate(this->entries_, this->entry_capacity());
                this->entries_ = nullptr;
            }
            this->entry_size_ = 0;
        }
    }

    template <bool initialize = false>
    void create_cluster(size_type init_capacity) {
        init_capacity = (std::max)(init_capacity, kMinimumCapacity);
        size_type new_capacity = align_to(init_capacity, kClusterEntries);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinimumCapacity);

        size_type cluster_count = new_capacity / kClusterEntries;
        assert(cluster_count > 0);
        cluster_type * clusters = new cluster_type[cluster_count];
        clusters_ = clusters;
        cluster_mask_ = cluster_count - 1;

        entry_type * entries = entry_allocator_.allocate(new_capacity);
        entries_ = entries;
        if (initialize)
            assert(entry_size_ == 0);
        else
            entry_size_ = 0;
        entry_mask_ = new_capacity - 1;
        entry_threshold_ = (size_type)(new_capacity * FLAT16_DEFAULT_LOAD_FACTOR);
    }

    JSTD_FORCED_INLINE
    size_type find_impl(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t control_hash = this->get_control_hash(hash_code);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_cluster = cluster_index;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterEntries;
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
        } while (cluster_index != start_cluster);

        return npos;
    }

    JSTD_FORCED_INLINE
    size_type find_impl(const key_type & key, index_type & first_cluster,
                        index_type & last_cluster, std::uint8_t & ctrl_hash) const {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t control_hash = this->get_control_hash(hash_code);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_cluster = cluster_index;
        first_cluster = start_cluster;
        ctrl_hash = control_hash;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterEntries;
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
    size_type find_and_erase(const key_type & key) {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t control_hash = this->get_control_hash(hash_code);
        index_type cluster_index = this->index_for(hash_code);
        index_type start_cluster = cluster_index;
        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterEntries;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    control_byte & control = this->get_control(index);
                    assert(control.isUsed());
                    if (cluster.hasAnyEmpty()) {
                        control.setEmpty();
                    } else {
                        cluster_index = this->next_cluster(cluster_index);
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
                    this->entry_allocator_.destroy(&target);
                    assert(this->entry_size_ > 0);
                    this->entry_size_--;
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
    std::pair<size_type, bool>
    find_and_prepare_insert(const key_type & key, std::uint8_t & ctrl_hash) {
        hash_code_t hash_code = this->get_hash(key);
        std::uint8_t control_hash = this->get_control_hash(hash_code);
        index_type cluster_index = this->index_for(hash_code);
        index_type first_cluster = cluster_index;
        ctrl_hash = control_hash;

        do {
            const cluster_type & cluster = this->get_cluster(cluster_index);
            std::uint32_t mask16 = cluster.matchHash(control_hash);
            size_type start_index = cluster_index * kClusterEntries;
            while (mask16 != 0) {
                size_type pos = BitUtils::bsf32(mask16);
                mask16 = BitUtils::clearLowBit32(mask16);
                size_type index = start_index + pos;
                const entry_type & target = this->get_entry(index);
                if (this->key_equal_(target.first, key)) {
                    return { index, true };
                }
            }
            std::uint32_t maskEmpty = cluster.matchEmpty();
            if (maskEmpty != 0) {
                index_type last_cluster = cluster_index;
                if (cluster_index != first_cluster) {
                    // Find the first DeletedEntry from first_cluster to last_cluster.
                    cluster_index = first_cluster;
                    do {
                        const cluster_type & cluster = this->get_cluster(cluster_index);
                        std::uint32_t maskDeleted = cluster.matchDeleted();
                        if (maskDeleted != 0) {
                            // Found a [DeletedEntry] to insert
                            size_type pos = BitUtils::bsf32(maskDeleted);
                            size_type start_index = cluster_index * kClusterEntries;
                            return { (start_index + pos), false };
                        }
                        if (cluster_index == last_cluster)
                            break;
                        cluster_index = this->next_cluster(cluster_index);
                    } while (cluster_index != first_cluster);

                    // Found a [EmptyEntry] to insert
                    // Skip to final processing
                } else {
                    std::uint32_t maskDeleted = cluster.matchDeleted();
                    if (maskDeleted != 0) {
                        // Found a [DeletedEntry] to insert
                        maskEmpty = maskDeleted;
                    } else {
                        // Found a [EmptyEntry] to insert
                        assert(maskEmpty != 0);
                    }
                }

                // Get [EmptyEntry] or [DeletedEntry] to insert
                size_type pos = BitUtils::bsf32(maskEmpty);
                size_type start_index = last_cluster * kClusterEntries;
                return { (start_index + pos), false };
            }
            cluster_index = this->next_cluster(cluster_index);
        } while (cluster_index != first_cluster);

        return { npos, false };
    }

    template <bool update_always>
    std::pair<iterator, bool> emplace_impl(value_type && value) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(value.first, ctrl_hash);
        size_type target = find_info.first;
        size_type is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            if (target != npos) {
                // Found a [DeletedEntry] or [EMptyEntry] to insert
                control_byte * control = this->control_at(target);
                assert(control->isEmptyOrDeleted());
                control->setUsed(ctrl_hash);
                entry_type * entry = this->entry_at(target);
                this->entry_allocator_.construct(entry, std::forward<value_type>(value));
                this->entry_size_++;
                return { this->iterator_at(target), true };
            } else {
                // Container is full and there is no anyone empty entry.
                this->grow();

                return this->emplace(std::forward<value_type>(value));
            }
        } else {
            // The key to be inserted already exists.
            if (update_always) {
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

    template <bool update_always, typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value) &&
              (!jstd::is_same_ex<KeyT, nc_value_type>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
              (jstd::is_same_ex<KeyT, key_type>::value ||
               std::is_constructible<key_type, KeyT &&>::value) &&
              !std::is_constructible<value_type, KeyT &&>::value>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(KeyT && key, Args && ... args) {
        std::uint8_t ctrl_hash;
        auto find_info = this->find_and_prepare_insert(key, ctrl_hash);
        size_type target = find_info.first;
        size_type is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            if (target != npos) {
                // Found a [DeletedEntry] or [EMptyEntry] to insert
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
                // Container is full and there is no anyone empty entry.
                this->grow();

                return this->emplace(std::forward<KeyT>(key), std::forward<Args>(args)...);
            }
        } else {
            // The key to be inserted already exists.
            if (update_always) {
                mapped_type mapped_value(std::forward<Args>(args)...);
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(mapped_value);
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool update_always, typename PieceWise, typename std::enable_if<
              (!jstd::is_same_ex<PieceWise, value_type>::value) &&
              (!jstd::is_same_ex<PieceWise, nc_value_type>::value) &&
              jstd::is_same_ex<PieceWise, std::piecewise_construct_t>::value &&
              (!jstd::is_same_ex<PieceWise, key_type>::value &&
               !std::is_constructible<key_type, PieceWise &&>::value) &&
              !std::is_constructible<value_type, PieceWise &&>::value>::type * = nullptr,
              typename ... Ts1, typename ... Ts2>
    std::pair<iterator, bool> emplace_impl(PieceWise && hint,
                                           std::tuple<Ts1...> && first,
                                           std::tuple<Ts2...> && second) {
        std::uint8_t ctrl_hash;
        tuple_wrapper2<key_type> key_wrapper(first);
        break_from_tuple(first);
        break_from_tuple(second);
        auto find_info = this->find_and_prepare_insert(key_wrapper.value(), ctrl_hash);
        size_type target = find_info.first;
        size_type is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            if (target != npos) {
                // Found a [DeletedEntry] or [EMptyEntry] to insert
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
                // Container is full and there is no anyone empty entry.
                this->grow();

                return this->emplace(std::forward<PieceWise>(hint),
                                     std::forward<std::tuple<Ts1...>>(first),
                                     std::forward<std::tuple<Ts2...>>(second));
            }
        } else {
            // The key to be inserted already exists.
            if (update_always) {
                tuple_wrapper2<mapped_type> mapped_wrapper(std::move(second));
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(mapped_wrapper.value());
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool update_always, typename First, typename std::enable_if<
              (!jstd::is_same_ex<First, value_type>::value) &&
              (!jstd::is_same_ex<First, nc_value_type>::value) &&
              (!jstd::is_same_ex<First, std::piecewise_construct_t>::value) &&
              (!jstd::is_same_ex<First, key_type>::value &&
               !std::is_constructible<key_type, First &&>::value) &&
              !std::is_constructible<value_type, First &&>::value>::type * = nullptr,
              typename ... Args>
    std::pair<iterator, bool> emplace_impl(First && first, Args && ... args) {
        std::uint8_t ctrl_hash;
        value_type value(std::forward<First>(first), std::forward<Args>(args)...);
        auto find_info = this->find_and_prepare_insert(value.first, ctrl_hash);
        size_type target = find_info.first;
        size_type is_exists = find_info.second;
        if (!is_exists) {
            // The key to be inserted is not exists.
            if (target != npos) {
                // Found a [DeletedEntry] or [EMptyEntry] to insert
                control_byte * control = this->control_at(target);
                assert(control->isEmptyOrDeleted());
                control->setUsed(ctrl_hash);
                entry_type * entry = this->entry_at(target);
                this->entry_allocator_.construct(entry, std::move(value));
                this->entry_size_++;
                return { this->iterator_at(target), true };
            } else {
                // Container is full and there is no anyone empty entry.
                this->grow();

                return this->emplace(std::forward<First>(first), std::forward<Args>(args)...);
            }
        } else {
            // The key to be inserted already exists.
            if (update_always) {
                entry_type * entry = this->entry_at(target);
                entry->second = std::move(value.second);
            }
            return { this->iterator_at(target), false };
        }
    }

    template <bool update_always>
    std::pair<iterator, bool> old_emplace_impl(value_type && value) {
        index_type first_cluster, last_cluster;
        std::uint8_t ctrl_hash;
        size_type index = this->find_impl(value.first, first_cluster, last_cluster, ctrl_hash);
        if (index == npos) {
            // The key to be inserted is not exists.

            // Find the first DeletedEntry from first_cluster to last_cluster.
            index_type cluster_index = first_cluster;
            do {
                cluster_type & cluster = this->get_cluster(cluster_index);
                std::uint32_t mask16 = cluster.matchDeleted();
                if (mask16 != 0) {
                    // Found a [DeletedEntry] to insert
                    size_type pos = BitUtils::bsf32(mask16);
                    size_type start_index = cluster_index * kClusterEntries;
                    index = start_index + pos;

                    control_byte * control = this->control_at(index);
                    assert(control->isDeleted());
                    control->setUsed(ctrl_hash);
                    entry_type * entry = this->entry_at(index);
                    this->entry_allocator_.construct(entry, std::forward<value_type>(value));
                    this->entry_size_++;
                    return { this->iterator_at(index), true };
                }
                if (cluster_index == last_cluster)
                    break;
                cluster_index = this->next_cluster(cluster_index);
            } while (cluster_index != first_cluster);

            if (last_cluster != npos) {
                cluster_type & cluster = this->get_cluster(last_cluster);
                std::uint32_t mask16 = cluster.matchEmpty();
                assert(mask16 != 0);

                size_type pos = BitUtils::bsf32(mask16);
                size_type start_index = cluster_index * kClusterEntries;
                index = start_index + pos;

                // Found a [EmptyEntry] to insert
                control_byte * control = this->control_at(index);
                assert(control->isEmpty());
                control->setUsed(ctrl_hash);
                entry_type * entry = this->entry_at(index);
                this->entry_allocator_.construct(entry, std::forward<value_type>(value));
                this->entry_size_++;
                return { this->iterator_at(index), true };
            } else {
                // Container is full and there is no anyone empty entry.
                this->grow();

                return this->emplace(std::forward<value_type>(value));
            }
        } else {
            // The key to be inserted already exists.
            if (update_always) {
                static constexpr bool is_rvalue_ref = std::is_rvalue_reference<decltype(value)>::value;
                entry_type * entry = this->entry_at(index);
                if (is_rvalue_ref)
                    entry->value.second = std::move(value.second);
                else
                    entry->value.second = value.second;
            }
            return { this->iterator_at(index), false };
        }
    }
};

} // namespace jstd
