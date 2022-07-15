
#ifndef JSTD_HASH_HELPER_H
#define JSTD_HASH_HELPER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <type_traits>

#include "jstd/hasher/hashes.h"
#include "jstd/hasher/hash_crc32c.h"
#include "jstd/string/string_libc.h"
#include "jstd/string/string_stl.h"

#define HASH_HELPER_CHAR(KeyType, ResultType, HashFuncId, HashFunc)             \
    template <>                                                                 \
    struct hash_helper<KeyType, ResultType, HashFuncId> {                       \
        typedef ResultType  result_type;                                        \
        typedef typename std::remove_pointer<KeyType>::type char_type;          \
                                                                                \
        typedef typename std::remove_pointer<                                   \
                    typename std::remove_cv<                                    \
                        typename std::remove_reference<KeyType>::type           \
                    >::type                                                     \
                >::type decay_type;                                             \
                                                                                \
        static ResultType getHashCode(char_type * data, std::size_t length) {   \
            if (likely(data != nullptr && length != 0))                         \
                return (ResultType)HashFunc((const char *)data,                 \
                                            length * sizeof(char_type));        \
            else                                                                \
                return 0;                                                       \
        }                                                                       \
                                                                                \
        static ResultType getHashCode(char_type * data) {                       \
            if (likely(data != nullptr))                                        \
                return (ResultType)HashFunc((const char *)data,                 \
                                jstd::libc::StrLen((const decay_type *)data)    \
                                            * sizeof(char_type));               \
            else                                                                \
                return 0;                                                       \
        }                                                                       \
    }

#define HASH_HELPER_CHAR_ALL(HashHelperClass, HashType, HashFuncId, HashFunc)   \
    HashHelperClass(char *,                 HashType, HashFuncId, HashFunc);    \
    HashHelperClass(const char *,           HashType, HashFuncId, HashFunc);    \
    HashHelperClass(unsigned char *,        HashType, HashFuncId, HashFunc);    \
    HashHelperClass(const unsigned char *,  HashType, HashFuncId, HashFunc);    \
    HashHelperClass(short *,                HashType, HashFuncId, HashFunc);    \
    HashHelperClass(const short *,          HashType, HashFuncId, HashFunc);    \
    HashHelperClass(unsigned short *,       HashType, HashFuncId, HashFunc);    \
    HashHelperClass(const unsigned short *, HashType, HashFuncId, HashFunc);    \
    HashHelperClass(wchar_t *,              HashType, HashFuncId, HashFunc);    \
    HashHelperClass(const wchar_t *,        HashType, HashFuncId, HashFunc);

#define HASH_HELPER_INTEGRAL(KeyType, ResultType, HashFuncId)                   \
    template <>                                                                 \
    struct hash_helper<KeyType, ResultType, HashFuncId> {                       \
        typedef ResultType  result_type;                                        \
        typedef typename std::remove_pointer<KeyType>::type pod_type;           \
                                                                                \
        typedef typename std::remove_pointer<                                   \
                    typename std::remove_cv<                                    \
                        typename std::remove_reference<KeyType>::type           \
                    >::type                                                     \
                >::type decay_type;                                             \
                                                                                \
        static ResultType getHashCode(pod_type data) {                          \
            return static_cast<result_type>(data);                              \
        }                                                                       \
    }

#define HASH_HELPER_INTEGRAL_ALL(HashHelperClass, HashType, HashFuncId)         \
    HashHelperClass(bool,                   HashType, HashFuncId);              \
    HashHelperClass(char,                   HashType, HashFuncId);              \
    HashHelperClass(signed char,            HashType, HashFuncId);              \
    HashHelperClass(unsigned char,          HashType, HashFuncId);              \
    HashHelperClass(short,                  HashType, HashFuncId);              \
    HashHelperClass(unsigned short,         HashType, HashFuncId);              \
    HashHelperClass(wchar_t,                HashType, HashFuncId);              \
    HashHelperClass(int,                    HashType, HashFuncId);              \
    HashHelperClass(unsigned int,           HashType, HashFuncId);              \
    HashHelperClass(long,                   HashType, HashFuncId);              \
    HashHelperClass(unsigned long,          HashType, HashFuncId);              \
    HashHelperClass(long long,              HashType, HashFuncId);              \
    HashHelperClass(unsigned long long,     HashType, HashFuncId);

#define HASH_HELPER_FLOAT(KeyType, ResultType, HashFuncId, HashFunc)            \
    template <>                                                                 \
    struct hash_helper<KeyType, ResultType, HashFuncId> {                       \
        typedef ResultType  result_type;                                        \
        typedef typename std::remove_pointer<KeyType>::type pod_type;           \
                                                                                \
        typedef typename std::remove_pointer<                                   \
                    typename std::remove_cv<                                    \
                        typename std::remove_reference<KeyType>::type           \
                    >::type                                                     \
                >::type decay_type;                                             \
                                                                                \
        static ResultType getHashCode(pod_type data) {                          \
            return static_cast<result_type>(HashFunc((const char *)&data, sizeof(data))); \
        }                                                                       \
    }

