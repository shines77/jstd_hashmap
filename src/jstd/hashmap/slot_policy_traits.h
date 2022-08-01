
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

#include "jstd/config/config.h"
#include "jstd/lang/launder.h"
#include "jstd/type_traits.h"

namespace jstd {

//
// Defines how slots are initialized, destroyed, moved,
//                       transfer, swap or exchange.
//
template <typename SlotPolicy, typename = void>
class slot_policy_traits {
public:
    // The slot policy of hash table.
    using slot_policy = SlotPolicy;

    // The type of the keys stored in the hash table.
    using key_type = typename SlotPolicy::key_type;
    using mapped_type = typename SlotPolicy::mapped_type;

    // The actual object stored in the hash table.
    using slot_type = typename SlotPolicy::slot_type;

    // The argument type for insertions into the hash table. This is different
    // from value_type for increased performance. See std::initializer_list<T> constructor
    // and insert() member functions for more details.
    using init_type = typename SlotPolicy::init_type;

    using reference = decltype(SlotPolicy::element(std::declval<slot_type *>()));
    using pointer = typename std::remove_reference<reference>::type *;
    using value_type = typename std::remove_reference<reference>::type;

private:
    struct ReturnKey {
        // When C++17 is available, we can use std::launder to provide mutable
        // access to the key for use in node handle.
        // Here, instead to use jstd::launder.
        template <typename Key, typename std::enable_if<std::is_lvalue_reference<Key>::value, int>::type = 0>
        static key_type & ReturnKey_Impl(Key && key, int) {
            return *launder(const_cast<key_type *>(std::addressof(std::forward<Key>(key))));
        }

        template <typename Key>
        static Key ReturnKey_Impl(Key && key, char) {
            return std::forward<Key>(key);
        }

        // When Key = T &, we forward the lvalue reference.
        // When Key = T, we return by value to avoid a dangling reference.
        // eg, for Key is std::string.
        template <typename Key, typename ... Args>
        auto operator () (Key && key, const Args & ...) const
            -> decltype(ReturnKey_Impl(std::forward<Key>(key), 0)) {
            return ReturnKey_Impl(std::forward<Key>(key), 0);
        }
    };

    template <typename Policy = SlotPolicy, typename = void>
    struct ConstantIteratorsImpl : std::false_type {
    };

    template <typename Policy>
    struct ConstantIteratorsImpl<Policy, void_t<typename Policy::constant_iterators>>
        : Policy::constant_iterators {
    };

public:
    // Policies can set this variable to tell hashmap that all iterators
    // should be constant, even `iterator`. This is useful for set-like containers.
    // Defaults to false if not provided by the policy.
    using constant_iterators = ConstantIteratorsImpl<>;

    // PRECONDITION:  `slot` is UNINITIALIZED
    // POSTCONDITION: `slot` is INITIALIZED
    template <typename Alloc, typename ... Args>
    static void construct(Alloc * alloc, slot_type * slot, Args && ... args) {
        SlotPolicy::construct(alloc, slot, std::forward<Args>(args)...);
    }

    // PRECONDITION:  `slot` is INITIALIZED
    // POSTCONDITION: `slot` is UNINITIALIZED
    template <typename Alloc>
    static void destroy(Alloc * alloc, slot_type * slot) {
        SlotPolicy::destroy(alloc, slot);
    }

    // Transfers the `old_slot` to `new_slot`. Any memory allocated by the
    // allocator inside `old_slot` to `new_slot` can be transferred.
    //
    // OPTIONAL: defaults to:
    //
    //     clone(new_slot, std::move(*old_slot));
    //     destroy(old_slot);
    //
    // PRECONDITION:  `new_slot` is UNINITIALIZED and `old_slot` is INITIALIZED
    // POSTCONDITION: `new_slot` is INITIALIZED and `old_slot` is
    //                UNINITIALIZED
    template <typename Alloc>
    static void transfer(Alloc * alloc, slot_type * new_slot, slot_type * old_slot) {
        slot_policy_traits::transfer_impl(alloc, new_slot, old_slot, 0);
    }

    // PRECONDITION:  `slot` is INITIALIZED
    // POSTCONDITION: `slot` is INITIALIZED
    template <typename Policy = SlotPolicy>
    static auto element(slot_type * slot) -> decltype(Policy::element(slot)) {
        return Policy::element(slot);
    }

private:
    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto transfer_impl(Alloc * alloc, slot_type * new_slot, slot_type * old_slot, int)
        -> decltype((void)Policy::transfer(alloc, new_slot, old_slot)) {
        Policy::transfer(alloc, new_slot, old_slot);
    }

    template <typename Alloc>
    static void transfer_impl(Alloc * alloc, slot_type * new_slot, slot_type * old_slot, char) {
        slot_policy_traits::construct(alloc, new_slot, std::move(element(old_slot)));
        slot_policy_traits::destroy(alloc, old_slot);
    }
};

} // namespace jstd