
#ifndef JSTD_CONFIG_CONFIG_H
#define JSTD_CONFIG_CONFIG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if 0
#include "jstd/config/config_pre.h"

#include "jstd/config/config_arch.h"
#include "jstd/config/config_hw.h"

#if defined(_MSC_VER) || defined(_WIN32) ||
#include "jstd/config/win32/config.h"
#elif defined(__linux__) || defined(__LINUX__)
#include "jstd/config/linux/config.h"
#elif defined(__unix__) || defined(__UNIX__)
#include "jstd/config/unix/config.h"
#elif defined(__apple__)
#include "jstd/config/macos/config.h"
#else
#include "jstd/config/default/config.h"
#endif

#include "jstd/config/config_post.h"
#endif

#endif // JSTD_CONFIG_CONFIG_H
