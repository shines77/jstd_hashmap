
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
#include <memory>

#include "jstd/basic/stddef.h"
#include "jstd/lang/launder.h"
#include "jstd/traits/type_traits.h"

namespace jstd {

//
// Defines how slots are initialized, destroyed, moved,
//                       transfer, swap or exchange.
//
template <typename SlotPolicy, typename = void>
class JSTD_DLL slot_policy_traits {
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
    struct ConstantIteratorsImpl<Policy, jstd::void_t<typename Policy::constant_iterators>>
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
        slot_policy::construct(alloc, slot, std::forward<Args>(args)...);
    }

    // PRECONDITION:  `slot` is INITIALIZED
    // POSTCONDITION: `slot` is UNINITIALIZED
    template <typename Alloc>
    static void destroy(Alloc * alloc, slot_type * slot) {
        slot_policy::destroy(alloc, slot);
    }

    //
    // Assign the `src_slot` to `dest_slot`.
    //
    // OPTIONAL: defaults to:
    //
    //     dest_slot = std::move(src_slot);
    //
    // PRECONDITION:  `dest_slot` is UNINITIALIZED and `src_slot` is INITIALIZED
    // POSTCONDITION: `dest_slot` is INITIALIZED   and `src_slot` is UNINITIALIZED
    //
    template <typename Alloc>
    static void assign(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot) {
        assign_impl(alloc, dest_slot, src_slot, 0);
    }

    template <typename Alloc>
    static void mutable_assign(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot) {
        mutable_assign_impl(alloc, dest_slot, src_slot, 0);
    }

    //
    // Assign the `src_slot` to `dest_slot`.
    //
    // OPTIONAL: defaults to:
    //
    //     dest_slot = src_slot;
    //
    // PRECONDITION:  `dest_slot` is UNINITIALIZED and `src_slot` is INITIALIZED
    // POSTCONDITION: `dest_slot` is INITIALIZED   and `src_slot` is INITIALIZED
    //
    template <typename Alloc>
    static void assign(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        assign_impl(alloc, dest_slot, src_slot, 0);
    }

    template <typename Alloc>
    static void mutable_assign(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot) {
        mutable_assign_impl(alloc, dest_slot, src_slot, 0);
    }

    //
    // Transfers the `old_slot` to `new_slot`. Any memory allocated by the
    // allocator inside `old_slot` to `new_slot` can be transferred.
    //
    // OPTIONAL: defaults to:
    //
    //     clone(new_slot, std::move(*old_slot));
    //     destroy(old_slot);
    //
    // PRECONDITION:  `new_slot` is UNINITIALIZED and `old_slot` is INITIALIZED
    // POSTCONDITION: `new_slot` is INITIALIZED   and `old_slot` is UNINITIALIZED
    //
    template <typename Alloc>
    static void transfer(Alloc * alloc, slot_type * new_slot, slot_type * old_slot) {
        transfer_impl(alloc, new_slot, old_slot, 0);
    }

    //
    // Swap the `slot1` and `slot2` by 'tmp`.
    //
    // OPTIONAL: defaults to:
    //
    //     swap(slot1, slot2, tmp);
    //
    // PRECONDITION:  `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    // POSTCONDITION: `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    //
    template <typename Alloc>
    static void swap(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        swap_impl(alloc, slot1, slot2, tmp, 0);
    }

    //
    // Swap the `slot1` and `slot2` by 'tmp` use move assignment.
    //
    // OPTIONAL: defaults to:
    //
    //     swap(slot1, slot2, tmp);
    //
    // PRECONDITION:  `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    // POSTCONDITION: `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    //
    template <typename Alloc>
    static void move_assign_swap(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp) {
        move_assign_swap_impl(alloc, slot1, slot2, tmp, 0);
    }

    //
    // Swap the `slot1` and `slot2` by 'tmp`.
    //
    // OPTIONAL: defaults to:
    //
    //     swap(slot1, slot2, tmp);
    //
    // PRECONDITION:  `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    // POSTCONDITION: `slot1` is INITIALIZED, `slot2` is INITIALIZED and `tmp` is UNINITIALIZED
    //
    template <typename Alloc>
    static void exchange(Alloc * alloc, slot_type * src, slot_type * dest, slot_type * empty) {
        exchange_impl(alloc, src, dest, empty, 0);
    }

    // PRECONDITION:  `slot` is INITIALIZED
    // POSTCONDITION: `slot` is INITIALIZED
    template <typename Policy = SlotPolicy>
    static auto element(slot_type * slot) -> decltype(Policy::element(slot)) {
        return Policy::element(slot);
    }

