/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2024 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/cluster_flat_map
  https://gitee.com/shines77/cluster_flat_map

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

#ifndef JSTD_HASHMAP_MAP_TYPES_CONSTRUCTIBILITY_HPP
#define JSTD_HASHMAP_MAP_TYPES_CONSTRUCTIBILITY_HPP

#pragma once

#include <type_traits>
#include <utility>      // For std::pair<F, S>
#include <memory>       // For std::allocator<T>

namespace jstd {

template <typename Key, typename... Args>
struct check_key_type_t
{
    static_assert(std::is_constructible<Key, Args...>::value,
                  "key_type must be constructible from Args");
};

template <typename Key>
struct check_key_type_t<Key>
{
    static_assert(std::is_constructible<Key>::value,
                  "key_type must be default constructible");
};

template <typename Key>
struct check_key_type_t<Key, const Key &>
{
    static_assert(std::is_constructible<Key, const Key &>::value,
                  "key_type must be copy constructible");
};

template <typename Key>
struct check_key_type_t<Key, Key &&>
{
    static_assert(std::is_constructible<Key, Key &&>::value,
                  "key_type must be move constructible");
};

template <typename Mapped, typename... Args>
struct check_mapped_type_t
{
    static_assert(std::is_constructible<Mapped, Args...>::value,
                  "mapped_type must be constructible from Args");
};

template <typename Mapped>
struct check_mapped_type_t<Mapped>
{
    static_assert(std::is_constructible<Mapped>::value,
                  "mapped_type must be default constructible");
};

template <typename Mapped>
struct check_mapped_type_t<Mapped, const Mapped &>
{
    static_assert(std::is_constructible<Mapped, const Mapped &>::value,
                  "mapped_type must be copy constructible");
};

template <typename Mapped>
struct check_mapped_type_t<Mapped, Mapped &&>
{
    static_assert(std::is_constructible<Mapped, Mapped &&>::value,
                  "mapped_type must be move constructible");
};

template <typename TypePolicy>
struct flat_map_types_constructibility
{
    using key_type = typename TypePolicy::key_type;
    using mapped_type = typename TypePolicy::mapped_type;
    using init_type = typename TypePolicy::init_type;
    using value_type = typename TypePolicy::value_type;

    template <typename A, typename X, typename... Args>
    static void check(A&, X*, Args&&...)
    {
        // Pass through, as we cannot say anything about a general allocator
    }

    template <typename ... Args>
    static void check_key_type()
    {
        (void)check_key_type_t<key_type, Args...>{};
    }

    template <typename ... Args>
    static void check_mapped_type()
    {
        (void)check_mapped_type_t<mapped_type, Args...>{};
    }

    template <typename Arg>
    static void check(std::allocator<value_type> &, key_type *, Arg &&)
    {
        check_key_type<Arg &&>();
    }

    template <typename Arg1, typename Arg2>
    static void check(
        std::allocator<value_type> &, value_type *, Arg1 &&, Arg2 &&)
    {
        check_key_type<Arg1 &&>();
        check_mapped_type<Arg2 &&>();
    }

    template <typename Arg1, typename Arg2>
    static void check(std::allocator<value_type> &, value_type *,
                      const std::pair<Arg1, Arg2> &)
    {
        check_key_type<const Arg1 &>();
        check_mapped_type<const Arg2 &>();
    }

    template <typename Arg1, typename Arg2>
    static void check(
        std::allocator<value_type> &, value_type *, std::pair<Arg1, Arg2> &&)
    {
        check_key_type<Arg1 &&>();
        check_mapped_type<Arg2 &&>();
    }

    template <typename ... Args1, typename ... Args2>
    static void check(std::allocator<value_type> &, value_type *,
                      std::piecewise_construct_t, std::tuple<Args1...> &&,
                      std::tuple<Args2...> &&)
    {
        check_key_type<Args1 && ...>();
        check_mapped_type<Args2 && ...>();
    }

    template <typename Arg1, typename Arg2>
    static void check(std::allocator<value_type> &, init_type *, Arg1 &&, Arg2 &&)
    {
        check_key_type<Arg1 &&>();
        check_mapped_type<Arg2 &&>();
    }

    template <typename Arg1, typename Arg2>
    static void check(std::allocator<value_type> &, init_type *,
                      const std::pair<Arg1, Arg2> &)
    {
        check_key_type<const Arg1 &>();
        check_mapped_type<const Arg2 &>();
    }

    template <typename Arg1, typename Arg2>
    static void check(
        std::allocator<value_type> &, init_type *, std::pair<Arg1, Arg2> &&)
    {
        check_key_type<Arg1 &&>();
        check_mapped_type<Arg2 &&>();
    }

    template <typename ... Args1, typename ... Args2>
    static void check(std::allocator<value_type> &, init_type *,
                      std::piecewise_construct_t, std::tuple<Args1...> &&,
                      std::tuple<Args2...> &&)
    {
        check_key_type<Args1 && ...>();
        check_mapped_type<Args2 && ...>();
    }
};

template <typename TypePolicy>
struct set_types_constructibility
{
    using key_type = typename TypePolicy::key_type;
    using value_type = typename TypePolicy::value_type;
    static_assert(std::is_same<key_type, value_type>::value, "");

    template <typename A, typename X, typename ... Args>
    static void check(A &, X *, Args && ...)
    {
        // Pass through, as we cannot say anything about a general allocator
    }

    template <typename ... Args>
    static void check(std::allocator<value_type> &, key_type *, Args && ...)
    {
        (void)check_key_type_t<key_type, Args && ...>{};
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_MAP_TYPES_CONSTRUCTIBILITY_HPP
