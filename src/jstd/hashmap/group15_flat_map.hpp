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

#ifndef JSTD_HASHMAP_GROUP15_FLAT_MAP_HPP
#define JSTD_HASHMAP_GROUP15_FLAT_MAP_HPP

#pragma once

#include <stdint.h>

#include <cstdint>
#include <memory>               // For std::allocator<T>
#include <functional>           // For std::hash<Key>
#include <initializer_list>
#include <type_traits>
#include <utility>              // For std::pair<F, S>
#include <exception>
#include <stdexcept>

#include "jstd/hashmap/detail/hashmap_traits.h"
#include "jstd/hashmap/flat_map_type_policy.hpp"
#include "jstd/hashmap/group15_flat_table.hpp"

namespace jstd {

template <typename TypePolicy, typename Hash,
          typename KeyEqual, typename Allocator>
class group15_flat_table;

template <typename Key, typename Value,
          typename Hash = std::hash< typename std::remove_const<Key>::type >,
          typename KeyEqual = std::equal_to< typename std::remove_const<Key>::type >,
          typename Allocator = std::allocator< std::pair<const typename std::remove_const<Key>::type,
                                                         typename std::remove_const<Value>::type> > >
class JSTD_DLL group15_flat_map
{
public:
    typedef flat_map_type_policy<Key, Value>    type_policy;
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

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;

    typedef typename std::allocator_traits<allocator_type>::pointer         pointer;
    typedef typename std::allocator_traits<allocator_type>::const_pointer   const_pointer;

    typedef group15_flat_table<type_policy, Hash, KeyEqual,
        typename std::allocator_traits<Allocator>::template rebind_alloc<value_type>>
                                                table_type;

    typedef typename table_type::ctrl_type      ctrl_type;
    typedef typename table_type::slot_type      slot_type;

    typedef typename table_type::iterator       iterator;
    typedef typename table_type::const_iterator const_iterator;

    using this_type = group15_flat_map<Key, Value, Hash, KeyEqual, Allocator>;

private:
    table_type table_;

public:
    ///
    /// Constructors
    ///
    group15_flat_map() : group15_flat_map(0) {}

    explicit group15_flat_map(size_type capacity, hasher const & hash = hasher(),
                              key_equal const & pred = key_equal(),
                              allocator_type const & allocator = allocator_type())
        : table_(capacity, hash, pred, allocator) {
    }

    group15_flat_map(size_type capacity, allocator_type const & allocator)
        : group15_flat_map(capacity, hasher(), key_equal(), allocator) {
    }

    group15_flat_map(size_type capacity, hasher const & hash, allocator_type const & allocator)
        : group15_flat_map(capacity, hash, key_equal(), allocator) {
    }

    template <typename InputIterator>
    group15_flat_map(InputIterator first, InputIterator last, allocator_type const & allocator)
        : group15_flat_map(first, last, size_type(0), hasher(), key_equal(), allocator) {
    }

    explicit group15_flat_map(allocator_type const & allocator)
        : group15_flat_map(0, allocator) {
    }

    template <typename Iterator>
    group15_flat_map(Iterator first, Iterator last, size_type capacity = 0,
                     hasher const & hash = hasher(), key_equal const & pred = key_equal(),
                     allocator_type const & allocator = allocator_type())
        : group15_flat_map(capacity, hash, pred, allocator) {
        this->insert(first, last);
    }

    template <typename Iterator>
    group15_flat_map(Iterator first, Iterator last, size_type capacity, allocator_type const & allocator)
        : group15_flat_map(first, last, capacity, hasher(), key_equal(), allocator) {
    }

    template <typename Iterator>
    group15_flat_map(Iterator first, Iterator last, size_type capacity,
                     hasher const & hash, allocator_type const & allocator)
        : group15_flat_map(first, last, capacity, hash, key_equal(), allocator) {
    }

