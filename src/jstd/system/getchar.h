
#ifndef JSTD_SYSTEM_GETCHAR_H
#define JSTD_SYSTEM_GETCHAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/// <comment>
///
/// What is equivalent to getch() & getche() in Linux?
///
/// See: http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
///
/// </comment>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER)) || defined(__linux__)

/* Read 1 character - echo defines echo mode */
int jstd_getch_term(int echo);

#endif // __GNUC__

/* Read 1 character without echo */
int jstd_getch(void);

/* Read 1 character with echo */
int jstd_getche(void);

#ifdef __cplusplus
}
#endif

#endif // JSTD_SYSTEM_GETCHAR_H
