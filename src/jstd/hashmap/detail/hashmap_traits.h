
#ifndef JSTD_HASHMAP_DETAIL_HASHMAP_TRAITS_H
#define JSTD_HASHMAP_DETAIL_HASHMAP_TRAITS_H

#pragma once

#include <cstdint>
#include <type_traits>

#include "jstd/traits/type_traits.h"

namespace jstd {
namespace detail {

template <typename Container>
using is_map = std::integral_constant<bool, !std::is_same<typename Container::key_type,
                                                          typename Container::value_type>::value>;

template <typename Container, typename K>
using is_emplace_kv_able = std::integral_constant<bool,
    jstd::detail::is_map<Container>::value &&
    (jstd::is_similar<K, typename Container::key_type>::value ||
     jstd::is_complete_and_move_constructible<typename Container::key_type>::value)>;

namespace hash_detail {

template <typename IsAvalanching>
struct avalanching_value
{
    static constexpr bool value = IsAvalanching::value;
};

/* may be explicitly marked as DEPRECATED in the future */
template <>
struct avalanching_value<void>
{
    static constexpr bool value = true;
};

template <typename Hash, typename = void>
struct hash_is_avalanching_impl : std::false_type{};

template <typename Hash>
struct hash_is_avalanching_impl<Hash, jstd::void_t<typename Hash::is_avalanching>>
    : std::integral_constant<bool, avalanching_value<typename Hash::is_avalanching>::value>
{};

/* Hash::is_avalanching is not a type: compile error downstream */
template <typename Hash>
struct hash_is_avalanching_impl<Hash, typename std::enable_if<((void)Hash::is_avalanching, true)>::type>
{};

} // namespace hash_detail

/*
 * Each trait can be partially specialized by users for concrete hash functions
 * when actual characterization differs from default.
 */

/* 
 * hash_is_avalanching<Hash>::value is:
 *   - false if Hash::is_avalanching is not present.
 *   - Hash::is_avalanching::value if this is present and constexpr-convertible to a bool.
 *   - true if Hash::is_avalanching is void (deprecated).
 *   - ill-formed otherwise.
 */
template <typename Hash>
struct hash_is_avalanching : hash_detail::hash_is_avalanching_impl<Hash>::type {};

} // namespace detail
} // namespace jstd

#endif // JSTD_HASHMAP_DETAIL_HASHMAP_TRAITS_H
