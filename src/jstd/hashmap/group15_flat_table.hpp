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

#ifndef JSTD_HASHMAP_GROUP15_FLAT_TABLE_HPP
#define JSTD_HASHMAP_GROUP15_FLAT_TABLE_HPP

#pragma once

#define NOMINMAX
#include <stdint.h>
#include <stddef.h>

#include <cstdint>
#include <cstddef>
#include <memory>           // For std::allocator<T>
#include <limits>           // For std::numeric_limits<T>, CHAR_BIT
#include <initializer_list>
#include <type_traits>
#include <algorithm>        // For std::max()
#include <utility>          // For std::pair<F, S>

#include <assert.h>

#include "jstd/basic/stddef.h"

#include "jstd/support/Power2.h"
#include "jstd/support/BitUtils.h"
#include "jstd/support/CPUPrefetch.h"

#include "jstd/traits/type_traits.h"    // For jstd::narrow_cast<T>()
#include "jstd/hasher/hashes.h"
#include "jstd/utility/utility.h"

#include "jstd/hashmap/flat_map_iterator15.hpp"
#include "jstd/hashmap/flat_map_group15.hpp"
#include "jstd/hashmap/group_quadratic_prober.hpp"

#include "jstd/hashmap/detail/hashmap_traits.h"

#include "jstd/hashmap/flat_map_type_policy.hpp"
#include "jstd/hashmap/flat_map_slot_policy.hpp"
#include "jstd/hashmap/slot_policy_traits.h"

#define GROUP15_USE_HASH_POLICY     0
#define GROUP15_USE_SEPARATE_SLOTS  1

#define GROUP15_USE_GROUP_SCAN      1
#define GROUP15_USE_INDEX_SHIFT     1

#define GROUP15_USE_NEW_OVERFLOW    1

#ifdef _DEBUG
#define GROUP15_DISPLAY_DEBUG_INFO  0
#endif

namespace jstd {

template <typename TypePolicy, typename Hash,
          typename KeyEqual, typename Allocator>
class JSTD_DLL group15_flat_table
{
public:
    typedef TypePolicy                          type_policy;
    typedef std::size_t                         size_type;
    typedef std::intptr_t                       ssize_type;
    typedef std::ptrdiff_t                      difference_type;

    typedef typename type_policy::key_type      key_type;
    typedef typename type_policy::mapped_type   mapped_type;
    typedef typename type_policy::value_type    value_type;
    typedef typename type_policy::init_type     init_type;
    typedef typename type_policy::element_type  element_type;
    typedef Hash                                hasher;
    typedef KeyEqual                            key_equal;
    typedef Allocator                           allocator_type;
    //typedef typename Hash::result_type          hash_result_t;
    typedef std::size_t                         hash_result_t;

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;

    typedef typename std::allocator_traits<allocator_type>::pointer         pointer;
    typedef typename std::allocator_traits<allocator_type>::const_pointer   const_pointer;

    using this_type = group15_flat_table<TypePolicy, Hash, KeyEqual, Allocator>;

    static constexpr bool kUseIndexSalt = false;
    static constexpr bool kEnableExchange = true;

    static constexpr bool kIsTransparent = (jstd::is_transparent<Hash>::value && jstd::is_transparent<KeyEqual>::value);
    static constexpr bool kIsLayoutCompatible = jstd::is_layout_compatible_kv<key_type, mapped_type>::value;

    static constexpr size_type npos = static_cast<size_type>(-1);

    using ctrl_type = group15_meta_ctrl;
    using group_type = flat_map_group15<group15_meta_ctrl>;
    using prober_type = group_quadratic_prober;

    static constexpr const std::uint8_t kEmptySlot    = ctrl_type::kEmptySlot;
    static constexpr const std::uint8_t kSentinelSlot = ctrl_type::kSentinelSlot;
    static constexpr const std::uint8_t kEmptyHash    = ctrl_type::kEmptyHash;
    static constexpr const std::uint8_t kSentinelHash = ctrl_type::kSentinelHash;

    static constexpr const size_type kGroupSize  = group_type::kGroupSize;
    static constexpr const size_type kGroupWidth = group_type::kGroupWidth;

    static constexpr bool kIsPlainKey    = jstd::is_plain_type<key_type>::value;
    static constexpr bool kIsPlainMapped = jstd::is_plain_type<mapped_type>::value;

    static constexpr bool kIsPlainKV = kIsPlainKey && kIsPlainMapped;

    static constexpr bool kHasSwapKey    = jstd::has_member_swap<key_type, key_type &>::value;
    static constexpr bool kHasSwapMapped = jstd::has_member_swap<mapped_type, mapped_type &>::value;

    static constexpr bool kIsSwappableKey    = jstd::is_swappable<key_type>::value;
    static constexpr bool kIsSwappableMapped = jstd::is_swappable<mapped_type>::value;

    static constexpr bool kIsSwappableKV = kIsSwappableKey && kIsSwappableMapped;

    static constexpr bool kIsMoveAssignKey    = std::is_move_assignable<key_type>::value;
    static constexpr bool kIsMoveAssignMapped = std::is_move_assignable<mapped_type>::value;

    static constexpr bool is_slot_trivial_copyable =
            (std::is_trivially_copyable<value_type>::value ||
            (std::is_trivially_copyable<key_type>::value &&
             std::is_trivially_copyable<mapped_type>::value) ||
            (std::is_scalar<key_type>::value && std::is_scalar<mapped_type>::value));

    static constexpr bool is_slot_trivial_destructor =
            (std::is_trivially_destructible<value_type>::value ||
            (std::is_trivially_destructible<key_type>::value &&
             std::is_trivially_destructible<mapped_type>::value) ||
            (jstd::is_plain_type<key_type>::value &&
             jstd::is_plain_type<mapped_type>::value));

    static constexpr size_type kWordLength = sizeof(std::size_t) * CHAR_BIT;
    static constexpr size_type kSizeTypeLength = sizeof(std::size_t);

    static constexpr bool kIsSmallKeyType   = (sizeof(key_type)    <= kSizeTypeLength * 2);
    static constexpr bool kIsSmallValueType = (sizeof(mapped_type) <= kSizeTypeLength * 2);

    static constexpr bool kDetectIsIndirectKey = !(jstd::is_plain_type<key_type>::value ||
                                                  (sizeof(key_type) <= kSizeTypeLength * 2) ||
                                                 ((sizeof(key_type) <= (kSizeTypeLength * 4)) &&
                                                   is_slot_trivial_destructor));

    static constexpr bool kDetectIsIndirectValue = !(jstd::is_plain_type<mapped_type>::value ||
                                                    (sizeof(mapped_type) <= kSizeTypeLength * 2) ||
                                                   ((sizeof(mapped_type) <= (kSizeTypeLength * 4)) &&
                                                     is_slot_trivial_destructor));

    static constexpr bool kIsIndirectKey = false;
    static constexpr bool kIsIndirectValue = false;
    static constexpr bool kIsIndirectKV = kIsIndirectKey | kIsIndirectValue;
    static constexpr bool kNeedStoreHash = true;

    using slot_type = map_slot_type<key_type, mapped_type>;
    using slot_policy_t = flat_map_slot_policy<slot_type>;
    using SlotPolicyTraits = slot_policy_traits<slot_policy_t>;

    static constexpr size_type kCacheLineSize = 64;
    static constexpr size_type kGroupAlignment_ = compile_time::cmax<sizeof(group_type), alignof(group_type)>::value;
    static constexpr size_type kGroupAlignment  = compile_time::is_pow2<kGroupAlignment_>::value ?
                                                  kGroupAlignment_ :
                                                  compile_time::round_up_pow2<kGroupAlignment_>::value;
    static constexpr size_type kSlotAlignment_ = compile_time::cmax<sizeof(slot_type), alignof(slot_type)>::value;
    static constexpr size_type kSlotAlignment  = compile_time::is_pow2<kSlotAlignment_>::value ?
                                                 kSlotAlignment_ :
                                                 compile_time::round_up_pow2<kSlotAlignment_>::value;

    using iterator       = flat_map_iterator15<this_type, value_type, kIsIndirectKV>;
    using const_iterator = flat_map_iterator15<this_type, const value_type, kIsIndirectKV>;

    using locator_t = flat_map_locator15<this_type, kIsIndirectKV>;

    static constexpr size_type kDefaultCapacity = 0;
    // kMinCapacity must be >= (kGroupWidth * 2)
    static constexpr size_type kMinCapacity = kGroupWidth * 2;

    /* When capacity is small, we allow 100% usage. */
    /* Due to the use of quadratic prober, a maximum of 2 can only be obtained here. */
    static constexpr size_type kSmallCapacity = kGroupWidth * 2;

    static constexpr float kMinLoadFactorF = 0.5f;
    static constexpr float kMaxLoadFactorF = 0.875f;
    static constexpr float kDefaultLoadFactorF = 0.875f;
    // Default load factor = 224 / 256 = 0.875
    static constexpr size_type kLoadFactorAmplify = 256;
    static constexpr size_type kDefaultMaxLoadFactor =
        static_cast<size_type>((double)kLoadFactorAmplify * (double)kDefaultLoadFactorF + 0.5);

    static constexpr size_type kSkipGroupsLimit = 5;

    using group_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<group_type>;
    using slot_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<slot_type>;

    using AllocTraits = std::allocator_traits<allocator_type>;

    using GroupAllocTraits = typename std::allocator_traits<allocator_type>::template rebind_traits<group_type>;
    using SlotAllocTraits = typename std::allocator_traits<allocator_type>::template rebind_traits<slot_type>;

    using hash_policy_t = typename hash_policy_selector<Hash>::type;

private:
    group_type *    groups_;
    slot_type *     slots_;
    size_type       slot_size_;
    size_type       slot_mask_;         // slot_mask_ = ctrl_capacity - 1    
    size_type       slot_threshold_;
    size_type       slot_capacity_;     // slot_capacity = ctrl_capacity / kGroupWidth * kGroupSize
    size_type       group_mask_;        // Use in class group_quadratic_prober
#if GROUP15_USE_INDEX_SHIFT
    size_type       index_shift_;
#endif
    size_type       mlf_;
#if GROUP15_USE_SEPARATE_SLOTS
    group_type *    groups_alloc_;
#endif
#if GROUP15_USE_HASH_POLICY
    hash_policy_t   hash_policy_;
#endif

    hasher                  hasher_;
    key_equal               key_equal_;

    allocator_type          allocator_;
    group_allocator_type    group_allocator_;
    slot_allocator_type     slot_allocator_;

    static constexpr bool kIsKeyExists = false;
    static constexpr bool kNeedInsert = true;

public:
    group15_flat_table() : group15_flat_table(kDefaultCapacity) {}

    explicit group15_flat_table(size_type capacity, hasher const & hash = hasher(),
                                key_equal const & pred = key_equal(),
                                allocator_type const & allocator = allocator_type())
        : groups_(default_empty_groups()), slots_(nullptr),
          slot_size_(0),
          slot_mask_(size_type(-1)),
          slot_threshold_(0),
          slot_capacity_(0),
          group_mask_(0),
#if GROUP15_USE_INDEX_SHIFT
          index_shift_(kWordLength - 1),
#endif
          mlf_(kDefaultMaxLoadFactor),
#if GROUP15_USE_SEPARATE_SLOTS
          groups_alloc_(nullptr),
#endif
#if GROUP15_USE_HASH_POLICY
          hash_policy_(),
#endif
          hasher_(hash), key_equal_(pred),
          allocator_(allocator), group_allocator_(allocator), slot_allocator_(allocator)
    {
        if (capacity != 0) {
            this->reserve_for_insert(capacity);
        }
    }

    group15_flat_table(group15_flat_table const & other)
        : group15_flat_table(other, std::allocator_traits<allocator_type>::
                                    select_on_container_copy_construction(other.get_allocator_ref())) {
    }

