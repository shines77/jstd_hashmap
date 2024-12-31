
#ifndef JSTD_ITERATOR_H
#define JSTD_ITERATOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stdint.h"
#include "jstd/traits/type_traits.h"

#include <cstdint>
#include <memory>           // For std::pointer_traits<T>
#include <initializer_list> // For std::initializer_list<T>

//
// See: https://www.cnblogs.com/mangoyuan/p/6446046.html
//

namespace jstd {

/////////////////////////////////////////////////////////////////

// Iterator stuff (from <iterator>)
// Iterator tags  (from <iterator>)

// identifying tag for input iterators
struct input_iterator_tag
{
    input_iterator_tag() = default;
};

// TRANSITION, remove for Dev15

// identifying tag for mutable iterators
struct mutable_iterator_tag
{
    mutable_iterator_tag() = default;
};

// identifying tag for output iterators
struct output_iterator_tag : mutable_iterator_tag
{
    output_iterator_tag() = default;
};

// identifying tag for forward iterators
struct forward_iterator_tag : input_iterator_tag, mutable_iterator_tag
{
    forward_iterator_tag() = default;
};

// identifying tag for bidirectional iterators
struct bidirectional_iterator_tag : forward_iterator_tag
{
    bidirectional_iterator_tag() = default;
};

// identifying tag for random-access iterators
struct random_access_iterator_tag : bidirectional_iterator_tag
{
    random_access_iterator_tag() = default;
};

// base type for iterator classes
template <class Category, class T, class Difference = std::ptrdiff_t,
          class Pointer = T *, class Reference = T &>
struct iterator
{
    typedef Category        iterator_category;
    typedef T               value_type;
    typedef Difference      difference_type;

    typedef Pointer         pointer;
    typedef Reference       reference;
};

// base for output iterators
typedef iterator<output_iterator_tag, void, void, void, void>   OutputIter;

/////////////////////////////////////////////////////////////////

// TEMPLATE CLASS iterator_traits

// empty for non-iterators
template <class T, class = void>
struct iterator_traits_base {
};

// defined if Iter::* types exist
template <class InputIter>
struct iterator_traits_base< InputIter, jstd::void_t<
    typename InputIter::iterator_category,
    typename InputIter::value_type,
    typename InputIter::difference_type,
    typename InputIter::pointer,
    typename InputIter::reference> >
{
    typedef typename InputIter::iterator_category    iterator_category;
    typedef typename InputIter::value_type           value_type;
    typedef typename InputIter::difference_type      difference_type;

    typedef typename InputIter::pointer              pointer;
    typedef typename InputIter::reference            reference;
};

// get traits from iterator Iter, if possible
template <class InputIter>
struct iterator_traits : iterator_traits_base<InputIter> {
};

// get traits from pointer
template <class T>
struct iterator_traits<T *>
{
    typedef random_access_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef std::ptrdiff_t              difference_type;

    typedef T *                         pointer;
    typedef T &                         reference;
};

// get traits from const pointer
template <class T>
struct iterator_traits<const T *>
{
    typedef random_access_iterator_tag  iterator_category;
    typedef T                           value_type;
    typedef std::ptrdiff_t              difference_type;

    typedef const T *                   pointer;
    typedef const T &                   reference;
};

// Alias template iter_value_t
template <class InputIter>
using iter_value_t = typename iterator_traits<InputIter>::value_type;

// Alias template iter_diff_t
template <class InputIter>
using iter_diff_t = typename iterator_traits<InputIter>::difference_type;

// Alias template iter_cat_t
template <class InputIter>
using iter_cat_t = typename iterator_traits<InputIter>::iterator_category;

// Template class is_iterator

template <class T, class = void>
struct is_iterator : std::false_type
{
    // default definition
};

template <class T>
struct is_iterator< T, jstd::void_t<
    typename iterator_traits<T>::iterator_category> > : std::true_type
{
    // defined if iterator_category is provided by iterator_traits<T>
};

struct iterator_utils {

    typedef iterator_utils this_type;

    // increment iterator by offset, input iterators
    template <class InputIter, class Difference>
    static inline
    void advance_impl(InputIter & where, Difference offset, input_iterator_tag) {
        for (; 0 < offset; --offset) {
            ++where;
        }
    }

    // increment iterator by offset, bidirectional iterators
    template <class BidIter, class Difference>
    static inline
    void advance_impl(BidIter & where, Difference offset, bidirectional_iterator_tag) {
        for (; offset > 0; --offset) {
            ++where;
        }
        for (; offset < 0; ++offset) {
            --where;
        }
    }

