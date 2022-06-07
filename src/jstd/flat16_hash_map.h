/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2022 XiongHui Guo (gz_shines@msn.com)

  https://github.com/shines77/jstd_hash_map
  https://gitee.com/shines77/jstd_hash_map

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

#pragma once

#include <memory.h>
#include <math.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::ptrdiff_t, std::size_t
#include <cstdbool>
#include <cassert>
#include <memory>       // For std::swap(), std::pointer_traits<T>
#include <limits>       // For std::numeric_limits<T>
#include <cstring>      // For std::memset()
#include <vector>
#include <type_traits>
#include <utility>
#include <algorithm>    // For std::max(), std::min()

#include <nmmintrin.h>
#include <immintrin.h>

#define FLAT16_DEFAULT_LOAD_FACTOR  (0.5)

namespace jstd {

static inline
constexpr
std::size_t round_size(std::size_t size, std::size_t alignment)
{
    return (size + alignment - 1) / alignment * alignment;
}

template < typename Key, typename Value,
           typename Hasher = std::hash<Key>,
           typename KeyEqual = std::equal_to<Key>,
           typename Allocator = std::allocator<std::pair<const Key, Value>> >
class flat16_hash_map {
public:
    typedef Key                             key_type;
    typedef Value                           mapped_type;
    typedef std::pair<const Key, Value>     value_type;
    typedef std::pair<Key, Value>           n_value_type;

    typedef Hasher                          hasher_type;
    typedef KeyEqual                        key_equal;
    typedef Allocator                       allocator_type;
    typedef typename Hasher::result_type    hash_result_t;

    typedef std::size_t                     size_type;
    typedef typename std::make_signed<size_type>::type
                                            ssize_type;
    typedef std::size_t                     index_type;
    typedef std::size_t                     hash_code_t;
    typedef flat16_hash_map<Key, Value, Hasher, KeyEqual, Allocator>
                                            this_type;

    struct hash_entry {
        value_type value;
    };

    static constexpr std::uint8_t kEmptyEntry   = 0b11111111;
    static constexpr std::uint8_t kDeletedEntry = 0b10000000;
    static constexpr std::uint8_t kFullMask     = 0b10000000;
    static constexpr std::uint8_t kHash2Mask    = 0b01111111;
    static constexpr size_type kControlHashMask = 0x0000007Ful;

    static constexpr size_type kClusterEntries  = 16;
    static constexpr size_type kDefaultInitialCapacity = kClusterEntries;

    struct alignas(16) cluster {
        std::uint8_t controls[kClusterEntries];
        hash_entry   entries[kClusterEntries];

        cluster() {
#if defined(__SSE2__)
            __m128i empty_bits = _mm_set1_epi8(kEmptyEntry);
            _mm_store_si128((__m128i *)&controls[0], empty_bits);
#else
            std::memset((void *)&controls[0], kEmptyEntry, kClusterEntries * sizeof(std::uint8_t));
#endif
        }

        ~cluster() {
            //
        }
    };

private:
    cluster *   clusters_;
    size_type   cluster_count_;

    size_type   entry_size_;
    size_type   entry_capacity_;

    size_type   entry_limit_;
    double      load_factor_;

public:
    flat16_hash_map() :
        clusters_(nullptr), cluster_count_(0),
        entry_size_(0), entry_capacity_(kDefaultInitialCapacity),
        entry_limit_((size_type)(kDefaultInitialCapacity * FLAT16_DEFAULT_LOAD_FACTOR)),
        load_factor_(FLAT16_DEFAULT_LOAD_FACTOR) {
        clusters_ = new cluster[1];
        cluster_count_ = 1;
    }

    explicit flat16_hash_map(size_type initialCapacity) :
        clusters_(nullptr), cluster_count_(0),
        entry_size_(0), entry_capacity_(round_size(initialCapacity, kClusterEntries)),
        entry_limit_((size_type)(round_size(initialCapacity, kClusterEntries) * FLAT16_DEFAULT_LOAD_FACTOR)),
        load_factor_(FLAT16_DEFAULT_LOAD_FACTOR) {
        size_type cluster_count = round_size(initialCapacity, kClusterEntries) / kClusterEntries;
        clusters_ = new cluster[cluster_count];
        cluster_count_ = cluster_count;
    }

    ~flat16_hash_map() {
        if (clusters_ != nullptr) {
            delete[] clusters_;
            clusters_ = nullptr;
        }
    }

    size_type size() const {
        return entry_size_;
    }

    size_type capacity() const {
        return entry_capacity_;
    }
};

} // namespace jstd
