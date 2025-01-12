/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2024 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

  -------------------------------------------------------------------

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

************************************************************************************/

#ifndef JSTD_HASHMAP_FLAT_MAP_SLOT_POLICY_HPP
#define JSTD_HASHMAP_FLAT_MAP_SLOT_POLICY_HPP

#pragma once

#include <type_traits>

#include "jstd/basic/stddef.h"
#include "jstd/traits/type_traits.h"
#include "jstd/hashmap/map_slot_policy.h"

namespace jstd {

template <typename Key, typename Value, typename SlotType>
class JSTD_DLL flat_map_slot_policy
{
public:
    using slot_policy = map_slot_policy<Key, Value, SlotType>;
    using slot_type = typename slot_policy::slot_type;
    using key_type = typename slot_policy::key_type;
    using mapped_type = typename slot_policy::mapped_type;
    using value_type = typename slot_policy::value_type;
    using mutable_value_type = typename slot_policy::mutable_value_type;
    using init_type = typename slot_policy::init_type;

    using this_type = flat_map_slot_policy<Key, Value, SlotType>;

    template <typename Allocator, typename ... Args>
    static void construct(Allocator * alloc, slot_type * slot, Args &&... args) {
        slot_policy::construct(alloc, slot, std::forward<Args>(args)...);
    }

    template <typename Allocator>
    static void destroy(Allocator * alloc, slot_type * slot) {
        slot_policy::destroy(alloc, slot);
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, slot_type * src_slot) {
        slot_policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        slot_policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Allocator>
    static void transfer(Allocator * alloc, slot_type * new_slot, slot_type * old_slot) {
        slot_policy::transfer(alloc, new_slot, old_slot);
    }

    template <typename Allocator>
    static void swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        slot_policy::swap(alloc, slot1, slot2, tmp);
    }

    template <typename Allocator>
    static void exchange(Allocator * alloc, slot_type * src, slot_type * dest, slot_type * empty) {
        slot_policy::exchange(alloc, src, dest, empty);
    }

    template <typename Allocator>
    static void move_assign_swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        slot_policy::move_assign_swap(alloc, slot1, slot2, tmp);
    }

    static std::size_t extra_space(const slot_type *) {
        return 0;
    }

    template <typename First, typename ... Args>
    static decltype(jstd::DecomposePair(
        std::declval<First>(), std::declval<Args>()...))
        apply(First && f, Args &&... args) {
        return jstd::DecomposePair(std::forward<First>(f), std::forward<Args>(args)...);
    }

    static std::pair<const key_type, mapped_type> & element(slot_type * slot) {
        return slot->value;
    }

    static Value & value(std::pair<const key_type, mapped_type> * kv) {
        return kv->second;
    }

    static const Value & value(const std::pair<const key_type, mapped_type> * kv) {
        return kv->second;
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_FLAT_MAP_SLOT_POLICY_HPP
