
#ifndef JSTD_HASH_KEY_EXTRACTOR_H
#define JSTD_HASH_KEY_EXTRACTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <utility>      // For std::pair<First, Second>
#include <type_traits>

namespace jstd {

//////////////////////////////////////////////////////////////////////////
//
// pair_traits<T>
//
// Used to get the types from a pair without instantiating it.
//
//////////////////////////////////////////////////////////////////////////

template <typename Pair>
struct JSTD_DLL pair_traits {
    typedef typename Pair::first_type   first_type;
    typedef typename Pair::second_type  second_type;
};

template <typename T1, typename T2>
struct JSTD_DLL pair_traits< std::pair<T1, T2> > {
    typedef T1  first_type;
    typedef T2  second_type;
};

struct JSTD_DLL no_key_t {
    no_key_t() {}

    template <typename T>
    no_key_t(T const &) {}
};

//////////////////////////////////////////////////////////////////////////
//
// key_extractor<T>
//
// Used to get the type and value from argument list.
//
//////////////////////////////////////////////////////////////////////////

template <typename ValueType>
struct JSTD_DLL key_extractor {
    typedef ValueType   value_type;
    typedef typename std::remove_const<
                typename pair_traits<ValueType>::first_type
            >::type     key_type;
    typedef typename std::remove_const<
                typename pair_traits<ValueType>::second_type
            >::type     mapped_type;

    static key_type const & extract(value_type const & val) {
        return val.first;
    }

    template <typename Second>
    static key_type const & extract(std::pair<key_type, Second> const & val) {
        return val.first;
    }

    template <typename Second>
    static key_type const & extract(std::pair<const key_type, Second> const & val) {
        return val.first;
    }

    template <typename Second>
    static key_type const & extract(std::pair<key_type, Second> && val) {
        return val.first;
    }

    template <typename Second>
    static key_type const & extract(std::pair<const key_type, Second> && val) {
        return val.first;
    }

    template <typename Arg1>
    static key_type const & extract(key_type const & key, Arg1 const &) {
        return key;
    }

    template <typename Arg1>
    static key_type const & extract(key_type && key, Arg1 const &) {
        return key;
    }

    static no_key_t extract() {
        return no_key_t();
    }

    template <typename Arg>
    static no_key_t extract(Arg const &) {
        return no_key_t();
    }

    template <typename Arg>
    static no_key_t extract(Arg &&) {
        return no_key_t();
    }

    template <typename Arg1, typename Arg2>
    static no_key_t extract(Arg1 const & , Arg2 const & ) {
        return no_key_t();
    }

    template <typename Arg1, typename Arg2, typename Arg3, typename... Args>
    static no_key_t extract(Arg1 const &, Arg2 const &, Arg3 const &, Args const & ...) {
        return no_key_t();
    }
};

} // namespace jstd

#endif // JSTD_HASH_KEY_EXTRACTOR_H
