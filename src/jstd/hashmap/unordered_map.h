
/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2022 XiongHui Guo (gz_shines at msn.com)

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

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"
#include "jstd/basic/inttypes.h"

#include <memory.h>
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
#include <algorithm>    // For std::max(), std::min()
#include <type_traits>
#include <stdexcept>

#include "jstd/hashmap/map_layout_policy.h"
#include "jstd/hashmap/hash_chunk_list.h"
#include "jstd/traits/type_traits.h"
#include "jstd/utility/utility.h"
#include "jstd/support/BitUtils.h"
#include "jstd/support/Power2.h"
#include "jstd/support/BitVec.h"

namespace jstd {

template < typename Key, typename Value,
           typename Hash = std::hash<Key>,
           typename KeyEqual = std::equal_to<Key>,
           typename LayoutPolicy = jstd::default_layout_policy<Key, Value>,
           typename Allocator = std::allocator<std::pair<const Key, Value>> >
class JSTD_DLL unordered_map {
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
    typedef std::uint32_t                   hash_code_t;
    typedef unordered_map<Key, Value, Hash, KeyEqual, LayoutPolicy, Allocator>
                                            this_type;

    typedef LayoutPolicy                    layout_policy_t;

    static constexpr size_type npos = size_type(-1);

    // kMinCapacity must be >= 2
    static constexpr size_type kMinCapacity = 4;
    // Maximum capacity is 1 << (sizeof(std::size_t) - 1).
    static constexpr size_type kMaximumCapacity = (std::numeric_limits<size_type>::max)() / 2 + 1;

    // kDefaultCapacity must be >= kMinCapacity
    static constexpr size_type kDefaultCapacity = 4;

    static constexpr float kMinLoadFactor = 0.2f;
    static constexpr float kMaxLoadFactor = 0.8f;

    // Must be kMinLoadFactor <= loadFactor <= kMaxLoadFactor
    static constexpr float kDefaultLoadFactor = 0.5f;

#if (JSTD_WORD_LEN == 64)
    static constexpr std::size_t kWordLen = 64;
    static constexpr bool kIs64Bit = true;
#else
    static constexpr std::size_t kWordLen = 32;
    static constexpr bool kIs64Bit = false;
#endif
    static constexpr bool kDetectStoreHash = detail::is_plain_type<key_type>::value;

    static constexpr bool kNeedStoreHash =
        (!layout_policy_t::autoDetectStoreHash && layout_policy_t::needStoreHash) ||
         (layout_policy_t::autoDetectStoreHash && (kDetectStoreHash));

    enum entry_type_t {
        kEntryTypeShfit  = 31,
        kIsEmptyEntry    = 0UL << kEntryTypeShfit,
        kIsUsedEntry     = 1UL << kEntryTypeShfit,

        kEntryTypeMask   = 0x80000000ul,
        kEntryIndexMask  = 0x7FFFFFFFul
    };

    struct entry_attr_t {
        std::uint32_t value;

        entry_attr_t(std::uint32_t value = 0) noexcept : value(value) {}
        entry_attr_t(std::uint32_t type, std::uint32_t chunk_id) noexcept
            : value(makeAttr(type, chunk_id)) {}
        entry_attr_t(const entry_attr_t & src) noexcept : value(src.value) {}
        ~entry_attr_t() {}

        entry_attr_t & operator = (const entry_attr_t & rhs) noexcept {
            this->value = rhs.value;
            return *this;
        }

        entry_attr_t & operator = (std::uint32_t value) noexcept {
            this->value = value;
            return *this;
        }

        uint32_t makeAttr(std::uint32_t type, std::uint32_t chunk_id) {
            assert((type & kEntryIndexMask) == 0);
            //return ((type & kEntryTypeMask) | (chunk_id & kEntryIndexMask));
            return (type | (chunk_id & kEntryIndexMask));
        }

        uint32_t getType() const {
            return (this->value & kEntryTypeMask);
        }

        uint32_t getChunkId() const {
            return (this->value & kEntryIndexMask);
        }

        void setValue(std::uint32_t type, std::uint32_t chunk_id) {
            this->value = this->makeAttr(type, chunk_id);
        }

        void setType(std::uint32_t type) {
            assert((type & kEntryIndexMask) == 0);
            // Here is a small optimization.
            // this->value = (type & kEntryAttrMask) | this->getChunkId();
            this->value = type | this->getChunkId();
        }

        void setChunkId(std::uint32_t chunk_id) {
            this->value = this->getEntryType() | (chunk_id & kEntryIndexMask);
        }

#if (JSTD_WORD_LEN == 64)
        void setChunkId(size_type chunk_id) {
            this->value = this->getEntryType() | (static_cast<std::uint32_t>(chunk_id) & kEntryIndexMask);
        }
#endif

        bool isEmptyEntry() {
            return (this->getEntryType() == kIsEmptyEntry);
        }

        bool isUsedEntry() {
            return (this->getEntryType() == kIsUsedEntry);
        }

        void setEmptyEntry() {
            setEntryType(kIsEmptyEntry);
        }

        void setUsedEntry() {
            setEntryType(kIsUsedEntry);
        }
    };

    static constexpr std::size_t kTypeMask64    = 0x8000000000000000ull;
    static constexpr std::size_t kIndexMask64   = 0x7FFFFFFFFFFFFFFFull;
    static constexpr std::size_t kHashMask64    = 0x00000000FFFFFFFFull;

    static constexpr std::size_t kEmptyEntry64  = 0x0000000000000000ull;
    static constexpr std::size_t kUsedEntry64   = 0x8000000000000000ull;

    static constexpr std::uint32_t kTypeMask32  = 0x80000000ul;
    static constexpr std::uint32_t kIndexMask32 = 0x7FFFFFFFul;
    static constexpr std::uint32_t kHashMask32  = 0xFFFFFFFFul;

    static constexpr std::uint32_t kEmptyEntry32 = 0x00000000ul;
    static constexpr std::uint32_t kUsedEntry32  = 0x80000000ul;

#if (JSTD_WORD_LEN == 64)
    struct entry_attr {
        union {
            struct {
                std::size_t type      : 1;
                std::size_t chunk_id  : 31:
                std::size_t hash_code : 32;
            };
            struct {
                std::uint32_t low:
                std::uint32_t high;
            };
            std::size_t value;
        };

        explicit entry_attr(std::size_t value = 0) noexcept : value(value) {}
        entry_attr(std::size_t type, std::size_t chunk_id, std::size_t hash_code) noexcept
            : value(makeAttr(type, chunk_id, hash_code)) {}
        entry_attr(const entry_attr_t & src) noexcept : value(src.value) {}
        ~entry_attr() {}

        entry_attr & operator = (const entry_attr & rhs) noexcept {
            this->value = rhs.value;
            return *this;
        }

        entry_attr & operator = (std::size_t value) noexcept {
            this->value = value;
            return *this;
        }

