
#ifndef JSTD_CONFIG_CONFIG_H
#define JSTD_CONFIG_CONFIG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Minimum requirements: gcc/clang C++ 11 or MSVC 2015 Update 3.
#if (!defined(_MSC_VER) && defined(__cplusplus) && (__cplusplus < 201103L)) \
 || (defined(_MSVC_LANG) && (_MSVC_LANG < 201103L)) \
 || (defined(_MSC_VER) && (_MSC_FULL_VER < 190024210))
#error "jstd requires C++ 11 support."
#endif

#include "jstd/config/config_pre.h"

// Could not change their order.
#include "jstd/basic/platform.h"
#include "jstd/basic/compiler.h"
#include "jstd/basic/export.h"

#include "jstd/config/version.h"
#include "jstd/config/config_jstd.h"
#include "jstd/config/config_hw.h"
#include "jstd/config/config_cxx.h"
#include "jstd/config/config_post.h"

#endif // JSTD_CONFIG_CONFIG_H
