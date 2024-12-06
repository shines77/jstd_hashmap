
#ifndef JSTD_BASIC_ASSERT_H
#define JSTD_BASIC_ASSERT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/config/config.h"

#include <stddef.h>
#include <assert.h>

#ifndef JSTD_EXPORTED_FUNC
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define JSTD_EXPORTED_FUNC   __cdecl
#else
#define JSTD_EXPORTED_FUNC
#endif
#endif // JSTD_EXPORTED_FUNC

#if JSTD_USE_ASSERT

//! Assert that x is true.
/** If x is false, print assertion failure message.  
    If the comment argument is not NULL, it is printed as part of the failure message.  
    The comment argument has no other effect. */
#if 0
#if defined(__cplusplus)
#define JSTD_ASSERT_MARCO(predicate, message) \
    do { \
        if (!predicate) \
            ::jstd_assertion_failure(__FILE__, __LINE__, #predicate, message); \
    } while (0)
#else
#define JSTD_ASSERT_MARCO(predicate, message) \
    do { \
        if (!predicate) \
            jstd_assertion_failure(__FILE__, __LINE__, #predicate, message); \
    } while (0)
#endif  /* __cplusplus */
#else
#define JSTD_ASSERT_MARCO(predicate, message) \
    ((predicate) ? ((void)0) : jstd_assertion_failure(__FILE__, __LINE__, #predicate, message))
#endif

#ifndef JSTD_ASSERT
#define JSTD_ASSERT_TRUE(predicate)                JSTD_ASSERT_MARCO(!(predicate),  NULL)
#define JSTD_ASSERT_FALSE(predicate)               JSTD_ASSERT_MARCO(!!(predicate), NULL)
#define JSTD_ASSERT(predicate)                     JSTD_ASSERT_FALSE(predicate)
#endif

#ifndef JSTD_ASSERT_EX
#define JSTD_ASSERT_EX_TRUE(predicate, comment)    JSTD_ASSERT_MARCO(!(predicate),  comment)
#define JSTD_ASSERT_EX_FALSE(predicate, comment)   JSTD_ASSERT_MARCO(!!(predicate), comment)
#define JSTD_ASSERT_EX(predicate, comment)         JSTD_ASSERT_EX_FALSE(predicate,  comment)
#endif

#else  /* !JSTD_USE_ASSERT */

//! No-op version of JSTD_ASSERT.
#define JSTD_ASSERT_TRUE(predicate)                ((void)0)
#define JSTD_ASSERT_FALSE(predicate)               ((void)0)
#define JSTD_ASSERT(predicate)                     JSTD_ASSERT_FALSE(predicate)

//! "Extended" version is useful to suppress warnings if a variable is only used with an assert
#define JSTD_ASSERT_EX_TRUE(predicate, comment)    ((void)0)
#define JSTD_ASSERT_EX_FALSE(predicate, comment)   ((void)0)
#define JSTD_ASSERT_EX(predicate, comment)         JSTD_ASSERT_EX_FALSE(predicate, comment)

#endif  /* !JSTD_USE_ASSERT */

#ifndef jstd_assert
#define jstd_assert_true           JSTD_ASSERT_TRUE
#define jstd_assert_false          JSTD_ASSERT_FALSE
#define jstd_assert                JSTD_ASSERT
#endif

#ifndef jstd_assert_ex
#define jstd_assert_ex_true        JSTD_ASSERT_EX_TRUE
#define jstd_assert_ex_false       JSTD_ASSERT_EX_FALSE
#define jstd_assert_ex             JSTD_ASSERT_EX
#endif

#ifdef __cplusplus
extern "C" {
#endif

    //! Type for an assertion handler
    typedef void (*jstd_assertion_handler_type)(const char * filename, int line,
                                                 const char * expression, const char * comment);

    //! Set assertion handler and return previous value of it.
    jstd_assertion_handler_type
    JSTD_EXPORTED_FUNC set_c_assertion_handler(jstd_assertion_handler_type new_handler);

    //! Process an assertion failure.
    /** Normally called from JSTD_ASSERT macro.
        If assertion handler is null, print message for assertion failure and abort.
        Otherwise call the assertion handler. */
    void JSTD_EXPORTED_FUNC jstd_assertion_failure(const char * filename, int line,
                                                   const char * expression, const char * comment);

#if !JSTD_MALLOC_BUILD
    //! Report a runtime warning.
    void JSTD_EXPORTED_FUNC jstd_runtime_warning(const char * format, ...);
#endif  /* !JSTD_MALLOC_BUILD */

#ifdef __cplusplus
}
#endif

#endif // JSTD_BASIC_ASSERT_H
