
#ifndef JSTD_STRING_HELPER_H
#define JSTD_STRING_HELPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <string.h>
#include <assert.h>

#include <cstdint>  // for std::intptr_t
#include <cstddef>  // for std::ptrdiff_t
#include <string>
#include <memory>

#include "jstd/string/string_view.h"

namespace jstd {

template <typename StringTy, typename CharTy>
class JSTD_DLL basic_string_helper {
public:
    typedef basic_string_helper<StringTy, CharTy>
                                this_type;
    typedef StringTy            string_type;
    typedef CharTy              char_type;

private:
    string_type str_;
    bool        truncated_;
    char_type   save_char_;

public:
    basic_string_helper()
        : truncated_(false),
          save_char_(static_cast<char_type>('\0')) {
        (void)save_char_;
    }
    basic_string_helper(const this_type & src)
        : str_(src.str_),
          truncated_(src.truncated_),
          save_char_(src.save_char_) {
    }
    basic_string_helper(this_type && src)
        : truncated_(false),
          save_char_(static_cast<char_type>('\0')) {
        this->swap(src);
    }
    ~basic_string_helper() {
        detach();
    }

    bool attach(const string_type & str) {
        // If the string reference don't recover the truncated char,
        // don't accept the new attach.
        if (likely(!truncated_)) {
            str_ = str;
        }
        return (!truncated_);
    }

    void detach() {
        // If have be truncated, recover the saved terminator char first,
        // and then clear the string reference.
        if (unlikely(truncated_)) {
            recover();
            str_.clear();
        }
    }

    void truncate() {
        if (likely(!truncated_)) {
            char_type * first = (char_type *)str_.data();
            char_type * last = first + str_.size();
            assert(last != nullptr);
            save_char_ = *last;
            *last = static_cast<char_type>('\0');
            truncated_ = true;
        }
    }

    void recover() {
        if (likely(truncated_)) {
            char_type * first = (char_type *)str_.data();
            char_type * last = first + str_.size();
            assert(last != nullptr);
            *last = save_char_;
            truncated_ = false;
        }
    }

    void swap(this_type & right) {
        if (&right != this) {
            this->str_.swap(right.str_);
            std::swap(this->truncated_, right.truncated_);
            std::swap(this->save_char_, right.save_char_);
        }
    }
};

template <typename StringTy, typename CharTy>
inline
void swap(basic_string_helper<StringTy, CharTy> & lhs,
          basic_string_helper<StringTy, CharTy> & rhs) {
    lhs.swap(rhs);
}

typedef basic_string_helper<std::string, char>          string_helper;
typedef basic_string_helper<std::wstring, wchar_t>      wstring_helper;

typedef basic_string_helper<jstd::string_view, char>        string_view_helper;
typedef basic_string_helper<jstd::wstring_view, wchar_t>    wstring_view_helper;

} // namespace jstd

namespace std {

template <typename StringTy, typename CharTy>
inline
void swap(jstd::basic_string_helper<StringTy, CharTy> & lhs,
          jstd::basic_string_helper<StringTy, CharTy> & rhs) {
    lhs.swap(rhs);
}

} // namespace std

#endif // JSTD_STRING_HELPER_H
