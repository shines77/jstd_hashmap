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

#ifndef JSTD_HASHMAP_FLAT_MAP_SLOT_STORAGE_HPP
#define JSTD_HASHMAP_FLAT_MAP_SLOT_STORAGE_HPP

#pragma once

#include <type_traits>

#include "jstd/basic/stddef.h"

namespace jstd {

template <typename TypePolicy,
          bool kIsIndirectKey /* = false */,
          bool kIsIndirectValue /* = false */>
class JSTD_DLL flat_map_slot_storage
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

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;

    flat_map_slot_storage() : slots_(nullptr) {}
    ~flat_map_slot_storage() {}

    value_type * slots() { return this->slots_; }
    const value_type * slots() const {
        return reinterpret_cast<const value_type *>(this->slots_);
    }

private:
    value_type slots_;
};

template <typename TypePolicy>
class JSTD_DLL flat_map_slot_storage<TypePolicy, true, true>
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

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;
};

template <typename TypePolicy>
class JSTD_DLL flat_map_slot_storage<TypePolicy, true, false>
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

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;
};

template <typename TypePolicy>
class JSTD_DLL flat_map_slot_storage<TypePolicy, false, true>
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

    typedef value_type &                        reference;
    typedef value_type const &                  const_reference;
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_SLOT_STORAGE_HPP

