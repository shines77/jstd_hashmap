
#ifndef JSTD_SYSTEM_LIBCRAND_H
#define JSTD_SYSTEM_LIBCRAND_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <stdlib.h>     // For ::srand(), ::rand()
#include <time.h>

#include <cstdint>
#include <cstddef>
#include <cstdlib>      // For std::srand(), std::rand()

namespace jstd {

struct LibcRand {
    typedef std::uint32_t value_type;
    typedef std::size_t   size_type;

    static std::uint32_t rand_max() {
#if defined(RAND_MAX)
        return static_cast<std::uint32_t>(RAND_MAX);
#else
        return 0;
#endif
    }

    static void srand(std::uint32_t initSeed = 0) {
        if (initSeed == 0) {
#if 1
            time_t timer;
            ::time(&timer);
            initSeed = static_cast<std::uint32_t>(timer);
            ::srand(initSeed);
#else
            ::srand(static_cast<std::uint32_t>(::time(NULL)));
#endif
        }
        else {
            ::srand(static_cast<std::uint32_t>(initSeed));
        }
    }

    static std::uint32_t rand() {
        return static_cast<std::uint32_t>(::rand());
    }

    static std::uint32_t rand32() {
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return (std::uint32_t)(
          (((std::uint32_t)::rand() & RAND_MAX) << 30)
        | (((std::uint32_t)::rand() & RAND_MAX) << 15)
        |  ((std::uint32_t)::rand() & RAND_MAX)
        );
#elif defined(RAND_MAX) && (RAND_MAX >= 0xFFFF)
    return (std::uint32_t)(
          (((std::uint32_t)::rand() & RAND_MAX) << 16)
        |  ((std::uint32_t)::rand() & RAND_MAX)
        );
#else
    return (std::uint32_t)(
          (((std::uint32_t)::rand() & 0x00FF) << 24)
        | (((std::uint32_t)::rand() & 0x00FF) << 16)
        | (((std::uint32_t)::rand() & 0x00FF) << 8)
        |  ((std::uint32_t)::rand() & 0x00FF)
        );
#endif
    }

    static std::uint64_t rand64() {
#if defined(RAND_MAX) && (RAND_MAX == 0x7FFF)
    return (std::uint64_t)(
          (((std::uint64_t)::rand() & RAND_MAX) << 60)
        | (((std::uint64_t)::rand() & RAND_MAX) << 45)
        | (((std::uint64_t)::rand() & RAND_MAX) << 30)
        | (((std::uint64_t)::rand() & RAND_MAX) << 15)
        |  ((std::uint64_t)::rand() & RAND_MAX)
        );
#elif defined(RAND_MAX) && (RAND_MAX >= 0xFFFF)
    return (std::uint64_t)(
          (((std::uint64_t)::rand() & RAND_MAX) << 48)
        | (((std::uint64_t)::rand() & RAND_MAX) << 32)
        | (((std::uint64_t)::rand() & RAND_MAX) << 16)
        |  ((std::uint64_t)::rand() & RAND_MAX)
        );
#else
    return (std::uint64_t)(
          (((std::uint64_t)::rand() & 0x00FF) << 56)
        | (((std::uint64_t)::rand() & 0x00FF) << 48)
        | (((std::uint64_t)::rand() & 0x00FF) << 40)
        | (((std::uint64_t)::rand() & 0x00FF) << 32)
        | (((std::uint64_t)::rand() & 0x00FF) << 24)
        | (((std::uint64_t)::rand() & 0x00FF) << 16)
        | (((std::uint64_t)::rand() & 0x00FF) << 8)
        |  ((std::uint64_t)::rand() & 0x00FF)
        );
#endif
    }
};

} // namespace jstd

#endif // JSTD_SYSTEM_LIBCRAND_H
