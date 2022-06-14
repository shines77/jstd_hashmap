
#ifndef JSTD_TEST_EXPERT_H
#define JSTD_TEST_EXPERT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/test/FloatEpsinon.h"

//
// JTEST_EXPECT_BOOLEAN
//

#define JTEST_EXPECT_TRUE(condition) \
    do { \
        if (!!(condition)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_FALSE(condition) \
    do { \
        if (!!!(condition)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_EXPECT_XX
//

#define JTEST_EXPECT_EQ(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_NE(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_GE(val1, val2) \
    do { \
        if ((val1) >= (val2)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_LE(val1, val2) \
    do { \
        if ((val1) <= (val2)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_GT(val1, val2) \
    do { \
        if ((val1) > (val2)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_LT(val1, val2) \
    do { \
        if ((val1) < (val2)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_EXPECT_STR_XX
//

#define JTEST_EXPECT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_NE(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_GT(str1, str2) \
    do { \
        if (strcmp((str1), (str2)) > 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_LT(str1, str2) \
    do { \
        if (strcmp((str1), (str2)) < 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_EXPECT_STR_NOCASE_XX
//

#define JTEST_EXPECT_STR_NOCASE_EQ(expected, actual) \
    do { \
        if (stricmp((expected), (actual)) == 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_NOCASE_NE(expected, actual) \
    do { \
        if (stricmp((expected), (actual)) != 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_NOCASE_GT(str1, str2) \
    do { \
        if (stricmp((str1), (str2)) > 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_STR_NOCASE_LT(str1, str2) \
    do { \
        if (stricmp((str1), (str2)) < 0) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_EXPECT_FLOAT_XX
//

#define JTEST_EXPECT_FLOAT_EQ(expected, actual) \
    do { \
        if (((expected) - (actual) <= FLOAT_POSITIVE_EPSINON) && \
            ((expected) - (actual) >= FLOAT_NEGATIVE_EPSINON)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_FLOAT_NE(expected, actual) \
    do { \
        if (((expected) - (actual) > FLOAT_POSITIVE_EPSINON) || \
            ((expected) - (actual) < FLOAT_NEGATIVE_EPSINON)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

//
// JTEST_EXPECT_DOUBLE_XX
//

#define JTEST_EXPECT_DOUBLE_EQ(expected, actual) \
    do { \
        if (((expected) - (actual) <= DOUBLE_POSITIVE_EPSINON) && \
            ((expected) - (actual) >= DOUBLE_NEGATIVE_EPSINON)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#define JTEST_EXPECT_DOUBLE_NE(expected, actual) \
    do { \
        if (((expected) - (actual) > DOUBLE_POSITIVE_EPSINON) || \
            ((expected) - (actual) < DOUBLE_NEGATIVE_EPSINON)) { \
            jstd::print_passed_ln(); \
        } \
        else { \
            jstd::print_failed_ln(); \
        } \
    } while (0)

#endif // JSTD_TEST_EXPERT_H
