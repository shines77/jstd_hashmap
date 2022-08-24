
/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2022 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

  -------------------------------------------------------------------

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

************************************************************************************/

#pragma once

#include <type_traits>
#include "jstd/traits/has_member.h"

namespace jstd {

namespace detail {

template <typename T>
struct is_plain_type {
    static constexpr bool value = (std::is_arithmetic<T>::value || std::is_enum<T>::value);
};

template <typename T>
struct is_swappable {
    static constexpr bool value = is_plain_type<T>::value || has_member_swap<T, T &>::value;
};

} // namespace detail

template <typename Key, typename Value>
struct default_layout_policy {
    static constexpr bool autoDetectPairLayout = true;
    static constexpr bool isIsolatedKeyValue = false;

    static constexpr bool autoDetectIsIndirectKey = true;
    static constexpr bool isIndirectKey = false;

    static constexpr bool autoDetectIsIndirectValue = true;
    static constexpr bool isIndirectValue = true;

    static constexpr bool autoDetectStoreHash = true;
    static constexpr bool needStoreHash = true;
};

} // namespace jstd
