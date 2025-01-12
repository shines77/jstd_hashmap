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

#ifndef JSTD_HASHMAP_FLAT_MAP_TYPE_POLICY_HPP
#define JSTD_HASHMAP_FLAT_MAP_TYPE_POLICY_HPP

#pragma once

#include <type_traits>
#include <utility>          // For std::pair<F, S>

#include "jstd/basic/stddef.h"
#include "jstd/traits/type_traits.h"
#include "jstd/hashmap/map_types_constructibility.hpp"

namespace jstd {

template <typename Key, typename Value>
class JSTD_DLL flat_map_type_policy
{
public:
    typedef Key                                             key_type;
    typedef Value                                           mapped_type;
    typedef typename std::remove_const<Key>::type           raw_key_type;
    typedef typename std::remove_const<Value>::type         raw_mapped_type;

    typedef std::pair<raw_key_type, raw_mapped_type>        init_type;
    typedef std::pair<raw_key_type &&, raw_mapped_type &&>  moved_type;
    typedef std::pair<const raw_key_type, raw_mapped_type>  value_type;

    typedef value_type                                      element_type;

    typedef flat_map_type_policy<Key, Value>                this_type;

    using constructibility_checker = flat_map_types_constructibility<this_type>;

    static value_type & value_from(element_type & x) {
        return x;
    }

    template <typename K, typename V>
    static const raw_key_type & extract(const std::pair<K, V> & kv) {
        return kv.first;
    }

    static moved_type move(init_type & x) {
        return { std::move(x.first), std::move(x.second) };
    }

    static moved_type move(element_type & x) {
        // TODO: we probably need to launder here
        return { std::move(const_cast<raw_key_type &>(x.first)),
                 std::move(const_cast<raw_mapped_type &>(x.second)) };
    }

    template <typename Allocator, typename ... Args>
    static void construct(Allocator & al, init_type * p, Args &&... args) {
        constructibility_checker::check(al, p, std::forward<Args>(args)...);
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::construct(al, p, std::forward<Args>(args)...);
    }

    template <typename Allocator, typename ... Args>
    static void construct(Allocator & al, value_type * p, Args &&... args)
    {
        constructibility_checker::check(al, p, std::forward<Args>(args)...);
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::construct(al, p, std::forward<Args>(args)...);
    }

    template <typename Allocator, typename ... Args>
    static void construct(Allocator & al, key_type * p, Args &&... args)
    {
        constructibility_checker::check(al, p, std::forward<Args>(args)...);
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::construct(al, p, std::forward<Args>(args)...);
    }

    template <typename Allocator>
    static void destroy(Allocator & al, init_type * p) noexcept {
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::destroy(al, p);
    }

    template <typename Allocator>
    static void destroy(Allocator & al, value_type * p) noexcept {
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::destroy(al, p);
    }

    template <typename Allocator>
    static void destroy(Allocator & al, key_type * p) noexcept {
        std::allocator_traits<jstd::remove_cvref_t<decltype(al)>>::destroy(al, p);
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_TYPE_POLICY_HPP
