
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
    is_map<Container>::value &&
    (jstd::is_similar<K, typename Container::key_type>::value ||
     jstd::is_complete_and_move_constructible<typename Container::key_type>::value)>;

} // namespace detail
} // namespace jstd

#endif // JSTD_HASHMAP_DETAIL_HASHMAP_TRAITS_H