    group15_flat_table(group15_flat_table const & other, allocator_type const & allocator) :
        groups_(default_empty_groups()), slots_(nullptr),
        slot_size_(0),
        slot_mask_(size_type(-1)),
        slot_threshold_(0),
        slot_capacity_(0),
        group_mask_(0),
#if GROUP15_USE_INDEX_SHIFT
        index_shift_(kWordLength - 1),
#endif
        mlf_(other.mlf_),
#if GROUP15_USE_SEPARATE_SLOTS
        groups_alloc_(nullptr),
#endif
#if GROUP15_USE_HASH_POLICY
        hash_policy_(),
#endif
        hasher_(other.hash_function_ref()), key_equal_(other.key_eq_ref()),
        allocator_(allocator), group_allocator_(allocator), slot_allocator_(allocator)
    {
        // Prepare enough space to ensure that no expansion is required during the insertion process.
        size_type other_size = other.size();
        if (other_size != 0) {
            this->reserve_for_insert(other_size);
            this->copy_slots_from(other);
        }
    }

    group15_flat_table(group15_flat_table && other) noexcept(
            std::is_nothrow_copy_constructible<hasher>::value &&
            std::is_nothrow_copy_constructible<key_equal>::value &&
            std::is_nothrow_copy_constructible<allocator_type>::value) :
        groups_(jstd::exchange(other.groups_, this_type::default_empty_groups())),
        slots_(jstd::exchange(other.slots_, nullptr)),
        slot_size_(jstd::exchange(other.slot_size_, 0)),
        slot_mask_(jstd::exchange(other.slot_mask_, size_type(-1))),
        slot_threshold_(jstd::exchange(other.slot_threshold_, 0)),
        slot_capacity_(jstd::exchange(other.slot_capacity_, 0)),
        group_mask_(jstd::exchange(other.group_mask_, 0)),
#if GROUP15_USE_INDEX_SHIFT
        index_shift_(jstd::exchange(other.index_shift_, kWordLength - 1)),
#endif
        mlf_(jstd::exchange(other.mlf_, kDefaultMaxLoadFactor)),
#if GROUP15_USE_SEPARATE_SLOTS
        groups_alloc_(jstd::exchange(other.groups_alloc_, nullptr)),
#endif
#if GROUP15_USE_HASH_POLICY
        hash_policy_(jstd::exchange(other.hash_policy_ref(), hash_policy_t())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(std::move(other.get_allocator_ref())),
        group_allocator_(std::move(other.get_ctrl_allocator_ref())),
        slot_allocator_(std::move(other.get_slot_allocator_ref())) {
    }

    group15_flat_table(group15_flat_table && other, allocator_type const & allocator) :
        groups_(default_empty_groups()), slots_(nullptr),
        slot_size_(0),
        slot_mask_(size_type(-1)),
        slot_threshold_(0),
        slot_capacity_(0),
        group_mask_(0),
#if GROUP15_USE_INDEX_SHIFT
        index_shift_(kWordLength - 1),
#endif
        mlf_(kDefaultMaxLoadFactor),
#if GROUP15_USE_SEPARATE_SLOTS
        groups_alloc_(nullptr),
#endif
#if GROUP15_USE_HASH_POLICY
        hash_policy_(std::move(other.hash_policy_ref())),
#endif
        hasher_(std::move(other.hash_function_ref())),
        key_equal_(std::move(other.key_eq_ref())),
        allocator_(allocator), group_allocator_(allocator), slot_allocator_(allocator) {
        if (this->get_allocator_ref() == other.get_allocator_ref()) {
            // Swap content only
            this->swap_content(other);
        } else {
            // Prepare enough space to ensure that no expansion is required during the insertion process.
            size_type other_size = other.size();
            if (other_size > 0) {
                this->reserve_for_insert(other_size);
                // Here we will move elements of [other] hashmap to this hashmap.
                this->move_slots_from(other);
            }
            // Destroy all data but needn't clear_slots().
            other.destroy<false>();
        }
    }

    ~group15_flat_table() {
        this->destroy<true>();
    }

    group15_flat_table & operator = (const group15_flat_table & other) {
        group15_flat_table tmp(other,
                           AllocTraits::propagate_on_container_copy_assignment::value
                           ? other.get_allocator_ref()
                           : this->get_allocator_ref());
        this->swap_impl(tmp);
        return *this;
    }

    group15_flat_table & operator = (group15_flat_table && other) noexcept(
        AllocTraits::is_always_equal::value &&
        std::is_nothrow_move_assignable<hasher>::value &&
        std::is_nothrow_move_assignable<key_equal>::value) {
        // TODO: We should only use the operations from the noexcept clause
        // to make sure we actually adhere to other contract.
        return this->move_assign(std::move(other),
                        typename AllocTraits::propagate_on_container_move_assignment());
    }

    ///
    /// Observers
    ///
    hasher hash_function() const noexcept {
        return this->hasher_;
    }

    key_equal key_eq() const noexcept {
        return this->key_equal_;
    }

    allocator_type get_allocator() const noexcept {
        return this->allocator_;
    }

    group_allocator_type get_group_allocator() noexcept {
        return this->group_allocator_;
    }

    slot_allocator_type get_slot_allocator() const noexcept {
        return this->slot_allocator_;
    }

    hasher & hash_function_ref() noexcept {
        return this->hasher_;
    }

    const hasher & hash_function_ref() const noexcept {
        return this->hasher_;
    }

    key_equal & key_eq_ref() noexcept {
        return this->key_equal_;
    }

    const key_equal & key_eq_ref() const noexcept {
        return this->key_equal_;
    }

#if GROUP15_USE_HASH_POLICY
    hash_policy_t & hash_policy_ref() noexcept {
        return this->hash_policy_;
    }

    const hash_policy_t & hash_policy_ref() const noexcept {
        return this->hash_policy_;
    }
#endif

    allocator_type & get_allocator_ref() noexcept {
        return this->allocator_;
    }

    group_allocator_type & get_group_allocator_ref() noexcept {
        return this->group_allocator_;
    }

    slot_allocator_type & get_slot_allocator_ref() noexcept {
        return this->slot_allocator_;
    }

    const allocator_type & get_allocator_ref() const noexcept {
        return this->allocator_;
    }

    const group_allocator_type & get_group_allocator_ref() const noexcept {
        return this->group_allocator_;
    }

    const slot_allocator_type & get_slot_allocator_ref() const noexcept {
        return this->slot_allocator_;
    }

    static const char * name() noexcept {
        return "jstd::group15_flat_map";
    }

    ///
    /// Iterators
    ///
    iterator begin() noexcept {
        locator_t locator = this->find_first_used_index();
        return { locator };
    }

    iterator end() noexcept {
        return { };
    }

    const_iterator begin() const noexcept {
        return const_cast<this_type *>(this)->begin();
    }
    const_iterator end() const noexcept {
        return const_cast<this_type *>(this)->end();
    }

    const_iterator cbegin() const noexcept { return this->begin(); }
    const_iterator cend() const noexcept { return this->end(); }

    ///
    /// Capacity
    ///
    bool empty() const noexcept { return (this->size() == 0); }
    size_type size() const noexcept { return this->slot_size(); }
    size_type capacity() const noexcept { return this->slot_capacity(); }
    size_type max_size() const noexcept {
        return (std::numeric_limits<difference_type>::max)() / sizeof(value_type);
    }

    size_type slot_size() const noexcept { return this->slot_size_; }
    size_type slot_mask() const noexcept { return this->slot_mask_; }
    size_type slot_threshold() const noexcept { return this->slot_threshold_; }
    size_type slot_capacity() const noexcept {
        //return calc_slot_capacity(this->group_capacity(), this->ctrl_capacity());
        return this->slot_capacity_;
    }

    size_type ctrl_capacity() const noexcept { return (this->slot_mask_ + 1); }
    size_type max_ctrl_capacity() const noexcept { return (this->group_capacity() * kGroupWidth); }

    size_type group_mask() const noexcept {
        return this->group_mask_;
    }
    size_type group_capacity() const noexcept {
        return (this->group_mask_ + 1);
    }

    bool is_valid() const noexcept { return (this->groups() != nullptr); }
    bool is_empty() const noexcept { return (this->size() == 0); }

    ///
    /// Bucket interface
    ///
    size_type bucket_size(size_type n) const noexcept { return 1; }
    size_type bucket_count() const noexcept { return this->slot_capacity(); }
    size_type max_bucket_count() const noexcept {
        return (std::numeric_limits<difference_type>::max)() / sizeof(ctrl_type);
    }

    size_type bucket(const key_type & key) const {
        size_type ctrl_index = this->find_impl(key);
        return ctrl_index;
    }

    ///
    /// Hash policy
    ///
    float load_factor() const {
        if (this->slot_capacity() != 0)
            return ((float)this->slot_size() / this->slot_capacity());
        else
            return 0.0;
    }

    float max_load_factor() const {
        return ((float)this->mlf_ / kLoadFactorAmplify);
    }

    void max_load_factor(float mlf) {
        // mlf: [0.5, 0.875]
        if (mlf < kMinLoadFactorF)
            mlf = kMinLoadFactorF;
        if (mlf > kMaxLoadFactorF)
            mlf = kMaxLoadFactorF;
        size_type mlf_int = static_cast<size_type>((float)kLoadFactorAmplify * mlf);
        this->mlf_ = mlf_int;

        size_type new_capacity = this->shrink_to_fit_capacity(this->ctrl_capacity());
        size_type new_ctrl_capacity = this->calc_capacity(new_capacity);
        if (new_ctrl_capacity > this->ctrl_capacity()) {
            this->rehash(new_ctrl_capacity);
        }
    }

    ///
    /// Pointers
    ///
    group_type * groups() { return this->groups_; }
    const group_type * groups() const {
        return const_cast<const group_type *>(this->groups_);
    }

#if GROUP15_USE_SEPARATE_SLOTS
    group_type * groups_alloc() { return this->groups_alloc_; }
    const group_type * groups_alloc() const {
        return const_cast<const group_type *>(this->groups_alloc_);
    }
#else
    group_type * groups_alloc() { return nullptr; }
    const group_type * groups_alloc() const {
        return nullptr;
    }
#endif

    group_type * last_group() {
        return (this->groups() + this->group_capacity());
    }
    const group_type * last_group() const {
        return (this->groups() + this->group_capacity());
    }

    ctrl_type * ctrls() {
        return reinterpret_cast<ctrl_type *>(this->groups());
    }
    const ctrl_type * ctrls() const {
        return reinterpret_cast<const ctrl_type *>(this->groups());
    }

    ctrl_type * last_ctrl() {
        return (this->ctrls() + this->ctrl_capacity());
    }
    const ctrl_type * last_ctrl() const {
        return (this->ctrls() + this->ctrl_capacity());
    }

    slot_type * slots() { return this->slots_; }
    const slot_type * slots() const {
        return const_cast<const slot_type *>(this->slots_);
    }

    slot_type * last_slot() {
        if (!kIsIndirectKV)
            return (this->slots() + this->slot_capacity());
        else
            return (this->slots() + this->slot_size());
    }
    const slot_type * last_slot() const {
        if (!kIsIndirectKV)
            return (this->slots() + this->slot_capacity());
        else
            return (this->slots() + this->slot_size());
    }

    ///
    /// Hash policy
    ///

    //
    // See: https://en.cppreference.com/w/cpp/container/unordered_map/reserve
    //
    // Sets the number of buckets to the number needed to accommodate
    // at least count elements without exceeding maximum load factor
    // and rehashes the container, i.e. puts the elements into
    // appropriate buckets considering that total number of buckets has changed.
    // Effectively calls rehash(std::ceil(new_capacity / max_load_factor())).
    //
    JSTD_FORCED_INLINE
    void reserve(size_type new_capacity) {
        if (JSTD_LIKELY(new_capacity != 0)) {
            new_capacity = this->shrink_to_fit_capacity(new_capacity);
            this->rehash_impl<false>(new_capacity);
        } else {
            this->destroy<true>();
        }
    }

    JSTD_FORCED_INLINE
    void reserve_for_insert(size_type init_capacity) {
        assert(init_capacity > 0);
        size_type new_capacity = this->shrink_to_fit_capacity(init_capacity);
        new_capacity = this->calc_capacity(new_capacity);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);
        this->create_slots<true>(new_capacity);
    }

