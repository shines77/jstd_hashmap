
#ifndef JSTD_BASIC_PLATFORM_H
#define JSTD_BASIC_PLATFORM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define JSTD_PLATFORM_WINDOWS   0
#define JSTD_PLATFORM_UNIX      1
#define JSTD_PLATFORM_APPLE     2
#define JSTD_PLATFORM_INTEL     3

// Must be first (win 64 also define _WIN32)
#if defined(_WIN64)
#  define JSTD_PLATFORM     JSTD_PLATFORM_WINDOWS
#elif defined(__WIN32__) || defined(WIN32) || defined(_WIN32)
#  define JSTD_PLATFORM     JSTD_PLATFORM_WINDOWS
#elif defined(__APPLE_CC__)
#  define JSTD_PLATFORM     JSTD_PLATFORM_APPLE
#elif defined(__INTEL_COMPILER)
#  define JSTD_PLATFORM     JSTD_PLATFORM_INTEL
#else
#  define JSTD_PLATFORM     JSTD_PLATFORM_UNIX
#endif

#if defined(_M_X64) || defined(_M_IX64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
  #define JSTD_IS_X86           1
  #define JSTD_IS_X86_64        1
  #define JSTD_WORD_LEN         64
#elif defined (_M_IX86) || defined(__i386__)
  #define JSTD_IS_X86           1
  #define JSTD_IS_X86_I386      1
  #define JSTD_WORD_LEN         32
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM64__) || defined(__arm64__)
  #define JSTD_IS_ARM           1
  #define JSTD_IS_ARM64         1
  #define JSTD_WORD_LEN         64
#elif defined(__aarch32__) || defined(_M_ARM) || defined(__ARM__) || defined(__arm__)
  #define JSTD_IS_ARM           1
  #define JSTD_IS_ARM32         1
  #define JSTD_WORD_LEN         32
#elif defined(_M_MPPC)
  // Power Macintosh PowerPC
  #define JSTD_WORD_LEN         32
#elif defined(_M_PPC)
  // PowerPC
  #define JSTD_WORD_LEN         32
#endif

#ifndef JSTD_WORD_LEN
  #if defined(WIN32) || defined(_WIN32)
    #define JSTD_WORD_LEN       32
  #elif defined(WIN64) || defined(_WIN64)
    #define JSTD_WORD_LEN       64
  #endif
#endif // !JSTD_WORD_LEN

/**
 Config of jimi project, per target platform.
 */

//////////////////////////////////////////////////////////////////////////
// pre configure
//////////////////////////////////////////////////////////////////////////

// define supported target platform macro which JIMI uses.
#define JSTD_PLATFORM_UNKNOWN           0
// Windows family
#define JSTD_PLATFORM_WIN32             1
#define JSTD_PLATFORM_WIN64             2
#define JSTD_PLATFORM_WINCE             3
#define JSTD_PLATFORM_WINRT             4
#define JSTD_PLATFORM_WP8               5
// Linux family
#define JSTD_PLATFORM_LINUX             20
// Apple family
#define JSTD_PLATFORM_MAC               40
#define JSTD_PLATFORM_IOS               41
// Mobile family
#define JSTD_PLATFORM_ANDROID           50
#define JSTD_PLATFORM_BLACKBERRY        60
// Other systems
#define JSTD_PLATFORM_MARMALADE         100
#define JSTD_PLATFORM_BADA              101
#define JSTD_PLATFORM_NACL              102
#define JSTD_PLATFORM_EMSCRIPTEN        103
#define JSTD_PLATFORM_TIZEN             104

// Determine target platform by compile environment macro.
#undef  JSTD_TARGET_PLATFORM
#define JSTD_TARGET_PLATFORM            JSTD_PLATFORM_UNKNOWN

#ifndef JSTD_PLATFORM_NAME
#define JSTD_PLATFORM_NAME(X)           JSTD_PLATFORM_##X
#endif

#ifndef JSTD_CHECK_PLATFORM
    #ifdef JSTD_TARGET_PLATFORM
        #define JSTD_CHECK_PLATFORM(X)  ((JSTD_PLATFORM_##X) && (JSTD_TARGET_PLATFORM == JSTD_PLATFORM_##X))
    #else
        #define JSTD_CHECK_PLATFORM(X)  (JSTD_PLATFORM_##X)
    #endif
#endif  // JSTD_CHECK_PLATFORM

#ifndef JSTD_CHECK_OS
#define JSTD_CHECK_OS                   JSTD_CHECK_PLATFORM
#endif

// marmalade
#if defined(MARMALADE)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_MARMALADE
    #ifndef JSTD_IS_MARMALADE
    #define JSTD_IS_MARMALADE           1
    #endif
#endif

// bada
#if defined(SHP)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_BADA
    #ifndef JSTD_IS_BADA
    #define JSTD_IS_BADA                1
    #endif
#endif

// native client
#if defined(__native_client__)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_NACL
    #ifndef JSTD_IS_NACL
    #define JSTD_IS_NACL                1
    #endif
#endif

// Emscripten
#if defined(EMSCRIPTEN)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_EMSCRIPTEN
    #ifndef JSTD_IS_EMSCRIPTEN
    #define JSTD_IS_EMSCRIPTEN          1
    #endif
#endif

// tizen
#if defined(TIZEN)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_TIZEN
    #ifndef JSTD_IS_WINRT
    #define JSTD_IS_TIZEN               1
    #endif
#endif

// qnx
#if defined(__QNX__)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_BLACKBERRY
    #ifndef JSTD_IS_BLACKBERRY
    #define JSTD_IS_BLACKBERRY          1
    #endif
#endif