    // Provides generalized access to the key for elements, both for elements in
    // the table and for elements that have not yet been inserted (or even
    // constructed).  We would like an API that allows us to say: `key(args...)`
    // but we cannot do that for all cases, so we use this more general API that
    // can be used for many things, including the following:
    //
    //   - Given an element in a table, get its key.
    //   - Given an element initializer, get its key.
    //   - Given `emplace()` arguments, get the element key.
    //
    // Implementations of this must adhere to a very strict technical
    // specification around aliasing and consuming arguments:
    //
    // Let `value_type` be the result type of `element()` without ref- and
    // cv-qualifiers. The first argument is a functor, the rest are constructor
    // arguments for `value_type`. Returns `std::forward<F>(f)(k, xs...)`, where
    // `k` is the element key, and `xs...` are the new constructor arguments for
    // `value_type`. It's allowed for `k` to alias `xs...`, and for both to alias
    // `ts...`. The key won't be touched once `xs...` are used to construct an
    // element; `ts...` won't be touched at all, which allows `apply()` to consume
    // any rvalues among them.
    //
    // If `value_type` is constructible from `Ts&&...`, `Policy::apply()` must not
    // trigger a hard compile error unless it originates from `f`. In other words,
    // `Policy::apply()` must be SFINAE-friendly. If `value_type` is not
    // constructible from `Ts&&...`, either SFINAE or a hard compile error is OK.
    //
    // If `Ts...` is `[cv] value_type[&]` or `[cv] init_type[&]`,
    // `Policy::apply()` must work. A compile error is not allowed, SFINAE or not.
    template <typename First, typename ... Ts, typename Policy = SlotPolicy>
    static auto apply(First && f, Ts && ... ts)
        -> decltype(Policy::apply(std::forward<First>(f), std::forward<Ts>(ts)...)) {
        return Policy::apply(std::forward<First>(f), std::forward<Ts>(ts)...);
    }

    // Returns the "key" portion of the slot.
    // Used for node handle manipulation.
    template <typename Policy = SlotPolicy>
    static auto mutable_key(slot_type * slot)
        -> decltype(Policy::apply(ReturnKey(), element(slot))) {
        return Policy::apply(ReturnKey(), element(slot));
    }

    // Returns the "value" (as opposed to the "key") portion of the element. Used
    // by maps to implement `operator[]`, `at()` and `insert_or_assign()`.
    template <typename T, typename Policy = SlotPolicy>
    static auto value(T * _element) -> decltype(Policy::value(_element)) {
        return Policy::value(_element);
    }

private:
    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto assign_impl(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot, int)
        -> decltype((void)Policy::assign(alloc, dest_slot, src_slot)) {
        Policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Alloc>
    static void assign_impl(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot, char) {
        construct(alloc, dest_slot, std::move(element(src_slot)));
        destroy(alloc, src_slot);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto assign_impl(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot, int)
        -> decltype((void)Policy::assign(alloc, dest_slot, src_slot)) {
        Policy::assign(alloc, dest_slot, src_slot);
    }

    template <typename Alloc>
    static void assign_impl(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot, char) {
        construct(alloc, dest_slot, std::move(element(src_slot)));
        destroy(alloc, src_slot);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto mutable_assign_impl(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot, int)
        -> decltype((void)Policy::mutable_assign(alloc, dest_slot, src_slot)) {
        Policy::mutable_assign(alloc, dest_slot, src_slot);
    }

    template <typename Alloc>
    static void mutable_assign_impl(Alloc * alloc, slot_type * dest_slot, slot_type * src_slot, char) {
        construct(alloc, dest_slot, std::move(element(src_slot)));
        destroy(alloc, src_slot);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto mutable_assign_impl(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot, int)
        -> decltype((void)Policy::mutable_assign(alloc, dest_slot, src_slot)) {
        Policy::mutable_assign(alloc, dest_slot, src_slot);
    }

    template <typename Alloc>
    static void mutable_assign_impl(Alloc * alloc, slot_type * dest_slot, const slot_type * src_slot, char) {
        construct(alloc, dest_slot, std::move(element(src_slot)));
        destroy(alloc, src_slot);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto transfer_impl(Alloc * alloc, slot_type * new_slot, slot_type * old_slot, int)
        -> decltype((void)Policy::transfer(alloc, new_slot, old_slot)) {
        Policy::transfer(alloc, new_slot, old_slot);
    }

    template <typename Alloc>
    static void transfer_impl(Alloc * alloc, slot_type * new_slot, slot_type * old_slot, char) {
        construct(alloc, new_slot, std::move(element(old_slot)));
        destroy(alloc, old_slot);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto swap_impl(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp, int)
        -> decltype((void)Policy::swap(alloc, slot1, slot2, tmp)) {
        Policy::swap(alloc, slot1, slot2, tmp);
    }

    template <typename Alloc>
    static void swap_impl(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp, char) {
        construct(alloc, tmp, std::move(element(slot2)));
        destroy(alloc, slot2);
        construct(alloc, slot2, std::move(element(slot1)));
        destroy(alloc, slot1);
        construct(alloc, slot1, std::move(element(tmp)));
        destroy(alloc, tmp);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto exchange_impl(Alloc * alloc, slot_type * src, slot_type * dest, slot_type * empty, int)
        -> decltype((void)Policy::exchange(alloc, src, dest, empty)) {
        Policy::exchange(alloc, src, dest, empty);
    }

    template <typename Alloc>
    static void exchange_impl(Alloc * alloc,  slot_type * src, slot_type * dest, slot_type * empty, char) {
        construct(alloc, empty, std::move(element(dest)));
        destroy(alloc, dest);
        construct(alloc, dest, std::move(element(src)));
        destroy(alloc, src);
    }

    // Use auto -> decltype as an enabler.
    template <typename Alloc, typename Policy = SlotPolicy>
    static auto move_assign_swap_impl(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp, int)
        -> decltype((void)Policy::move_assign_swap(alloc, slot1, slot2, tmp)) {
        Policy::move_assign_swap(alloc, slot1, slot2, tmp);
    }

    template <typename Alloc>
    static void move_assign_swap_impl(Alloc * alloc, slot_type * slot1, slot_type * slot2, slot_type * tmp, char) {
        mutable_assign(alloc, tmp, slot2);
        mutable_assign(alloc, slot2, slot1);
        mutable_assign(alloc, slot1, tmp);
    }
};

} // namespace jstd
