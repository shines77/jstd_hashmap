
#ifndef JSTD_STRING_VIEW_H
#define JSTD_STRING_VIEW_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <string.h>
#include <assert.h>

#include <cstdint>      // For std::intptr_t
#include <cstddef>      // For std::ptrdiff_t
#include <limits>       // For std::numeric_limits<T>
#include <ostream>
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <cassert>

#include "jstd/string/string_traits.h"
#include "jstd/string/string_iterator.h"
#include "jstd/string/string_libc.h"
#include "jstd/string/string_stl.h"
#include "jstd/string/string_utils.h"

namespace jstd {

template <typename CharT, typename Traits = jstd::string_traits<CharT>>
class JSTD_DLL basic_string_view {
public:
    typedef CharT               char_type;
    typedef CharT               value_type;
    typedef std::size_t         size_type;
    typedef std::intptr_t       ssize_type;
    typedef std::ptrdiff_t      difference_type;
    typedef CharT *             pointer;
    typedef const CharT *       const_pointer;
    typedef CharT &             reference;
    typedef const CharT &       const_reference;
    typedef Traits              traits_type;

    typedef jstd::string_iterator<basic_string_view<CharT>>       iterator;
    typedef jstd::const_string_iterator<basic_string_view<CharT>> const_iterator;

    typedef std::basic_string<char_type>                        string_type;
    typedef std::basic_ostream<CharT, std::char_traits<CharT>>  ostream_type;
    typedef basic_string_view<char_type, Traits>                this_type;

    static constexpr size_type npos = size_type(-1);

protected:
    const char_type * data_;
    size_type         size_;

public:
    basic_string_view() noexcept : data_(nullptr), size_(0) {
    }

    explicit basic_string_view(const char_type * data) noexcept
        : data_(data), size_((data != nullptr) ? libc::StrLen(data) : 0) {
    }

    basic_string_view(const char_type * data, size_type size) noexcept
        : data_(data), size_(size) {
    }

    basic_string_view(const char_type * first, const char_type * last) noexcept
        : data_(first), size_(size_type(last - first)) {
    }

    template <size_type N>
    basic_string_view(const char_type(&data)[N]) noexcept
        : data_(data), size_(N - 1) {
    }

    basic_string_view(std::nullptr_t) = delete;

    basic_string_view(const this_type & src) noexcept = default;

    basic_string_view(const string_type & src) noexcept
        : data_(src.data()), size_(src.size()) {
    }

    basic_string_view(const std::vector<char_type> & vec) noexcept
        : data_(vec.data()), size_(vec.size()) {
    }

    ~basic_string_view() = default;

    char_type * data()  noexcept { return const_cast<char_type *>(this->data_); }
    char_type * c_str() noexcept { return const_cast<char_type *>(this->data()); }

    const char_type * data() const noexcept  { return this->data_; }
    const char_type * c_str() const noexcept { return this->data(); }

    constexpr size_t size() const noexcept { return this->size_; }
    constexpr size_t length() const noexcept { return this->size(); }

    constexpr bool empty() const noexcept { return (this->size() == 0); }

    constexpr size_type max_size() const noexcept {
        return ((std::numeric_limits<size_type>::max)() / (sizeof(char_type) * 4));
    }

    iterator begin() const { return iterator(this->data()); }
    iterator end() const { return iterator(this->data() + this->size()); }

    const_iterator cbegin() const { return const_iterator(this->data()); }
    const_iterator cend() const { return const_iterator(this->data() + this->size()); }

    const_reference front() const { return this->data_[0]; }
    const_reference back() const { return this->data_[this->size() - 1]; }

    basic_string_view & operator = (const this_type & rhs) noexcept = default;

    basic_string_view & operator = (const char_type * data) noexcept {
        this->data_ = data;
        this->size_ = (data != nullptr) ? libc::StrLen(data) : 0;
        return *this;
    }

    basic_string_view & operator = (const string_type & rhs) noexcept {
        this->data_ = rhs.c_str();
        this->size_ = rhs.size();
        return *this;
    }

    void commit(size_type count) noexcept {
        this->data_ += count;
    }

    void comsume(size_type count) noexcept {
        this->data_ -= count;
    }

    void apply(ssize_type offset) noexcept {
        this->data_ += offset;
    }

    void expand(size_type count) noexcept {
        this->size_ += count;
    }