    //
    // See: https://en.cppreference.com/w/cpp/container/unordered_map/rehash
    //
    // Changes the number of buckets to a value n that is not less than count
    // and satisfies n >= std::ceil(size() / max_load_factor()), then rehashes the container,
    // i.e. puts the elements into appropriate buckets considering that
    // total number of buckets has changed.
    //
    JSTD_FORCED_INLINE
    void rehash(size_type new_capacity) {
        size_type fit_to_now = this->shrink_to_fit_capacity(this->size());
        new_capacity = (std::max)(fit_to_now, new_capacity);
        if (JSTD_LIKELY(new_capacity != 0)) {
            this->rehash_impl<true>(new_capacity);
        } else {
            this->destroy<true>();
        }
    }

    JSTD_FORCED_INLINE
    void shrink_to_fit(bool read_only = false) {
        size_type new_capacity;
        if (JSTD_LIKELY(!read_only))
            new_capacity = this->shrink_to_fit_capacity(this->size());
        else
            new_capacity = this->size();
        if (JSTD_LIKELY(new_capacity != 0)) {
            this->rehash_impl<true>(new_capacity);
        } else {
            this->destroy<true>();
        }
    }

    ///
    /// Lookup
    ///
    JSTD_FORCED_INLINE
    size_type count(const key_type & key) const {
        locator_t locator = this->find_impl(key);
        return (locator.slot() != nullptr) ? 1 : 0;
    }

    JSTD_FORCED_INLINE
    bool contains(const key_type & key) const {
        locator_t locator = this->find_impl(key);
        return (locator.slot() != nullptr);
    }

    ///
    /// find(key)
    ///
    JSTD_FORCED_INLINE
    iterator find(const key_type & key) {
        return make_iterator(this->find_impl(key));
    }

    JSTD_FORCED_INLINE
    const_iterator find(const key_type & key) const {
        return const_cast<this_type *>(this)->find(key);
    }

    template <typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, key_type>::value) &&
                std::is_constructible<key_type, const KeyT &>::value>::type * = nullptr>
    JSTD_FORCED_INLINE
    iterator find(const KeyT & key_t) {
        key_type key(key_t);
        return make_iterator(this->find_impl(key));
    }

