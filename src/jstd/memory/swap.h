
#ifndef JSTD_MEMORY_SWAP_H
#define JSTD_MEMORY_SWAP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"

#include <assert.h>
#include <memory>

#include "jstd/iterator.h"

#ifndef MUST_BE_A_DERIVED_CLASS_OF
#if 0
#define MUST_BE_A_DERIVED_CLASS_OF(Base, Derived)
#else
#define MUST_BE_A_DERIVED_CLASS_OF(Base, Derived) \
        static_assert(!std::is_base_of<Base, Derived>::value, \
            "Error: [" JSTD_TO_STRING(Base) "] must be a derived class of [" JSTD_TO_STRING(Derived) "].")
#endif
#endif

namespace jstd {

///////////////////////////////////////////////////
// struct delete_helper<T, IsArray>
///////////////////////////////////////////////////

template <typename T, bool IsArray>
struct delete_helper {};

template <typename T>
struct delete_helper<T, false> {
    static void delete_it(T * p) {
        delete p;
    }
};

template <typename T>
struct delete_helper<T, true> {
    static void delete_it(T * p) {
        delete[] p;
    }
};

template <typename T, typename std::enable_if<!std::is_pointer<T>::value>::type * p = nullptr>
void swap(T & a, T & b) noexcept(std::is_nothrow_move_constructible<T>::value &&
                                 std::is_nothrow_move_assignable<T>::value)
{
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value>::type * p = nullptr>
void swap(T * & a, T * & b) noexcept(std::is_nothrow_move_constructible<T>::value &&
                                     std::is_nothrow_move_assignable<T>::value)
{
    T * tmp = a;
    a = b;
    b = tmp;
}

//
// See: https://en.cppreference.com/w/cpp/algorithm/swap
//
template <typename ForwardIter1, typename ForwardIter2>
ForwardIter2 swap_ranges(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
{
    ForwardIter2 first2_save = first2;
    while (first1 != last1) {
        std::iter_swap(first1++, first2++);
    }
    assert(first2 == std::next(first2_save, jstd::distance(first1, last1)));
    return first2;
}

template <typename T, std::size_t N>
void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b)))
{
    swap_ranges(a, a + N, b);
}

} // namespace jstd

#endif // JSTD_MEMORY_SWAP_H
