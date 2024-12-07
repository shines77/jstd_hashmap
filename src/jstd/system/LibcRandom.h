
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

#ifndef JSTD_SYSTEM_LIBCRANDOM_H
#define JSTD_SYSTEM_LIBCRANDOM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <cstdint>
#include <cstddef>
#include <cstdlib>      // For std::srand(), std::rand()

#include "jstd/system/LibcRand.h"

namespace jstd {

class JSTD_DLL LibcRandom {
public:
    typedef std::uint32_t   value_type;
    typedef std::size_t     size_type;
    typedef LibcRandom      this_type;

    explicit LibcRandom(std::uint32_t initSeed = 0) {
        this->srand(initSeed);
    }

    ~LibcRandom() {}

    value_type rand_max() const {
        return static_cast<value_type>(LibcRand::rand_max());
    }

    void srand(value_type initSeed = 0) {
        LibcRand::srand(initSeed);
    }

    value_type rand() {
        return static_cast<value_type>(LibcRand::rand());
    }

    std::int32_t nextInt32() {
        return static_cast<std::int32_t>(this->nextUInt32());
    }

    std::uint32_t nextUInt32() {
        return static_cast<std::uint32_t>(LibcRand::rand32());
    }

    std::int64_t nextInt64() {
        return static_cast<std::int64_t>(this->nextUInt64());
    }

    std::uint64_t nextUInt64() {
        return static_cast<std::uint64_t>(LibcRand::rand64());
    }

    std::intptr_t nextInt() {
        if (sizeof(std::intptr_t) == 4)
            return static_cast<std::intptr_t>(this->nextInt32());
        else
            return static_cast<std::intptr_t>(this->nextInt64());
    }

    std::size_t nextUInt() {
        if (sizeof(std::size_t) == 4)
            return static_cast<std::size_t>(this->nextUInt32());
        else
            return static_cast<std::size_t>(this->nextUInt64());
    }
};

} // namespace jstd

#endif // JSTD_SYSTEM_LIBCRANDOM_H