// android
#if defined(ANDROID)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_ANDROID
    #ifndef JSTD_IS_ANDROID
    #define JSTD_IS_ANDROID             1
    #endif
#endif

// iphone
#if defined(JSTD_TARGET_OS_IPHONE)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_IOS
    #ifndef JSTD_IS_OS_IPHONE
    #define JSTD_IS_OS_IPHONE           1
    #endif
#endif

// ipad, perhaps this precompiled entry is invalid.
#if defined(JSTD_TARGET_OS_IPAD)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_IOS
    #ifndef JSTD_IS_OS_IPAD
    #define JSTD_IS_OS_IPAD             1
    #endif
#endif

// mac
#if defined(JSTD_TARGET_OS_MAC)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_MAC
    #ifndef JSTD_IS_OS_MAC
    #define JSTD_IS_OS_MAC              1
    #endif
#endif

// linux
#if defined(__linux__) || defined(__linux) || defined(LINUX)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_LINUX
    // JSTD_IS_LINUX has defined below
    #ifndef JSTD_IS_OS_LINUX
    #define JSTD_IS_OS_LINUX            1
    #endif
#endif

// WinCE (Windows CE)
#if defined(WCE) || defined(_WCE) || defined(WINCE) || defined(_WIN32_WCE)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_WINCE
    #ifndef JSTD_IS_WINCE
    #define JSTD_IS_WINCE               1
    #endif
#endif

// WinRT (Windows Store App)
#if defined(WINRT) || defined(_WINRT)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_WINRT
    #ifndef JSTD_IS_WINRT
    #define JSTD_IS_WINRT               1
    #endif
#endif

// WP8 (Windows Phone 8 App)
#if defined(WP8) || defined(_WP8)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_WP8
    #ifndef JSTD_IS_WP8
    #define JSTD_IS_WP8                 1
    #endif
#endif

// win32
#if defined(WIN32) || defined(_WIN32) || defined(_WINDOWS) || defined(__WIN32__) || defined(__NT__)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_WIN32
    #ifndef JSTD_IS_WIN32
    #define JSTD_IS_WIN32               1
    #endif
#endif

// win64
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
    #undef  JSTD_TARGET_PLATFORM
    #define JSTD_TARGET_PLATFORM        JSTD_PLATFORM_WIN64
    #ifndef JSTD_IS_WIN64
    #define JSTD_IS_WIN64               1
    #endif
#endif

#if (defined(JSTD_IS_WIN32) || defined(JSTD_IS_WIN64)) && !(defined(__GNUC__) || defined(__MINGW__) \
    || defined(__MINGW32__) || defined(__MINGW64__))
#ifndef JSTD_IS_WINDOWS
#define JSTD_IS_WINDOWS                 (JSTD_CHECK_PLATFORM(WIN32) || JSTD_CHECK_PLATFORM(WIN64))
#endif
#endif

#if defined(JSTD_IS_WIN32) || defined(JSTD_IS_WIN64) || defined(JSTD_IS_WINCE) \
 || defined(JSTD_IS_WINRT) || defined(JSTD_IS_WP8)
#ifndef JSTD_IS_WINFAMILY
#define JSTD_IS_WINFAMILY               (JSTD_CHECK_PLATFORM(WIN32) || JSTD_CHECK_PLATFORM(WIN64) \
                                      || JSTD_CHECK_PLATFORM(WINCE) || JSTD_CHECK_PLATFORM(WINRT) \
                                      || JSTD_CHECK_PLATFORM(WP8))
#endif
#endif

#if defined(JSTD_IS_OS_UNIX)
#ifndef JSTD_IS_UNIX
#define JSTD_IS_UNIX                    (JSTD_CHECK_PLATFORM(LINUX))
#endif
#endif

#if defined(JSTD_IS_OS_LINUX)
#ifndef JSTD_IS_LINUX
#define JSTD_IS_LINUX                   (JSTD_CHECK_PLATFORM(LINUX))
#endif
#endif

#if defined(JSTD_IS_OS_IPHONE) || defined(JSTD_IS_OS_IPAD)
#ifndef JSTD_IS_IOS
#define JSTD_IS_IOS                     (JSTD_CHECK_PLATFORM(OS_IPHONE) || JSTD_CHECK_PLATFORM(OS_IPAD))
#endif
#endif

#if defined(JSTD_IS_OS_MAC)
#ifndef JSTD_IS_MAC
#define JSTD_IS_MAC                     (JSTD_CHECK_PLATFORM(OS_MAC))
#endif
#endif

#if defined(JSTD_IS_IOS) || defined(JSTD_IS_MAC)
#ifndef JSTD_IS_APPLE
#define JSTD_IS_APPLE                   (JSTD_CHECK_PLATFORM(IOS) || JSTD_CHECK_PLATFORM(MAC))
#endif
#endif

// for DOXYGEN
#if defined(DOXYGEN)
  #ifndef JSTD_IS_DOXYGEN
    #define JSTD_IS_DOXYGEN             1
  #endif
#endif

//////////////////////////////////////////////////////////////////////////
// post configure
//////////////////////////////////////////////////////////////////////////

// check user set platform
#if (!defined(JSTD_TARGET_PLATFORM)) || (JSTD_TARGET_PLATFORM == JSTD_PLATFORM_UNKNOWN)
    #error "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif

#if (JSTD_TARGET_PLATFORM == JSTD_PLATFORM_WIN32 || JSTD_TARGET_PLATFORM == JSTD_PLATFORM_WIN64)
#pragma warning (disable:4127)
#endif // JSTD_PLATFORM_WIN32

#endif // JSTD_BASIC_PLATFORM_H