    template <typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, key_type>::value) &&
                std::is_constructible<key_type, const KeyT &>::value>::type * = nullptr>
    JSTD_FORCED_INLINE
    const_iterator find(const KeyT & key) const {
        return const_cast<this_type *>(this)->find(key);
    }

    ///
    /// Modifiers
    ///
    JSTD_FORCED_INLINE
    void clear(bool need_destroy = false) noexcept {
        if (!need_destroy) {
            this->clear_data();
        } else {
            this->destroy<true>();
        }
        assert(this->slot_size() == 0);
    }

    ///
    /// insert(value)
    ///
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(const value_type & value) {
        return this->emplace_impl<false>(value);
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(value_type && value) {
        return this->emplace_impl<false>(std::move(value));
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(const init_type & value) {
        return this->emplace_impl<false>(value);
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(init_type && value) {
        return this->emplace_impl<false>(std::move(value));
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value &&
               !jstd::is_same_ex<P, init_type >::value) &&
               (std::is_constructible<value_type, P &&>::value ||
                std::is_constructible<init_type,  P &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(P && value) {
        return this->emplace_impl<false>(std::forward<P>(value));
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, const value_type & value) {
        return this->emplace_impl<false>(value).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, value_type && value) {
        return this->emplace_impl<false>(std::move(value)).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, const init_type & value) {
        return this->emplace_impl<false>(value).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, init_type && value) {
        return this->emplace_impl<false>(std::move(value)).first;
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value &&
               !jstd::is_same_ex<P, init_type >::value) &&
               (std::is_constructible<value_type, P &&>::value ||
                std::is_constructible<init_type,  P &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, P && value) {
        return this->emplace_impl<false>(std::forward<P>(value)).first;
    }

    template <typename InputIter>
    JSTD_FORCED_INLINE
    void insert(InputIter first, InputIter last) {
        for (; first != last; ++first) {
            this->emplace_impl<false>(*first);
        }
    }

    void insert(std::initializer_list<value_type> ilist) {
        this->insert(ilist.begin(), ilist.end());
    }

    ///
    /// insert_or_assign(key, value)
    ///
    template <typename MappedT>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert_or_assign(const key_type & key, MappedT && value) {
        return this->emplace_impl<true>(key, std::forward<MappedT>(value));
    }

    template <typename MappedT>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert_or_assign(key_type && key, MappedT && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedT>(value));
    }

    template <typename KeyT, typename MappedT>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert_or_assign(KeyT && key, MappedT && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedT>(value));
    }

    template <typename MappedT>
    JSTD_FORCED_INLINE
    iterator insert_or_assign(const_iterator hint, const key_type & key, MappedT && value) {
        return this->emplace_impl<true>(key, std::forward<MappedT>(value))->first;
    }

    template <typename MappedT>
    JSTD_FORCED_INLINE
    iterator insert_or_assign(const_iterator hint, key_type && key, MappedT && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedT>(value))->first;
    }

    template <typename KeyT, typename MappedT>
    JSTD_FORCED_INLINE
    iterator insert_or_assign(const_iterator hint, KeyT && key, MappedT && value) {
        return this->emplace_impl<true>(std::move(key), std::forward<MappedT>(value))->first;
    }

    ///
    /// emplace(args...)
    ///
    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace(Args && ... args) {
        return this->emplace_impl<false>(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    iterator emplace_hint(const_iterator hint, Args && ... args) {
        return this->emplace_impl<false>(std::forward<Args>(args)...).first;
    }

    ///
    /// try_emplace(key, args...)
    ///
    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(const key_type & key, Args && ... args) {
        return this->try_emplace_impl(key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(key_type && key, Args && ... args) {
        return this->try_emplace_impl(std::move(key), std::forward<Args>(args)...);
    }

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(KeyT && key, Args && ... args) {
        return this->try_emplace_impl(std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(const_iterator hint, const key_type & key, Args && ... args) {
        return this->try_emplace_impl(key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(const_iterator hint, key_type && key, Args && ... args) {
        return this->try_emplace_impl(std::move(key), std::forward<Args>(args)...);
    }

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(const_iterator hint, KeyT && key, Args && ... args) {
        return this->try_emplace_impl(std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    ///
    /// erase(key)
    ///
    JSTD_FORCED_INLINE
    size_type erase(const key_type & key) {
        size_type num_deleted = this->find_and_erase(key);
        return num_deleted;
    }

    JSTD_FORCED_INLINE
    iterator erase(iterator pos) {
        locator_t & locator = pos.locator();
        this->erase_slot(locator);
        locator.increment();
        return { locator };
    }

    JSTD_FORCED_INLINE
    iterator erase(const_iterator pos) {
        return this->erase(iterator(pos));
    }

    JSTD_FORCED_INLINE
    void swap(this_type & other) {
        if (std::addressof(other) != this) {
            this->swap_impl(other);
        }
    }

    JSTD_FORCED_INLINE
    friend void swap(this_type & lhs, this_type & rhs)
        noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    ///
    /// For iterator
    ///
    JSTD_FORCED_INLINE
    ctrl_type * ctrl_at(size_type ctrl_index) noexcept {
        assert(ctrl_index <= this->ctrl_capacity());
        return (this->ctrls() + std::ptrdiff_t(ctrl_index));
    }

    JSTD_FORCED_INLINE
    const ctrl_type * ctrl_at(size_type ctrl_index) const noexcept {
        assert(ctrl_index <= this->ctrl_capacity());
        return (this->ctrls() + std::ptrdiff_t(ctrl_index));
    }

    JSTD_FORCED_INLINE
    group_type * group_at(size_type group_index) noexcept {
        assert(group_index <= this->group_capacity());
        group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (group_start + std::ptrdiff_t(group_index));
    }

    JSTD_FORCED_INLINE
    const group_type * group_at(size_type group_index) const noexcept {
        assert(group_index <= this->group_capacity());
        const group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (this->groups() + std::ptrdiff_t(group_index));
    }

    inline group_type * group_by_ctrl_index(size_type ctrl_index) noexcept {
        assert(ctrl_index <= this->ctrl_capacity());
        size_type group_index = ctrl_index / kGroupWidth;
        group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (group_start + std::ptrdiff_t(group_index));
    }

    inline const group_type * group_by_ctrl_index(size_type ctrl_index) const noexcept {
        assert(ctrl_index <= this->ctrl_capacity());
        size_type group_index = ctrl_index / kGroupWidth;
        const group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (group_start + std::ptrdiff_t(group_index));
    }

    inline group_type * group_by_slot_index(size_type slot_index) noexcept {
        assert(slot_index <= this->slot_capacity());
        size_type group_index = slot_index / kGroupSize;
        group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (group_start + std::ptrdiff_t(group_index));
    }

    inline const group_type * group_by_slot_index(size_type slot_index) const noexcept {
        assert(slot_index <= this->slot_capacity());
        size_type group_index = slot_index / kGroupSize;
        const group_type * group_start = this->groups();
        JSTD_ASSUME(group_start != nullptr);
        return (group_start + std::ptrdiff_t(group_index));
    }

    JSTD_FORCED_INLINE
    slot_type * slot_at(size_type slot_index) noexcept {
        assert(slot_index <= this->slot_capacity());
        slot_type * slot_start = this->slots();
        JSTD_ASSUME(slot_start != nullptr);
        return (slot_start + std::ptrdiff_t(slot_index));
    }

    JSTD_FORCED_INLINE
    const slot_type * slot_at(size_type slot_index) const noexcept {
        assert(slot_index <= this->slot_capacity());
        const slot_type * slot_start = this->slots();
        JSTD_ASSUME(slot_start != nullptr);
        return (slot_start + std::ptrdiff_t(slot_index));
    }

    JSTD_FORCED_INLINE
    locator_t find_first_used_index() const {
        if (JSTD_LIKELY(this->size() != 0)) {
            const group_type * group = this->groups();
            const group_type * last_group = this->last_group();
            size_type slot_base_index = 0;
            for (; group < last_group; ++group) {
                std::uint32_t used_mask = group->match_used();
                if (JSTD_LIKELY(used_mask != 0)) {
                    std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                    if (JSTD_LIKELY(!group->is_sentinel(used_pos))) {
                        size_type slot_index = slot_base_index + used_pos;
                        const slot_type * slot = this->slot_at(slot_index);
                        return { group, used_pos, slot };
                    } else {
                        return {};
                    }
                }
                slot_base_index += kGroupSize;
            }
        }
        return {};
    }

    JSTD_FORCED_INLINE
    size_type skip_empty_slots(size_type start_ctrl_index) const {
        if (this->size() != 0) {
            start_ctrl_index++;
            if (JSTD_UNLIKELY(start_ctrl_index >= this->ctrl_capacity()))
                return this->ctrl_capacity();
            const group_type * group = this->group_by_ctrl_index(start_ctrl_index);
            const group_type * last_group = this->last_group();
            size_type ctrl_pos = start_ctrl_index % kGroupWidth;
            size_type ctrl_base_index = start_ctrl_index - ctrl_pos;
            // Last 4 items use ctrl seek, maybe faster.
            static const size_type kCtrlFasterSeekPos = 6;
            if (JSTD_UNLIKELY(ctrl_pos > (kGroupWidth - kCtrlFasterSeekPos))) {
                if (group < last_group) {
                    std::uint32_t used_mask = group->match_used();
                    // Filter out the bits in the leading position
                    // std::uint32_t non_excluded_mask = ~((std::uint32_t(1) << std::uint32_t(slot_pos)) - 1);
                    std::uint32_t non_excluded_mask = (std::uint32_t(0xFFFFFFFFu) << std::uint32_t(ctrl_pos));
                    used_mask &= non_excluded_mask;
                    if (JSTD_LIKELY(used_mask != 0)) {
                        std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                        size_type slot_index = ctrl_base_index + used_pos;
                        return slot_index;
                    }
                }
            } else {
                size_type last_index = ctrl_base_index + kGroupWidth;
                const ctrl_type * ctrl = this->ctrl_at(start_ctrl_index);
                while (start_ctrl_index < last_index) {
                    if (ctrl->is_used()) {
                        return start_ctrl_index;
                    }
                    ++ctrl;
                    ++start_ctrl_index;
                }
            }
            ctrl_base_index += kGroupWidth;
            group++;
            for (; group < last_group; ++group) {
                std::uint32_t used_mask = group->match_used();
                if (JSTD_LIKELY(used_mask != 0)) {
                    std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                    size_type ctrl_index = ctrl_base_index + used_pos;
                    return ctrl_index;
                }
                ctrl_base_index += kGroupWidth;
            }
        }
        return this->ctrl_capacity();
    }

private:
    static inline group_type * default_empty_groups() noexcept {
        alignas(16) static const ctrl_type s_empty_ctrls[kGroupWidth * 2] = {
            // Group 0
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kSentinelSlot }, { 0 },

            // Group 1
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kEmptySlot }, { kEmptySlot },
            { kEmptySlot }, { kEmptySlot }, { kSentinelSlot }, { 0 },
        };

        return reinterpret_cast<group_type *>(const_cast<ctrl_type *>(&s_empty_ctrls[0]));
    }

    static inline ctrl_type * default_empty_ctrls() noexcept {
        return reinterpret_cast<ctrl_type *>(this_type::default_empty_groups());
    }

    JSTD_FORCED_INLINE
    size_type calc_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = (std::max)(init_capacity, kMinCapacity);
                  new_capacity = (std::max)(new_capacity, this->slot_size());
        if (!pow2::is_pow2(new_capacity)) {
            new_capacity = pow2::round_up<size_type, kMinCapacity>(new_capacity);
        }
        return new_capacity;
    }

    static inline constexpr size_type round_up_pow2(size_type capacity) noexcept {
        if (!pow2::is_pow2(capacity)) {
            capacity = pow2::round_up<size_type, 0>(capacity);
        }
        return capacity;
    }

    static inline constexpr size_type calc_slot_index(size_type ctrl_index) noexcept {
        size_type group_index = ctrl_index / kGroupWidth;
        size_type group_pos = ctrl_index % kGroupWidth;
        assert(group_index != this->group_capacity());
        assert(group_pos != kGroupSize);
        return (group_index * kGroupSize + group_pos);
    }

    static inline constexpr size_type calc_slot_capacity(size_type group_capacity, size_type init_capacity) noexcept {
        // Exclude a sentinel mark
        return (init_capacity >= kGroupWidth) ? (group_capacity * kGroupSize - 1) : init_capacity;
    }

    static inline constexpr size_type calc_slot_threshold(size_type mlf, size_type slot_capacity) noexcept {
        /* When capacity is small, we allow 100% usage. */
        return (slot_capacity > kSmallCapacity) ? (slot_capacity * mlf / kLoadFactorAmplify) : slot_capacity;
    }

    static inline constexpr size_type calc_index_shift(size_type capacity) noexcept {
        assert(jstd::pow2::is_pow2(capacity));
#if GROUP15_USE_INDEX_SHIFT
        return (kWordLength - (((capacity / kGroupWidth) <= 2) ? 1 : BitUtils::bsr((capacity / kGroupWidth))));
#else
        return (kWordLength - ((capacity <= 2) ? 1 : BitUtils::bsr(capacity)));
#endif
    }

    inline size_type calc_slot_threshold(size_type slot_capacity) const noexcept {
        return this_type::calc_slot_threshold(this->mlf_, slot_capacity);
    }

    static inline size_type calc_group_capacity(size_type capacity) noexcept {
        assert(pow2::is_pow2(capacity));
        size_type group_capacity = (capacity + (kGroupWidth - 1)) / kGroupWidth;
        assert(pow2::is_pow2(group_capacity));
        return group_capacity;
    }

    static inline size_type calc_group_mask(size_type capacity) noexcept {
        size_type group_capacity = this_type::calc_group_capacity(capacity);
        return static_cast<size_type>(group_capacity - 1);
    }

    static inline size_type calc_group_mask(size_type group_capacity, size_type capacity) noexcept {
        JSTD_UNUSED(capacity);
        return static_cast<size_type>(group_capacity - 1);
    }

    static inline constexpr size_type calc_index_shift_round(size_type capacity) noexcept {
        capacity = this_type::round_up_pow2(capacity);
        return this_type::calc_index_shift(capacity);
    }

    static inline constexpr size_type calc_slot_mask_round(size_type capacity) noexcept {
        capacity = this_type::round_up_pow2(capacity);
        return static_cast<size_type>(capacity - 1);
    }

    static inline constexpr size_type calc_slot_threshold_round(size_type capacity) noexcept {
        capacity = this_type::round_up_pow2(capacity);
        return this_type::calc_slot_threshold(kDefaultMaxLoadFactor, capacity);
    }

    static inline constexpr size_type calc_group_mask_round(size_type capacity) noexcept {
        capacity = this_type::round_up_pow2(capacity);
        return this_type::calc_group_mask(capacity);
    }

    inline size_type shrink_to_fit_capacity(size_type init_capacity) const noexcept {
        size_type new_capacity = init_capacity * kLoadFactorAmplify / this->mlf_;
        return new_capacity;
    }

    static inline bool is_positive(size_type value) noexcept {
        return (static_cast<intptr_t>(value) >= 0);
    }

    inline iterator iterator_at(size_type ctrl_index, size_type slot_index) noexcept {
        if (!kIsIndirectKV) {
#if ITERATOR15_USE_LOCATOR15
            return { this->group_at(ctrl_index / kGroupWidth),
                     (ctrl_index % kGroupWidth),
                     this->slot_at(slot_index) };
#else
            return { this->ctrl_at(ctrl_index), this->slot_at(slot_index) };
#endif
        } else {
            return { this->slot_at(index) };
        }
    }

    inline const_iterator iterator_at(size_type ctrl_index, size_type slot_index) const noexcept {
        if (!kIsIndirectKV) {
#if ITERATOR15_USE_LOCATOR15
            return { this->group_at(ctrl_index / kGroupWidth),
                     (ctrl_index % kGroupWidth),
                     this->slot_at(slot_index) };
#else
            return { this->ctrl_at(ctrl_index), this->slot_at(slot_index) };
#endif
        } else {
            return { this->slot_at(index) };
        }
    }

    inline iterator iterator_at(const locator_t & locator) noexcept {
        if (!kIsIndirectKV)
            return { locator };
        else
            return { locator.slot() };
    }

    inline const_iterator iterator_at(const locator_t & locator) const noexcept {
        if (!kIsIndirectKV)
            return { locator };
        else
            return { locator.slot() };
    }

    inline size_type index_salt() const noexcept {
        return (size_type)((std::uintptr_t)this->ctrls() >> 12);
    }

    JSTD_FORCED_INLINE
    std::size_t hash_for(const key_type & key) const
        noexcept(noexcept(this->hasher_(key))) {
#if GROUP15_USE_HASH_POLICY
        std::size_t key_hash = static_cast<std::size_t>(this->hash_policy_.get_hash_code(key));
#else
  #if defined(_MSC_VER) && !defined(__clang__)
        std::size_t key_hash;
        if (std::is_integral<key_type>::value && jstd::is_default_std_hash<Hash, key_type>::value)
            key_hash = static_cast<std::size_t>(jstd::SimpleHash<Hash>()(key));
        else
            key_hash = static_cast<std::size_t>(this->hasher_(key));
  #else
        std::size_t key_hash = static_cast<std::size_t>(this->hasher_(key));
  #endif
        if (!jstd::detail::hash_is_avalanching<Hash>::value)
            key_hash = hashes::mum_mul_mix(key_hash);
#endif
        return key_hash;
    }

    //
    // Do the index hash on the basis of hash code for the index_for_hash().
    //
    JSTD_FORCED_INLINE
    std::size_t index_hasher(std::size_t key_hash) const noexcept {
#if GROUP15_USE_INDEX_SHIFT
        return (key_hash >> this->index_shift_);
#else
        return key_hash;
#endif
    }

    //
    // Do the ctrl hash on the basis of hash code for the ctrl hash.
    //
    JSTD_FORCED_INLINE
    std::size_t ctrl_hasher(std::size_t key_hash) const noexcept {
#if (GROUP15_USE_HASH_POLICY != 0) || (GROUP15_USE_INDEX_SHIFT != 0)
        return key_hash;
#elif 1
        return (size_type)hashes::fibonacci_hash(key_hash);
#elif 0
        return (size_type)hashes::mum_hash(key_hash);
#endif
    }

    JSTD_FORCED_INLINE
    size_type index_for_hash(std::size_t key_hash) const noexcept {
        if (kUseIndexSalt) {
            key_hash ^= this->index_salt();
        }
#if GROUP15_USE_HASH_POLICY
        size_type index = this->hash_policy_.template index_for_hash<key_type>(key_hash, this->slot_mask());
        return (index / kGroupWidth);
#else
        std::size_t index_hash = this->index_hasher(key_hash);
  #if GROUP15_USE_INDEX_SHIFT
        size_type index = static_cast<size_type>(index_hash);
        return index;
  #else
        size_type index = (size_type)index_hash & this->slot_mask();
        return (index / kGroupWidth);
  #endif // GROUP15_USE_INDEX_SHIFT
#endif // GROUP15_USE_HASH_POLICY
    }

    JSTD_FORCED_INLINE
    std::size_t ctrl_for_hash(std::size_t key_hash) const noexcept {
        std::size_t ctrl_hash = this->ctrl_hasher(key_hash);
#if GROUP15_USE_LOOK_UP_TABLE
        std::uint8_t ctrl_hash8 = ctrl_type::reduced_hash(ctrl_hash);
        return static_cast<std::size_t>(ctrl_hash8);
#else
        std::uint8_t ctrl_hash8 = jstd::narrow_cast<std::uint8_t>(ctrl_hash);
        if (JSTD_LIKELY(ctrl_hash8 > kSentinelSlot))
            return static_cast<std::size_t>(ctrl_hash8);
        else if (JSTD_LIKELY(ctrl_hash8 == kEmptySlot))
            return static_cast<std::size_t>(kEmptyHash);
        else
            return static_cast<std::size_t>(kSentinelHash);
#endif
    }

    JSTD_FORCED_INLINE size_type index_of(iterator iter) const {
        if (!kIsIndirectKV) {
            return iter.index(this);
        } else {
            const slot_type * slot = iter.slot();
            size_type ctrl_index = this->bucket(slot->key);
            return ctrl_index;
        }
    }

    JSTD_FORCED_INLINE size_type index_of(const_iterator iter) const {
        if (!kIsIndirectKV) {
            return iter.index(this);
        } else {
            const slot_type * slot = iter.slot();
            size_type ctrl_index = this->bucket(slot->key);
            return ctrl_index;
        }
    }

    JSTD_FORCED_INLINE size_type index_of(ctrl_type * ctrl) const {
        assert(ctrl != nullptr);
        assert(ctrl >= this->ctrls());
        size_type ctrl_index = static_cast<size_type>(ctrl - this->ctrls());
        assert(is_positive(index));
        return ctrl_index;
    }

    JSTD_FORCED_INLINE size_type index_of(const ctrl_type * ctrl) const {
        return this->index_of(const_cast<ctrl_type *>(ctrl));
    }

    JSTD_FORCED_INLINE size_type index_of(slot_type * slot) const {
        assert(slot != nullptr);
        assert(slot >= this->slots());
        size_type slot_index = static_cast<size_type>(slot - this->slots());
        assert(is_positive(slot_index));
        return slot_index;
    }

    JSTD_FORCED_INLINE size_type index_of(const slot_type * slot) const {
        return this->index_of(const_cast<slot_type *>(slot));
    }

    template <typename U>
    inline char * PtrOffset(U * ptr, std::ptrdiff_t offset) {
        return (reinterpret_cast<char *>(ptr) + offset);
    }

    template <typename U>
    inline const char * PtrOffset(U * ptr, std::ptrdiff_t offset) const {
        return const_cast<const char *>(reinterpret_cast<char *>(ptr) + offset);
    }

    template <bool NeedClearSlots>
    void destroy() {
        this->destroy_data<NeedClearSlots>();
    }

    template <bool NeedClearSlots>
    JSTD_NO_INLINE
    void destroy_data() {
        // Note!!: destroy_slots() need use this->ctrls(), so must destroy slots first.
        size_type group_capacity = this->group_capacity();
        this->destroy_slots<NeedClearSlots>();
        this->destroy_groups(group_capacity);
    }

    JSTD_FORCED_INLINE
    void destroy_groups(size_type group_capacity) noexcept {
        JSTD_UNUSED(group_capacity);
        if (this->groups_ != this_type::default_empty_groups()) {
            // Reset groups state
            this->groups_ = this_type::default_empty_groups();
#if GROUP15_USE_SEPARATE_SLOTS
            size_type total_group_alloc_count = this->TotalGroupAllocCount<kGroupAlignment>(group_capacity);
            GroupAllocTraits::deallocate(this->group_allocator_, this->groups_alloc_, total_group_alloc_count);
            this->groups_alloc_ = nullptr;           
#endif
        }
    }

    template <bool NeedClearSlots>
    JSTD_FORCED_INLINE
    void destroy_slots() {
        if (NeedClearSlots) {
            this->clear_slots();
        }

        if (this->slots_ != nullptr) {
#if GROUP15_USE_SEPARATE_SLOTS
            SlotAllocTraits::deallocate(this->slot_allocator_, this->slots_, this->slot_capacity());
#else
            size_type total_slot_alloc_size = this->TotalSlotAllocCount<kGroupAlignment>(
                                                    this->group_capacity(), this->slot_capacity());
            SlotAllocTraits::deallocate(this->slot_allocator_, this->slots_, total_slot_alloc_size);
#endif
            // Reset slots state
            this->slots_ = nullptr;
            this->slot_size_ = 0;
            this->slot_mask_ = size_type(-1);
            this->slot_threshold_ = 0;
            this->slot_capacity_ = 0;
            this->group_mask_ = 0;
#if GROUP15_USE_INDEX_SHIFT
            this->index_shift_ = kWordLength - 1;
#endif
#if GROUP15_USE_HASH_POLICY
            this->hash_policy_.reset();
#endif
        }
    }

    JSTD_FORCED_INLINE
    static void init_groups(group_type * groups, size_type group_capacity, std::true_type) {
        /*
         * memset faster/not slower than manual, assumes all zeros is group_type's
         * default layout.
         * reinterpret_cast: GCC may complain about group_type not being trivially
         * copy-assignable when we're relying on trivial copy constructibility.
         */
        if (groups != this_type::default_empty_groups()) {
            JSTD_ASSUME_ALIGNED(groups, kGroupAlignment);
            std::memset(reinterpret_cast<unsigned char *>(groups),
                        kEmptySlot, sizeof(group_type) * group_capacity);
            if (kEmptySlot != 0) {
                group_type * group = groups;
                group_type * last_group = groups + group_capacity;
                for (; group < last_group; ++group) {
                    group->clear_overflow();
                }
            }
        }
    }

    JSTD_FORCED_INLINE
    static void init_groups(group_type * groups, size_type group_capacity, std::false_type) {
        if (groups != this_type::default_empty_groups()) {
            group_type * group = groups;
            group_type * last_group = groups + group_capacity;
            for (; group < last_group; ++group) {
                group->init();
            }
        }
    }

    JSTD_FORCED_INLINE
    void clear_data() {
        // Note!!: clear_slots() need use this->ctrls(), so must clear slots first.
        this->clear_slots();
        this->clear_groups(this->groups(), this->group_capacity());
    }

    JSTD_FORCED_INLINE
    void clear_groups(group_type * groups, size_type group_capacity) {
        this_type::init_groups(groups, group_capacity, std::is_trivially_default_constructible<group_type>{});
    }

    JSTD_FORCED_INLINE
    void clear_slots() {
        if (!is_slot_trivial_destructor && (this->slots_ != nullptr)) {
            if (!kIsIndirectKV) {
#if GROUP15_USE_GROUP_SCAN
                group_type * group = this->groups();
                group_type * last_group = this->last_group();
                slot_type * slot_base = this->slots();
                for (; group < last_group; ++group) {
                    //JSTD_ASSUME(slot_base != nullptr);
                    //jstd::CPU_Prefetch_Read_T0((const void *)&slot_base->get_key());
                    std::uint32_t used_mask = group->match_used();
                    if (used_mask != 0) {
                        do {
                            std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                            if (JSTD_LIKELY(!group->is_sentinel(used_pos))) {
                                slot_type * slot = slot_base + used_pos;
                                this->destroy_slot(slot);
                            } else {
                                break;
                            }
                            used_mask = BitUtils::clearLowBit32(used_mask);
                        } while (used_mask != 0);
                    }
                    slot_base += kGroupSize;
                }
#else
                ctrl_type * ctrl = this->ctrls();
                size_type slot_index = 0;
                for (size_type ctrl_index = 0; ctrl_index < this->ctrl_capacity(); ctrl_index++) {
                    if (JSTD_LIKELY((ctrl_index % kGroupWidth) != kGroupSize) {
                        if (ctrl->is_valid()) {
                            this->destroy_slot(slot_index);
                        }
                        slot_index++;
                    }
                    ctrl++;
                }
#endif
            } else {
                slot_type * slot = this->slots();
                slot_type * last_slot = this->last_slot();
                for (; slot < last_slot; ++slot) {
                    this->destroy_slot(slot);
                }
            }
        }

        this->slot_size_ = 0;
    }

    //
    // copy_slots_from()
    //
    JSTD_FORCED_INLINE
    void copy_slots_from(group15_flat_table const & other) {
        assert(this->empty());
        assert(this != std::addressof(other));
        assert(other.size() > 0);
        if (this->ctrl_capacity() == other.ctrl_capacity()) {
            this->fast_copy_slots_from(other);
        } else {
            try {
                this->unique_insert(other.begin(), other.end());
            } catch (const std::bad_alloc & ex) {
                JSTD_UNUSED(ex);
                this->destroy<true>();
                throw std::bad_alloc();
            } catch (...) {
                this->destroy<true>();
                throw;
            }
        }
    }

    JSTD_FORCED_INLINE
    void fast_copy_slots_from(group15_flat_table const & other) {
        if (this->slots() != nullptr && other.slots() != nullptr) {
            copy_groups_array_from(other);
            copy_slots_array_from(other);
        } else {
            assert(false);
        }
    }

    JSTD_FORCED_INLINE
    void copy_groups_array_from(group15_flat_table const & other) {
        this->copy_groups_array_from(other, std::is_trivially_copy_assignable<group_type>{});
    }

    JSTD_FORCED_INLINE
    void copy_groups_array_from(group15_flat_table const & other, std::true_type /* -> memcpy */) {
        std::memcpy(
            this->groups(), other.groups(),
            other.group_capacity() * sizeof(group_type));
    }

    JSTD_FORCED_INLINE
    void copy_groups_array_from(group15_flat_table const & other, std::false_type /* -> manual */) {
        const group_type * other_group = other.groups();
        group_type * group = this->groups();
        size_type index = 0;
        size_type group_capacity = other.group_capacity();
        while (index < group_capacity) {
            group[index] = other_group[index];
            ++index;
        }
    }

    JSTD_FORCED_INLINE
    void copy_slots_array_from(group15_flat_table const & other) {
        this->copy_slots_array_from(
            other,
            std::integral_constant<bool, std::is_trivially_copy_constructible<element_type>::value &&
                                        (jstd::is_std_allocator<Allocator>::value ||
                                        !jstd::alloc_has_construct<Allocator, value_type *, const value_type &>::value)>{}
        );
    }

    JSTD_FORCED_INLINE
    void copy_slots_array_from(group15_flat_table const & other, std::true_type /* -> memcpy */) {
        /*
         * reinterpret_cast: GCC may complain about value_type not being trivially
         * copy-assignable when we're relying on trivial copy constructibility.
         */
        std::memcpy(
            reinterpret_cast<unsigned char *>(this->slots()),
            reinterpret_cast<unsigned char *>(other.slots()),
            other.slot_capacity() * sizeof(slot_type));
    }

    JSTD_FORCED_INLINE
    void copy_slots_array_from(group15_flat_table const & other, std::false_type /* -> manual */) {
        const ctrl_type * ctrl = this->ctrls();
        const ctrl_type * last_ctrl = this->last_ctrl();
        const slot_type * other_slot = other.slots();
        slot_type * slot = this->slots();
        ssize_type index = 0;

        try {
            while (ctrl < last_ctrl) {
                if (ctrl->is_used()) {
                    if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                        SlotPolicyTraits::construct(&this->slot_allocator_, &slot[index], &other_slot[index]);
                    } else {
                        break;
                    }
                }
                ++index;
                if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != (kGroupSize - 1))) {
                    ++ctrl;
                } else {
                    ctrl += 2;
                }
            }
        }
        catch (const std::bad_alloc & ex) {
            JSTD_UNUSED(ex);
            if (index != 0) {
                do {
                    if (ctrl->is_used()) {
                        if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                            SlotPolicyTraits::destroy(&this->slot_allocator_, &slot[index]);
                        } else {
                            break;
                        }
                    }
                    --index;
                    if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != 0)) {
                        --ctrl;
                    } else {
                        ctrl -= 2;
                    }
                } while (index >= 0);
            }
            throw std::bad_alloc();
        } catch (...) {
            if (index != 0) {
                do {
                    if (ctrl->is_used()) {
                        if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                            SlotPolicyTraits::destroy(&this->slot_allocator_, &slot[index]);
                        } else {
                            break;
                        }
                    }
                    --index;
                    if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != 0)) {
                        --ctrl;
                    } else {
                        ctrl -= 2;
                    }
                } while (index >= 0);
            }
            throw;
        }
    }

    //
    // move_slots_from()
    //
    JSTD_FORCED_INLINE
    void move_slots_from(group15_flat_table & other) {
        assert(this->empty());
        assert(this != std::addressof(other));
        assert(other.size() > 0);
        if (this->ctrl_capacity() == other.ctrl_capacity()) {
            this->fast_move_slots_from(other);
        } else {
            try {
                this->move_unique_insert(other, other.begin(), other.end());
            } catch (const std::bad_alloc & ex) {
                JSTD_UNUSED(ex);
                this->destroy<true>();
                throw std::bad_alloc();
            } catch (...) {
                this->destroy<true>();
                throw;
            }
        }
    }

    JSTD_FORCED_INLINE
    void fast_move_slots_from(group15_flat_table & other) {
        if (this->slots() != nullptr && other.slots() != nullptr) {
            move_groups_array_from(other);
            move_slots_array_from(other);
        } else {
            assert(false);
        }
    }

    JSTD_FORCED_INLINE
    void move_groups_array_from(group15_flat_table & other) {
        this->copy_groups_array_from(other, std::is_trivially_copy_assignable<group_type>{});
    }

    JSTD_FORCED_INLINE
    void move_groups_array_from(group15_flat_table & other, std::true_type /* -> memcpy */) {
        std::memcpy(
            this->groups(), other.groups(),
            other.group_capacity() * sizeof(group_type));

        // Reset all of other groups
        this->clear_groups(other.groups(), other.group_capacity());
    }

    JSTD_FORCED_INLINE
    void move_groups_array_from(group15_flat_table & other, std::false_type /* -> manual */) {
        group_type * other_group = other.groups();
        group_type * group = this->groups();
        size_type index = 0;
        size_type group_capacity = other.group_capacity();
        while (index < group_capacity) {
            group[index] = other_group[index];
            // Reset one other group
            other_group[index].init();
            ++index;
        }
    }

    JSTD_FORCED_INLINE
    void move_slots_array_from(group15_flat_table & other) {
        this->move_slots_array_from(
            other,
            std::integral_constant<bool, std::is_trivially_copy_constructible<element_type>::value &&
                                        (jstd::is_std_allocator<Allocator>::value ||
                                        !jstd::alloc_has_construct<Allocator, value_type *, const value_type &>::value)>{}
        );
    }

    JSTD_FORCED_INLINE
    void move_slots_array_from(group15_flat_table & other, std::true_type /* -> memcpy */) {
        /*
         * reinterpret_cast: GCC may complain about value_type not being trivially
         * copy-assignable when we're relying on trivial copy constructibility.
         */
        std::memcpy(
            reinterpret_cast<unsigned char *>(this->slots()),
            reinterpret_cast<unsigned char *>(other.slots()),
            other.slot_capacity() * sizeof(slot_type));

        // Reset all of other slots
        std::fill_n(other.slots(), other.slot_capacity(), slot_type());
    }

    JSTD_FORCED_INLINE
    void move_slots_array_from(group15_flat_table & other, std::false_type /* -> manual */) {
        const ctrl_type * ctrl = this->ctrls();
        const ctrl_type * last_ctrl = this->last_ctrl();
        slot_type * other_slot = other.slots();
        slot_type * slot = this->slots();
        ssize_type index = 0;

        try {
            while (ctrl < last_ctrl) {
                if (ctrl->is_used()) {
                    if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                        SlotPolicyTraits::construct(&this->slot_allocator_, &slot[index], &other_slot[index]);
                        other.destroy_slot(&other_slot[index]);
                    } else {
                        break;
                    }
                }
                ++index;
                if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != (kGroupSize - 1))) {
                    ++ctrl;
                } else {
                    ctrl += 2;
                }
            }
        }
        catch (const std::bad_alloc & ex) {
            JSTD_UNUSED(ex);
            if (index != 0) {
                do {
                    if (ctrl->is_used()) {
                        if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                            SlotPolicyTraits::destroy(&this->slot_allocator_, &slot[index]);
                        } else {
                            break;
                        }
                    }
                    --index;
                    if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != 0)) {
                        --ctrl;
                    } else {
                        ctrl -= 2;
                    }
                } while (index >= 0);
            }
            throw std::bad_alloc();
        } catch (...) {
            if (index != 0) {
                do {
                    if (ctrl->is_used()) {
                        if (JSTD_LIKELY(!ctrl->is_sentinel())) {
                            SlotPolicyTraits::destroy(&this->slot_allocator_, &slot[index]);
                        } else {
                            break;
                        }
                    }
                    --index;
                    if (JSTD_LIKELY((reinterpret_cast<size_type>(ctrl) % kGroupWidth) != 0)) {
                        --ctrl;
                    } else {
                        ctrl -= 2;
                    }
                } while (index >= 0);
            }
            throw;
        }
    }

    JSTD_FORCED_INLINE
    bool need_grow() const noexcept {
        return (this->slot_size() >= this->slot_threshold());
    }

    JSTD_FORCED_INLINE
    void grow_if_necessary() {
        // The growth rate is 2 times
        size_type new_capacity = this->ctrl_capacity() * 2;
        this->rehash_impl<false>(new_capacity);
    }

    inline bool is_valid_capacity(size_type capacity) const noexcept {
        return ((capacity >= kMinCapacity) && pow2::is_pow2(capacity));
    }

    //
    // Given the pointer of actual allocated groups, computes the padding of
    // first groups (from the start of the backing allocation)
    // and return the beginning of groups.
    //
    template <size_type GroupAlignment>
    JSTD_FORCED_INLINE
    group_type * AlignedGroups(const group_type * groups_alloc) noexcept {
        static_assert((GroupAlignment > 0),
                      "jstd::group15_flat_map::AlignedGroups<N>(): GroupAlignment must bigger than 0.");
        static_assert(((GroupAlignment & (GroupAlignment - 1)) == 0),
                      "jstd::group15_flat_map::AlignedGroups<N>(): GroupAlignment must be power of 2.");
        size_type groups_start = reinterpret_cast<size_type>(groups_alloc);
        size_type groups_first = (groups_start + GroupAlignment - 1) & (~(GroupAlignment - 1));
        assert(groups_first >= groups_start);
        size_type groups_padding = static_cast<size_type>(groups_first - groups_start);
        assert(groups_padding < GroupAlignment);
        group_type * groups = reinterpret_cast<group_type *>(reinterpret_cast<char *>(groups_first));
        return groups;
    }

    //
    // Given the pointer of slots and the capacity of slot, computes the padding of
    // between slots and groups (from the start of the backing allocation)
    // and return the beginning of groups.
    //
    template <size_type GroupAlignment>
    JSTD_FORCED_INLINE
    group_type * AlignedSlotsAndGroups(const slot_type * slots, size_type slot_capacity) noexcept {
        static_assert((GroupAlignment > 0),
                      "jstd::group15_flat_map::AlignedSlotsAndGroups<N>(): GroupAlignment must bigger than 0.");
        static_assert(((GroupAlignment & (GroupAlignment - 1)) == 0),
                      "jstd::group15_flat_map::AlignedSlotsAndGroups<N>(): GroupAlignment must be power of 2.");
        const slot_type * last_slots = slots + slot_capacity;
        size_type last_slot = reinterpret_cast<size_type>(last_slots);
        size_type groups_first = (last_slot + GroupAlignment - 1) & (~(GroupAlignment - 1));
        assert(groups_first >= last_slot);
        size_type groups_padding = static_cast<size_type>(groups_first - last_slot);
        assert(groups_padding < GroupAlignment);
        group_type * groups = reinterpret_cast<group_type *>(reinterpret_cast<char *>(groups_first));
        return groups;
    }

    //
    // Given the pointer of groups, the capacity of a group,
    // computes the total allocate count of the backing group array.
    //
    template <size_type GroupAlignment>
    JSTD_FORCED_INLINE
    size_type TotalGroupAllocCount(size_type group_capacity) noexcept {
        const size_type num_group_bytes = group_capacity * sizeof(group_type);
        const size_type total_bytes = num_group_bytes + GroupAlignment;
        const size_type total_alloc_count = (total_bytes + sizeof(group_type) - 1) / sizeof(group_type);
        return total_alloc_count;
    }

    //
    // Given the pointer of slots, the capacity of a group and slot,
    // computes the total allocate count of the backing slot array.
    //
    template <size_type GroupAlignment>
    JSTD_FORCED_INLINE
    size_type TotalSlotAllocCount(size_type group_capacity, size_type slot_capacity) noexcept {
        const size_type num_group_bytes = group_capacity * sizeof(group_type);
        const size_type num_slot_bytes = slot_capacity * sizeof(slot_type);
        const size_type total_bytes = num_slot_bytes + GroupAlignment + num_group_bytes;
        const size_type total_alloc_count = (total_bytes + sizeof(slot_type) - 1) / sizeof(slot_type);
        return total_alloc_count;
    }

    static inline void set_sentinel_mark(group_type * groups, size_type group_capacity) noexcept {
        assert(groups != nullptr);
        assert(group_capacity > 0);
        group_type * last_group = groups + group_capacity;
        group_type * tail_group = groups + group_capacity - 1;
        ctrl_type * ctrl = reinterpret_cast<ctrl_type *>(last_group) - 2;
        assert(ctrl->is_empty());
        tail_group->set_sentinel();
    }

    template <bool IsInitialize>
    JSTD_FORCED_INLINE
    void create_slots(size_type new_capacity) {
        assert(pow2::is_pow2(new_capacity));
        if (JSTD_LIKELY(IsInitialize || (new_capacity != 0))) {
#if GROUP15_USE_HASH_POLICY
            auto hash_policy_setting = this->hash_policy_.calc_next_capacity(new_capacity);
            this->hash_policy_.commit(hash_policy_setting);
#endif
            size_type new_ctrl_capacity = new_capacity;
            size_type new_group_capacity = this_type::calc_group_capacity(new_ctrl_capacity);
            assert(new_group_capacity > 0);

            size_type direct_slot_capacity = this_type::calc_slot_capacity(new_group_capacity, new_capacity);
            size_type indirect_slot_capacity = new_capacity * this->mlf_ / kLoadFactorAmplify;
            size_type new_slot_capacity = (!kIsIndirectKV) ? direct_slot_capacity : indirect_slot_capacity;

#if GROUP15_USE_SEPARATE_SLOTS
            size_type total_group_alloc_count = this->TotalGroupAllocCount<kGroupAlignment>(new_group_capacity);
            group_type * new_groups_alloc = GroupAllocTraits::allocate(this->group_allocator_, total_group_alloc_count);
            group_type * new_groups = this->AlignedGroups<kGroupAlignment>(new_groups_alloc);

            slot_type * new_slots = SlotAllocTraits::allocate(this->slot_allocator_, new_slot_capacity);
#else
            size_type total_slot_alloc_count = this->TotalSlotAllocCount<kGroupAlignment>(new_group_capacity, new_slot_capacity);

            slot_type * new_slots = SlotAllocTraits::allocate(this->slot_allocator_, total_slot_alloc_count);
            group_type * new_groups = this->AlignedSlotsAndGroups<kGroupAlignment>(new_slots, new_slot_capacity);
#endif

            // Reset groups to default state
            this->clear_groups(new_groups, new_group_capacity);

            // Set the sentinel mark
            this_type::set_sentinel_mark(new_groups, new_group_capacity);

            this->groups_ = new_groups;
            this->slots_ = new_slots;
            this->slot_size_ = 0;
            this->slot_mask_ = new_capacity - 1;
            this->slot_threshold_ = this->calc_slot_threshold(new_slot_capacity);
            this->slot_capacity_ = new_slot_capacity;
            assert(new_capacity > 0);
            // Because (new_capacity != 0), so (group_mask_ != -1) too.
            this->group_mask_ = this_type::calc_group_mask(new_group_capacity, new_capacity);
#if GROUP15_USE_INDEX_SHIFT
            this->index_shift_ = this_type::calc_index_shift(new_capacity);
#endif
#if GROUP15_USE_SEPARATE_SLOTS
            this->groups_alloc_ = new_groups_alloc;
#endif
        } else {
            this->destroy<true>();
        }
    }

    template <bool AllowShrink>
    JSTD_NO_INLINE
    void rehash_impl(size_type new_capacity) {
        new_capacity = this->calc_capacity(new_capacity);
        assert(new_capacity > 0);
        assert(new_capacity >= kMinCapacity);
        if ((!AllowShrink && (new_capacity > this->ctrl_capacity())) ||
            (AllowShrink && (new_capacity != this->ctrl_capacity()))) {
            if (!AllowShrink) {
                assert(new_capacity >= this->slot_size());
            }

            group_type * old_groups = this->groups();
            group_type * old_groups_alloc = this->groups_alloc();
            size_type old_group_capacity = this->group_capacity();

            slot_type * old_slots = this->slots();
            slot_type * old_last_slot = this->last_slot();
            size_type old_slot_size = this->slot_size();
            size_type old_slot_mask = this->slot_mask();
            size_type old_slot_capacity = this->slot_capacity();
            size_type old_slot_threshold = this->slot_threshold();

            this->create_slots<false>(new_capacity);

            if (old_groups != this_type::default_empty_groups()) {
                group_type * group = old_groups;
                group_type * last_group = old_groups + old_group_capacity;
                slot_type * slot_base = old_slots;

                for (; group < last_group; ++group) {
                    //JSTD_ASSUME(slot_base != nullptr);
                    //jstd::CPU_Prefetch_Read_T0((const void *)&slot_base->get_key());
                    std::uint32_t used_mask = group->match_used();
                    if (used_mask != 0) {
                        do {
                            std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                            if (JSTD_LIKELY(!group->is_sentinel(used_pos))) {
                                slot_type * old_slot = slot_base + used_pos;
                                assert(old_slot < old_last_slot);
                                this->move_no_grow_unique_insert(old_slot);
                                //this->destroy_slot(old_slot);
                            } else {
                                break;
                            }
                            used_mask = BitUtils::clearLowBit32(used_mask);
                        } while (used_mask != 0);
                    }
                    slot_base += kGroupSize;
                }
            }

            assert(this->slot_size() == old_slot_size);

#if GROUP15_USE_SEPARATE_SLOTS
            if (old_groups != this_type::default_empty_groups()) {
                assert(old_groups_alloc != nullptr);
                size_type total_group_alloc_count = this->TotalGroupAllocCount<kGroupAlignment>(old_group_capacity);
                GroupAllocTraits::deallocate(this->group_allocator_, old_groups_alloc, total_group_alloc_count);
            }
            if (old_slots != nullptr) {
                SlotAllocTraits::deallocate(this->slot_allocator_, old_slots, old_slot_capacity);
            }
#else
            if (old_slots != nullptr) {
                size_type total_slot_alloc_count = this->TotalSlotAllocCount<kGroupAlignment>(
                                                         old_group_capacity, old_slot_capacity);
                SlotAllocTraits::deallocate(this->slot_allocator_, old_slots, total_slot_alloc_count);
            }
#endif
        }
    }

    JSTD_FORCED_INLINE
    void construct_slot(slot_type * slot) {
        SlotPolicyTraits::construct(&this->slot_allocator_, slot);
    }

    JSTD_FORCED_INLINE
    void construct_slot(size_type slot_index) {
        slot_type * slot = this->slot_at(slot_index);
        this->construct_slot(slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot(slot_type * slot) {
        if (!is_slot_trivial_destructor) {
            SlotPolicyTraits::destroy(&this->slot_allocator_, slot);
        }
    }

    JSTD_FORCED_INLINE
    void destroy_slot(size_type slot_index) {
        slot_type * slot = this->slot_at(slot_index);
        this->destroy_slot(slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot_data(ctrl_type * ctrl, slot_type * slot) {
        assert(ctrl->is_used() && !ctrl->is_sentinel());
        ctrl->set_empty();
        this->destroy_slot(slot);
    }

    JSTD_FORCED_INLINE
    void destroy_slot_data(locator_t & locator) {
        ctrl_type * ctrl = locator.ctrl();
        slot_type * slot = locator.slot();
        this->destroy_slot_data(ctrl, slot);
    }

    inline void display_meta_datas(group_type * group) const {
        const ctrl_type * ctrl = reinterpret_cast<const ctrl_type *>(group);
        printf("[");
        for (std::size_t i = 0; i < kGroupWidth; i++) {
            if (i < kGroupWidth - 1)
                printf(" %02x,", (int)ctrl->value());
            else
                printf(" %02x", (int)ctrl->value());
            ctrl++;
        }
        printf(" ]\n");
    }

    JSTD_FORCED_INLINE
    iterator make_iterator(size_type ctrl_index) const noexcept {
        size_type group_index = ctrl_index / kGroupWidth;
        size_type group_pos = ctrl_index % kGroupWidth;
        assert(group_index < this->group_capacity());
        assert(group_pos != kGroupSize);
        const group_type * group = this->group_at(group_index);
        size_type slot_index = group_index * kGroupSize + group_pos;
        const slot_type * slot = this->slot_at(slot_index);
        return { group, group_pos, slot };
    }

    JSTD_FORCED_INLINE
    iterator make_iterator(size_type ctrl_index, size_type slot_index) const noexcept {
        const ctrl_type * ctrl = this->ctrl_at(ctrl_index);
        const slot_type * slot = this->slot_at(slot_index);
        return { ctrl, slot };
    }

    static JSTD_FORCED_INLINE
    iterator make_iterator(const locator_t & locator) noexcept {
        return { locator.group(), locator.pos(), locator.slot() };
    }

#if 0
    template <typename KeyT>
    JSTD_FORCED_INLINE
    locator_t find_impl(const KeyT & key) {
        return const_cast<const this_type *>(this)->find_impl<KeyT>(key);
    }
#endif

    template <typename KeyT>
    JSTD_FORCED_INLINE
    locator_t find_impl(const KeyT & key) const {
        std::size_t key_hash = this->hash_for(key);
        size_type group_index = this->index_for_hash(key_hash);
        std::size_t ctrl_hash = this->ctrl_for_hash(key_hash);
        return this->find_impl(key, group_index, ctrl_hash);
    }

#if defined(_MSC_VER)
/* warning: forcing value to bool 'true' or 'false' in bool(key_equal()...) */
#pragma warning(push)
#pragma warning(disable:4800)
#endif

    template <typename KeyT>
    JSTD_FORCED_INLINE
    locator_t find_impl(const KeyT & key, size_type group_index0, std::size_t ctrl_hash) const {
        auto hash_bits = group_type::make_hash_bits(ctrl_hash);
        prober_type prober(group_index0);

        do {
            size_type group_index = prober.get();
            const group_type * group = this->groups() + group_index;
            std::uint32_t match_mask = group->match_hash(hash_bits);
            if (JSTD_LIKELY(match_mask != 0)) {
                const slot_type * slot_start = this->slots();
                JSTD_ASSUME(slot_start != nullptr);
                const slot_type * slot_base = slot_start + group_index * kGroupSize;
                if (sizeof(value_type) <= 64) {
                    jstd::CPU_Prefetch_Read_T0((const void *)&slot_base->get_key());
                }
                do {
                    std::uint32_t match_pos = BitUtils::bsf32(match_mask);
                    const slot_type * slot = slot_base + match_pos;
                    if (JSTD_LIKELY(bool(this->key_equal_(key, slot->get_key())))) {
                        return { group, match_pos, slot };
                    }
                    match_mask = BitUtils::clearLowBit32(match_mask);
                } while (match_mask != 0);
            }

            // If it's not overflow, means it hasn't been found.
            if (JSTD_LIKELY(group->is_not_overflow(ctrl_hash))) {
                return {};
            }

#if GROUP15_DISPLAY_DEBUG_INFO
            if (JSTD_UNLIKELY(prober.steps() > kSkipGroupsLimit)) {
                std::cout << "find_impl(): key = " << key <<
                             ", skip_groups = " << prober.steps() <<
                             ", load_factor = " << this->load_factor() << std::endl;
            }
#endif
        } while (JSTD_LIKELY(prober.next_bucket(this->group_mask())));

        return {};
    }

#if defined(_MSC_VER)
#pragma warning(pop) /* C4800 */
#endif

    template <bool IsNoCheck, typename KeyT = key_type>
    JSTD_FORCED_INLINE
    locator_t find_empty_to_insert(const KeyT & key, size_type group_index0, std::size_t ctrl_hash) {
        static constexpr bool IsNoGrow = true;
        prober_type prober(group_index0);

        do {
            size_type group_index = prober.get();
            group_type * group = this->groups() + group_index;
            std::uint32_t empty_mask = group->match_empty();
            if (JSTD_LIKELY(empty_mask != 0)) {
                std::uint32_t empty_pos = BitUtils::bsf32(empty_mask);
                const slot_type * slot_start = this->slots();
                JSTD_ASSUME(slot_start != nullptr);
                const slot_type * slot_base = slot_start + group_index * kGroupSize;
                const slot_type * slot = slot_base + empty_pos;
                assert(group->is_empty(empty_pos));
                group->set_used(empty_pos, ctrl_hash);
                if (!IsNoCheck) {
#if GROUP15_USE_NEW_OVERFLOW
                    // If any overflow bit is not 0, it means that the group was once full.
                    bool maybe_overflow = group->has_any_overflow();
                    bool is_deleted_slot;
                    if (JSTD_LIKELY(!maybe_overflow)) {
#if 1
                        std::uint32_t empty_bits = empty_mask ^ (empty_mask - 1);
                        bool is_last_empty = (((empty_mask + empty_bits) & 0x8000u) != 0);
                        is_deleted_slot = !is_last_empty;
#else
                        std::uint32_t used_mask = (~empty_mask) & 0x7FFFu;
                        std::uint32_t last_used_pos = (used_mask != 0) ? BitUtils::bsr32(used_mask) : 32;
                        is_deleted_slot = (last_used_pos < empty_pos);
#endif
                    } else {
                        is_deleted_slot = true;
                    }
                    this->slot_threshold_ += is_deleted_slot;
                    assert(this->slot_threshold_ < this->slot_capacity());
#endif // GROUP15_USE_NEW_OVERFLOW
                }
                return { group, empty_pos, slot };
            } else {
                // If it's not overflow, set the overflow bit.
                group->set_overflow(ctrl_hash);
            }
#if GROUP15_DISPLAY_DEBUG_INFO
            if (JSTD_UNLIKELY(prober.steps() > kSkipGroupsLimit)) {
                std::cout << "find_empty_to_insert(): key = " << key <<
                             ", skip_groups = " << prober.steps() <<
                             ", load_factor = " << this->load_factor() << std::endl;
                display_meta_datas(group);
            }
#endif
            if (IsNoGrow) {
                prober.next_bucket(this->group_mask());
            }
        } while (JSTD_LIKELY(IsNoGrow || prober.next_bucket(this->group_mask())));

        return {};
    }

    template <typename KeyT>
    JSTD_FORCED_INLINE
    std::pair<locator_t, bool> find_or_insert(const KeyT & key) {
        std::size_t key_hash = this->hash_for(key);
        size_type group_index = this->index_for_hash(key_hash);
        std::size_t ctrl_hash = this->ctrl_for_hash(key_hash);

        locator_t locator = this->find_impl(key, group_index, ctrl_hash);
        if (locator.slot() != nullptr) {
            return { locator, kIsKeyExists };
        }

        if (JSTD_UNLIKELY(this->need_grow())) {
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            group_index = this->index_for_hash(key_hash);
            // Ctrl hash will not change
            // ctrl_hash = this->ctrl_for_hash(key_hash);
        }

        locator = this->find_empty_to_insert<false, KeyT>(key, group_index, ctrl_hash);
        if (JSTD_LIKELY(true || (locator.slot() != nullptr))) {
            return { locator, kNeedInsert };
        }
        else {
            printf("\nfind_or_insert(): overflow, size() = %d, load_factor = %0.3f\n",
                    (int)this->size(), this->load_factor());
            // The size of slot reach the slot threshold or hashmap is full.
            this->grow_if_necessary();

            group_index = this->index_for_hash(key_hash);
            locator = this->find_empty_to_insert<false, KeyT>(key, group_index, ctrl_hash);
            assert(locator.slot() < this->last_slot());
            return { locator, kNeedInsert };
        }
    }

    JSTD_FORCED_INLINE
    locator_t no_grow_unique_insert(const key_type & key) {
        std::size_t key_hash = this->hash_for(key);
        size_type group_index = this->index_for_hash(key_hash);
        std::size_t ctrl_hash = this->ctrl_for_hash(key_hash);

        locator_t locator = this->find_empty_to_insert<true, key_type>(key, group_index, ctrl_hash);
        return locator;
    }

    ///
    /// Use in rehash_impl()
    ///
    JSTD_FORCED_INLINE
    void move_no_grow_unique_insert(slot_type * old_slot) {
        assert(old_slot != nullptr);
        locator_t locator = this->no_grow_unique_insert(old_slot->get_key());
        slot_type * new_slot = locator.slot();
        assert(new_slot != nullptr);

        SlotPolicyTraits::construct(&this->slot_allocator_, new_slot, old_slot);
        this->destroy_slot(old_slot);
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    JSTD_FORCED_INLINE
    void move_no_grow_unique_insert(group15_flat_table * other, slot_type * old_slot) {
        assert(old_slot != nullptr);
        locator_t locator = this->no_grow_unique_insert(old_slot->get_key());
        slot_type * new_slot = locator.slot();
        assert(new_slot != nullptr);

        SlotPolicyTraits::construct(&this->slot_allocator_, new_slot, old_slot);
        other->destroy_slot(old_slot);
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    ///
    /// Use in unique_insert(first, last)
    ///
    JSTD_FORCED_INLINE
    void no_grow_unique_insert(const slot_type * old_slot) {
        assert(old_slot != nullptr);
        locator_t locator = this->no_grow_unique_insert(old_slot->get_key());
        slot_type * new_slot = locator.slot();
        assert(new_slot != nullptr);

        SlotPolicyTraits::construct(&this->slot_allocator_, new_slot, old_slot);
        this->slot_size_++;
        assert(this->slot_size() <= this->slot_capacity());
    }

    JSTD_FORCED_INLINE
    void unique_insert(iterator first, iterator last) {
        for (; first != last; ++first) {
            const slot_type * old_slot = first.slot();
            this->no_grow_unique_insert(old_slot);
        }
    }

    JSTD_FORCED_INLINE
    void move_unique_insert(iterator first, iterator last) {
        for (; first != last; ++first) {
            slot_type * old_slot = first.slot();
            this->move_no_grow_unique_insert(old_slot);
        }
    }

    JSTD_FORCED_INLINE
    void move_unique_insert(group15_flat_table & other, iterator first, iterator last) {
        for (; first != last; ++first) {
            slot_type * old_slot = first.slot();
            this->move_no_grow_unique_insert(&other, old_slot);
        }
    }

    template <bool AlwaysUpdate, typename ValueT, typename std::enable_if<
              (jstd::is_same_ex<ValueT, value_type>::value ||
               std::is_constructible<value_type, const ValueT &>::value) ||
              (jstd::is_same_ex<ValueT, init_type>::value ||
               std::is_constructible<init_type, const ValueT &>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(const ValueT & value) {
        auto find_info = this->find_or_insert(value.first);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;        
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            SlotPolicyTraits::construct(&this->slot_allocator_, slot, value);
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = locator.slot();
                slot->value.second = value.second;
            }
        }
        return { locator, need_insert };
    }

    template <bool AlwaysUpdate, typename ValueT, typename std::enable_if<
              (jstd::is_same_ex<ValueT, value_type>::value ||
               std::is_constructible<value_type, ValueT &&>::value) ||
              (jstd::is_same_ex<ValueT, init_type>::value ||
               std::is_constructible<init_type, ValueT &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(ValueT && value) {
        static constexpr bool is_rvalue_ref = std::is_rvalue_reference<decltype(value)>::value;
        auto find_info = this->find_or_insert(value.first);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot, std::move(value));
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = locator.slot();
                if (is_rvalue_ref) {
                    //slot->value.second = std::move(value.second);
                    if (kIsLayoutCompatible)
                        jstd::move_assign_if<is_rvalue_ref>(slot->mutable_value.second, value.second);
                    else
                        jstd::move_assign_if<is_rvalue_ref>(slot->value.second, value.second);
                } else {
                    slot->value.second = value.second;
                }
            }
        }
        return { locator, need_insert };
    }

    template <bool AlwaysUpdate, typename KeyT, typename MappedT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, init_type>::value &&
               !std::is_constructible<init_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
               (jstd::is_same_ex<KeyT, key_type>::value ||
                std::is_constructible<key_type, KeyT &&>::value) &&
               (jstd::is_same_ex<MappedT, mapped_type>::value ||
                std::is_constructible<mapped_type, MappedT &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(KeyT && key, MappedT && value) {
        auto find_info = this->find_or_insert(key);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot,
                                        std::forward<KeyT>(key),
                                        std::forward<MappedT>(value));
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            static constexpr bool isMappedType = jstd::is_same_ex<MappedT, mapped_type>::value;
            if (AlwaysUpdate) {
                slot_type * slot = locator.slot();
                if (isMappedType) {
                    slot->value.second = std::forward<MappedT>(value);
                } else {
                    mapped_type mapped_value(std::forward<MappedT>(value));
                    slot->value.second = std::move(mapped_value);
                }
            }
        }
        return { locator, need_insert };
    }

    template <bool AlwaysUpdate, typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, value_type>::value &&
               !std::is_constructible<value_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, init_type>::value &&
               !std::is_constructible<init_type, KeyT &&>::value) &&
              (!jstd::is_same_ex<KeyT, std::piecewise_construct_t>::value) &&
               (jstd::is_same_ex<KeyT, key_type>::value ||
                std::is_constructible<key_type, KeyT &&>::value)>::type * = nullptr,
                typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(KeyT && key, Args && ... args) {
        auto find_info = this->find_or_insert(key);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot,
                                        std::piecewise_construct,
                                        std::forward_as_tuple(std::forward<KeyT>(key)),
                                        std::forward_as_tuple(std::forward<Args>(args)...));
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = locator.slot();
                mapped_type mapped_value(std::forward<Args>(args)...);
                slot->value.second = std::move(mapped_value);
            }
        }
        return { locator, need_insert };
    }

    template <bool AlwaysUpdate, typename PieceWise, typename std::enable_if<
              (!jstd::is_same_ex<PieceWise, value_type>::value &&
               !std::is_constructible<value_type, PieceWise &&>::value) &&
              (!jstd::is_same_ex<PieceWise, init_type>::value &&
               !std::is_constructible<init_type, PieceWise &&>::value) &&
                jstd::is_same_ex<PieceWise, std::piecewise_construct_t>::value &&
              (!jstd::is_same_ex<PieceWise, key_type>::value &&
               !std::is_constructible<key_type, PieceWise &&>::value)>::type * = nullptr,
                typename ... Ts1, typename ... Ts2>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(PieceWise && hint,
                                           std::tuple<Ts1...> && first,
                                           std::tuple<Ts2...> && second) {
        jstd::tuple_wrapper2<key_type> key_wrapper(first);
        auto find_info = this->find_or_insert(key_wrapper.value());
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot,
                                        std::piecewise_construct,
                                        std::forward<std::tuple<Ts1...>>(first),
                                        std::forward<std::tuple<Ts2...>>(second));
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                jstd::tuple_wrapper2<mapped_type> mapped_wrapper(std::move(second));
                slot_type * slot = locator.slot();
                slot->value.second = std::move(mapped_wrapper.value());
            }
        }
        return { locator, need_insert };
    }

    template <bool AlwaysUpdate, typename First, typename std::enable_if<
              (!jstd::is_same_ex<First, value_type>::value &&
               !std::is_constructible<value_type, First &&>::value) &&
              (!jstd::is_same_ex<First, init_type>::value &&
               !std::is_constructible<init_type, First &&>::value) &&
              (!jstd::is_same_ex<First, std::piecewise_construct_t>::value) &&
              (!jstd::is_same_ex<First, key_type>::value &&
               !std::is_constructible<key_type, First &&>::value)>::type * = nullptr,
                typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> emplace_impl(First && first, Args && ... args) {
        alignas(slot_type) unsigned char raw[sizeof(slot_type)];
        slot_type * tmp_slot = reinterpret_cast<slot_type *>(&raw);

        SlotPolicyTraits::construct(&this->slot_allocator_, tmp_slot,
                                    std::forward<First>(first),
                                    std::forward<Args>(args)...);

        auto find_info = this->find_or_insert(tmp_slot->get_key());
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::transfer(&this->slot_allocator_, slot, tmp_slot);
            this->slot_size_++;
        } else {
            // The key to be inserted already exists.
            if (AlwaysUpdate) {
                slot_type * slot = locator.slot();
                slot->value.second = std::move(tmp_slot->value.second);
            }
        }
        SlotPolicyTraits::destroy(&this->slot_allocator_, tmp_slot);
        return { locator, need_insert };
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace_impl(const KeyT & key, Args && ... args) {
        auto find_info = this->find_or_insert(key);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot,
                                        std::piecewise_construct,
                                        std::forward_as_tuple(key),
                                        std::forward_as_tuple(std::forward<Args>(args)...));
            this->slot_size_++;
        }
        return { locator, need_insert };
    }

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace_impl(KeyT && key, Args && ... args) {
        auto find_info = this->find_or_insert(key);
        locator_t & locator = find_info.first;
        bool need_insert = find_info.second;
        if (need_insert) {
            // The key to be inserted is not exists.
            slot_type * slot = locator.slot();
            assert(slot != nullptr);
            assert(slot < this->last_slot());
            SlotPolicyTraits::construct(&this->slot_allocator_, slot,
                                        std::piecewise_construct,
                                        std::forward_as_tuple(std::forward<KeyT>(key)),
                                        std::forward_as_tuple(std::forward<Args>(args)...));
            this->slot_size_++;
        }
        return { locator, need_insert };
    }

    JSTD_FORCED_INLINE
    bool maybe_caused_overflow(const group_type * group, size_type group_pos) const noexcept {
        std::size_t ctrl_hash = static_cast<std::size_t>(group->value(group_pos));
        return group->is_overflow(ctrl_hash);
    }

    JSTD_FORCED_INLINE
    bool maybe_is_deleted_slot(const group_type * group, size_type group_pos) const noexcept {
        bool maybe_overflow = group->has_any_overflow();
        if (JSTD_LIKELY(!maybe_overflow)) {
            return maybe_caused_overflow(group, group_pos);
        } else {
            return maybe_overflow;
        }
    }

    JSTD_FORCED_INLINE
    void reset_ctrl(locator_t & locator) {
        group_type * group = locator.group();
        size_type group_pos = locator.pos();
#if GROUP15_USE_NEW_OVERFLOW
        bool maybe_overflow = this->maybe_is_deleted_slot(group, group_pos);
#else
        bool maybe_overflow = this->maybe_caused_overflow(group, group_pos);
#endif
        assert(group->is_used(group_pos));
        group->set_empty(group_pos);
        assert(this->slot_threshold_ > 0);
        this->slot_threshold_ -= maybe_overflow;
        assert(this->slot_size_ > 0);
        this->slot_size_--;
    }

    JSTD_FORCED_INLINE
    void erase_slot(locator_t & locator) {
        assert(locator.slot() >= this->slots() && locator.slot() < this->last_slot());
        this->destroy_slot(locator.slot());
        this->reset_ctrl(locator);
    }

    JSTD_FORCED_INLINE
    size_type find_and_erase(const key_type & key) {
        std::size_t key_hash = this->hash_for(key);
        size_type group_index = this->index_for_hash(key_hash);
        std::size_t ctrl_hash = this->ctrl_for_hash(key_hash);

        locator_t locator = this->find_impl(key, group_index, ctrl_hash);
        if (JSTD_LIKELY(locator.slot() != nullptr)) {
            this->erase_slot(locator);
            return 1;
        } else {
            return 0;
        }
    }

    // TODO: Optimize this assuming *this and other don't overlap.
    JSTD_FORCED_INLINE
    this_type & move_assign(this_type && other, std::true_type) {
        if (std::addressof(other) != this) {
            this_type tmp(std::move(other));
            this->swap_impl(tmp);
        }
        return *this;
    }

    JSTD_FORCED_INLINE
    this_type & move_assign(this_type && other, std::false_type) {
        if (std::addressof(other) != this) {
            this_type tmp(std::move(other), this->get_allocator_ref());
            this->swap_impl(tmp);
        }
        return *this;
    }

    JSTD_FORCED_INLINE
    void swap_content(this_type & other) noexcept {
        using std::swap;
        swap(this->groups_, other.groups_);
        swap(this->slots_, other.slots_);
        swap(this->slot_size_, other.slot_size_);
        swap(this->slot_mask_, other.slot_mask_);        
        swap(this->slot_threshold_, other.slot_threshold_);
        swap(this->slot_capacity_, other.slot_capacity_);
        swap(this->group_mask_, other.group_mask_);
#if GROUP15_USE_INDEX_SHIFT
        swap(this->index_shift_, other.index_shift_);
#endif
        swap(this->mlf_, other.mlf_);
#if GROUP15_USE_SEPARATE_SLOTS
        swap(this->groups_alloc_, other.groups_alloc_);
#endif
    }

    JSTD_FORCED_INLINE
    void swap_policy(this_type & other) noexcept {
        using std::swap;
#if GROUP15_USE_HASH_POLICY
        swap(this->hash_policy_, other.hash_policy_ref());
#endif
        swap(this->hasher_, other.hash_function_ref());
        swap(this->key_equal_, other.key_eq_ref());
        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
            swap(this->allocator_, other.get_allocator_ref());
        }
        if (std::allocator_traits<group_allocator_type>::propagate_on_container_swap::value) {
            swap(this->group_allocator_, other.get_group_allocator_ref());
        }
        if (std::allocator_traits<slot_allocator_type>::propagate_on_container_swap::value) {
            swap(this->slot_allocator_, other.get_slot_allocator_ref());
        }
    }

    JSTD_FORCED_INLINE
    void swap_impl(this_type & other) noexcept {
        this->swap_content(other);
        this->swap_policy(other);
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_GROUP15_FLAT_TABLE_HPP