        std::uint32_t makeAttr(std::uint32_t type32, std::uint32_t chunk_id) {
            assert((type32 & kIndexMask32) == 0);
            return (type32 | (chunk_id & kIndexMask32));
        }

        std::size_t makeAttr(std::size_t type64, std::size_t chunk_id, std::size_t hash_code) {
            assert((type64 & kIndexMask64) == 0);
            return (type64 | ((chunk_id & kIndexMask64) << 32) | (hash_code & kHashMask64));
        }

        std::size_t getType() const {
            return (this->value & kTypeMask64);
        }

        std::uint32_t getType32() const {
            return (this->low & kTypeMask32);
        }

        std::size_t getChunkId() const {
            return std::size_t(this->low & kIndexMask32);
        }

        std::size_t getHashCode() const {
            return std::size_t(this->hash_code);
        }

        void setType(std::size_t type) {
            assert((type & kIndexMask64) == 0);
            this->value = (type & kTypeMask64) | (this->value & kIndexMask64);
        }

        void setChunkId(std::size_t chunk_id) {
            this->low = this->getType32() | (std::uint32_t(chunk_id) & kIndexMask32);
        }

        void setHashCode(std::size_t hash_code) {
            this->hash_code = std::uint32_t(hash_code);
        }

        std::size_t getLow() const {
            return this->low;
        }

        std::size_t getValue() const {
            return this->value;
        }

        void setValue(std::size_t type32, std::size_t chunk_id) {
            this->low = this->makeAttr(type32, chunk_id);
        }

        void setValue(std::size_t type, std::size_t chunk_id, std::size_t hash_code) {
            this->value = this->makeAttr(type32, chunk_id, hash_code);
        }

        bool isEmptyEntry() {
            return (this->getType() == kEmptyEntry64);
        }

        bool isUsedEntry() {
            return (this->getType() == kUsedEntry64);
        }

        void setEmptyEntry() {
            this->setType(kEmptyEntry64);
        }

        void setUsedEntry() {
            this->setType(kUsedEntry64);
        }
    };

#else // !(JSTD_WORD_LEN == 64)

    struct entry_attr {
        union {
            struct {
                std::uint32_t type     : 1;
                std::uint32_t chunk_id : 31:
                std::uint32_t hash_code;
            };
            struct {
                std::uint32_t low:
                std::uint32_t high;
            };
            std::uint64_t value;
        };

        static constexpr std::uint32_t kTypeMask32  = 0x80000000ul;
        static constexpr std::uint32_t kIndexMask32 = 0x7FFFFFFFul;
        static constexpr std::uint32_t kHashMask32  = 0xFFFFFFFFul;

        entry_attr(std::uint64_t value = 0) noexcept : value(value) {}
        entry_attr(std::uint32_t type, std::uint32_t chunk_id, std::uint32_t hash_code) noexcept
            : low(makeAttr(type, chunk_id)), high(hash_code) {}
        entry_attr(const entry_attr_t & src) noexcept : value(src.value) {}
        ~entry_attr() {}

        entry_attr & operator = (const entry_attr & rhs) noexcept {
            this->value = rhs.value;
            return *this;
        }

        entry_attr & operator = (std::uint64_t value) noexcept {
            this->value = value;
            return *this;
        }

        std::uint32_t makeAttr(std::uint32_t type, std::uint32_t chunk_id) {
            assert((type & kIndexMask32) == 0);
            return (type | (chunk_id & kIndexMask32));
        }

        std::uint64_t makeAttr(std::uint32_t type, std::uint32_t chunk_id, std::uint32_t hash_code) {
            assert((type & kIndexMask32) == 0);
            return ((std::uint64_t(type | (chunk_id & kIndexMask32)) << 32) | std::uint64_t(hash_code & kHashMask32));
        }

        std::uint32_t getType() const {
            return (this->low & kTypeMask32);
        }

        std::uint32_t getChunkId() const {
            return (this->low & kIndexMask32);
        }

        std::uint32_t getHashCode() const {
            return this->hash_code;
        }

        void setType(std::uint32_t type) {
            assert((type & kIndexMask32) == 0);
            this->low = (type & kTypeMask32) | (this->low & kIndexMask32);
        }

        void setChunkId(std::uint32_t chunk_id) {
            this->low = this->getType32() | (chunk_id & kIndexMask32);
        }

        void setHashCode(std::uint32_t hash_code) {
            this->hash_code = hash_code;
        }

        std::uint32_t getLow() const {
            return this->low;
        }

        std::uint64_t getValue() const {
            return this->value;
        }

        void setValue(std::uint32_t type, std::uint32_t chunk_id) {
            this->low = this->makeAttr(type32, chunk_id);
        }

        void setValue(std::uint32_t type, std::uint32_t chunk_id, std::uint32_t hash_code) {
            this->value = this->makeAttr(type, chunk_id, hash_code);
        }

        bool isEmptyEntry() {
            return (this->getType() == kEmptyEntry64);
        }

        bool isUsedEntry() {
            return (this->getType() == kUsedEntry64);
        }

        void setEmptyEntry() {
            this->setType(kEmptyEntry64);
        }

        void setUsedEntry() {
            this->setType(kUsedEntry64);
        }
    };

#endif // (JSTD_WORD_LEN == 64)

    template <bool NeedStoreHash = kNeedStoreHash>
    struct bucket_entry {
        bucket_entry *  next;
        entry_attr      attrib;
        value_type      value;

        hash_code_t get_hash() const {
            return static_cast<hash_code_t>(this->attrib.getHashCode());
        }

        bool compare_hash(hash_code_t hash_code) const {
            return (hash_code == this->get_hash());
        }

        void store_hash(hash_code_t hash_code) {
            this->attrib.setHashCode(static_cast<std::uint32_t>(hash_code));
        }
    };

    // NoStoreHash bucket_entry
    template <>
    struct bucket_entry<false> {
        bucket_entry *  next;
        value_type      value;

        hash_code_t get_hash() const {
            return 0;
        }

        constexpr bool compare_hash(hash_code_t hash_code) const {
            return false;
        }

        void store_hash(hash_code_t hash_code) {
            /* Not implement */
        }
    };

    typedef bucket_entry<kNeedStoreHash>    entry_type;
    typedef value_type                      node_type;

    // The maximum entry's chunk bytes, default is 32 MB bytes.
    static constexpr size_type kMaxEntryChunkBytes = 32 * 1024 * 1024;
    // The entry's block size per chunk (entry_type).
    static constexpr size_type kMaxEntryChunkSize =
            compile_time::round_to_power2<kMaxEntryChunkBytes / sizeof(entry_type)>::value;

    template <typename T, bool Is64Bit = kIs64Bit>
    struct bucket_pointer {
    public:
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef std::size_t size_type;

    private:
        pointer ptr_;

    public:
        bucket_pointer(pointer ptr = nullptr) noexcept : ptr_(ptr) {}
        bucket_pointer(void * ptr = nullptr) noexcept : ptr_(reinterpret_cast<pointer>(ptr)) {}
        bucket_pointer(const bucket_pointer & src) noexcept : ptr_(src.ptr_) {}

