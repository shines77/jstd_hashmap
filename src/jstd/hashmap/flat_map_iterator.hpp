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

#ifndef JSTD_HASHMAP_FLAT_MAP_ITERATOR_HPP
#define JSTD_HASHMAP_FLAT_MAP_ITERATOR_HPP

#pragma once

#include <cstdint>
#include <iterator>     // For std::forward_iterator_tag
#include <type_traits>  // For std::conditional, and so on...
#include <memory>       // For std::addressof()

#include <assert.h>

#define ITERATOR_USE_GROUP_SCAN  0

namespace jstd {

template <typename HashMap, typename T, bool IsIndirectKV /* = false */>
class flat_map_iterator {
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
    using opp_flat_map_iterator = flat_map_iterator<HashMap, opp_value_type, IsIndirectKV>;

    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using slot_type = typename HashMap::slot_type;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

private:
    const hashmap_type * hashmap_;
    ssize_type           index_;

public:
    flat_map_iterator() noexcept : hashmap_(nullptr), index_(0) {
    }
    flat_map_iterator(slot_type * slot) noexcept
        : hashmap_(nullptr), index_(0) {
    }
    flat_map_iterator(const slot_type * slot) noexcept
        : hashmap_(nullptr), index_(0) {
    }
    flat_map_iterator(hashmap_type * hashmap, size_type index) noexcept
        : hashmap_(const_cast<const hashmap_type *>(hashmap)),
          index_(static_cast<ssize_type>(index)) {
    }
    flat_map_iterator(const hashmap_type * hashmap, size_type index) noexcept
        : hashmap_(hashmap), index_(static_cast<ssize_type>(index)) {
    }
    flat_map_iterator(const flat_map_iterator & src) noexcept
        : hashmap_(src.hashmap()), index_(src.index()) {
    }
    flat_map_iterator(const opp_flat_map_iterator & src) noexcept
        : hashmap_(src.hashmap()), index_(src.index()) {
    }

    inline flat_map_iterator & operator = (const flat_map_iterator & rhs) noexcept {
        this->hashmap_ = rhs.hashmap();
        this->index_ = rhs.index();
        return *this;
    }

    inline flat_map_iterator & operator = (const opp_flat_map_iterator & rhs) noexcept {
        this->hashmap_ = rhs.hashmap();
        this->index_ = rhs.index();
        return *this;
    }

    friend inline bool operator == (const flat_map_iterator & lhs, const flat_map_iterator & rhs) noexcept {
        return (lhs.index() == rhs.index()) && (lhs.hashmap() == rhs.hashmap());
    }

    friend inline bool operator != (const flat_map_iterator & lhs, const flat_map_iterator & rhs) noexcept {
        return (lhs.index() != rhs.index()) || (lhs.hashmap() != rhs.hashmap());
    }

    friend inline bool operator == (const flat_map_iterator & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.index() == rhs.index()) && (lhs.hashmap() == rhs.hashmap());
    }

