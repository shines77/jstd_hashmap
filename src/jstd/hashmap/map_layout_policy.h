
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

namespace jstd {

namespace detail {

template <typename T>
struct is_plain_type {
    static constexpr bool value = (std::is_arithmetic<T>::value || std::is_enum<T>::value);
};

} // namespace detail

template <typename Key, typename Value>
struct default_layout_policy {
    static constexpr bool autoDetectPairLayout = true;
    static constexpr bool keyValueIsIsolated = false;

    static constexpr bool autoDetectKeyInlined = true;
    static constexpr bool keyIsInlined = true;

    static constexpr bool autoDetectValueInlined = true;
    static constexpr bool valueIsInlined = true;

    static constexpr bool autoDetectStoreHash = true;
    static constexpr bool needStoreHash = true;
};

} // namespace jstd