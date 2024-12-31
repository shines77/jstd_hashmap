
#ifndef JSTD_STRING_ITERATOR_H
#define JSTD_STRING_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <assert.h>

#include <memory>
#include <type_traits>

#include "jstd/iterator.h"
#include "jstd/utility/utility.h"

namespace jstd {

// iterator for non-mutable string

template <typename T>
class JSTD_DLL const_string_iterator
        : public iterator<random_access_iterator_tag,
                          typename T::value_type,
                          typename T::difference_type,
                          typename T::const_pointer,
                          typename T::const_reference> {
public:
    typedef const_string_iterator<T>        this_type;
    typedef std::random_access_iterator_tag iterator_category;

    typedef typename T::value_type          value_type;
    typedef typename T::difference_type     difference_type;
    typedef typename T::const_pointer       pointer;
    typedef typename T::const_reference     reference;

protected:
    pointer ptr_;

public:
    // construct with null pointer
    const_string_iterator(pointer ptr = nullptr) : ptr_(ptr) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    const_string_iterator(std::nullptr_t) : ptr_(nullptr) {}
#endif

    reference operator * () const {
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
        assert(ptr_ != std::nullptr_t{});
#endif
        return (*ptr_);
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    // return pointer to class object
    pointer operator -> () const {
        return (std::pointer_traits<pointer>::pointer_to(**this));
    }
#endif

    // pre-increment
    this_type & operator ++ () {
        ++(this->ptr_);
        return (*this);
    }

    // post-increment
    this_type & operator ++ (int) {
        this_type tmp = *this;
        ++(*this);
        return tmp;
    }

    // pre-decrement
    this_type & operator -- () {
        --(this->ptr_);
        return (*this);
    }

    // post-decrement
    this_type & operator -- (int) {
        this_type tmp = *this;
        --(*this);
        return tmp;
    }

    // increment by integer
    this_type & operator += (difference_type offset) {
        this->ptr_ += offset;
        return (*this);
    }

    // return this + integer
    this_type operator + (difference_type offset) const {
        this_type tmp = *this;
        return (tmp += offset);
    }

    // decrement by integer
    this_type & operator -= (difference_type offset) {
        this->ptr_ -= offset;
        return (*this);
    }

    // return this - integer
    this_type operator - (difference_type offset) const {
        this_type tmp = *this;
        return (tmp -= offset);
    }

    // return difference of iterators
    difference_type operator - (const this_type & rhs) const {
        return (this->ptr_ - rhs.ptr_);
    }

    // return difference of iterators
    difference_type operator + (const this_type & rhs) const {
        return (this->ptr_ + rhs.ptr_);
    }

    // subscript
    reference operator [] (difference_type offset) const {
        return (*(*this + offset));
    }

    // test for iterator equality
    bool operator == (const this_type & rhs) const {
        return (this->ptr_ == rhs.ptr_);
    }

    // test for iterator inequality
    bool operator != (const this_type & rhs) const {
        return (!(*this == rhs));
    }

    // test if this < rhs
    bool operator < (const this_type & rhs) const {
        return (this->ptr_ < rhs.ptr_);
    }

    // test if this > rhs
    bool operator > (const this_type & rhs) const {
        return (rhs < *this);
    }

    // test if this <= rhs
    bool operator <= (const this_type & rhs) const {
        return (!(rhs < *this));
    }

    // test if this >= rhs
    bool operator >= (const this_type & rhs) const {
        return (!(*this < rhs));
    }
};

// add offset to iterator

template <typename T>
inline
const_string_iterator<T> operator + (
    typename const_string_iterator<T>::difference_type offset,
    const_string_iterator<T> next) {
    return (next += offset);
}

// iterator for non-mutable string

template <typename T>
class string_iterator : public const_string_iterator<T> {
public:
    typedef string_iterator<T>              this_type;
    typedef const_string_iterator<T>        base_type;
    typedef std::random_access_iterator_tag iterator_category;

    typedef typename T::value_type          value_type;
    typedef typename T::difference_type     difference_type;
    typedef typename T::const_pointer       pointer;
    typedef typename T::const_reference     reference;

    typedef pointer _Unchecked_type;

public:
    // construct with null pointer
    string_iterator(pointer ptr = nullptr) : base_type(ptr) {}
#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
    string_iterator(std::nullptr_t) : base_type(nullptr) {}
#endif

    // reset from unchecked iterator
    this_type & _Rechecked(_Unchecked_type rhs) {
        this->ptr_ = rhs;
        return (*this);
    }

    // make an unchecked iterator
    _Unchecked_type _Unchecked() const {
#if !defined(_MSC_VER) || (_MSC_VER >= 1900)
        return (detail::const_castor(this->ptr_));
#else
        return _Unchecked_type(this->ptr_);
#endif
    }

    reference operator * () const {
        assert(this->ptr_ != nullptr_t{});
        return (*this->ptr_);
    }

    // return pointer to class object
    pointer operator -> () const {
        return (std::pointer_traits<pointer>::pointer_to(**this));
    }

    // pre-increment
    this_type & operator ++ () {
        ++(this->ptr_);
        return (*this);
    }

    // post-increment
    this_type & operator ++ (int) {
        this_type tmp = *this;
        ++(*this);
        return tmp;
    }

    // pre-decrement
    this_type & operator -- () {
        --(this->ptr_);
        return (*this);
    }

    // post-decrement
    this_type & operator -- (int) {
        this_type tmp = *this;
        --(*this);
        return tmp;
    }

    // increment by integer
    this_type & operator += (difference_type offset) {
        this->ptr_ += offset;
        return (*this);
    }

    // return this + integer
    this_type operator + (difference_type offset) const {
        this_type tmp = *this;
        return (tmp += offset);
    }

    // decrement by integer
    this_type & operator -= (difference_type offset) {
        this->ptr_ -= offset;
        return (*this);
    }

    // return this - integer
    this_type operator - (difference_type offset) const {
        this_type tmp = *this;
        return (tmp -= offset);
    }

    // return difference of iterators
    difference_type operator - (const this_type & rhs) const {
        return (this->ptr_ - rhs.ptr_);
    }

    // return difference of iterators
    difference_type operator + (const this_type & rhs) const {
        return (this->ptr_ + rhs.ptr_);
    }

    // subscript
    reference operator [] (difference_type offset) const {
        return (*(*this + offset));
    }

    // test for iterator equality
    bool operator == (const this_type & rhs) const {
        return (this->ptr_ == rhs.ptr_);
    }

    // test for iterator inequality
    bool operator != (const this_type & rhs) const {
        return (!(*this == rhs));
    }

    // test if this < rhs
    bool operator < (const this_type & rhs) const {
        return (this->ptr_ < rhs.ptr_);
    }

    // test if this > rhs
    bool operator > (const this_type & rhs) const {
        return (rhs < *this);
    }

    // test if this <= rhs
    bool operator <= (const this_type & rhs) const {
        return (!(rhs < *this));
    }

    // test if this >= rhs
    bool operator >= (const this_type & rhs) const {
        return (!(*this < rhs));
    }
};

// convert to unchecked

template <typename T>
inline
typename string_iterator<T>::_Unchecked_type
_Unchecked(string_iterator<T> iter) {
    return (iter._Unchecked());
}

// convert to checked

template <typename T>
inline
string_iterator<T>
_Rechecked(string_iterator<T> & iter,
    typename string_iterator<T>::_Unchecked_type rhs) {
    return (iter._Rechecked(rhs));
}

// add offset to iterator

template <typename T>
inline
string_iterator<T> operator + (
    typename string_iterator<T>::difference_type offset,
    string_iterator<T> next) {
    return (next += offset);
}

} // namespace jstd

#endif // JSTD_STRING_ITERATOR_H