    void shrink(size_type count) noexcept {
        this->size_ -= count;
    }

    void offset(ssize_type step) noexcept {
        this->size_ += step;
    }

    void slide(ssize_type offset) noexcept {
        this->data_ += offset;
        this->size_ += offset;
    }

    void attach(const char_type * data, size_t size) {
        this->data_ = data;
        this->size_ = size;
    }

    void attach(const char_type * data) {
        this->attach(data, libc::StrLen(data));
    }

    void attach(const char_type * first, const char_type * last) {
        assert(last >= first);
        this->attach(first, size_type(last - first));
    }

    void attach(const char_type * data, size_type first, size_type last) {
        assert(last >= first);
        this->attach(data + first, size_type(last - first));
    }

    template <size_t N>
    void attach(const char_type(&data)[N]) {
        this->attach(data, N - 1);
    }

    void attach(const string_type & src) {
        this->attach(src.c_str(), src.size());
    }

    void attach(const this_type & src) {
        this->attach(src.data(), src.size());
    }

    void clear() noexcept {
        this->data_ = nullptr;
        this->size_ = 0;
    }

    void swap(this_type & other) noexcept {
        std::swap(this->data_, other.data_);
        std::swap(this->size_, other.size_);
    }

    this_type & append(const char_type * data, size_type count) {
        while (count > 0) {
            this->push_back_impl(*data);
            ++data;
            count--;
        }
        this->write_null();
        return *this;
    }

    this_type & append(size_type count, char_type ch) {
        while (count > 0) {
            this->push_back_impl(ch);
            count--;
        }
        this->write_null();
        return *this;
    }

    this_type & append(const_pointer first, const_pointer last) {
        while (first != last) {
            this->push_back_impl(*first);
            ++first;
        }
        this->write_null();
        return *this;
    }

    this_type & append(const_iterator first, const_iterator last) {
        while (first != last) {
            this->push_back_impl(*first);
            ++first;
        }
        this->write_null();
        return *this;
    }

    template <typename InputIter>
    typename std::enable_if<jstd::is_iterator<InputIter>::value, this_type &>::type
    append(InputIter first, InputIter last) {
#if 1
        while (first != last) {
            this->push_back_impl(*first);
            ++first;
        }
        this->write_null();
#else
        bool is_iterator = jstd::is_iterator<InputIter>::value;
        bool is_forward_iterator = std::is_base_of<forward_iterator_tag, InputIter>::value;
        bool is_std_forward_iterator = std::is_base_of<std::forward_iterator_tag, InputIter>::value;
        if (!is_iterator || (is_iterator && (is_forward_iterator || is_std_forward_iterator))) {
            while (first != last) {
                this->push_back_impl(*first);
                ++first;
            }
            this->write_null();
        }
        else {
            static_assert(false,
                "basic_string_view<T>::append(): InputIter type must be is a forward_iterator.");
        }
#endif
        return *this;
    }

    // Only push for temporary, no grow().
    inline this_type & push_back(char_type ch) {
        char_type * last_ptr = const_cast<char_type *>(this->data_ + this->size_);
        *last_ptr++ = ch;
        *last_ptr = char_type('\0');
        ++(this->size_);
        return *this;
    }

    inline this_type & write_null() {
        this->write_last(char_type('\0'));
        return *this;
    }

    inline char_type read_last() noexcept {
        char_type * last_ptr = const_cast<char_type *>(this->data_ + this->size_);
        return (*last_ptr);
    }

    inline const char_type read_last() const noexcept {
        const char_type * last_ptr = this->data_ + this->size_;
        return (*last_ptr);
    }

    inline this_type & write_last(char_type ch) {
        char_type * last_ptr = const_cast<char_type *>(this->data_ + this->size_);
        *last_ptr = ch;
        return *this;
    }

    reference operator [] (size_type pos) {
        return const_cast<char_type *>(this->data_)[pos];
    }

    const_reference operator [] (size_type pos) const {
        return this->data_[pos];
    }

    reference at(size_type pos) {
        if (pos < this->size())
            return this->data_[pos];
        else
            throw std::out_of_range("basic_string_view<T>::at(pos): out of range.");
    }

    const_reference at(size_type pos) const {
        if (pos < this->size())
            return this->data_[pos];
        else
            throw std::out_of_range("basic_string_view<T>::at(pos): out of range.");
    }