    group15_flat_map(group15_flat_map const & other) : table_(other.table_) {
    }

    group15_flat_map(group15_flat_map const & other, allocator_type const & allocator)
        : table_(other.table_, allocator) {
    }

    group15_flat_map(group15_flat_map && other)
        noexcept(std::is_nothrow_move_constructible<table_type>::value)
        : table_(std::move(other.table_)) {
    }

    group15_flat_map(group15_flat_map && other, allocator_type const & allocator)
        : table_(std::move(other.table_), allocator) {
    }

    group15_flat_map(std::initializer_list<value_type> ilist,
                     size_type capacity = 0, hasher const & hash = hasher(),
                     key_equal const & pred = key_equal(),
                     allocator_type const & allocator = allocator_type())
        : group15_flat_map(ilist.begin(), ilist.end(), capacity, hash, pred, allocator) {
    }

    group15_flat_map(std::initializer_list<value_type> ilist, allocator_type const & allocator)
        : group15_flat_map(ilist, size_type(0), hasher(), key_equal(), allocator) {
    }

    group15_flat_map(std::initializer_list<value_type> init, size_type capacity,
                     allocator_type const & allocator)
        : group15_flat_map(init, capacity, hasher(), key_equal(), allocator) {
    }

    group15_flat_map(std::initializer_list<value_type> init, size_type capacity,
                     hasher const & hash, allocator_type const & allocator)
        : group15_flat_map(init, capacity, hash, key_equal(), allocator) {
    }

    ~group15_flat_map() = default;

    group15_flat_map & operator = (group15_flat_map const & other) {
        table_ = other.table_;
        return *this;
    }

    group15_flat_map & operator = (group15_flat_map && other) noexcept(
        noexcept(std::declval<table_type &>() = std::declval<table_type &&>())) {
        table_ = std::move(other.table_);
        return *this;
    }

    group15_flat_map & operator = (std::initializer_list<value_type> il) {
        this->clear();
        this->insert(il.begin(), il.end());
        return *this;
    }

    ///
    /// Observers
    ///
    allocator_type get_allocator() const noexcept {
        return table_.get_allocator();
    }

    hasher hash_function() const noexcept {
        return table_.hash_function();
    }

    key_equal key_eq() const noexcept {
        return table_.key_eq();
    }

    static const char * name() noexcept {
        return table_type::name();
    }

    ///
    /// Iterators
    ///
    iterator begin() noexcept { return table_.begin(); }
    iterator end() noexcept { return table_.end(); }

    const_iterator begin() const noexcept { return table_.begin(); }
    const_iterator end() const noexcept { return table_.end(); }

    const_iterator cbegin() const noexcept { return table_.cbegin(); }
    const_iterator cend() const noexcept { return table_.cend(); }

    ///
    /// Capacity
    ///
    bool empty() const noexcept { return table_.empty(); }
    size_type size() const noexcept { return table_.size(); }
    size_type capacity() const noexcept { return table_.capacity(); }
    size_type max_size() const noexcept { return table_.max_size(); }

    size_type slot_size() const noexcept { return table_.slot_size(); }
    size_type slot_mask() const noexcept { return table_.slot_mask(); }
    size_type slot_capacity() const noexcept { return table_.slot_capacity(); }
    size_type slot_threshold() const noexcept { return table_.slot_threshold(); }

    size_type group_mask() const noexcept { return table_.group_mask(); }
    size_type group_capacity() const noexcept { return table_.group_capacity(); }

    bool is_valid() const noexcept { return table_.is_valid(); }
    bool is_empty() const noexcept { return table_.is_empty(); }

    ///
    /// Bucket interface
    ///
    size_type bucket_size(size_type n) const noexcept {
        return table_.bucket_size(n);
    }
    size_type bucket_count() const noexcept {
        return table_.bucket_count();
    }
    size_type max_bucket_count() const noexcept {
        return table_.max_bucket_count();
    }

