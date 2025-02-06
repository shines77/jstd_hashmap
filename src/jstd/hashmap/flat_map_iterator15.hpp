/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 20242-2025 XiongHui Guo (gz_shines at msn.com)

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

#ifndef JSTD_HASHMAP_FLAT_MAP_ITERATOR15_HPP
#define JSTD_HASHMAP_FLAT_MAP_ITERATOR15_HPP

#pragma once

#include <cstdint>
#include <iterator>     // For std::forward_iterator_tag
#include <type_traits>  // For std::conditional, and so on...
#include <memory>       // For std::addressof()

#include <assert.h>

#define ITERATOR15_USE_GROUP_SCAN  0
#define ITERATOR15_USE_LOCATOR15   0

namespace jstd {

template <typename HashMap, bool IsIndirectKV /* = false */>
class flat_map_locator15 {
public:
    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using group_type = typename HashMap::group_type;
    using slot_type = typename HashMap::slot_type;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

    static constexpr const size_type kGroupSize  = group_type::kGroupSize;
    static constexpr const size_type kGroupWidth = group_type::kGroupWidth;

public:
    flat_map_locator15() noexcept : group_(nullptr), pos_(0), slot_(nullptr) {}
    flat_map_locator15(const group_type * group, size_type pos, const slot_type * slot) noexcept
        : group_(group), pos_(pos), slot_(slot) {}
#if 0
    flat_map_locator15(const hashmap_type * hashmap, size_type index) noexcept
        : flat_map_locator15() {
        size_type group_index = index / kGroupWidth;
        size_type group_pos = index % kGroupWidth;
        assert(group_index != this->group_capacity());
        assert(group_pos != kGroupSize);
        this->group_ = hashmap->group_at(group_index);
        this->pos_ = group_pos;
        size_type slot_index = group_index * kGroupSize + group_pos;
        this->slot_ = hashmap->slot_at(slot_index);
    }
#endif

#if 1
    flat_map_locator15(const flat_map_locator15 & locator) noexcept
        : group_(locator.group()), pos_(locator.pos()), slot_(locator.slot()) {}

    inline flat_map_locator15 & operator = (const flat_map_locator15 & rhs) noexcept {
        this->group_  = rhs.group();
        this->pos_    = rhs.pos();
        this->slot_   = rhs.slot();
        return *this;
    }
#endif

    explicit operator bool() const noexcept {
        return (this->slot_ != nullptr);
    }

    friend inline bool operator == (const flat_map_locator15 & lhs, const flat_map_locator15 & rhs) noexcept {
        //return (lhs.slot() == rhs.slot()) && (lhs.pos() == rhs.pos()) && (lhs.group() == rhs.group());
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_locator15 & lhs, const flat_map_locator15 & rhs) noexcept {
        //return (lhs.slot() != rhs.slot()) || (lhs.pos() != rhs.pos()) || (lhs.group() != rhs.group());
        return (lhs.slot() != rhs.slot());
    }

    inline group_type * group() noexcept { return const_cast<group_type *>(this->group_); }
    inline const group_type * group() const noexcept { return this->group_; }

    inline size_type pos() const noexcept { return this->pos_; }

    inline slot_type * slot() noexcept { return const_cast<slot_type *>(this->slot_); }
    inline const slot_type * slot() const noexcept { return this->slot_; }

    inline ctrl_type * ctrl() noexcept {
        ctrl_type * _ctrl = reinterpret_cast<ctrl_type *>(this->group()) + this->pos();
        return _ctrl;
    }

    inline const ctrl_type * ctrl() const noexcept {
        const ctrl_type * _ctrl = reinterpret_cast<const ctrl_type *>(this->group()) + this->pos();
        return _ctrl;
    }

    JSTD_FORCED_INLINE
    void increment() noexcept {
        for (;;) {
            ++this->slot_;
            if (this->pos() == (kGroupSize - 1)) {
                ++this->group_;
                //this->pos_ = 0;
                break;
            }
            ++this->pos_;
            if (this->group_->is_empty(this->pos_))
                continue;
            if (likely(!(this->group_->is_sentinel(this->pos_)))) {
                return;
            } else {
                this->slot_ = nullptr;
                return;
            }
        }

        for (;;) {
            std::uint32_t used_mask = this->group_->match_used();
            if (used_mask != 0) {
                std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                if (likely(!(this->group_->is_sentinel(used_pos)))) {
                    this->pos_ = static_cast<size_type>(used_pos);
                    this->slot_ += static_cast<difference_type>(used_pos);
                } else {
                    this->slot_ = nullptr;
                }
                return;
            }
            ++this->group_;
            //this->pos_ = 0;
            this->slot_ += static_cast<difference_type>(kGroupSize);
        }
    }

