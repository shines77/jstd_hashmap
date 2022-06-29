
#include "jstd/basic/assert.h"
//#include "jstd/string/jm_strings.h"

// include headers for required function declarations
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if _MSC_VER
#include <crtdbg.h>
#ifndef JSTD_USE_DBGBREAK_DLG
#define JSTD_USE_DBGBREAK_DLG   JSTD_IS_DEBUG
#endif
#endif

//! Type for an assertion handler
//typedef void (*jstd_assertion_handler_type)(const char * filename, int line,
//                                            const char * expression, const char * comment);

static jstd_assertion_handler_type jstd_assertion_handler = NULL;

jstd_assertion_handler_type
JSTD_EXPORTED_FUNC set_c_assertion_handler(jstd_assertion_handler_type new_handler) {
    jstd_assertion_handler_type old_handler = jstd_assertion_handler;
    jstd_assertion_handler = new_handler;
    return old_handler;
}

void JSTD_EXPORTED_FUNC jstd_assertion_failure(const char * filename, int line,
                                               const char * expression, const char * comment) {
    static int already_failed;
    jstd_assertion_handler_type assert_handler = jstd_assertion_handler;    
    if (assert_handler) {
        (*assert_handler)(filename, line, expression, comment);
    }
    else {
        if (!already_failed) {
            already_failed = 1;
            fprintf(stderr, "Assertion %s failed on line %d of file %s\n", expression, line, filename);
            if (comment)
                fprintf(stderr, "Detailed description: %s\n", comment);
#if JSTD_USE_DBGBREAK_DLG
            if (1 == _CrtDbgReport(_CRT_ASSERT, filename, line, "jstd_shared_debug.dll",
                "%s\r\n%s", expression, comment ? comment : "")) {
                _CrtDbgBreak();
            }
#else
            fflush(stderr);
            abort();
#endif
        }
    }
}

#if JSTD_USE_ASSERT

#if !JIMI_MALLOC_BUILD
    //! Report a runtime warning.
    void JSTD_EXPORTED_FUNC jstd_runtime_warning(const char * format, ...)
    {
        va_list args;
        char str[1024];
        memset(str, 0, 1024);
        va_start(args, format);
        vsnprintf(str, 1024, format, args);
        va_end(args);
        fprintf(stderr, "Jstd warning: %s\n", str);
    }
#endif

#endif // JSTD_USE_ASSERT
