
#ifndef JSTD_CONFIG_CONFIG_CXX_H
#define JSTD_CONFIG_CONFIG_JSTD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

//
// [Visual Studio version] ==> _MSC_VER
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170
//
// 1. MSVC
//
// default: __cplusplus = 199711L, /Zc:__cplusplus
//
// C++11: from Visual Studio 2015 Update 3:
// _MSC_FULL_VER = 190024210, _MSVC_LANG = 201402L
// or __cplusplus >= 201103L
//
// C++14: _MSVC_LANG = 201402L, /std:c++14
// C++17: _MSVC_LANG = 201703L, /std:c++17
// C++20: _MSVC_LANG = 202002L, /std:c++20
//
// _MSVC_LANG: Since Visual Studio 2015 Update 3
//
// Visual Studio 2015 Update 3: _MSC_FULL_VER = 190024210, _MSVC_LANG = 201402L
//
// 2. gcc and clang
//
// C++11: __cplusplus >= 201103L
// C++14: __cplusplus >= 201402L
// C++17: __cplusplus >= 201703L,
//
// GCC 9.0.0: 201709L (C++ 2a), Clang 8.0.0: 201707L, VC++ 15.9.3: 201704L
//
// C++20: __cplusplus >= 202002L
//
// GCC 11.0.0, Clang 10.0.0, VC++ 19.29, ICX: 2021, ICC
//

#if defined(_MSC_VER) && !defined(__clang__)
  #if (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)
    #ifndef JSTD_IS_CXX_20
    #define JSTD_IS_CXX_20  1
    #endif
    #define jstd_cplusplus  2020L
  #elif (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L && _MSVC_LANG < 202002L)) \
     || (__cplusplus >= 201703L && __cplusplus < 202002L)
    #ifndef JSTD_IS_CXX_17
    #define JSTD_IS_CXX_17  1
    #endif
    #define jstd_cplusplus  2017L
  #elif (defined(_MSVC_LANG) && (_MSVC_LANG >= 201402L && _MSVC_LANG < 201703L \
     && _MSC_VER >= 1910)) \
     || (__cplusplus >= 201402L && __cplusplus < 201703L && _MSC_VER >= 1910)
    #ifndef JSTD_IS_CXX_14
    #define JSTD_IS_CXX_14  1
    #endif
    #define jstd_cplusplus  2014L
  #elif defined(_MSC_VER) && (_MSC_FULL_VER >= 190024210) \
     || (__cplusplus >= 201103L)
    #ifndef JSTD_IS_CXX_11
    #define JSTD_IS_CXX_11  1
    #endif
    #define jstd_cplusplus  2011L
  #else
    #ifndef JSTD_IS_CXX_98
    #define JSTD_IS_CXX_98  1
    #endif
    #define jstd_cplusplus  1997L
  #endif
#elif defined(__GNUC__) || defined(__clang__)
  #if (__cplusplus >= 202002L)
    #ifndef JSTD_IS_CXX_20
    #define JSTD_IS_CXX_20  1
    #endif
    #define jstd_cplusplus  2020L
  #elif (__cplusplus >= 201703L && __cplusplus < 202002L)
    #ifndef JSTD_IS_CXX_17
    #define JSTD_IS_CXX_17  1
    #endif
    #define jstd_cplusplus  2017L
  #elif (__cplusplus >= 201402L && __cplusplus < 201703L)
    #ifndef JSTD_IS_CXX_14
    #define JSTD_IS_CXX_14  1
    #endif
    #define jstd_cplusplus  2017L
  #elif (__cplusplus >= 201103L && __cplusplus < 201402L)
    #ifndef JSTD_IS_CXX_11
    #define JSTD_IS_CXX_11  1
    #endif
    #define jstd_cplusplus  2011L
  #else
    #ifndef JSTD_IS_CXX_98
    #define JSTD_IS_CXX_98  1
    #endif
    #define jstd_cplusplus  1997L
  #endif
#endif // _MSC_VER && !__clang__

#endif // JSTD_CONFIG_CONFIG_JSTD_H