#define HASH_HELPER_FLOAT_ALL(HashHelperClass, HashType, HashFuncId, HashFunc)  \
    HashHelperClass(float,  HashType, HashFuncId, HashFunc);                    \
    HashHelperClass(double, HashType, HashFuncId, HashFunc);

namespace jstd {

enum hashes_id_t {
    HashFunc_CRC32C,
    HashFunc_Time31,
    HashFunc_Time31Std,
    HashFunc_Last,
    HashFunc_Default = HashFunc_CRC32C
};

template <typename T,
          typename ResultType = std::uint32_t,
          std::size_t HashFunc = HashFunc_Default>
struct hash_helper {
    typedef ResultType  result_type;

    typedef typename std::remove_pointer<
                typename std::remove_cv<
                    typename std::remove_reference<T>::type
                >::type
            >::type     key_type;

    static ResultType getHashCode(const key_type & key) {
        if (HashFunc == HashFunc_CRC32C)
            return static_cast<result_type>(hashes::hash_crc32c((const char *)&key, sizeof(key)));
        else if (HashFunc == HashFunc_Time31)
            return static_cast<result_type>(hashes::Times31((const char *)&key, sizeof(key)));
        else if (HashFunc == HashFunc_Time31Std)
            return static_cast<result_type>(hashes::Times31Std((const char *)&key, sizeof(key)));
        else
            return static_cast<result_type>(hashes::hash_crc32c((const char *)&key, sizeof(key)));
    }
};

template <typename T,
         typename ResultType,
         std::size_t HashFunc>
struct hash_helper<T *, ResultType, HashFunc> {
    typedef ResultType  result_type;

    typedef typename std::remove_pointer<
                typename std::remove_cv<
                    typename std::remove_reference<T>::type
                >::type
            >::type     key_type;

    static result_type getHashCode(const key_type * key) {
        if (HashFunc == HashFunc_CRC32C)
            return static_cast<result_type>(hashes::hash_crc32c((const char *)key, sizeof(key_type *)));
        else if (HashFunc == HashFunc_Time31)
            return static_cast<result_type>(hashes::Times31((const char *)key, sizeof(key_type *)));
        else if (HashFunc == HashFunc_Time31Std)
            return static_cast<result_type>(hashes::Times31Std((const char *)key, sizeof(key_type *)));
        else
            return static_cast<result_type>(hashes::hash_crc32c((const char *)key, sizeof(key_type *)));
    }
};

template <typename T,
          typename ResultType = std::uint32_t,
          std::size_t HashFunc = HashFunc_Default>
struct string_hash_helper {
    typedef ResultType  result_type;

    typedef typename std::remove_pointer<
                typename std::remove_cv<
                    typename std::remove_reference<T>::type
                >::type
            >::type     key_type;

    typedef typename T::value_type char_type;

    static ResultType getHashCode(const key_type & key) {
        if (key.c_str() != nullptr) {
            if (HashFunc == HashFunc_CRC32C)
                return static_cast<result_type>(hashes::hash_crc32c((const char *)key.c_str(), key.size() * sizeof(char_type)));
            else if (HashFunc == HashFunc_Time31)
                return static_cast<result_type>(hashes::Times31(key.c_str(), key.size()));
            else if (HashFunc == HashFunc_Time31Std)
                return static_cast<result_type>(hashes::Times31Std(key.c_str(), key.size()));
            else
                return static_cast<result_type>(hashes::hash_crc32c((const char *)key.c_str(), key.size() * sizeof(char_type)));
        }
        else {
            return static_cast<ResultType>(0);
        }
    }
};

#if JSTD_HAVE_SSE42_CRC32C

/***************************************************************************
template <>
struct hash_helper<const char *, std::uint32_t, HashFunc_CRC32C> {
    static std::uint32_t getHashCode(const char * data, size_t length) {
        return hashes::hash_crc32c(data, length);
    }
};
****************************************************************************/

HASH_HELPER_CHAR_ALL(HASH_HELPER_CHAR, std::uint32_t, HashFunc_CRC32C, jstd::hashes::hash_crc32c);
HASH_HELPER_INTEGRAL_ALL(HASH_HELPER_INTEGRAL, std::uint32_t, HashFunc_CRC32C);
HASH_HELPER_FLOAT_ALL(HASH_HELPER_FLOAT, std::uint32_t, HashFunc_CRC32C, jstd::hashes::hash_crc32c);

template <>
struct hash_helper<std::string, std::uint32_t, HashFunc_CRC32C> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::string & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::hash_crc32c(key.c_str(), key.size());
        else
            return 0;
    }
};