    size_type bucket(const key_type & key) const {
        return table_.bucket(key);
    }

    ///
    /// Hash policy
    ///
    float load_factor() const { return table_.load_factor(); }
    float max_load_factor() const { return table_.max_load_factor(); }

    void max_load_factor(float mlf) { table_.max_load_factor(mlf); }

    ///
    /// Hash policy
    ///
    void reserve(size_type new_capacity) {
        table_.reserve(new_capacity);
    }

    void rehash(size_type new_capacity) {
        table_.rehash(new_capacity);
    }

    void shrink_to_fit(bool read_only = false) {
        table_.shrink_to_fit(read_only);
    }

    ///
    /// Lookup
    ///
    size_type count(const key_type & key) const {
        return table_.count(key);
    }

    bool contains(const key_type & key) const {
        return table_.contains(key);
    }

    mapped_type & at(const key_type & key) {
        auto pos = table_.find(key);
        if (pos != table_.end()) {
            return pos->second;
        }
        //
        // TODO: someday refactor this to conditionally serialize the key and
        // include it in the error message
        //
        throw std::out_of_range("key was not found in jstd::group15_flat_map");
    }

    const mapped_type & at(const key_type & key) const {
        auto pos = table_.find(key);
        if (pos != table_.end()) {
            return pos->second;
        }

        throw std::out_of_range("key was not found in jstd::group15_flat_map");
    }

    JSTD_FORCED_INLINE
    mapped_type & operator [] (const key_type & key) {
        return table_.try_emplace(key).first->second;
    }

    JSTD_FORCED_INLINE
    mapped_type & operator [] (key_type && key) {
        return table_.try_emplace(std::move(key)).first->second;
    }

    ///
    /// find(key)
    ///
    JSTD_FORCED_INLINE
    iterator find(const key_type & key) {
        return table_.find(key);
    }

    JSTD_FORCED_INLINE
    const_iterator find(const key_type & key) const {
        return table_.find(key);
    }

    template <typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, key_type>::value) &&
                std::is_constructible<key_type, const KeyT &>::value>::type * = nullptr>
    JSTD_FORCED_INLINE
    iterator find(const KeyT & key) {
        return table_.find(key);
    }

    template <typename KeyT, typename std::enable_if<
              (!jstd::is_same_ex<KeyT, key_type>::value) &&
                std::is_constructible<key_type, const KeyT &>::value>::type * = nullptr>
    JSTD_FORCED_INLINE
    const_iterator find(const KeyT & key) const {
        return table_.find(key);
    }

    ///
    /// Modifiers
    ///
    JSTD_FORCED_INLINE
    void clear(bool need_destroy = false) noexcept {
        table_.clear(need_destroy);
    }