    friend inline bool operator != (const flat_map_iterator & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.index() != rhs.index()) || (lhs.hashmap() != rhs.hashmap());
    }

    JSTD_FORCED_INLINE
    flat_map_iterator & operator ++ () {
#if ITERATOR_USE_GROUP_SCAN
        ssize_type next_used_index = this->hashmap_->skip_empty_slots(this->index_);
        this->index_ = next_used_index;
        return *this;
#else
        ssize_type index = this->index_;
        const ctrl_type * ctrl = this->hashmap_->ctrl_at(index);
        ssize_type max_index = static_cast<ssize_type>(this->hashmap_->slot_capacity());

        while (index < max_index) {
            ++index;
            ++ctrl;
            if (!ctrl->is_empty())
                break;
        }

        this->index_ = index;
        return *this;
#endif // ITERATOR_USE_GROUP_SCAN
    }

    JSTD_FORCED_INLINE
    flat_map_iterator operator ++ (int) {
        flat_map_iterator copy(*this);
        ++*this;
        return copy;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator & operator -- () {
        ssize_type index = this->index_;
        const ctrl_type * ctrl = this->hashmap_->ctrl_at(index);

        while (index > 0) {
            --index;
            --ctrl;
            if (!ctrl->is_empty())
                break;
        }

        this->index_ = index;
        return *this;
    }

    JSTD_FORCED_INLINE
    flat_map_iterator operator -- (int) {
        flat_map_iterator copy(*this);
        --*this;
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

    inline hashmap_type * hashmap() noexcept {
        return this->hashmap_;
    }

    inline const hashmap_type * hashmap() const noexcept {
        return this->hashmap_;
    }

    inline ssize_type index() const noexcept {
        return this->index_;
    }

    inline ctrl_type * ctrl() noexcept {
        const ctrl_type * _ctrl = this->hashmap_->ctrl_at(this->index_);
        return const_cast<slot_type *>(_ctrl);
    }

    inline const ctrl_type * ctrl() const noexcept {
        const ctrl_type * _ctrl = this->hashmap_->ctrl_at(this->index_);
        return _ctrl;
    }

    inline slot_type * slot() noexcept {
        const slot_type * _slot = this->hashmap_->slot_at(this->index_);
        return const_cast<slot_type *>(_slot);
    }

    inline const slot_type * slot() const noexcept {
        const slot_type * _slot = this->hashmap_->slot_at(this->index_);
        return _slot;
    }
};

template <typename HashMap, typename T>
class flat_map_iterator<HashMap, T, true> {
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
    using opp_flat_map_iterator = flat_map_iterator<HashMap, opp_value_type, true>;

    using hashmap_type = HashMap;
    using ctrl_type = typename HashMap::ctrl_type;
    using slot_type = typename HashMap::slot_type;
    using size_type = typename HashMap::size_type;
    using ssize_type = typename HashMap::ssize_type;
    using difference_type = typename HashMap::difference_type;

private:
    const slot_type * slot_;

public:
    flat_map_iterator() noexcept : slot_(nullptr) {
    }
    flat_map_iterator(slot_type * slot) noexcept
        : slot_(const_cast<const slot_type *>(slot)) {
    }
    flat_map_iterator(const slot_type * slot) noexcept
        : slot_(slot) {
    }
    flat_map_iterator(hashmap_type * hashmap, size_type index) noexcept
        : slot_(hashmap->slot_at(index)) {
    }
    flat_map_iterator(const hashmap_type * hashmap, size_type index) noexcept
        : slot_(hashmap->slot_at(index)) {
    }
    flat_map_iterator(const flat_map_iterator & src) noexcept
        : slot_(src.slot()) {
    }
    flat_map_iterator(const opp_flat_map_iterator & src) noexcept
        : slot_(src.slot()) {
    }

    inline flat_map_iterator & operator = (const flat_map_iterator & rhs) noexcept {
        this->slot_ = rhs.slot();
        return *this;
    }

    inline flat_map_iterator & operator = (const opp_flat_map_iterator & rhs) noexcept {
        this->slot_ = rhs.slot();
        return *this;
    }

    friend inline bool operator == (const flat_map_iterator & lhs, const flat_map_iterator & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator & lhs, const flat_map_iterator & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    friend inline bool operator == (const flat_map_iterator & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() == rhs.slot());
    }

    friend inline bool operator != (const flat_map_iterator & lhs, const opp_flat_map_iterator & rhs) noexcept {
        return (lhs.slot() != rhs.slot());
    }

    inline flat_map_iterator & operator ++ () {
        ++(this->slot_);
        return *this;
    }

    inline flat_map_iterator operator ++ (int) {
        flat_map_iterator copy(*this);
        ++*this;
        return copy;
    }

    inline flat_map_iterator & operator -- () {
        --(this->slot_);
        return *this;
    }

    inline flat_map_iterator operator -- (int) {
        flat_map_iterator copy(*this);
        --*this;
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

    inline hashmap_type * hashmap() noexcept {
        return nullptr;
    }

    inline const hashmap_type * hashmap() const noexcept {
        return nullptr;
    }

    inline size_type index() const noexcept {
        return 0;
    }

    inline ctrl_type * ctrl() noexcept {
        return nullptr;
    }

    inline const ctrl_type * ctrl() const noexcept {
        return nullptr;
    }

    inline slot_type * slot() noexcept {
        return const_cast<slot_type *>(this->slot_);
    }

    inline const slot_type * slot() const noexcept {
        return this->slot_;
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_ITERATOR_HPP
