
#ifndef JSTD_SYSTEM_RANDOMGEN_H
#define JSTD_SYSTEM_RANDOMGEN_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <stdlib.h>     // For ::srand(), ::rand()
#include <time.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <cstdlib>      // For std::srand(), std::rand()

#include "jstd/system/LibcRandom.h"
#include "jstd/system/MT19937_32.h"
#include "jstd/system/MT19937_64.h"

namespace jstd {

template <typename RandomAlgorithm>
class BasicRandomGenerator {
public:
    typedef BasicRandomGenerator<RandomAlgorithm>   this_type;
    typedef RandomAlgorithm                         random_algorithm_t;
    typedef typename RandomAlgorithm::value_type    value_type;
    typedef typename RandomAlgorithm::size_type     size_type;

public:
    explicit BasicRandomGenerator(value_type initSeed = 0) {
        this_type::srand(initSeed);
    }

    ~BasicRandomGenerator() {}

    static random_algorithm_t & getInstance() {
        static random_algorithm_t random_;
        return random_;
    }

    static value_type rand_max() {
        return this_type::getInstance::rand_max();
    }

    static void srand(value_type initSeed = 0) {
        this_type::getInstance::srand(initSeed);
    }

    static value_type rand() {
        return this_type::getInstance::rand();
    }

    static std::int32_t nextInt32() {
        return this_type::getInstance::nextInt32();
    }

    static std::uint32_t nextUInt32() {
        return this_type::getInstance::nextUInt32();
    }

    static std::int64_t nextInt64() {
        return this_type::getInstance::nextInt64();
    }

    static std::uint64_t nextUInt64() {
        return this_type::getInstance::nextUInt64();
    }

    static std::size_t nextInt() {
        return this_type::getInstance::nextInt();
    }

    static std::size_t nextUInt() {
        return this_type::getInstance::nextUInt();
    }

    static std::int32_t nextInt32(std::int32_t minValue, std::int32_t maxValue) {
        std::int32_t result = this_type::nextInteger<std::int32_t, std::uint32_t>(minValue, maxValue);
        return result;
    }

    static std::int32_t nextInt32(std::int32_t maxValue) {
        return this_type::nextInt32(0, maxValue);
    }

    static std::uint32_t nextUInt32(std::uint32_t minValue, std::uint32_t maxValue) {
        std::uint32_t result = this_type::nextInteger<std::uint32_t>(minValue, maxValue);
        return result;
    }

    static std::uint32_t nextUInt32(std::uint32_t maxValue) {
        return this_type::nextUInt32(0, maxValue);
    }

    static std::int64_t nextInt64(std::int64_t minValue, std::int64_t maxValue) {
        std::int64_t result = this_type::nextInteger<std::int64_t, std::uint64_t>(minValue, maxValue);
        return result;
    }

    static std::int64_t nextInt64(std::int64_t maxValue) {
        return this_type::nextInt64(0, maxValue);
    }

    static std::uint64_t nextUInt64(std::uint64_t minValue, std::uint64_t maxValue) {
        std::uint64_t result = this_type::nextInteger<std::uint64_t>(minValue, maxValue);
        return result;
    }

    static std::uint64_t nextUInt64(std::uint64_t maxValue) {
        return this_type::nextUInt64(0, maxValue);
    }

    static std::intptr_t nextInt(std::intptr_t minValue, std::intptr_t maxValue) {
        std::intptr_t result = this_type::nextInteger<std::intptr_t, std::size_t>(minValue, maxValue);
        return result;
    }

    static std::intptr_t nextInt(std::intptr_t maxValue) {
        return this_type::nextUInt(0, maxValue);
    }

    static std::size_t nextUInt(std::size_t minValue, std::size_t maxValue) {
        std::size_t result = this_type::nextInteger<std::size_t>(minValue, maxValue);
        return result;
    }

    static std::size_t nextUInt(std::size_t maxValue) {
        return this_type::nextUInt(0, maxValue);
    }

    static float nextFloat() {
        return ((float)this_type::nextUInt32() / 0xFFFFFFFFUL);
    }

    static double nextDouble() {
        return ((double)this_type::nextUInt32() / 0xFFFFFFFFUL);
    }

    // Generates a random number on [0, 1) with 53-bit resolution.
    static double nextDouble53() {
        value_type a = this_type::nextUInt32() >> 5, b = this_type::nextUInt32() >> 6;
        return (((double)a * 67108864.0 + b) / 9007199254740992.0);
    }

    static float nextFloat(float minValue, float maxValue) {
        float result;
        if (minValue < maxValue) {
            result = minValue + (this_type::nextFloat() * (maxValue - minValue));
        }
        else if (minValue > maxValue) {
            result = maxValue + (this_type::nextFloat() * (minValue - maxValue));
        }
        else {
            result = minValue;
        }
        return result;
    }

    static double nextDouble(double minValue, double maxValue) {
        double result;
        if (minValue < maxValue) {
            result = minValue + (this_type::nextDouble() * (maxValue - minValue));
        }
        else if (minValue > maxValue) {
            result = maxValue + (this_type::nextDouble() * (minValue - maxValue));
        }
        else {
            result = minValue;
        }
        return result;
    }

    static double nextDouble53(double minValue, double maxValue) {
        double result;
        if (minValue < maxValue) {
            result = minValue + (this_type::nextDouble53() * (maxValue - minValue));
        }
        else if (minValue > maxValue) {
            result = maxValue + (this_type::nextDouble53() * (minValue - maxValue));
        }
        else {
            result = minValue;
        }
        return result;
    }

private:
    template <typename ValueType>
    static ValueType nextInteger() {
        if (sizeof(ValueType) == 4)
            return static_cast<ValueType>(this_type::nextInt32());
        else
            return static_cast<ValueType>(this_type::nextInt64());
    }

    template <typename ValueType, typename UnsignedType = ValueType>
    static ValueType nextInteger(ValueType minValue, ValueType maxValue) {
        ValueType result;
        if (minValue <= maxValue) {
            result = minValue + (this_type::template nextInteger<ValueType>() %
                                 UnsignedType(maxValue - minValue + 1));
        }
        else {
            assert(minValue > maxValue);
            result = maxValue + (this_type::template nextInteger<ValueType>() %
                                 UnsignedType(minValue - maxValue + 1));
        }
        return result;
    }

    template <typename ValueType, typename UnsignedType = ValueType>
    static ValueType nextInteger(ValueType maxValue) {
        return this_type::nextInteger<ValueType, UnsignedType>(0, maxValue);
    }
};

typedef BasicRandomGenerator<LibcRandom>    RandomGen;
typedef BasicRandomGenerator<MT19937_32>    Mt32RandomGen;
typedef BasicRandomGenerator<MT19937_64>    Mt64RandomGen;

#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
    defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM64__) || defined(__arm64__)
typedef BasicRandomGenerator<MT19937_64>    MtRandomGen;
#else
typedef BasicRandomGenerator<MT19937_32>    MtRandomGen;
#endif

} // namespace jstd

#endif // JSTD_SYSTEM_RANDOMGEN_H