        bucket_pointer & operator = (const bucket_pointer & rhs) noexcept {
            this->ptr_ = rhs.ptr_;
        }

        ~bucket_pointer() = default;

        size_type count() const noexcept { return npos; }

        pointer ptr() noexcept { return this->ptr_; }
        const_pointer ptr() const noexcept {
            return const_cast<const_pointer>(this->ptr_);
        }

        void set_count(size_type count) noexcept {
        }

        void set_ptr(pointer ptr) noexcept {
            this->ptr_ = ptr;
        }

        void set_ptr(pointer ptr, size_type count) noexcept {
            this->ptr_ = ptr;
        }
    };

#if (JSTD_WORD_LEN == 64)
    template <typename T>
    class bucket_pointer<T, true> {
    public:
        typedef T           value_type;
        typedef T *         pointer;
        typedef const T *   const_pointer;
        typedef T &         reference;
        typedef const T &   const_reference;

        typedef std::size_t size_type;

        static constexpr std::uintptr_t kPtrMask = 0x00FFFFFFFFFFFFFFull;

    private:
        union {
            struct {
                std::uintptr_t count_: 8;
                std::uintptr_t iptr_:  56;
            };
            pointer ptr_;
        };

    public:
        bucket_pointer(pointer ptr = nullptr) noexcept : ptr_(ptr) {}
        bucket_pointer(void * ptr = nullptr) noexcept : ptr_(reinterpret_cast<bucket_entry *>(ptr)) {}
        bucket_pointer(const bucket_pointer & src) noexcept : ptr_(src.ptr_) {}

        bucket_pointer & operator = (const bucket_pointer & rhs) noexcept {
            this->ptr_ = rhs.ptr_;
        }

        ~bucket_pointer() = default;

        size_type count() const noexcept { return static_cast<size_type>(this->count_); }

        pointer ptr() noexcept {
            return reinterpret_cast<bucket_entry *>(this->iptr_);
        }
        const_pointer ptr() const noexcept {
            return const_cast<const bucket_entry *>(reinterpret_cast<bucket_entry *>(this->iptr_));
        }

        void set_count(size_type count) noexcept{
            this->count_ = count;
        }

        void set_ptr(pointer ptr) noexcept {
            this->iptr_ = (std::uintptr_t)ptr;
        }

        void set_ptr(pointer ptr, size_type count) noexcept {
            this->count_ = count;
            this->ptr_ = (std::uintptr_t)ptr;
        }
    };
#endif // (JSTD_WORD_LEN == 64)

    typedef bucket_pointer<bucket_entry, kIs64Bit> bucket_type;

    template <typename ValueType>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using owner_type = this_type;

        using value_type = ValueType;
        using pointer = ValueType *;
        using const_pointer = const ValueType *;
        using reference = ValueType &;
        using const_reference = const ValueType &;

        using mutable_value_type = typename std::remove_const<ValueType>::type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

    private:
        pointer             entry_;
        const owner_type *  owner_;

    public:
        basic_iterator() noexcept : entry_(nullptr), owner_(nullptr) {
        }
        basic_iterator(const owner_type * owner, pointer entry) noexcept
            : entry_(entry), owner_(owner) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : entry_(src.entry()), owner_(src.owner()) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) {
            this->entry_ = rhs.entry();
            this->owner_ = rhs.owner();
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) const noexcept {
            return (lhs.entry_ == rhs.entry());
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) const noexcept {
            return (lhs.entry_ != rhs.entry());
        }

        basic_iterator & operator ++ () {
            this->entry_ = static_cast<pointer>(this->owner_->next_link_entry(this->entry_));
            return (*this);
        }

        basic_iterator operator ++ (int) {
            basic_iterator copy(*this);
            ++*this;
            return copy;
        }

        reference operator * () const {
            return *(this->entry_->value);
        }

        pointer operator -> () const {
            return std::addressof(this->entry_->value);
        }

        operator basic_iterator<const mutable_value_type>() const {
            return { this->entry_, this->owner_ };
        }

        owner_type * owner() {
            return const_cast<owner_type *>(this->owner_);
        }

        const owner_type * owner() const {
            return this->owner_;
        }

        pointer entry()  {
            return this->entry_;
        }

        const_pointer entry() const {
            return const_cast<const_pointer>(this->entry_);
        }

        pointer value() {
            return this->entry_;
        }

