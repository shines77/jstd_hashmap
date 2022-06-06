/** 
 * This file is part of the Mingw32 package.
 *  unistd.h maps     (roughly) to io.h
 */
#ifndef JSTD_UNISTD_H
#define JSTD_UNISTD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER)
#include <io.h>
#include <process.h>
#else
// defined(__linux__) || defined(__clang__) || defined(__FreeBSD__) || (defined(__GNUC__) && defined(__cygwin__))
#include <unistd.h>
#endif // _MSC_VER

#endif // JSTD_UNISTD_H