    JSTD_FORCED_INLINE
    void decrement() noexcept {
        for (;;) {
            --this->slot_;
            if (this->pos() == 0) {
                --this->group_;
                //this->pos_ = kGroupSize - 1;
                break;
            }
            --this->pos_;
            if (this->group_->is_empty(this->pos_))
                continue;
            if (likely(!(this->group_->is_sentinel(this->pos_)))) {
                return;
            } else {
                this->slot_ = nullptr;
                return;
            }
        }

        for (;;) {
            std::uint32_t used_mask = this->group_->match_used();
            if (used_mask != 0) {
                std::uint32_t used_pos = BitUtils::bsr32(used_mask);
                if (likely(!(this->group_->is_sentinel(used_pos)))) {
                    this->pos_ = static_cast<size_type>(used_pos);
                    this->slot_ -= (kGroupSize - 1) - static_cast<difference_type>(used_pos);
                } else {
                    this->slot_ = nullptr;
                }
                return;
            }
            --this->group_;
            //this->pos_ = kGroupSize - 1;
            this->slot_ -= static_cast<difference_type>(kGroupSize);
        }
    }

protected:
    const group_type *  group_;
    size_type           pos_;
    const slot_type *   slot_;
};

#if ITERATOR15_USE_LOCATOR15

template <typename HashMap, typename T, bool IsIndirectKV /* = false */>
class flat_map_iterator15 {
public:
    using iterator_category = std::forward_iterator_tag;

    using value_type = T;
    using raw_value_type = typename std::remove_const<T>::type;
    using pointer = raw_value_type *;
    using const_pointer = const raw_value_type *;
    using reference = raw_value_type &;
    using const_reference = const raw_value_type &;

    using mutable_value_type = raw_value_type;
    using const_value_type = typename std::add_const<raw_value_type>::type;

    using opp_value_type = typename std::conditional<std::is_const<value_type>::value,
                                                     mutable_value_type,
                                                     const_value_type>::type;
    using opp_flat_map_iterator = flat_map_iterator15<HashMap, opp_value_type, IsIndirectKV>;

    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using group_type = typename HashMap::group_type;
    using slot_type = typename HashMap::slot_type;
    using locator_t = typename HashMap::locator_t;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

    static constexpr const size_type kGroupSize  = group_type::kGroupSize;
    static constexpr const size_type kGroupWidth = group_type::kGroupWidth;

protected:
    locator_t locator_;

public:
    flat_map_iterator15() noexcept : locator_() {}

    flat_map_iterator15(slot_type * slot) noexcept
        : locator_(nullptr, 0, const_cast<const slot_type *>(slot)) {}
    flat_map_iterator15(const slot_type * slot) noexcept
        : locator_(nullptr, 0, slot) {}

    flat_map_iterator15(const group_type * group, size_type pos, const slot_type * slot) noexcept
        : locator_(group, pos, slot) {}

    flat_map_iterator15(const locator_t & locator) noexcept
        : locator_(locator) {}

    flat_map_iterator15(const flat_map_iterator15 & src) noexcept
        : locator_(src.locator()) {}

    flat_map_iterator15(const opp_flat_map_iterator & src) noexcept
        : locator_(src.locator()) {}

    inline flat_map_iterator15 & operator = (const flat_map_iterator15 & rhs) noexcept {
        this->locator_ = rhs.locator();
        return *this;
    }

