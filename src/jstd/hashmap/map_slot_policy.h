/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2024 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

  -------------------------------------------------------------------

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

************************************************************************************/

#ifndef JSTD_HASHMAP_MAP_SLOT_POLICY_HPP
#define JSTD_HASHMAP_MAP_SLOT_POLICY_HPP

#pragma once

#include <memory>       // For std::allocator<T>
#include <utility>      // For std::pair<First, Second>
#include <type_traits>

#include "jstd/basic/stddef.h"
#include "jstd/lang/launder.h"
#include "jstd/traits/type_traits.h"

namespace jstd {

template <typename Key, typename Value>
union JSTD_DLL map_slot_type {
public:
    using key_type = typename std::remove_const<Key>::type;
    using mapped_type = typename std::remove_const<Value>::type;
    using value_type = std::pair<const key_type, mapped_type>;
    using mutable_value_type = std::pair<key_type, mapped_type>;
    using init_type = std::pair<key_type, mapped_type>;

    //
    // If std::pair<const K, V> and std::pair<K, V> are layout-compatible,
    // we can accept one or the other via slot_type. We are also free to
    // access the key via slot_type::key in this case.
    //
    static constexpr bool kIsMutableKey = jstd::is_layout_compatible_kv<Key, Value>::value;

    value_type          value;
    mutable_value_type  mutable_value;
    const key_type      key;
    key_type            mutable_key;

    map_slot_type() {}
    ~map_slot_type() = delete;
};

template <typename Key, typename Value, typename SlotType>
class JSTD_DLL map_slot_policy {
public:
    using slot_type = SlotType;
    using key_type = typename slot_type::key_type;
    using mapped_type = typename slot_type::mapped_type;
    using value_type = typename slot_type::value_type;
    using mutable_value_type = typename slot_type::mutable_value_type;
    using init_type = typename slot_type::init_type;

    using this_type = map_slot_policy<Key, Value, SlotType>;

    //
    // If std::pair<const K, V> and std::pair<K, V> are layout-compatible,
    // we can accept one or the other via slot_type. We are also free to
    // access the key via slot_type::key in this case.
    //
    static constexpr bool kIsMutableKey = slot_type::kIsMutableKey;

private:
    static void emplace(slot_type * slot) {
        // The construction of union doesn't do anything at runtime but it allows us
        // to access its members without violating aliasing rules.
        new (slot) slot_type;
    }

public:
    static value_type & element(slot_type * slot) {
        return slot->value;
    }

    static const value_type & element(const slot_type * slot) {
        return slot->value;
    }

    // When C++17 is available, we can use std::launder to provide mutable
    // access to the key for use in node handle.
#if defined(__cpp_lib_launder) && (__cpp_lib_launder >= 201606)
    static Key & mutable_key(slot_type * slot) {
        // Still check for kMutableKeys so that we can avoid calling std::launder
        // unless necessary because it can interfere with optimizations.
        return (kIsMutableKey ? slot->mutable_key :
                *jstd::launder(const_cast<Key *>(std::addressof(slot->value.first))));
    }
#else  // !(defined(__cpp_lib_launder) && (__cpp_lib_launder >= 201606))
    static const Key & mutable_key(slot_type * slot) {
        return key(slot);
    }
#endif

    static const Key & key(const slot_type * slot) {
        return (kIsMutableKey ? slot->mutable_key : slot->value.first);
    }

    template <typename Allocator, typename ... Args>
    static void construct(Allocator * alloc, slot_type * slot, Args && ... args) {
        this_type::emplace(slot);
        if (kIsMutableKey) {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->mutable_value,
                                                        std::forward<Args>(args)...);
        } else {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->value,
                                                        std::forward<Args>(args)...);
        }
    }

    //
    // Construct this slot by moving from another slot.
    //
    template <typename Allocator>
    static void construct(Allocator * alloc, slot_type * slot, slot_type * other) {
        this_type::emplace(slot);
        if (kIsMutableKey) {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->mutable_value,
                                                        std::move(other->mutable_value));
        } else {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->value,
                                                        std::move(other->value));
        }
    }

    //
    // Construct this slot by copying from another slot.
    //
    template <typename Allocator>
    static void construct(Allocator * alloc, slot_type * slot, const slot_type * other) {
        this_type::emplace(slot);
        if (kIsMutableKey) {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->mutable_value,
                                                        other->mutable_value);
        } else {
            std::allocator_traits<Allocator>::construct(*alloc, &slot->value, other->value);
        }
    }

    template <typename Allocator>
    static void destroy(Allocator * alloc, slot_type * slot) {
        if (kIsMutableKey) {
            std::allocator_traits<Allocator>::destroy(*alloc, &slot->mutable_value);
        } else {
            std::allocator_traits<Allocator>::destroy(*alloc, &slot->value);
        }
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, slot_type * src_slot) {
        if (kIsMutableKey) {
            dest_slot->mutable_value = std::move(src_slot->mutable_value);
        } else {
            dest_slot->value = std::move(src_slot->value);
        }
    }

    template <typename Allocator>
    static void assign(Allocator * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        if (kIsMutableKey) {
            dest_slot->mutable_value = src_slot->mutable_value;
        } else {
            dest_slot->value = src_slot->value;
        }
    }

    template <typename Allocator>
    static void mutable_assign(Allocator * alloc, slot_type * dest_slot, slot_type * src_slot) {
        dest_slot->mutable_value = std::move(src_slot->mutable_value);
    }

    template <typename Allocator>
    static void mutable_assign(Allocator * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        dest_slot->mutable_value = src_slot->mutable_value;
    }

    template <typename Allocator>
    static void transfer(Allocator * alloc, slot_type * new_slot, slot_type * old_slot) {
        this_type::emplace(new_slot);
        if (kIsMutableKey) {
            std::allocator_traits<Allocator>::construct(*alloc, &new_slot->mutable_value,
                                                        std::move(old_slot->mutable_value));
        } else {
            std::allocator_traits<Allocator>::construct(*alloc, &new_slot->value,
                                                        std::move(old_slot->value));
        }
        this_type::destroy(alloc, old_slot);
    }

    template <typename Allocator>
    static void swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        this_type::transfer(alloc, tmp, slot2);
        this_type::transfer(alloc, slot2, slot1);
        this_type::transfer(alloc, slot1, tmp);
    }

    template <typename Allocator>
    static void exchange(Allocator * alloc, slot_type * src, slot_type * dest, slot_type * empty) {
        this_type::transfer(alloc, empty, dest);
        this_type::transfer(alloc, dest, src);
    }

    template <typename Allocator>
    static void move_assign_swap(Allocator * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        this_type::mutable_assign(alloc, tmp, slot2);
        this_type::mutable_assign(alloc, slot2, slot1);
        this_type::mutable_assign(alloc, slot1, tmp);
    }
};

} // namespace jstd

#endif // JSTD_HASHMAP_MAP_SLOT_POLICY_HPP