template <>
struct hash_helper<std::wstring, std::uint32_t, HashFunc_CRC32C> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::wstring & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::hash_crc32c((const char *)key.c_str(), key.size() * sizeof(wchar_t));
        else
            return 0;
    }
};

#endif // JSTD_HAVE_SSE42_CRC32C

/***************************************************************************
template <>
struct hash_helper<const char *, std::uint32_t, HashFunc_Time31> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const char * data, size_t length) {
        return hashes::Times31(data, length);
    }
};
****************************************************************************/

HASH_HELPER_CHAR_ALL(HASH_HELPER_CHAR, std::uint32_t, HashFunc_Time31, hashes::Times31);
HASH_HELPER_INTEGRAL_ALL(HASH_HELPER_INTEGRAL, std::uint32_t, HashFunc_Time31);
HASH_HELPER_FLOAT_ALL(HASH_HELPER_FLOAT, std::uint32_t, HashFunc_Time31, hashes::Times31);

template <>
struct hash_helper<std::string, std::uint32_t, HashFunc_Time31> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::string & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::Times31(key.c_str(), key.size());
        else
            return 0;
    }
};

template <>
struct hash_helper<std::wstring, std::uint32_t, HashFunc_Time31> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::wstring & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::Times31((const char *)key.c_str(), key.size() * sizeof(wchar_t));
        else
            return 0;
    }
};

/***************************************************************************
template <>
struct hash_helper<const char *, std::uint32_t, HashFunc_Time31Std> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const char * data, size_t length) {
        return hashes::Times31_std(data, length);
    }
};
****************************************************************************/

HASH_HELPER_CHAR_ALL(HASH_HELPER_CHAR, std::uint32_t, HashFunc_Time31Std, hashes::Times31Std);
HASH_HELPER_INTEGRAL_ALL(HASH_HELPER_INTEGRAL, std::uint32_t, HashFunc_Time31Std);
HASH_HELPER_FLOAT_ALL(HASH_HELPER_FLOAT, std::uint32_t, HashFunc_Time31Std, hashes::Times31Std);

template <>
struct hash_helper<std::string, std::uint32_t, HashFunc_Time31Std> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::string & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::Times31Std(key.c_str(), key.size());
        else
            return 0;
    }
};

template <>
struct hash_helper<std::wstring, std::uint32_t, HashFunc_Time31Std> {
    typedef std::uint32_t  result_type;

    static std::uint32_t getHashCode(const std::wstring & key) {
        if (likely(key.c_str() != nullptr))
            return hashes::Times31Std((const char *)key.c_str(), key.size() * sizeof(wchar_t));
        else
            return 0;
    }
};

/***********************************************************************

    template <> struct hash<bool>;
    template <> struct hash<char>;
    template <> struct hash<signed char>;
    template <> struct hash<unsigned char>;
    template <> struct hash<char8_t>;        // C++20
    template <> struct hash<char16_t>;
    template <> struct hash<char32_t>;
    template <> struct hash<wchar_t>;
    template <> struct hash<short>;
    template <> struct hash<unsigned short>;
    template <> struct hash<int>;
    template <> struct hash<unsigned int>;
    template <> struct hash<long>;
    template <> struct hash<long long>;
    template <> struct hash<unsigned long>;
    template <> struct hash<unsigned long long>;
    template <> struct hash<float>;
    template <> struct hash<double>;
    template <> struct hash<long double>;
    template <> struct hash<std::nullptr_t>;
    template < class T > struct hash<T *>;

***********************************************************************/

template <typename ResultType>
struct hash_traits {

    // The invalid hash value.
    static const ResultType kInvalidHash = static_cast<ResultType>(-1);
    // The replacement value for invalid hash value.
    static const ResultType kReplacedHash = static_cast<ResultType>(0);

    static ResultType filter(ResultType hash_code) {
        // The hash code can't equal to kInvalidHash, replacement to kReplacedHash.
        return (hash_code != kInvalidHash) ? hash_code : kReplacedHash;
    }

};

template <typename Key, typename ResultType = std::uint32_t,
                        std::size_t HashFunc = HashFunc_Default>
struct hash {
    typedef typename std::remove_pointer<
                typename std::remove_cv<
                    typename std::remove_reference<Key>::type
                >::type
            >::type     key_type;

    typedef Key         argument_type;
    typedef ResultType  result_type;

    hash() {}
    ~hash() {}

    result_type operator() (const key_type & key) const {
        return hash_helper<key_type, result_type, HashFunc>::getHashCode(key);
    }

    result_type operator() (const volatile key_type & key) const {
        return hash_helper<key_type, result_type, HashFunc>::getHashCode(key);
    }

    result_type operator() (const key_type * key) const {
        return hash_helper<key_type *, result_type, HashFunc>::getHashCode(key);
    }

    result_type operator() (const volatile key_type * key) const {
        return hash_helper<key_type *, result_type, HashFunc>::getHashCode(key);
    }
};

} // namespace jstd

#endif // JSTD_HASH_HELPER_H
