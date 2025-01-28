/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2024-2025 XiongHui Guo (gz_shines at msn.com)

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

#ifndef JSTD_HASHMAP_GROUP_QUADRATIC_PROBER_HPP
#define JSTD_HASHMAP_GROUP_QUADRATIC_PROBER_HPP

#pragma once

#include <type_traits>
#include <utility>      // For std::pair<F, S>
#include <memory>       // For std::allocator<T>

namespace jstd {

/*
 * quadratic probing:
 *
 *   eg. 0  0+1 1+2 3+4 7+5 12+6 18+7  ...
 * index 0,  1,  3,  7,  12,  18,  25  ...
 *
 */
class group_quadratic_prober {
public:
    group_quadratic_prober(std::size_t index) : index_(index), step_(0) {}

    inline std::size_t get() const noexcept {
        return index_;
    }

    inline std::size_t steps() const noexcept {
        return step_;
    }

    inline std::size_t length() const noexcept {
        return (step_ + 1);
    }

    /*
     * next_bucket() returns false when the whole array has been traversed, which ends
     * probing (in practice, full-table probing will only happen with very small
     * arrays).
     */
    inline bool next_bucket(std::size_t bucket_mask) noexcept {
        step_ += 1;
        index_ = (index_ + step_) & bucket_mask;
        return (step_ <= bucket_mask);
    }

private:
    std::size_t index_;
    std::size_t step_;
};

} // namespace jstd

#endif // JSTD_HASHMAP_GROUP_QUADRATIC_PROBER_HPP