    // copy(dest, count, pos)
    size_type copy(char_type * dest, size_type count, size_type pos = 0) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "copy(dest, count, pos): pos out_of_range.");
        }
        size_type rcount = ((pos + count) <= this->size()) ? count : (this->size() - pos);
        return Traits::copy(dest, this->data() + pos, rcount);
    }

    // substr(pos, count)
    this_type substr(size_type pos = 0, size_type count = npos) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "substr(pos, count): pos out_of_range.");
        }
        size_type rcount;
        if (likely(count != npos))
            rcount = ((pos + count) <= this->size()) ? count : (this->size() - pos);
        else
            rcount = this->size() - pos;
        return this_type(this->data() + pos, rcount);
    }

    //
    // is_equal(rhs)
    //
    bool is_equal(const char_type * str) const {
        return Traits::is_equal(this->data(), str);
    }

    bool is_equal(const char_type * s1, const char_type * s2, size_type count) const noexcept {
        return Traits::is_equal(s1, s2, count);
    }

    bool is_equal(const char_type * s1, size_type len1, const char_type * s2, size_type len2) const noexcept {
        return Traits::is_equal(s1, len1, s2, len2);
    }

    bool is_equal(const this_type & rhs) const noexcept {
        return this->is_equal(this->data(), this->size(), rhs.data(), rhs.size());
    }

    bool is_equal(const string_type & rhs) const noexcept {
        return this->is_equal(this->data(), this->size(), rhs.data(), rhs.size());
    }

    bool is_equal(size_type pos, size_type count, const this_type & sv) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "is_equal(pos, count, sv): pos out_of_range.");
        }
        const char_type * s1 = this->data() + pos;
        return this->is_equal(s1, sv.data(), count);
    }

    bool is_equal(size_type pos, size_type count,
                          const char_type * str) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "is_equal(pos, count, str): pos out_of_range.");
        }
        constexpr size_type len2 = libc::StrLen(str);
        size_type rcount = ((pos + count) <= this->size()) ? count : (this->size() - pos);
        const char_type * s1 = this->data() + pos;
        return this->is_equal(s1, rcount, str, len2);
    }

    bool is_equal(size_type pos1, size_type count1, const this_type & sv,
                          size_type pos2, size_type count2) const {
        if (pos1 > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "is_equal(pos1, count1, sv, pos2, count2): pos1 out_of_range.");
        }
        if (pos2 > sv.size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "is_equal(pos1, count1, sv, pos2, count2): pos2 out_of_range.");
        }
        size_type rcount1 = ((pos1 + count1) <= this->size()) ? count1 : (this->size() - pos1);
        size_type rcount2 = ((pos2 + count2) <= sv.size()) ? count2 : (sv.size() - pos2);
        const char_type * s1 = this->data() + pos1;
        const char_type * s2 = sv.data() + pos2;
        return this->is_equal(s1, rcount1, s2, rcount2);
    }

    bool is_equal(size_type pos1, size_type count1,
                          const char_type * str, size_type count2) const {
        if (pos1 > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "is_equal(pos1, count1, sv, pos2, count2): pos1 out_of_range.");
        }
        constexpr size_type pos2 = 0;
        constexpr size_type len2 = libc::StrLen(str);
        size_type rcount1 = ((pos1 + count1) <= this->size()) ? count1 : (this->size() - pos1);
        size_type rcount2 = ((pos2 + count2) <= len2) ? count2 : (len2 - pos2);
        const char_type * s1 = this->data() + pos1;
        const char_type * s2 = str + pos2;
        return this->is_equal(s1, rcount1, s2, rcount2);
    }

    //
    // compare(rhs)
    //
    int compare(const char_type * str) const noexcept {
        return Traits::compare(this->data(), str);
    }

    int compare(const char_type * s1,
                const char_type * s2,
                size_type count) const noexcept {
        return Traits::compare(s1, s2, count);
    }

    int compare(const char_type * s1, size_type len1,
                const char_type * s2, size_type len2) const noexcept {
        return Traits::compare(s1, len1, s2, len2);
    }

    int compare(const this_type & rhs) const noexcept {
        return this->compare(this->data(), this->size(), rhs.data(), rhs.size());
    }

    int compare(const string_type & rhs) const noexcept {
        return this->compare(this->data(), this->size(), rhs.data(), rhs.size());
    }

    int compare(size_type pos, size_type count, const this_type & sv) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "compare(pos, count, sv): pos out_of_range.");
        }
        const char_type * s1 = this->data() + pos;
        return this->compare(s1, sv.data(), count);
    }

    int compare(size_type pos, size_type count, const char_type * str) const {
        if (pos > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "compare(pos, count, str): pos out_of_range.");
        }
        constexpr size_type len2 = libc::StrLen(str);
        size_type rcount = ((pos + count) <= this->size()) ? count : (this->size() - pos);
        const char_type * s1 = this->data() + pos;
        return this->compare(s1, rcount, str, len2);
    }

    int compare(size_type pos1, size_type count1, const this_type & sv,
                size_type pos2, size_type count2) const {
        if (pos1 > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "compare(pos1, count1, sv, pos2, count2): pos1 out_of_range.");
        }
        if (pos2 > sv.size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "compare(pos1, count1, sv, pos2, count2): pos2 out_of_range.");
        }
        size_type rcount1 = ((pos1 + count1) <= this->size()) ? count1 : (this->size() - pos1);
        size_type rcount2 = ((pos2 + count2) <= sv.size()) ? count2 : (sv.size() - pos2);
        const char_type * s1 = this->data() + pos1;
        const char_type * s2 = sv.data() + pos2;
        return this->compare(s1, rcount1, s2, rcount2);
    }

    int compare(size_type pos1, size_type count1,
                const char_type * str, size_type count2) const {
        if (pos1 > this->size()) {
            throw std::out_of_range("basic_string_view<T>: "
                    "compare(pos1, count1, sv, pos2, count2): pos1 out_of_range.");
        }
        constexpr size_type pos2 = 0;
        constexpr size_type len2 = libc::StrLen(str);
        size_type rcount1 = ((pos1 + count1) <= this->size()) ? count1 : (this->size() - pos1);
        size_type rcount2 = ((pos2 + count2) <= len2) ? count2 : (len2 - pos2);
        const char_type * s1 = this->data() + pos1;
        const char_type * s2 = str + pos2;
        return this->compare(s1, rcount1, s2, rcount2);
    }

    string_type to_string() const {
        // Use RVO (return value optimization)
        return string_type(this->data_, this->size_);
    }

    friend inline ostream_type & operator << (ostream_type & out, const this_type & view) {
        string_type text(view.data(), view.size());
        out << text.c_str();
        return out;
    }

    friend inline void swap(this_type & lhs, this_type & rhs) noexcept {
        lhs.swap(rhs);
    }

    friend inline bool operator == (const this_type & lhs, const this_type & rhs) noexcept {
        return lhs.is_equal(rhs);
    }

    friend inline bool operator != (const this_type & lhs, const this_type & rhs) noexcept {
        return !lhs.is_equal(rhs);
    }

    friend inline bool operator < (const this_type & lhs, const this_type & rhs) noexcept {
        return (lhs.compare(rhs) == CompareResult::IsSmaller);
    }

    friend inline bool operator <= (const this_type & lhs, const this_type & rhs) noexcept {
        return (lhs.compare(rhs) == CompareResult::IsBigger);
    }

    friend inline bool operator > (const this_type & lhs, const this_type & rhs) noexcept {
        return (lhs.compare(rhs) == CompareResult::IsBigger);
    }

    friend inline bool operator >= (const this_type & lhs, const this_type & rhs) noexcept {
        return (lhs.compare(rhs) == CompareResult::IsSmaller);
    }

private:
    inline void push_back_impl(char_type ch) {
        char_type * last_ptr = const_cast<char_type *>(this->data_ + this->size_);
        *last_ptr = ch;
        ++(this->size_);
    }
}; // class basic_string_view<CharT>

template <typename CharT, typename Traits>
inline
void swap(basic_string_view<CharT, Traits> & lhs, basic_string_view<CharT, Traits> & rhs) noexcept {
    lhs.swap(rhs);
}

typedef basic_string_view<char>         string_view;
typedef basic_string_view<wchar_t>      wstring_view;

typedef basic_string_view<char>         string_ref;
typedef basic_string_view<wchar_t>      wstring_ref;

} // namespace jstd

namespace std {

template <typename CharT, typename Traits>
inline
void swap(jstd::basic_string_view<CharT, Traits> & lhs, jstd::basic_string_view<CharT, Traits> & rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace std

#endif // JSTD_STRING_VIEW_H