        const_pointer value() const {
            return this->entry_;
        }
    };

    using iterator       = basic_iterator<entry_type>;
    using const_iterator = basic_iterator<const entry_type>;

    template <typename ValueType>
    class basic_local_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;

        using owner_type = this_type;

        using value_type = ValueType;
        using pointer = ValueType *;
        using const_pointer = const ValueType *;
        using reference = ValueType &;
        using const_reference = const ValueType &;

        using mutable_value_type = typename std::remove_const<ValueType>::type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

    private:
        pointer             node_;
        const owner_type *  owner_;

    public:
        basic_iterator() noexcept : node_(nullptr), owner_(nullptr) {
        }
        basic_iterator(const owner_type * owner, pointer node) noexcept
            : node_(node), owner_(owner) {
        }
        basic_iterator(const basic_iterator & src) noexcept
            : node_(src.node()), owner_(src.owner()) {
        }

        basic_iterator & operator = (const basic_iterator & rhs) {
            this->node_ = rhs.node();
            this->owner_ = rhs.owner();
            return *this;
        }

        friend bool operator == (const basic_iterator & lhs, const basic_iterator & rhs) const noexcept {
            return (lhs.node_ == rhs.node());
        }

        friend bool operator != (const basic_iterator & lhs, const basic_iterator & rhs) const noexcept {
            return (lhs.node_ != rhs.node());
        }

        basic_iterator & operator ++ () {
            this->node_ = static_cast<pointer>(this->owner_->next_link_entry(this->node_));
            return (*this);
        }

        basic_iterator operator ++ (int) {
            basic_iterator copy(*this);
            ++*this;
            return copy;
        }

        reference operator * () const {
            return *(this->node_);
        }

        pointer operator -> () const {
            return std::addressof(*this->node_);
        }

        operator basic_iterator<const mutable_value_type>() const {
            return { this->node_, this->owner_ };
        }

        owner_type * owner() {
            return const_cast<owner_type *>(this->owner_);
        }

        const owner_type * owner() const {
            return this->owner_;
        }

        pointer node()  {
            return this->node_;
        }

        const_pointer node() const {
            return const_cast<const_pointer>(this->node_);
        }

        pointer value() {
            return this->node_;
        }

        const_pointer value() const {
            return this->node_;
        }
    };

    using local_iterator       = basic_local_iterator<value_type>;
    using const_local_iterator = basic_local_iterator<const value_type>;

    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<bucket_pointer>
                                        bucket_allocator_type;
    typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<entry_type>
                                        entry_allocator_type;

    // free_list<T>
    template <typename T>
    class free_list {
    public:
        typedef T                               node_type;
        typedef typename this_type::size_type   size_type;

    protected:
        node_type * next_;
        size_type   size_;

    public:
        free_list(node_type * next = nullptr) :
            next_(next), size_((next == nullptr) ? 0 : 1) {
        }

        ~free_list() {
#ifdef _DEBUG
            this->clear();
#endif
        }

        node_type * begin() const { return this->next_; }
        node_type * end() const   { return nullptr; }

        node_type * next() const { return this->next_; }
        size_type   size() const { return this->size_; }

        bool is_valid() const { return (this->next_ != nullptr); }
        bool is_empty() const { return (this->size_ == 0); }

        void set_next(node_type * next) {
            this->next_ = next;
        }
        void set_size(size_type size) {
            this->size_ = size;
        }

        void set_list(node_type * next, size_type size) {
            this->next_ = next;
            this->size_ = size;
        }

        void clear() {
            this->next_ = nullptr;
            this->size_ = 0;
        }

        void reset(node_type * next) {
            this->next_ = next;
            this->size_ = 0;
        }

        void increase() {
            ++(this->size_);
        }

        void decrease() {
            assert(this->size_ > 0);
            --(this->size_);
        }

        void expand(size_type size) {
            this->size_ += size;
        }

        void shrink(size_type size) {
            assert(this->size_ >= size);
            this->size_ -= size;
        }

        void push_front(node_type * node) {
            assert(node != nullptr);
            node->next = this->next_;
            this->next_ = node;
            this->increase();
        }

        node_type * pop_front() {
            assert(this->next_ != nullptr);
            node_type * node = this->next_;
            this->next_ = node->next;
            this->decrease();
            return node;
        }

        node_type * front() {
            return this->next();
        }

        node_type * back() {
            node_type * prev = nullptr;
            node_type * node = this->next_;
            while (node != nullptr) {
                prev = node;
                node = node->next;
            }
            return prev;
        }

        void erase(node_type * where, node_type * node) {
            if (where != nullptr) {
                if (where->next == node) {
                    where->next = node->next;
                    this->decrease();
                }
            }
            else {
                if (this->next_ == node) {
                    this->next_ = node->next;
                    this->decrease();
                }
            }
        }

        void swap(free_list & other) {
            std::swap(this->next_, other.next_);
            std::swap(this->size_, other.size_);
        }
    };

    template <typename T>
    inline void swap(free_list<T> & lhs, free_list<T> & rhs) {
        lhs.swap(rhs);
    }

    typedef free_list<entry_type>       free_list_t;

    typedef hash_entry_chunk_list<entry_type, allocator_type, entry_allocator_type>
                                        entry_chunk_list_t;
    typedef typename entry_chunk_list_t::entry_chunk_t
                                        entry_chunk_t;

    typedef std::pair<iterator, bool>   insert_return_type;

private:
    bucket_pointer *    buckets_;
    entry_type *        entries_;
    size_type           bucket_mask_;
    size_type           entry_size_;
    size_type           entry_capacity_;
    size_type           entry_threshold_;
    free_list_t         freelist_;
    mutable entry_chunk_list_t
                        chunk_list_;
    float               load_factor_;

    hasher              hasher_;
    key_equal           key_equal_;

    allocator_type          allocator_;
    bucket_allocator_type   bucket_allocator_;
    entry_allocator_type    entry_allocator_;

public:
    unordered_map() : unordered_map(kDefaultCapacity) {
    }

    explicit unordered_map(size_type init_capacity,
                           const hasher & hash = hasher(),
                           const key_equal & equal = key_equal(),
                           const allocator_type & alloc = allocator_type()) :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), entry_allocator_(alloc) {
        this->create_bucket<true>(init_capacity);
    }

    explicit unordered_map(const allocator_type & alloc)
        : unordered_map(kDefaultCapacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    unordered_map(InputIter first, InputIter last,
                  size_type init_capacity = kDefaultCapacity,
                  const hasher & hash = hasher(),
                  const key_equal & equal = key_equal(),
                  const allocator_type & alloc = allocator_type()) :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal),
        allocator_(alloc), entry_allocator_(alloc) {
        this->create_bucket<true>(init_capacity);
        this->insert(first, last);
    }

    template <typename InputIter>
    unordered_map(InputIter first, InputIter last,
                  size_type init_capacity,
                  const allocator_type & alloc)
        : unordered_map(first, last, init_capacity, hasher(), key_equal(), alloc) {
    }

    template <typename InputIter>
    unordered_map(InputIter first, InputIter last,
                  size_type init_capacity,
                  const hasher & hash,
                  const allocator_type & alloc)
        : unordered_map(first, last, init_capacity, hash, key_equal(), alloc) {
    }

    unordered_map(const unordered_map & other)
        : unordered_map(other, std::allocator_traits<allocator_type>::
                               select_on_container_copy_construction(other.get_allocator())) {
    }

    unordered_map(const unordered_map & other, const Allocator & alloc) :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
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

    unordered_map(unordered_map && other) noexcept :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(std::move(other.hash_function())),
        key_equal_(std::move(other.key_eq())),
        allocator_(std::move(other.get_allocator())),
        entry_allocator_(std::move(other.get_entry_allocator())) {
        this->swap_content(other);
    }

    unordered_map(unordered_map && other, const Allocator & alloc) noexcept :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(std::move(other.hash_function())),
        key_equal_(std::move(other.key_eq())),
        allocator_(alloc),
        entry_allocator_(std::move(other.get_entry_allocator())) {
        this->swap_content(other);
    }

    unordered_map(std::initializer_list<value_type> init_list,
                  size_type init_capacity = kDefaultCapacity,
                  const hasher & hash = hasher(),
                  const key_equal & equal = key_equal(),
                  const allocator_type & alloc = allocator_type()) :
        buckets_(nullptr), bucket_mask_(0),
        entries_(nullptr), entry_size_(0), entry_capacity_(0),
        entry_threshold_(0), load_factor_(kDefaultLoadFactor),
        hasher_(hash), key_equal_(equal), allocator_(alloc) {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type new_capacity = (init_capacity >= init_list.size()) ? init_capacity : init_list.size();
        this->reserve_for_insert(new_capacity);
        this->insert(init_list.begin(), init_list.end());
    }

    unordered_map(std::initializer_list<value_type> init_list,
                  size_type init_capacity,
                  const allocator_type & alloc)
        : unordered_map(init_list, init_capacity, hasher(), key_equal(), alloc) {
    }

    unordered_map(std::initializer_list<value_type> init_list,
                  size_type init_capacity,
                  const hasher & hash,
                  const allocator_type & alloc)
        : unordered_map(init_list, init_capacity, hash, key_equal(), alloc) {
    }

    ~unordered_map() {
        this->destroy<true>();
    }

    bool is_valid() const { return (this->buckets() != nullptr); }
    bool is_empty() const { return (this->size() == 0); }
    bool is_full() const  { return (this->size() >= this->entry_capacity()); }

    bool empty() const { return this->is_empty(); }

    size_type size() const { return this->entry_size_; }
    size_type capacity() const { return this->entry_capacity_; }

    entry_type ** buckets() { return this->buckets_; }
    const entry_type ** buckets() const { return this->buckets_; }

    size_type bucket_mask() const { return this->bucket_mask_; }
    size_type bucket_count() const { return this->bucket_capacity_; }
    size_type bucket_capacity() const { return this->bucket_capacity_; }

    entry_type * entries() { return this->entries_; }
    const entry_type * entries() const { return this->entries_; }

    size_type entry_size() const { return this->entry_size_; }
    size_type entry_capacity() const { return this->entry_capacity_; }
    size_type entry_threshold() const { return this->entry_threshold_; }

    size_type max_chunk_size() const { return kMaxEntryChunkSize; }
    size_type max_chunk_bytes() const { return kMaxEntryChunkBytes; }
    size_type actual_chunk_bytes() const { return (kMaxEntryChunkSize * sizeof(entry_type)); }

    size_type bucket_size(size_type index) const {
        assert(index < this->bucket_count());

        size_type count = 0;
        entry_type * node = this->get_bucket_head(index);
        while (likely(node != nullptr)) {
            count++;
            node = node->next;
        }
        return count;
    }

    size_type bucket(const key_type & key) const {
        size_type index = this->find_impl(key);
        return ((index != npos) ? (index / kClusterEntries) : npos);
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
        cluster_type * cluster = this->buckets();
        cluster_type * last_cluster = this->buckets() + this->cluster_count();
        size_type start_index = 0;
        for (; cluster != last_cluster; cluster++) {
            std::uint32_t maskUsed = cluster->matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = start_index + pos;
                return this->iterator_at(index);
            }
            start_index += kClusterEntries;
        }

        return this->iterator_at(this->entry_capacity());