    // increment iterator by offset, random-access iterators
    template <class RandomIter, class Difference>
    static inline
    void advance_impl(RandomIter & where, Difference offset, random_access_iterator_tag) {
        where += offset;
    }

    template <class InputIter, class Difference>
    static inline
    void advance(InputIter & where, Difference offset) {
        this_type::advance_impl(where, offset, iter_cat_t<typename std::remove_const<InputIter>::type>());
    }

    //////////////////////////////////////////////////////////////////////////////////////

    // return distance between iterators; input
    template <class InputIter>
    static inline
    iter_diff_t<InputIter>
    distance_impl(InputIter first, InputIter last, input_iterator_tag) {
        iter_diff_t<InputIter> offset = 0;
        for (; first != last; ++first) {
            ++offset;
        }

        return offset;
    }

    // return distance between iterators; random-access
    template <class RandomIter>
    static inline
    iter_diff_t<RandomIter>
    distance_impl(RandomIter first, RandomIter last, random_access_iterator_tag) {
        return (last - first);
    }

    // return distance between iterators
    template <class InputIter>
    static inline
    iter_diff_t<InputIter> distance(InputIter first, InputIter last) {
        return this_type::distance_impl(first, last, iter_cat_t<InputIter>());
    }

    //////////////////////////////////////////////////////////////////////////////////////

    // Template function next: increment iterator
    template <class InputIter>
    static inline
    InputIter next(InputIter first, iter_diff_t<InputIter> offset = 1) {
        static_assert(std::is_base_of<input_iterator_tag,
                      typename iterator_traits<InputIter>::iterator_category>::value,
                      "next requires input iterator");

        this_type::advance(first, offset);
        return first;
    }

