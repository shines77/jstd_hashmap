
#ifndef JSTD_BASIC_EXPORT_H
#define JSTD_BASIC_EXPORT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/platform.h"
#include "jstd/basic/compiler.h"

#if defined(JSTD_BUILD_DLL) || defined(BUILD_DLL) || defined(_USRDLL) \
    || defined(JSTD_SHARED) || defined(JSTD_BUILD_SHARED)
    || defined(JSTD_ENABLE_SHARED) || defined(ENABLE_SHARED)
    /* build a dll library */
    #undef JSTD_BUILD_STATIC
    #undef JSTD_USE_SHARED
    #undef JSTD_USE_STATIC

    #ifndef JSTD_BUILD_SHARED
        #define JSTD_BUILD_SHARED
    #endif
#elif defined(JSTD_USE_DLL) || defined(USE_DLL) \
    || defined(JSTD_USE_SHARED) || defined(USE_SHARED)
    /* use a dll library */
    #undef JSTD_BUILD_SHARED
    #undef JSTD_BUILD_STATIC
    #undef JSTD_USE_STATIC

    #ifndef JSTD_USE_SHARED
        #define JSTD_USE_SHARED
    #endif
#elif defined(JSTD_BUILD_LIB) || defined(BUILD_LIB) || defined(_LIB) \
    || defined(JSTD_LIB) || defined(JSTD_IS_LIB) \
    || defined(JSTD_STATIC) || defined(JSTD_BUILD_STATIC) \
    || defined(JSTD_ENABLED_STATIC) || defined(ENABLED_STATIC)
    /* build a static library */
    #undef JSTD_BUILD_SHARED
    #undef JSTD_USE_SHARED
    #undef JSTD_USE_STATIC

    #ifndef JSTD_BUILD_STATIC
        #define JSTD_BUILD_STATIC
    #endif
#else /* defined(USE_LIB) \
    || defined(JSTD_USE_LIB) || defined(USE_LIB) \
    || defined(JSTD_USE_STATIC) || defined(USE_STATIC) */
    /* use a static library */
    #undef JSTD_BUILD_SHARED
    #undef JSTD_BUILD_STATIC
    #undef JSTD_USE_SHARED

    #ifndef JSTD_USE_STATIC
        #define JSTD_USE_STATIC
    #endif
#endif

/**************************************************************************************
   Dynamic Library import / export / static control conditional
   (Define JSTD_DECLARE_EXPORT to export symbols, else they are imported or static)
**************************************************************************************/

#ifdef JSTD_DLL
#undef JSTD_DLL
#endif

#if defined(JSTD_IS_MSVC)  /* is microsoft visual studio ? */
    #if defined(JSTD_BUILD_SHARED)      /* build a dll library */
        #define JSTD_DLL                __declspec(dllexport)
        #define JSTD_DLL_TPL            __declspec(dllexport)
        #define JSTD_PRIVATE
        #define JSTD_EXPIMP_TEMPLATE
    #elif defined(JSTD_USE_SHARED)      /* use a dll library */
        #define JSTD_DLL                __declspec(dllimport)
        #define JSTD_DLL_TPL            __declspec(dllimport)   // or don't defined it!
        #define JSTD_PRIVATE
        #define JSTD_EXPIMP_TEMPLATE    extern
    #elif defined(JSTD_BUILD_STATIC)    /* build a static library */
        #define JSTD_DLL
        #define JSTD_DLL_TPL
        #define JSTD_PRIVATE
        #define JSTD_EXPIMP_TEMPLATE
    #else                               /* use a static library */
        #define JSTD_DLL
        #define JSTD_DLL_TPL
        #define JSTD_PRIVATE
        #define JSTD_EXPIMP_TEMPLATE
    #endif
#elif defined(JSTD_IS_GNUC)
    #define JSTD_DLL                    __attribute__ ((visibility ("default")))
    #define JSTD_DLL                    __attribute__ ((visibility ("default")))
    #define JSTD_DLL_TPL                __attribute__ ((visibility ("default")))
    #define JSTD_PRIVATE                __attribute__ ((visibility ("default")))
    #if defined(JSTD_USE_SHARED)
        #define JSTD_EXPIMP_TEMPLATE    extern
    #else
        #define JSTD_EXPIMP_TEMPLATE
    #endif
#else  /* not is msvc and not is gunc! */
    #define JSTD_DLL
    #define JSTD_DLL_TPL
    #define JSTD_PRIVATE
    #if defined(JSTD_USE_SHARED)
        #define JSTD_EXPIMP_TEMPLATE    extern
    #else
        #define JSTD_EXPIMP_TEMPLATE
    #endif
#endif

////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) || __has_declspec_attribute(dllexport)
  #define DLL_EXPORT        __declspec(dllexport)
#else
  #if defined(__GNUC__) || defined(__clang__) || defined(__linux__)
    #define DLL_EXPORT      __attribute__((visibility("default")))
  #else
    #define DLL_EXPORT
  #endif
#endif

#if defined(_MSC_VER) || __has_declspec_attribute(dllimport)
  #define DLL_IMPORT        __declspec(dllimport)
#else
  #define DLL_IMPORT
#endif

////////////////////////////////////////////////////////////////////////////////

/**
 * For exported func
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  #define JSTD_EXPORTED_FUNC        __cdecl
  #define JSTD_EXPORTED_METHOD      __thiscall
#else
  #define JSTD_EXPORTED_FUNC
  #define JSTD_EXPORTED_METHOD
#endif

#endif // JSTD_BASIC_EXPORT_H