    inline flat_map_iterator15 & operator = (const opp_flat_map_iterator & rhs) noexcept {
        this->locator_ = rhs.locator();
        return *this;
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.locator() == rhs.locator());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.locator() != rhs.locator());
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.locator() == rhs.locator());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.locator() != rhs.locator());
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 & operator ++ () {
        this->increment();
        return *this;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 operator ++ (int) {
        flat_map_iterator15 copy(*this);
        ++(*this);
        return copy;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 & operator -- () {
        this->decrement();
        return *this;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 operator -- (int) {
        flat_map_iterator15 copy(*this);
        --(*this);
        return copy;
    }

    inline reference operator * () {
        slot_type * _slot = this->slot();
        return _slot->value;
    }

    inline const_reference operator * () const {
        const slot_type * _slot = this->slot();
        return _slot->value;
    }

    inline pointer operator -> () {
        slot_type * _slot = this->slot();
        return std::addressof(_slot->value);
    }

    inline const_pointer operator -> () const {
        const slot_type * _slot = this->slot();
        return std::addressof(_slot->value);
    }

    inline locator_t & locator() noexcept {
        return this->locator_;
    }

    inline const locator_t & locator() const noexcept {
        return this->locator_;
    }

    inline group_type * group() noexcept { return const_cast<group_type *>(this->locator_.group()); }
    inline const group_type * group() const noexcept { return this->locator_.group(); }

    inline size_type pos() const noexcept { return this->locator_.pos(); }

    inline slot_type * slot() noexcept { return const_cast<slot_type *>(this->locator_.slot()); }
    inline const slot_type * slot() const noexcept { return this->locator_.slot(); }

    inline ctrl_type * ctrl() noexcept {
        return this->locator_.ctrl();
    }

    inline const ctrl_type * ctrl() const noexcept {
        return this->locator_.ctrl();
    }

    inline ssize_type index(const hashmap_type * hashmap) const noexcept {
        assert(hashmap != nullptr);
        return (hashmap->group_at(this->group()) + this->pos());
    }

private:
    inline void increment() noexcept {
        this->locator_.increment();
    }

    inline void decrement() noexcept {
        this->locator_.decrement();
    }
};

#else // !(ITERATOR15_USE_LOCATOR15 != 0)

template <typename HashMap, typename T, bool IsIndirectKV /* = false */>
class flat_map_iterator15 {
public:
    using iterator_category = std::forward_iterator_tag;

    using value_type = T;
    using raw_value_type = typename std::remove_const<T>::type;
    using pointer = raw_value_type *;
    using const_pointer = const raw_value_type *;
    using reference = raw_value_type &;
    using const_reference = const raw_value_type &;

    using mutable_value_type = raw_value_type;
    using const_value_type = typename std::add_const<raw_value_type>::type;

    using opp_value_type = typename std::conditional<std::is_const<value_type>::value,
                                                     mutable_value_type,
                                                     const_value_type>::type;
    using opp_flat_map_iterator = flat_map_iterator15<HashMap, opp_value_type, IsIndirectKV>;

    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using group_type = typename HashMap::group_type;
    using slot_type = typename HashMap::slot_type;
    using locator_t = typename HashMap::locator_t;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

    static constexpr const size_type kGroupSize  = group_type::kGroupSize;
    static constexpr const size_type kGroupWidth = group_type::kGroupWidth;

protected:
    const ctrl_type *   ctrl_;
    const slot_type *   slot_;

public:
    flat_map_iterator15() noexcept : ctrl_(nullptr), slot_(nullptr) {}

    flat_map_iterator15(slot_type * slot) noexcept
        : ctrl_(nullptr), slot_(const_cast<const slot_type *>(slot)) {}
    flat_map_iterator15(const slot_type * slot) noexcept
        : ctrl_(nullptr), slot_(slot) {}

    flat_map_iterator15(const group_type * group, size_type pos, const slot_type * slot) noexcept
        : ctrl_(const_cast<const ctrl_type *>(
                reinterpret_cast<ctrl_type *>(const_cast<group_type *>(group)) + pos)),
          slot_(slot) {}

    flat_map_iterator15(const locator_t & locator) noexcept
        : flat_map_iterator15(locator.group(), locator.pos(), locator.slot()) {}

    flat_map_iterator15(const flat_map_iterator15 & src) noexcept
        : ctrl_(src.ctrl()), slot_(src.slot()) {}

    flat_map_iterator15(const opp_flat_map_iterator & src) noexcept
        : ctrl_(src.ctrl()), slot_(src.slot()) {}

    inline flat_map_iterator15 & operator = (const flat_map_iterator15 & rhs) noexcept {
        this->ctrl_ = rhs.ctrl();
        this->slot_ = rhs.slot();
        return *this;
    }

    inline flat_map_iterator15 & operator = (const opp_flat_map_iterator & rhs) noexcept {
        this->ctrl_ = rhs.ctrl();
        this->slot_ = rhs.slot();
        return *this;
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 & operator ++ () {
        this->increment();
        return *this;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 operator ++ (int) {
        flat_map_iterator15 copy(*this);
        ++(*this);
        return copy;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 & operator -- () {
        this->decrement();
        return *this;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator15 operator -- (int) {
        flat_map_iterator15 copy(*this);
        --(*this);
        return copy;
    }

    inline reference operator * () {
        slot_type * _slot = this->slot();
        return _slot->value;
    }

    inline const_reference operator * () const {
        const slot_type * _slot = this->slot();
        return _slot->value;
    }

    inline pointer operator -> () {
        slot_type * _slot = this->slot();
        return std::addressof(_slot->value);
    }

    inline const_pointer operator -> () const {
        const slot_type * _slot = this->slot();
        return std::addressof(_slot->value);
    }

    inline ctrl_type * ctrl() noexcept { return const_cast<ctrl_type *>(this->ctrl_); }
    inline const ctrl_type * ctrl() const noexcept { return this->ctrl_; }

    inline slot_type * slot() noexcept { return const_cast<slot_type *>(this->slot_); }
    inline const slot_type * slot() const noexcept { return this->slot_; }

    inline ssize_type index(const hashmap_type * hashmap) const noexcept {
        assert(hashmap != nullptr);
        return hashmap->ctrl_at(this->ctrl());
    }

private:
    JSTD_FORCED_INLINE
    void increment() noexcept {
        for (;;) {
            ++this->slot_;
            if ((reinterpret_cast<std::uintptr_t>(this->ctrl()) % kGroupWidth) == (kGroupSize - 1)) {
                this->ctrl_ += static_cast<difference_type>(kGroupWidth - (kGroupSize - 1));
                break;
            }
            ++this->ctrl_;
            if (this->ctrl_->is_empty())
                continue;
            if (likely(!(this->ctrl_->is_sentinel()))) {
                return;
            } else {
                this->slot_ = nullptr;
                return;
            }
        }

        for (;;) {
            const group_type * group = reinterpret_cast<group_type *>(this->ctrl());
            assert(group != nullptr);
            std::uint32_t used_mask = group->match_used();
            if (used_mask != 0) {
                std::uint32_t used_pos = BitUtils::bsf32(used_mask);
                if (likely(!(group->is_sentinel(used_pos)))) {
                    this->ctrl_ += static_cast<difference_type>(used_pos);
                    this->slot_ += static_cast<difference_type>(used_pos);
                } else {
                    this->slot_ = nullptr;
                }
                return;
            }
            this->ctrl_ += static_cast<difference_type>(kGroupWidth);
            this->slot_ += static_cast<difference_type>(kGroupSize);
        }
    }

    JSTD_FORCED_INLINE
    void decrement() noexcept {        
        for (;;) {
            --this->slot_;
            std::uintptr_t pos = reinterpret_cast<std::uintptr_t>(this->ctrl()) % kGroupWidth;
            if (pos == 0) {
                this->ctrl_ -= static_cast<difference_type>(kGroupWidth - (kGroupSize - 1));
                break;
            }
            --this->ctrl_;
            if (this->ctrl_->is_empty())
                continue;
            if (likely(!(this->ctrl_->is_sentinel()))) {
                return;
            } else {
                this->slot_ = nullptr;
                return;
            }
        }

        for (;;) {
            const group_type * group = reinterpret_cast<group_type *>(
                    reinterpret_cast<ctrl_type *>(
                        reinterpret_cast<std::uintptr_t>(this->ctrl()) & ~(kGroupWidth - 1)
                    )
                );
            assert(group != nullptr);
            std::uint32_t used_mask = group->match_used();
            if (used_mask != 0) {
                std::uint32_t used_pos = BitUtils::bsr32(used_mask);
                if (likely(!(group->is_sentinel(used_pos)))) {
                    this->ctrl_ -= (kGroupSize - 1) - static_cast<difference_type>(used_pos);
                    this->slot_ -= (kGroupSize - 1) - static_cast<difference_type>(used_pos);
                } else {
                    this->slot_ = nullptr;
                }
                return;
            }
            this->ctrl_ -= static_cast<difference_type>(kGroupWidth);
            this->slot_ -= static_cast<difference_type>(kGroupSize);
        }
    }
};

#endif // ITERATOR15_USE_LOCATOR15 != 0

template <typename HashMap, typename T>
class flat_map_iterator15<HashMap, T, true> {
public:
    using iterator_category = std::forward_iterator_tag;

    using value_type = T;
    using raw_value_type = typename std::remove_const<T>::type;
    using pointer = raw_value_type *;
    using const_pointer = const raw_value_type *;
    using reference = raw_value_type &;
    using const_reference = const raw_value_type &;

    using mutable_value_type = raw_value_type;
    using const_value_type = typename std::add_const<mutable_value_type>::type;

    using opp_value_type = typename std::conditional<std::is_const<value_type>::value,
                                                     mutable_value_type,
                                                     const_value_type>::type;
    using opp_flat_map_iterator = flat_map_iterator15<HashMap, opp_value_type, true>;

    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using group_type = typename HashMap::group_type;
    using slot_type = typename HashMap::slot_type;
    using locator_t = typename HashMap::locator_t;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

private:
    const slot_type * slot_;

public:
    flat_map_iterator15() noexcept : slot_(nullptr) {}

    flat_map_iterator15(slot_type * slot) noexcept
        : slot_(const_cast<const slot_type *>(slot)) {}
    flat_map_iterator15(const slot_type * slot) noexcept
        : slot_(slot) {}

    flat_map_iterator15(const group_type * group, size_type pos, const slot_type * slot) noexcept
        : slot_(slot) {}

    flat_map_iterator15(const locator_t & locator) noexcept
        : slot_(locator.slot()) {}

    flat_map_iterator15(const flat_map_iterator15 & src) noexcept
        : slot_(src.slot()) {}
    flat_map_iterator15(const opp_flat_map_iterator & src) noexcept
        : slot_(src.slot()) {}

    inline flat_map_iterator15 & operator = (const flat_map_iterator15 & rhs) noexcept {
        this->slot_ = rhs.slot();
        return *this;
    }

    inline flat_map_iterator15 & operator = (const opp_flat_map_iterator & rhs) noexcept {
        this->slot_ = rhs.slot();
        return *this;
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const flat_map_iterator15 & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    friend inline bool operator == (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator15 & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    inline flat_map_iterator15 & operator ++ () {
        ++(this->slot_);
        return *this;
    }

    inline flat_map_iterator15 operator ++ (int) {
        flat_map_iterator15 copy(*this);
        ++(*this);
        return copy;
    }

    inline flat_map_iterator15 & operator -- () {
        --(this->slot_);
        return *this;
    }

    inline flat_map_iterator15 operator -- (int) {
        flat_map_iterator15 copy(*this);
        --(*this);
        return copy;
    }

    inline reference operator * () {
        return const_cast<slot_type *>(this->slot_)->value;
    }

    inline const_reference operator * () const {
        return const_cast<slot_type *>(this->slot_)->value;
    }

    inline pointer operator -> () {
        return std::addressof(const_cast<slot_type *>(this->slot_)->value);
    }

    inline const_pointer operator -> () const {
        return std::addressof(const_cast<slot_type *>(this->slot_)->value);
    }

    inline group_type * group() noexcept { return nullptr; }
    inline const group_type * group() const noexcept { return nullptr; }

    inline size_type pos() const noexcept { return 0; }

    inline slot_type * slot() noexcept { return const_cast<slot_type *>(this->slot_); }
    inline const slot_type * slot() const noexcept { return this->slot_; }

    inline ctrl_type * ctrl() noexcept { return nullptr; }
    inline const ctrl_type * ctrl() const noexcept { return nullptr; }

    inline ssize_type index(const hashmap_type * hashmap) const noexcept {
        assert(hashmap != nullptr);
        return (hashmap->group_at(this->group()) + this->pos());
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_ITERATOR15_HPP