#else
        control_byte * control = this->controls();
        size_type index;
        for (index = 0; index <= this->entry_capacity(); index++) {
            if (control->isUsed()) {
                return { control, this->entry_at(index) };
            }
            control++;
        }
        return { control, this->entry_at(index) };
#endif
    }

    const_iterator begin() const {
        cluster_type * cluster = this->buckets();
        cluster_type * last_cluster = this->buckets() + this->cluster_count();
        size_type start_index = 0;
        for (; cluster != last_cluster; cluster++) {
            std::uint32_t maskUsed = cluster->matchUsed();
            if (maskUsed != 0) {
                size_type pos = BitUtils::bsf32(maskUsed);
                size_type index = start_index + pos;
                return this->iterator_at(index);
            }
            start_index += kClusterEntries;
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
        return "jstd::unordered_map<K, V>";
    }

    void clear(bool need_destroy = false) noexcept {
        if (this->entry_capacity() > kDefaultCapacity) {
            if (need_destroy) {
                this->destroy<true>();
                this->create_bucket<false>(kDefaultCapacity);
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

    void swap(unordered_map & other) {
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
        entry_type * entry = this->find_entry(key);
        if (entry != nullptr) {
            return entry->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::unordered_map<K,V>::at(key), "
                                    "the specified key is not exists.");
        }
    }

    const mapped_type & at(const key_type & key) const {
        entry_type * entry = this->find_entry(key);
        if (entry != nullptr) {
            return entry->value.second;
        } else {
            throw std::out_of_range("std::out_of_range exception: jstd::unordered_map<K,V>::at(key) const, "
                                    "the specified key is not exists.");
        }
    }

    size_type count(const key_type & key) const {
        entry_type * entry = this->find_entry(key);
        return (entry != nullptr) ? size_type(1) : size_type(0);
    }

    bool contains(const key_type & key) const {
        entry_type * entry = this->find_entry(key);
        return (entry != nullptr);
    }

    iterator find(const key_type & key) {
        entry_type * entry = this->find_entry(key);
        return iterator(this, entry);
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
        size_type new_capacity = (std::max)(init_capacity, kMinCapacity);
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinCapacity>(new_capacity);
        }
        return new_capacity;
    }

    JSTD_FORCED_INLINE
    size_type capacity_for_reserve(size_type init_capacity) {
        size_type new_capacity = (size_type)std::ceil((float)init_capacity / this->max_load_factor());
        return new_capacity;
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

    inline index_type index_for(hash_code_t hash_code) const {
        return (index_type)((size_type)hash_code & this->bucket_mask());
    }

    inline index_type index_for(hash_code_t hash_code, size_type _bucket_mask) const {
        return (index_type)((size_type)hash_code & _bucket_mask);
    }

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    inline index_type index_for(size_type hash_code) const {
        return (index_type)(hash_code & this->bucket_mask());
    }

    inline index_type index_for(size_type hash_code, size_type _bucket_mask) const {
        return (index_type)(hash_code & _bucket_mask);
    }
#endif // __amd64__

    inline index_type next_index(index_type index, size_type _bucket_mask) const {
        ++index;
        return (index_type)((size_type)index & _bucket_mask);
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

    inline index_type index_for(hash_code_t hash_code) const noexcept {
        return (index_type)((size_type)hash_code & this->bucket_mask());
    }

    inline index_type index_for(hash_code_t hash_code, size_type bucket_mask) const noexcept {
        return (index_type)((size_type)hash_code & bucket_mask);
    }


    bucket_type * bucket_at(size_type bucket_index) noexcept {
        assert(bucket_index < this->bucket_count());
        return (this->buckets() + bucket_index);
    }

    const bucket_type * bucket_at(size_type bucket_index) const noexcept {
        assert(bucket_index < this->bucket_count());
        return (this->buckets() + bucket_index);
    }

    entry_type * entry_at(size_type index) noexcept {
        assert(index <= this->entry_capacity());
        return (this->entries() + index);
    }

    const entry_type * entry_at(size_type index) const noexcept {
        assert(index <= this->entry_capacity());
        return (this->entries() + index);
    }

    bucket_type & get_bucket(size_type bucket_index) {
        assert(bucket_index < this->bucket_count());
        return this->buckets_[bucket_index];
    }

    const bucket_type & get_bucket(size_type bucket_index) const {
        assert(bucket_index < this->bucket_count());
        return this->buckets_[bucket_index];
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

    template <bool finitial>
    void destroy() noexcept {
        this->destroy_entries<finitial>();

        // Note!!: destroy_entries() need use this->buckets()
        this->destroy_cluster<finitial>();
    }

    template <bool finitial>
    void destroy_cluster() noexcept {
        if (finitial) {
            if (this->buckets_ != nullptr) {
                delete[] this->buckets_;
                this->buckets_ = nullptr;
            }
        } else {
            for (size_type index = 0; index <= this->bucket_mask(); index++) {
                cluster_type * cluster = this->cluster_at(index);
                cluster->clear();
            }
        }
    }

    template <bool finitial>
    void destroy_entries() noexcept {
        // Destroy all entries.
        if (this->entries_ != nullptr) {
            control_byte * control = this->controls();
            for (size_type index = 0; index <= this->entry_capacity(); index++) {
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

    void assert_buckets_capacity(entry_type ** new_buckets, size_type new_bucket_capacity) {
        assert(this->buckets() != nullptr);
        assert(new_buckets != nullptr);
        (void)new_buckets;
        assert_bucket_capacity(new_bucket_capacity);
    }

    void assert_entries_capacity(entry_type * new_entries, size_type new_entry_capacity) {
        assert(this->entries() != nullptr);
        assert(new_entries != nullptr);
        (void)new_entries;
        assert_entry_capacity(new_entry_capacity);
    }

    void assert_bucket_capacity(size_type bucket_capacity) {
        assert(pow2::is_pow2(bucket_capacity));
        assert(bucket_capacity >= this->min_bucket_count(kMinCapacity));
        assert(bucket_capacity >= this->min_bucket_count());
        (void)bucket_capacity;
    }

    void assert_entry_capacity(size_type entry_capacity) {
        assert(pow2::is_pow2(entry_capacity));
        assert(entry_capacity >= kMinCapacity);
        assert(entry_capacity >= this->entry_size());
        (void)entry_capacity;
    }

    JSTD_FORCED_INLINE
    entry_type * get_bucket_head(index_type index) const {
        return this->buckets_[index];
    }

    JSTD_FORCED_INLINE
    void bucket_push_front(index_type index,
                           entry_type * new_entry) {
        new_entry->next = this->buckets_[index];
        this->buckets_[index] = new_entry;
    }

    JSTD_FORCED_INLINE
    void bucket_push_front(entry_type ** new_buckets,
                           index_type index,
                           entry_type * new_entry) {
        new_entry->next = new_buckets[index];
        new_buckets[index] = new_entry;
    }

    JSTD_FORCED_INLINE
    void bucket_push_back(index_type index,
                          entry_type * new_entry) {
        entry_type * first = this->buckets_[index];
        if (likely(first != nullptr)) {
            entry_type * prev = first;
            entry_type * enrty = first->next;
            while (enrty != nullptr) {
                prev = enrty;
                enrty = enrty->next;
            }
            prev->next = new_entry;
        }
        else {
            this->buckets_[index] = new_entry;
        }
    }

    JSTD_FORCED_INLINE
    void bucket_push_back(entry_type ** new_buckets,
                          index_type index,
                          entry_type * new_entry) {
        entry_type * first = new_buckets[index];
        if (likely(first != nullptr)) {
            entry_type * prev = first;
            entry_type * enrty = first->next;
            while (enrty != nullptr) {
                prev = enrty;
                enrty = enrty->next;
            }
            prev->next = new_entry;
        }
        else {
            new_buckets[index] = new_entry;
        }
    }

    inline bool need_grow() const {
        return (this->entry_size_ > this->entry_threshold_);
    }

    void grow_if_necessary() {
        size_type new_capacity = (this->entry_capacity_ + 1) * 2;
        this->rehash_impl<false, true>(new_capacity);
    }

    JSTD_FORCED_INLINE
    void reserve_for_insert(size_type init_capacity) {
        size_type new_capacity = this->capacity_for_reserve(init_capacity);
        this->create_bucket<true>(new_capacity);
    }

    template <bool initialize = false>
    void create_bucket(size_type init_capacity) {
        size_type new_capacity;
        if (initialize)
            new_capacity = calc_capacity(init_capacity);
        else
            new_capacity = init_capacity;
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);

        size_type cluster_count = (new_capacity + (kClusterEntries - 1)) / kClusterEntries;
        assert(cluster_count > 0);
        bucket_pointer * buckets = new bucket_pointer[cluster_count + 1];
        buckets_ = buckets;
        bucket_mask_ = cluster_count - 1;

        buckets[cluster_count].template fillAll8<kEndOfMark>();

        entry_type * entries = entry_allocator_.allocate(new_capacity);
        entries_ = entries;
        if (initialize) {
            assert(entry_size_ == 0);
        } else {
            entry_size_ = 0;
        }
        entry_capacity_ = new_capacity - 1;
        entry_threshold_ = (size_type)((float)new_capacity * this->max_load_factor());
    }

    template <bool AllowShrink, bool AlwaysResize>
    void rehash_impl(size_type new_capacity) {
        new_capacity = this->calc_capacity(new_capacity);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);
        if (AlwaysResize ||
            (!AllowShrink && (new_capacity > this->entry_capacity())) ||
            (AllowShrink && (new_capacity != this->entry_capacity()))) {
            if (!AlwaysResize && !AllowShrink) {
                assert(new_capacity >= this->entry_size());
            }

            cluster_type * old_buckets = this->buckets();
            control_byte * old_controls = this->controls();
            size_type old_bucket_mask = this->bucket_mask();
            size_type old_cluster_count = this->cluster_count();
            
            entry_type * old_entries = this->entries();
            size_type old_entry_size = this->entry_size();
            size_type old_entry_capacity = this->entry_capacity();
            size_type old_entry_capacity = this->entry_capacity();

            this->create_bucket<false>(new_capacity);

            //if (old_entry_capacity >= kClusterEntries)
            {
                cluster_type * last_cluster = old_buckets + old_cluster_count;
                entry_type * entry_start = old_entries;
                for (cluster_type * cluster = old_buckets; cluster != last_cluster; cluster++) {
                    std::uint32_t maskUsed = cluster->matchUsed();
                    while (maskUsed != 0) {
                        size_type pos = BitUtils::bsf32(maskUsed);
                        maskUsed = BitUtils::clearLowBit32(maskUsed);
                        entry_type * entry = entry_start + pos;
                        this->move_insert_unique(entry);
                        this->entry_allocator_.destroy(entry);
                    }
                    entry_start += kClusterEntries;
                }
            }
            assert(this->entry_size() == old_entry_size);

            this->entry_allocator_.deallocate(old_entries, old_entry_capacity);

            if (old_buckets != nullptr) {
                delete[] old_buckets;
            }
        }
    }

    entry_type * find_entry(const key_type & key) const {
        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        assert(this->buckets() != nullptr);
        entry_type * entry = this->buckets_[index];
        while (entry != nullptr) {
            if (likely(!entry->compare_hash(hash_code))) {
                entry = entry->next;
            } else {
                if (likely(!this->key_equal_(key, entry->value.first)))
                    entry = entry->next;
                else
                    return entry;
            }
        }

        return nullptr;  // Not found
    }

    entry_type * find_entry(const key_type & key, hash_code_t hash_code, index_type index) {
        assert(this->buckets() != nullptr);
        entry_type * entry = this->buckets_[index];
        while (entry != nullptr) {
            if (likely(!entry->compare_hash(hash_code))) {
                entry = entry->next;
            }
            else {
                if (likely(!this->key_equal_(key, entry->value.first)))
                    entry = entry->next;
                else
                    return entry;
            }
        }

        return nullptr;  // Not found
    }

    entry_type * find_before(const key_type & key, entry_type *& before, size_type & index) {
        hash_code_t hash_code = this->get_hash(key);
        index = this->index_for(hash_code);

        assert(this->buckets() != nullptr);
        entry_type * prev = nullptr;
        entry_type * entry = this->buckets_[index];
        while (entry != nullptr) {
            if (likely(!entry->compare_hash(hash_code))) {
                prev = entry;
                entry = entry->next;
            }
            else {
                if (likely(!this->key_equal_(key, entry->value.first))) {
                    if (!kNeedStoreHash) {
                        prev = entry;
                    }
                    entry = entry->next;
                } else {
                    before = prev;
                    return entry;
                }
            }
        }

        return nullptr;  // Not found
    }

    entry_type * find_first_valid_entry() const {
        entry_type * first_entry = nullptr;
        entry_type * last_entry;

        size_type chunk_index = 0;
        size_type chunk_total = this->chunk_list_.size();
        while (chunk_index < chunk_total) {
            // Find first of not nullptr chunk.
            const entry_chunk_t & cur_chunk = this->chunk_list_[chunk_index];
            if (cur_chunk.entries != nullptr) {
                first_entry = cur_chunk.entries;
                last_entry = cur_chunk.entries + cur_chunk.size;
                while (first_entry < last_entry) {
                    // Find first of used entry.
                    if (likely(!first_entry->attrib.isUsedEntry()))
                        ++first_entry;
                    else
                        return first_entry;
                }
            }
            chunk_index++;
        }

        return first_entry;
    }

    entry_type * next_valid_entry(entry_type * entry) const {
        assert(entry != nullptr);

        size_type chunk_index = entry->attrib.getChunkId();
        assert(chunk_index < this->chunk_list_.size());

        const entry_chunk_t & cur_chunk = this->chunk_list_[chunk_index];
        assert(cur_chunk.entries != nullptr);

        entry_type * next_entry = ++entry;
        entry_type * last_entry = cur_chunk.entries + cur_chunk.size;
        assert(next_entry >= cur_chunk.entries);

        while (next_entry < last_entry) {
            if (likely(!next_entry->attrib.isUsedEntry()))
                next_entry++;
            else
                return next_entry;
        }

        next_entry = nullptr;
        chunk_index++;

        size_type chunk_list_size = this->chunk_list_.size();
        while (chunk_index < chunk_list_size) {
            // Find first of not nullptr chunk.
            const entry_chunk_t & cur_chunk2 = this->chunk_list_[chunk_index];
            if (cur_chunk2.entries != nullptr) {
                next_entry = cur_chunk2.entries;
                last_entry = cur_chunk2.entries + cur_chunk2.size;
                while (next_entry < last_entry) {
                    // Find first of in use entry.
                    if (likely(!next_entry->attrib.isUsedEntry()))
                        next_entry++;
                    else
                        return next_entry;
                }
            }
            chunk_index++;
        }

        return next_entry;
    }

    const entry_type * next_const_valid_entry(const entry_type * centry) {
        entry_type * entry = const_cast<entry_type *>(centry);
        entry_type * next_entry = this->next_valid_entry(entry);
        return const_cast<const entry_type *>(next_entry);
    }

    entry_type * next_first_valid_entry(size_type chunk_index = 0,
                                        size_type entry_index = 0) const {
        size_type chunk_list_size = this->chunk_list_.size();
        assert(chunk_index < chunk_list_size);

        entry_type * first_entry = nullptr;
        entry_type * last_entry;
        while (chunk_index < chunk_list_size) {
            // Find first of not nullptr chunk.
            const entry_chunk_t & cur_chunk = this->chunk_list_[chunk_index];
            if (cur_chunk.entries != nullptr) {
                first_entry = cur_chunk.entries + entry_index;
                last_entry = cur_chunk.entries + cur_chunk.size;
                while (first_entry < last_entry) {
                    // Find first of in use entry.
                    if (likely(!first_entry->attrib.isUsedEntry()))
                        first_entry++;
                    else
                        return first_entry;
                }
            }
            entry_index = 0;
            chunk_index++;
        }

        return first_entry;
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
                size_type start_index = cluster_index * kClusterEntries;
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
        assert(control->isEmptyOrDeleted());
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
        assert(control->isEmptyOrDeleted());
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
        assert(control->isEmptyOrDeleted());
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

        // Find the first unused entry and insert
        if (cluster_index != first_cluster) {
            // Find the first [DeletedEntry] from first_cluster to last_cluster.
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

            // Not found any [DeletedEntry], and use [EmptyEntry] to insert
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
        size_type start_index = last_cluster * kClusterEntries;
        return { (start_index + pos), false };
    }

    JSTD_FORCED_INLINE
    void insert_to_bucket(entry_type * new_entry, hash_code_t hash_code,
                          index_type index) {
        assert(new_entry != nullptr);

        new_entry->next = this->buckets_[index];
        new_entry->store_hash(hash_code);
        this->buckets_[index] = new_entry;
    }

    JSTD_FORCED_INLINE
    void move_assign_or_swap_key(key_type & old_key, key_type && new_key) {
        bool has_noexcept_move_assignment = jstd::is_noexcept_move_assignable<key_type>::value;
        // Is noexcept move assignment operator ?
        if (has_noexcept_move_assignment) {
            old_key = std::move(new_key);
        } else {
            using std::swap;
            swap(old_key, new_key);
        }
    }

    JSTD_FORCED_INLINE
    void move_assign_or_swap_mapped_value(mapped_type & old_value, mapped_type && new_value) {
        bool has_noexcept_move_assignment = jstd::is_noexcept_move_assignable<mapped_type>::value;
        // Is noexcept move assignment operator ?
        if (has_noexcept_move_assignment) {
            old_value = std::move(new_value);
        } else {
            using std::swap;
            swap(old_value, new_value);
        }
    }

    JSTD_FORCED_INLINE
    void move_assign_or_swap_value(value_type & old_value, value_type && new_value) {
        bool has_noexcept_move_assignment = jstd::is_noexcept_move_assignable<value_type>::value;
        // Is noexcept move assignment operator ?
        if (has_noexcept_move_assignment) {
            old_value = std::move(new_value);
        } else {
            using std::swap;
            swap(old_value, new_value);
        }
    }

    JSTD_FORCED_INLINE
    void move_assign_or_swap_value(nc_value_type & old_value, nc_value_type && new_value) {
        bool has_noexcept_move_assignment = is_noexcept_move_assignable<nc_value_type>::value;
        // Is noexcept move assignment operator ?
        if (has_noexcept_move_assignment) {
            old_value = std::move(new_value);
        }
        else {
            using std::swap;
            swap(old_value, new_value);
        }
    }

    template <typename T>
    JSTD_FORCED_INLINE
    void move_assign_or_swap(T & old_value, T & new_value) {
        old_value = new_value;
    }

    template <typename T>
    JSTD_FORCED_INLINE
    void move_assign_or_swap(T & old_value, T && new_value) {
        bool has_noexcept_move_assignment = is_noexcept_move_assignable<T>::value;
        // Is noexcept move assignment operator ?
        if (has_noexcept_move_assignment) {
            old_value = std::move(new_value);
        }
        else {
            using std::swap;
            swap(old_value, new_value);
        }
    }

    JSTD_FORCED_INLINE
    void construct_value(entry_type * new_entry, const key_type & key,
                                                 const mapped_type & value) {
        // Use placement new method to construct value_type.
        this->allocator_.construct(&new_entry->value, key, value);
    }

    JSTD_FORCED_INLINE
    void construct_value(entry_type * new_entry, const key_type & key,
                                                 mapped_type && value) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value, key,
                                   std::forward<mapped_type>(value));
    }

    JSTD_FORCED_INLINE
    void construct_value(entry_type * new_entry, key_type && key,
                                                 mapped_type && value) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value,
                                   std::forward<key_type>(key),
                                   std::forward<mapped_type>(value));
    }

    JSTD_FORCED_INLINE
    void construct_value(entry_type * new_entry, const value_type & value) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value, value);
    }

    JSTD_FORCED_INLINE
    void construct_value(entry_type * new_entry, value_type && value) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value,
                                   std::forward<value_type>(value));
    }

    template <typename ...Args>
    JSTD_FORCED_INLINE
    void construct_value_args(entry_type * new_entry, const key_type & key, Args && ... args) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value, key, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    JSTD_FORCED_INLINE
    void construct_value_args(entry_type * new_entry, key_type && key, Args && ... args) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value, std::forward<key_type>(key),
                                                      std::forward<Args>(args)...);
    }

    template <typename ...Args>
    JSTD_FORCED_INLINE
    void construct_value_args(entry_type * new_entry, Args && ... args) {
        // Use placement new to construct value_type.
        this->allocator_.construct(&new_entry->value, std::forward<Args>(args)...);
    }

    bool add_chunk_or_grow_if_necessary(size_type increase_size) {
        return false;
    }

    entry_type * got_free_entry(hash_code_t hash_code, index_type & index) {
        if (likely(!this->freelist_.is_empty())) {
            // Pop a free entry from freelist.
            entry_type * free_entry = this->freelist_.pop_front();
            return free_entry;
        } else {
            if (likely(this->chunk_list_.is_full())) {
                // The chunk buffer is not enough
                bool growed = this->add_chunk_or_grow_if_necessary(1);
                if (growed) {
                    // Recalculate the bucket index.
                    index = this->index_for(hash_code);
                }
            }

            // Got a free entry.
            entry_type * new_entry = &this->entries_[this->chunk_list_.firstFreeIndex()];
            assert(new_entry != nullptr);
            uint32_t chunk_id = this->chunk_list_.lastChunkId();
            new_entry->attrib.setValue(kEmptyEntry32, chunk_id);
            return new_entry;
        }
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> insert_impl(const key_type & key, const mapped_type & value) {
        bool inserted;

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (likely(entry == nullptr)) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, key, value);
            this->entry_size_++;
            entry = new_entry;
            inserted = true;
        }
        else {
            assert(key == entry->value.first);
            if (AlwaysUpdate) {
                entry->value.second = value;
            }
            inserted = false;
        }

        return { iterator(this, entry), inserted };
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> insert_impl(const key_type & key, mapped_type && value) {
        bool inserted;

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (likely(entry == nullptr)) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, key, std::forward<mapped_type>(value));
            this->entry_size_++;
            entry = new_entry;
            inserted = true;
        }
        else {
            assert(key == entry->value.first);
            if (AlwaysUpdate) {
                entry->value.second = std::forward<mapped_type>(value);
            }
            inserted = false;
        }

        return { iterator(this, entry), inserted };
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> insert_impl(key_type && key, mapped_type && value) {
        bool inserted;

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (likely(entry == nullptr)) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, std::forward<key_type>(key),
                                             std::forward<mapped_type>(value));
            this->entry_size_++;
            entry = new_entry;
            inserted = true;
        }
        else {
            assert(key == entry->value.first);
            if (AlwaysUpdate) {
                entry->value.second = std::forward<mapped_type>(value);
            }
            inserted = false;
        }

        return { iterator(this, entry), inserted };
    }

    template <bool AlwaysUpdate>
    std::pair<iterator, bool> insert_impl(value_type && value) {
        bool inserted;

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (likely(entry == nullptr)) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, std::forward<value_type>(value));
            this->entry_size_++;
            entry = new_entry;
            inserted = true;
        }
        else {
            assert(value.first == entry->value.first);
            if (AlwaysUpdate) {
                entry->value.second = std::forward<mapped_type>(value.second);
            }
            inserted = false;
        }

        return { iterator(this, entry), inserted };
    }

    entry_type * try_emplace_impl(const key_type & key) {
        assert(this->buckets() != nullptr);

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (entry == nullptr) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, key, mapped_type());
            this->entry_size_++;
            entry = new_entry;
        }

        return entry;
    }

    entry_type * try_emplace_impl(key_type && key) {
        assert(this->buckets() != nullptr);

        hash_code_t hash_code = this->get_hash(key);
        index_type index = this->index_for(hash_code);

        entry_type * entry = this->find_entry(key, hash_code, index);
        if (entry == nullptr) {
            entry_type * new_entry = this->got_free_entry(hash_code, index);
            this->insert_to_bucket(new_entry, hash_code, index);
            this->construct_value(new_entry, std::forward<key_type>(key), mapped_type());
            this->entry_size_++;
            entry = new_entry;
        }

        return entry;
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
    void erase_entry(size_type index) {
        assert(index <= this->entry_capacity());
        control_byte & control = this->get_control(index);
        assert(control.isUsed());
        index_type start_cluster = (index_type)(index / kClusterEntries);
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

    void swap_content(unordered_map & other) {
        std::swap(this->buckets_, other.buckets());
        std::swap(this->bucket_mask_, other.bucket_mask());
        std::swap(this->entries_, other.entries());
        std::swap(this->entry_size_, other.entry_size());
        std::swap(this->entry_capacity_, other.entry_capacity());
        std::swap(this->entry_threshold_, other.entry_threshold());
        std::swap(this->freelist_, other.freelist_);
        std::swap(this->load_factor_, other.max_load_factor());
    }

    void swap_policy(unordered_map & other) {
        std::swap(this->hasher_, other.hash_function());
        std::swap(this->key_equal_, other.key_eq());
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
            std::swap(this->allocator_, other.get_allocator());
        }
        if (std::allocator_traits<entry_allocator_type>::propagate_on_container_swap::value) {
            std::swap(this->entry_allocator_, other.get_entry_allocator());
        }
    }

    void swap_impl(unordered_map & other) {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

} // namespace jstd