    ///
    /// insert(value)
    ///
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(const value_type & value) {
        return table_.emplace(value);
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(value_type && value) {
        return table_.emplace(std::move(value));
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(const init_type & value) {
        return table_.emplace(value);
    }

    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(init_type && value) {
        return table_.emplace(std::move(value));
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value &&
               !jstd::is_same_ex<P, init_type >::value) &&
               (std::is_constructible<value_type, P &&>::value ||
                std::is_constructible<init_type,  P &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> insert(P && value) {
        return table_.emplace(std::forward<P>(value));
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, const value_type & value) {
        return table_.emplace(value).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, value_type && value) {
        return table_.emplace(std::move(value)).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, const init_type & value) {
        return table_.emplace(value).first;
    }

    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, init_type && value) {
        return table_.emplace(std::move(value)).first;
    }

    template <typename P, typename std::enable_if<
              (!jstd::is_same_ex<P, value_type>::value &&
               !jstd::is_same_ex<P, init_type >::value) &&
               (std::is_constructible<value_type, P &&>::value ||
                std::is_constructible<init_type,  P &&>::value)>::type * = nullptr>
    JSTD_FORCED_INLINE
    iterator insert(const_iterator hint, P && value) {
        return table_.emplace(hint, std::forward<P>(value)).first;
    }

    template <typename InputIter>
    JSTD_FORCED_INLINE
    void insert(InputIter first, InputIter last) {
        for (InputIter pos = first; pos != last; ++pos) {
            table_.emplace(*first);
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

    template <typename KeyT, typename MappedT, typename std::enable_if<
              !jstd::is_same_ex<KeyT, key_type>::value &&
               std::is_constructible<key_type, KeyT &&>::value &&
              !std::is_convertible<KeyT, iterator>::value &&
              !std::is_convertible<KeyT, const_iterator>::value>::type * = nullptr>
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

    template <typename KeyT, typename MappedT, typename std::enable_if<
              !jstd::is_same_ex<KeyT, key_type>::value &&
               std::is_constructible<key_type, KeyT &&>::value &&
              !std::is_convertible<KeyT, iterator>::value &&
              !std::is_convertible<KeyT, const_iterator>::value>::type * = nullptr>
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
        return table_.emplace(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    iterator emplace_hint(const_iterator hint, Args && ... args) {
        return table_.emplace(std::forward<Args>(args)...).first;
    }

    ///
    /// try_emplace(key, args...)
    ///
    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(const key_type & key, Args && ... args) {
        return table_.try_emplace(key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    std::pair<iterator, bool> try_emplace(key_type && key, Args && ... args) {
        return table_.try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    typename std::enable_if<jstd::are_transparent<KeyT, Hash, KeyEqual>::value &&
                           (!std::is_convertible<key_type, const KeyT &>::value &&
                            !std::is_convertible<key_type, KeyT &&>::value) &&
                            !std::is_same<key_type, KeyT>::value &&
                            !std::is_convertible<KeyT, iterator>::value &&
                            !std::is_convertible<KeyT, const_iterator>::value,
                             std::pair<iterator, bool> >::type
    try_emplace(KeyT && key, Args && ... args) {
        return table_.try_emplace(std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    // For compatibility with versions below C++17 that do not support T::is_transparent
    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    typename std::enable_if<!jstd::are_transparent<KeyT, Hash, KeyEqual>::value ||
                            (std::is_convertible<key_type, const KeyT &>::value ||
                             std::is_convertible<key_type, KeyT &&>::value) &&
                            !std::is_same<key_type, KeyT>::value &&
                            !std::is_convertible<KeyT, iterator>::value &&
                            !std::is_convertible<KeyT, const_iterator>::value,
                             std::pair<iterator, bool> >::type
    try_emplace(KeyT && key, Args && ... args) {
        return table_.try_emplace(std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    iterator try_emplace(const_iterator hint, const key_type & key, Args && ... args) {
        return table_.try_emplace(hint, key, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    JSTD_FORCED_INLINE
    iterator try_emplace(const_iterator hint, key_type && key, Args && ... args) {
        return table_.try_emplace(hint, std::move(key), std::forward<Args>(args)...);
    }

    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    typename std::enable_if<jstd::are_transparent<KeyT, Hash, KeyEqual>::value &&
                           (!std::is_convertible<key_type, const KeyT &>::value &&
                            !std::is_convertible<key_type, KeyT &&>::value) &&
                            !std::is_same<key_type, KeyT>::value &&
                            !std::is_convertible<KeyT, iterator>::value &&
                            !std::is_convertible<KeyT, const_iterator>::value,
                             iterator >::type
    try_emplace(const_iterator hint, KeyT && key, Args && ... args) {
        return table_.try_emplace(hint, std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    // For compatibility with versions below C++17 that do not support T::is_transparent
    template <typename KeyT, typename ... Args>
    JSTD_FORCED_INLINE
    typename std::enable_if<!jstd::are_transparent<KeyT, Hash, KeyEqual>::value ||
                            (std::is_convertible<key_type, const KeyT &>::value ||
                             std::is_convertible<key_type, KeyT &&>::value) &&
                            !std::is_same<key_type, KeyT>::value &&
                            !std::is_convertible<KeyT, iterator>::value &&
                            !std::is_convertible<KeyT, const_iterator>::value,
                             iterator >::type
    try_emplace(const_iterator hint, KeyT && key, Args && ... args) {
        return table_.try_emplace(hint, std::forward<KeyT>(key), std::forward<Args>(args)...);
    }

    ///
    /// erase(key)
    ///
    JSTD_FORCED_INLINE
    size_type erase(const key_type & key) {
        return table_.erase(key);
    }

    JSTD_FORCED_INLINE
    size_type erase(iterator pos) {
        return table_.erase(pos);
    }

    JSTD_FORCED_INLINE
    size_type erase(const_iterator pos) {
        return table_.erase(pos);
    }

    JSTD_FORCED_INLINE
    iterator erase(iterator first, iterator last) {
        if (likely(first.hashmap() == this)) {
            for (; first != last; ++first) {
                this->erase(first);
            }
        } else {
            for (; first != last; ++first) {
                this->erase_slot(*first);
            }
        }
        return { last };
    }

    JSTD_FORCED_INLINE
    iterator erase(const_iterator first, const_iterator last) {
        return this->erase(first, last);
    }

    template <typename InputIter, typename std::enable_if<
              !jstd::is_same_ex<InputIter, iterator      >::value &&
              !jstd::is_same_ex<InputIter, const_iterator>::value>::type * = nullptr>
    JSTD_FORCED_INLINE
    iterator erase(InputIter first, InputIter last) {
        size_type num_deleted = 0;
        for (; first != last; ++first) {
            num_deleted += static_cast<size_type>(this->erase(*first));
        }
        return num_deleted;
    }

    JSTD_FORCED_INLINE
    void swap(this_type & other) noexcept(
        noexcept(std::declval<table_type &>().swap(std::declval<table_type &>()))) {
        table_.swap(other.table_);
    }

    JSTD_FORCED_INLINE
    friend void swap(this_type & lhs, this_type & rhs)
        noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }
};

/**
 * Specializes the @c std::swap algorithm for @c robin_hash_map. Calls @c lhs.swap(rhs).
 *
 * @param lhs the map on the left side to swap
 * @param lhs the map on the right side to swap
 */

template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Alloc>
inline
void swap(group15_flat_map<Key, Value, Hash, KeyEqual, Alloc> & lhs,
          group15_flat_map<Key, Value, Hash, KeyEqual, Alloc> & rhs)
          noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

} // namespace jstd

///////////////////////////////////////////////////////////
// std extensions: std::erase_if()
///////////////////////////////////////////////////////////

namespace std {

/**
 * Specializes the @c std::swap algorithm for @c robin_hash_map. Calls @c lhs.swap(rhs).
 *
 * @param lhs the map on the left side to swap
 * @param lhs the map on the right side to swap
 */

template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Alloc>
inline
void swap(jstd::group15_flat_map<Key, Value, Hash, KeyEqual, Alloc> & lhs,
          jstd::group15_flat_map<Key, Value, Hash, KeyEqual, Alloc> & rhs)
          noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Alloc, typename Pred>
typename jstd::group15_flat_map<Key, Value, Hash, KeyEqual, Alloc>::size_type
inline
erase_if(jstd::group15_flat_map<Key, Value, Hash, KeyEqual, Alloc> & hash_map, Pred pred)
{
    auto old_size = hash_map.size();

    auto first = hash_map.begin();
    auto last = hash_map.end();
    auto iter = last;
    while (iter != first) {
        --iter;
        if (pred(*iter)) {
            hash_map.erase(iter);
        }
    }

    return (old_size - hash_map.size());
}

} // namespace std

#endif // JSTD_HASHMAP_GROUP15_FLAT_MAP_HPP
