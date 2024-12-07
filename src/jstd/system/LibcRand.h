
/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2017-2022 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

*************************************************************************************

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

  You are free to:

    1. Share -- copy and redistribute the material in any medium or format.

    2. Adapt -- remix, transforn, and build upon the material for any purpose,
    even commerically.

    The licensor cannot revoke these freedoms as long as you follow the license terms.

  Under the following terms:

    * Attribution -- You must give appropriate credit, provide a link to the license,
    and indicate if changes were made. You may do so in any reasonable manner,
    but not in any way that suggests the licensor endorses you or your use.

    * ShareAlike -- If you remix, transform, or build upon the material, you must
    distribute your contributions under the same license as the original.

    * No additional restrictions -- You may not apply legal terms or technological
    measures that legally restrict others from doing anything the license permits.

  Notices:

    * You do not have to comply with the license for elements of the material
    in the public domain or where your use is permitted by an applicable exception
    or limitation.

    * No warranties are given. The license may not give you all of the permissions
    necessary for your intended use. For example, other rights such as publicity,
    privacy, or moral rights may limit how you use the material.

************************************************************************************/

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

struct JSTD_DLL LibcRand {
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
