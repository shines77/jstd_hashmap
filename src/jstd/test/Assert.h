
#ifndef JSTD_TEST_ASSERT_H
#define JSTD_TEST_ASSERT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/test/FloatEpsinon.h"

//
// JTEST_ASSERT_XX
//

#define JTEST_ASSERT_TRUE(condition) \
    do { \
        if (!!(condition)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_FALSE(condition) \
    do { \
        if (!!!(condition)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_NE(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_GE(val1, val2) \
    do { \
        if ((val1) >= (val2)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_LE(val1, val2) \
    do { \
        if ((val1) <= (val2)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_GT(val1, val2) \
    do { \
        if ((val1) > (val2)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_LT(val1, val2) \
    do { \
        if ((val1) < (val2)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_ASSERT_STR_XX
//

#define JTEST_ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_NE(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_GT(str1, str2) \
    do { \
        if (strcmp((str1), (str2)) > 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_LT(str1, str2) \
    do { \
        if (strcmp((str1), (str2)) < 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_ASSERT_STR_NOCASE_XX
//

#define JTEST_ASSERT_STR_NOCASE_EQ(expected, actual) \
    do { \
        if (stricmp((expected), (actual)) == 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_NOCASE_NE(expected, actual) \
    do { \
        if (stricmp((expected), (actual)) != 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_NOCASE_GT(str1, str2) \
    do { \
        if (stricmp((str1), (str2)) > 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_STR_NOCASE_LT(str1, str2) \
    do { \
        if (stricmp((str1), (str2)) < 0) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_ASSERT_FLOAT_XX
//

#define JTEST_ASSERT_FLOAT_EQ(expected, actual) \
    do { \
        if (((expected) - (actual) <= FLOAT_POSITIVE_EPSINON) && \
            ((expected) - (actual) >= FLOAT_NEGATIVE_EPSINON)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_FLOAT_NE(expected, actual) \
    do { \
        if (((expected) - (actual) > FLOAT_POSITIVE_EPSINON) || \
            ((expected) - (actual) < FLOAT_NEGATIVE_EPSINON)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_ASSERT_DOUBLE_XX
//

#define JTEST_ASSERT_DOUBLE_EQ(expected, actual) \
    do { \
        if (((expected) - (actual) <= DOUBLE_POSITIVE_EPSINON) && \
            ((expected) - (actual) >= DOUBLE_NEGATIVE_EPSINON)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#define JTEST_ASSERT_DOUBLE_NE(expected, actual) \
    do { \
        if (((expected) - (actual) > DOUBLE_POSITIVE_EPSINON) || \
            ((expected) - (actual) < DOUBLE_NEGATIVE_EPSINON)) { \
            print_passed_ln(); \
        } \
        else { \
            print_failed_ln(); \
        } \
    } while (0)

#endif // JSTD_TEST_ASSERT_H