    // Template function prev: decrement iterator
    template <class BidIter>
    static inline
    BidIter prev(BidIter first, iter_diff_t<BidIter> offset = 1) {
        static_assert(std::is_base_of<bidirectional_iterator_tag,
                      typename iterator_traits<BidIter>::iterator_category>::value,
                      "prev requires bidirectional iterator");

        this_type::advance(first, -offset);
        return first;
    }
};

template <class T>
struct pointer_traits;

//////////////////////////////////////////////////////////////////////////////////////

// Template class reverse_iterator

// wrap iterator to run it backwards
template <class RandomIter>
class reverse_iterator
    : public iterator<
        typename iterator_traits<RandomIter>::iterator_category,
        typename iterator_traits<RandomIter>::value_type,
        typename iterator_traits<RandomIter>::difference_type,
        typename iterator_traits<RandomIter>::pointer,
        typename iterator_traits<RandomIter>::reference>
{
    typedef reverse_iterator<RandomIter> this_type;

public:
    typedef typename iterator_traits<RandomIter>::value_type        value_type;
	typedef typename iterator_traits<RandomIter>::difference_type   difference_type;
	typedef typename iterator_traits<RandomIter>::pointer           pointer;
	typedef typename iterator_traits<RandomIter>::reference         reference;
	typedef RandomIter                                              iterator_type;

protected:
    iterator_type iter_;

public:
    // construct with value-initialized wrapped iterator
    reverse_iterator() : iter_() {
    }

    // construct wrapped iterator from _Right
    explicit reverse_iterator(iterator_type right) : iter_(right) {
    }

    // initialize with compatible base
    template <class other>
    reverse_iterator(const reverse_iterator<other> & right)
        : iter_(right.base()) {
    }

    // assign from compatible base
    template <class other>
    this_type & operator = (const reverse_iterator<other> & right) {
        this->iter_ = right.base();
        return (*this);
    }

    // return wrapped iterator
    iterator_type base() const {
        return this->iter_;
    }

    // return designated value
    reference operator * () const {
        iterator_type tmp = this->iter_;
        return (*--tmp);
    }

    // return pointer to class object
    pointer operator -> () const {
        return (std::pointer_traits<pointer>::pointer_to(**this));
    }

    // preincrement: ++iter
    this_type & operator ++ () {
        --(this->iter_);
        return (*this);
    }

    // postincrement: iter++
    this_type operator ++ (int) {
        this_type tmp = *this;
        --(this->iter_);
        return (tmp);
    }

    // predecrement: --iter
    this_type & operator -- () {
        ++(this->iter_);
        return (*this);
    }

    // postdecrement: iter--
    this_type operator -- (int) {
        this_type tmp = *this;
        ++(this->iter_);
        return (tmp);
    }

    // increment by integer
    this_type & operator += (difference_type offset) {
        this->iter_ -= offset;
        return (*this);
    }

    // return this + integer
    this_type operator + (difference_type offset) const {
        return (this_type(this->iter_ - offset));
    }

    // decrement by integer
    this_type & operator -= (difference_type offset) {
        this->iter_ += offset;
        return (*this);
    }

    // return this - integer
    this_type operator - (difference_type offset) const {
        return (this_type(this->iter_ + offset));
    }

    // subscript
    reference operator [] (difference_type offset) const {
        return (*(*this + offset));
    }
};

/////////////////////////////////////////////////////////////////

// reverse_iterator template operators

// return reverse_iterator + integer
template <class RandomIter>
inline
reverse_iterator<RandomIter> operator + (
    typename reverse_iterator<RandomIter>::difference_type offset,
    const reverse_iterator<RandomIter> & right)
{
    return (right + offset);
}

// return difference of reverse_iterators
template <class RandomIter1, class RandomIter2>
inline
auto operator - (const reverse_iterator<RandomIter1> & left,
                 const reverse_iterator<RandomIter2> & right)
    -> decltype(right.base() - left.base())
{
    return (right.base() - left.base());
}

// test for reverse_iterator equality
template <class RandomIter1, class RandomIter2>
inline
bool operator == (const reverse_iterator<RandomIter1> & left,
                  const reverse_iterator<RandomIter2> & right)
{
    return (left.base() == right.base());
}

// test for reverse_iterator inequality
template <class RandomIter1, class RandomIter2>
inline
bool operator != (const reverse_iterator<RandomIter1> & left,
                  const reverse_iterator<RandomIter2> & right)
{
    return (!(left == right));
}

// test for reverse_iterator < reverse_iterator
template <class RandomIter1, class RandomIter2>
inline
bool operator < (const reverse_iterator<RandomIter1> & left,
                 const reverse_iterator<RandomIter2> & right)
{
    return (right.base() < left.base());
}

// test for reverse_iterator > reverse_iterator
template <class RandomIter1, class RandomIter2>
inline
bool operator > (const reverse_iterator<RandomIter1> & left,
                 const reverse_iterator<RandomIter2> & right)
{
    return (right < left);
}

// test for reverse_iterator <= reverse_iterator
template <class RandomIter1, class RandomIter2>
inline
bool operator <= (const reverse_iterator<RandomIter1> & left,
                  const reverse_iterator<RandomIter2> & right)
{
    return (!(right < left));
}

// test for reverse_iterator >= reverse_iterator
template <class RandomIter1, class RandomIter2>
inline
bool operator >= (const reverse_iterator<RandomIter1> & left,
                  const reverse_iterator<RandomIter2> & right)
{
    return (!(left < right));
}

/////////////////////////////////////////////////////////////////

// Template function: make_reverse_iterator

// make reverse_iterator from iterator
template <class RandomIter>
inline
reverse_iterator<RandomIter> make_reverse_iterator(RandomIter iter)
{
    return (reverse_iterator<RandomIter>(iter));
}

/////////////////////////////////////////////////////////////////

// Template functions: begin() AND end()

// get beginning of sequence
template <class Container>
inline
auto begin(Container & container) -> decltype(container.begin())
{
    return (container.begin());
}

// get beginning of sequence
template <class Container>
inline
auto begin(const Container & container) -> decltype(container.begin())
{
    return (container.begin());
}

// get end of sequence
template <class Container>
inline
auto end(Container & container) -> decltype(container.end())
{
    return (container.end());
}

// get end of sequence
template <class Container>
inline
auto end(const Container & container) -> decltype(container.end())
{
    return (container.end());
}

/////////////////////////////////////////////////////////////////

// Template functions: cbegin() AND cend()

// get beginning of sequence
template <class Container>
inline
constexpr auto cbegin(const Container & container)
noexcept(noexcept(begin(container)))
-> decltype(begin(container))
{
    return (begin(container));
}

// get end of sequence
template <class Container>
inline
constexpr auto cend(const Container & container)
noexcept(noexcept(end(container)))
-> decltype(end(container))
{
    return (end(container));
}

/////////////////////////////////////////////////////////////////

// Template functions: rbegin() AND rend()

// get beginning of reversed sequence
template <class Container>
inline
auto rbegin(Container & container) -> decltype(container.rbegin())
{
    return (container.rbegin());
}

// get beginning of reversed sequence
template <class Container>
inline
auto rbegin(const Container & container) -> decltype(container.rbegin())
{
    return (container.rbegin());
}

// get end of reversed sequence
template <class Container>
inline
auto rend(Container & container) -> decltype(container.rend())
{
    return (container.rend());
}

// get end of reversed sequence
template <class Container>
inline
auto rend(const Container & container) -> decltype(container.rend())
{
    return (container.rend());
}

// get beginning of reversed array
template <class T, std::size_t Size>
inline
reverse_iterator<T *> rbegin(T(&Array)[Size])
{
    return (reverse_iterator<T *>(Array + Size));
}

// get end of reversed array
template <class T, std::size_t Size>
inline
reverse_iterator<T *> rend(T(&Array)[Size])
{
    return (reverse_iterator<T *>(Array));
}

// get beginning of reversed sequence
template <class Element>
inline
reverse_iterator<const Element *>
rbegin(std::initializer_list<Element> initList)
{
    return (reverse_iterator<const Element *>(initList.end()));
}

// get end of reversed sequence
template <class Element>
inline
reverse_iterator<const Element *>
rend(std::initializer_list<Element> initList)
{
    return (reverse_iterator<const Element *>(initList.begin()));
}

/////////////////////////////////////////////////////////////////

// Template functions: crbegin() AND crend()

// get beginning of reversed sequence
template <class Container>
inline
auto crbegin(const Container & container) -> decltype(rbegin(container))
{
    return (rbegin(container));
}

// get end of reversed sequence
template <class Container>
inline
auto crend(const Container & container) -> decltype(rend(container))
{
    return (rend(container));
}

/////////////////////////////////////////////////////////////////

// Template functions: container common interface

// get size() for container
template <class Container>
inline
constexpr auto size(const Container & container) -> decltype(container.size())
{
    return (container.size());
}

// get dimension for array
template <class T, std::size_t Size>
inline
constexpr std::size_t size(const T(&)[Size]) noexcept
{
    return Size;
}

// get empty() for container
template <class Container>
inline
constexpr auto empty(const Container & container)
-> decltype(container.empty())
{
    return (container.empty());
}

// get dimension == 0 for array (can't happen)
template <class T, std::size_t Size>
inline
constexpr bool empty(const T(&)[Size]) noexcept
{
    return (false);
}

// get dimension == 0 for initializer_list
template <class Element>
inline
constexpr bool empty(std::initializer_list<Element> initList) noexcept
{
    return (initList.size() == 0);
}

// get data() for container
template <class Container>
inline
constexpr auto data(Container & container) -> decltype(container.data())
{
    return (container.data());
}

// get pointer to data of const container
template <class Container>
inline
constexpr auto data(const Container & container) -> decltype(container.data())
{
    return (container.data());
}

// get pointer to data of array
template <class T, std::size_t Size>
inline
constexpr T * data(T(&Array)[Size]) noexcept
{
    return Array;
}

// get pointer to data of initializer_list
template <class Element>
inline
constexpr const Element * data(
    std::initializer_list<Element> initList) noexcept
{
    return (initList.begin());
}

/////////////////////////////////////////////////////////////////

#if (jstd_cplusplus >= 2017L)

// implementation via constexpr if, available in C++17
template<class Iter>
constexpr typename std::iterator_traits<Iter>::difference_type
    distance(Iter first, Iter last)
{
    using category = typename std::iterator_traits<Iter>::iterator_category;
    static_assert(std::is_base_of<std::input_iterator_tag, category>::value);

    if constexpr (std::is_base_of<std::random_access_iterator_tag, category>::value) {
        return (last - first);
    } else {
        typename std::iterator_traits<Iter>::difference_type result = 0;
        while (first != last) {
            ++first;
            ++result;
        }
        return result;
    }
}

#else // !JSTD_IS_CXX_17

// implementation via tag dispatch, available in C++98 with constexpr removed
namespace detail {

template <typename Iter>
typename std::iterator_traits<Iter>::difference_type
do_distance(Iter first, Iter last, std::input_iterator_tag)
{
    typename std::iterator_traits<Iter>::difference_type result = 0;
    while (first != last) {
        ++first;
        ++result;
    }
    return result;
}

template <typename Iter>
typename std::iterator_traits<Iter>::difference_type
do_distance(Iter first, Iter last, std::random_access_iterator_tag)
{
    return (last - first);
}

} // namespace detail

template <typename Iter>
typename std::iterator_traits<Iter>::difference_type
distance(Iter first, Iter last)
{
    return detail::do_distance(first, last,
                               typename std::iterator_traits<Iter>::iterator_category());
}

#endif // JSTD_IS_CXX_17

} // namespace jstd

#endif // JSTD_ITERATOR_H
