//
// Copyright (c) 2024 Jackson L. Allan.
// Distributed under the MIT License (see the accompanying LICENSE file).
//

#include "jstd/basic/stddef.h"

#include <stdlib.h>
#include <stdio.h>

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <array>
#include <random>
#include <chrono>
#include <thread>
#include <type_traits>
#include <algorithm>
#if JSTD_IS_CXX_20
#include <concepts>
#endif
#include "bench_config.h"

//
// Variable printed before the program closes to prevent compiler from optimizing out function calls during the
// benchmarks.
// This approach proved to be more reliable than local volatile variables.
//
std::size_t do_not_optimize = 0;

// Standard stringification macro, used to form the file paths of the blueprints and shims.
#define STRINGIFY_(x)   #x
#define STRINGIFY(x)    STRINGIFY_(x)

// Concept to check that a blueprint is correctly formed.
#if JSTD_IS_CXX_20
template <typename BluePrint>
concept check_blueprint =
    std::is_object<typename BluePrint::value_type>::value &&
    std::same_as<decltype(BluePrint::label), const char * const> &&
    std::same_as<decltype(BluePrint::hash_key), uint64_t(const typename BluePrint::key_type &)> &&
    std::same_as<
        decltype(BluePrint::cmpr_keys),
        bool(const typename BluePrint::key_type &, const typename BluePrint::key_type &)
    > &&
    std::same_as<decltype(BluePrint::fill_unique_keys), void(std::vector<typename BluePrint::key_type> &)>
;
#define STATIC_ASSERT(expr) static_assert(expr)
#else
#define STATIC_ASSERT(expr)
#endif // JSTD_IS_CXX_20

// #include blueprints and check them for correctness.
#ifdef BLUEPRINT_1
#include STRINGIFY(blueprints/BLUEPRINT_1/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_1>);
#endif
#ifdef BLUEPRINT_2
#include STRINGIFY(blueprints/BLUEPRINT_2/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_2>);
#endif
#ifdef BLUEPRINT_3
#include STRINGIFY(blueprints/BLUEPRINT_3/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_3>);
#endif
#ifdef BLUEPRINT_4
#include STRINGIFY(blueprints/BLUEPRINT_4/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_4>);
#endif
#ifdef BLUEPRINT_5
#include STRINGIFY(blueprints/BLUEPRINT_5/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_5>);
#endif
#ifdef BLUEPRINT_6
#include STRINGIFY(blueprints/BLUEPRINT_6/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_6>);
#endif
#ifdef BLUEPRINT_7
#include STRINGIFY(blueprints/BLUEPRINT_7/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_7>);
#endif
#ifdef BLUEPRINT_8
#include STRINGIFY(blueprints/BLUEPRINT_8/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_8>);
#endif
#ifdef BLUEPRINT_9
#include STRINGIFY(blueprints/BLUEPRINT_9/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_9>);
#endif
#ifdef BLUEPRINT_10
#include STRINGIFY(blueprints/BLUEPRINT_10/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_10>);
#endif
#ifdef BLUEPRINT_11
#include STRINGIFY(blueprints/BLUEPRINT_11/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_11>);
#endif
#ifdef BLUEPRINT_12
#include STRINGIFY(blueprints/BLUEPRINT_12/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_12>);
#endif
#ifdef BLUEPRINT_13
#include STRINGIFY(blueprints/BLUEPRINT_13/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_13>);
#endif
#ifdef BLUEPRINT_14
#include STRINGIFY(blueprints/BLUEPRINT_14/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_14>);
#endif
#ifdef BLUEPRINT_15
#include STRINGIFY(blueprints/BLUEPRINT_15/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_15>);
#endif
#ifdef BLUEPRINT_16
#include STRINGIFY(blueprints/BLUEPRINT_16/blueprint.h)
STATIC_ASSERT(check_blueprint<BLUEPRINT_16>);
#endif

// #include hashmaps and check them for correctness.

#ifdef HASHMAP_1
#include STRINGIFY(hashmaps/HASHMAP_1/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_2
#include STRINGIFY(hashmaps/HASHMAP_2/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_3
#include STRINGIFY(hashmaps/HASHMAP_3/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_4
#include STRINGIFY(hashmaps/HASHMAP_4/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_5
#include STRINGIFY(hashmaps/HASHMAP_5/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_6
#include STRINGIFY(hashmaps/HASHMAP_6/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_7
#include STRINGIFY(hashmaps/HASHMAP_7/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_8
#include STRINGIFY(hashmaps/HASHMAP_8/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_9
#include STRINGIFY(hashmaps/HASHMAP_9/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_10
#include STRINGIFY(hashmaps/HASHMAP_10/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_11
#include STRINGIFY(hashmaps/HASHMAP_11/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_12
#include STRINGIFY(hashmaps/HASHMAP_12/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_13
#include STRINGIFY(hashmaps/HASHMAP_13/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_14
#include STRINGIFY(hashmaps/HASHMAP_14/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_15
#include STRINGIFY(hashmaps/HASHMAP_15/hashmap_wrapper.h)
#endif
#ifdef HASHMAP_16
#include STRINGIFY(hashmaps/HASHMAP_16/hashmap_wrapper.h)
#endif

// Benchmark ids.
enum benchmark_ids
{
    id_find_existing,
    id_find_non_existing,
    id_insert_existing,
    id_insert_non_existing,
    id_replace_existing,
    id_erase_existing,
    id_erase_non_existing,
    id_iteration
};

// Benchmark names used in the graphs.
const char * benchmark_names[] = {
    "Total time to look up 1,000 existing keys with N keys in the table",
    "Total time to look up 1,000 non-existing keys with N keys in the table",
    "Total time to insert N existing keys",
    "Total time to insert N non-existing keys",
    "Total time to replace 1,000 existing keys with N keys in the table",
    "Total time to erase 1,000 existing keys with N keys in the table",
    "Total time to erase 1,000 non-existing keys with N keys in the table",
    "Total time to iterate over 5,000 keys with N keys in the table"
};

// Benchmark names used in the heatmap.
const char * benchmark_short_names[] = {
    "Look up existing",
    "Look up non-existing",
    "Insert non-existing",
    "Replace existing",
    "Erase existing",
    "Erase non-existing",
    "Iterate"
};

// Random number generator.
std::default_random_engine random_number_generator(20250118U);

// Function for providing unique keys for a given blueprint in random order.
// Besides the KEY_COUNT keys to be inserted, it also provides an extra KEY_COUNT / KEY_COUNT_MEASUREMENT_INTERVAL *
// 1000 keys for testing failed look-ups.
template <typename BluePrint>
const typename BluePrint::key_type & shuffled_unique_key(std::size_t index)
{
    static auto keys = []() {
        std::vector<typename BluePrint::key_type> keys(KEY_COUNT + KEY_COUNT / KEY_COUNT_MEASUREMENT_INTERVAL * 1000);
        BluePrint::fill_unique_keys(keys);
        std::shuffle(keys.begin(), keys.end(), random_number_generator);
        return keys;
    }();

    return keys[index];
}

// Function for recording and accessing a single result, i.e. one measurement for a particular table, blueprint, and
// benchmark during a particular run.
template <template<typename> typename shim, typename BluePrint, benchmark_ids benchmark_id>
std::uint64_t & results(std::size_t run_index, std::size_t result_index)
{
    static auto results = std::vector<std::uint64_t>(RUN_COUNT * (KEY_COUNT / KEY_COUNT_MEASUREMENT_INTERVAL));
    return results[run_index * (KEY_COUNT / KEY_COUNT_MEASUREMENT_INTERVAL) + result_index];
}

// Function that attempts to reset the cache to the same state before each table is benchmarked.
// The strategy is to iterate over an array that is at least as large as the L1, L2, and L3 caches combined.
void flush_cache()
{
    static auto buffer = std::vector<std::uint64_t>(APPROXIMATE_CACHE_SIZE / sizeof(std::uint64_t) + 1);

    std::size_t do_not_optimize = 0;
    for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
        do_not_optimize += *iter;
    }

    ::srand(static_cast<unsigned int>(do_not_optimize));
}

void benchmark_find_existing()
{
    //
}

void benchmark_find_non_existing()
{
    //
}

void benchmark_insert_existing()
{
    //
}

void benchmark_insert_non_existing()
{
    //
}

void benchmark_replace_existing()
{
    //
}

void benchmark_erase_existing()
{
    //
}

void benchmark_erase_non_existing()
{
    //
}

void benchmark_iteration()
{
    //
}

template <typename BluePrint, template <typename> typename HashMap, std::size_t BenchmardId>
void run_benchmark(std::size_t run)
{
    std::cout << "Run " << run << '\n';
    std::cout << HashMap<void>::label << ": " << BluePrint::label << "\n";


}

template <typename BluePrint, template <typename> typename HashMap>
void run_benchmarks()
{
#ifdef BENCHMARK_FIND_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_find_existing>(run);
    }
#endif

#ifdef BENCHMARK_FIND_NON_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_find_non_existing>(run);
    }
#endif

#ifdef BENCHMARK_INSERT_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_insert_existing>(run);
    }
#endif

#ifdef BENCHMARK_INSERT_NON_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_insert_non_existing>(run);
    }
#endif

#ifdef BENCHMARK_REPLACE_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_replace_existing>(run);
    }
#endif

#ifdef BENCHMARK_ERASE_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_erase_existing>(run);
    }
#endif

#ifdef BENCHMARK_ERASE_NON_EXISTING
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_erase_non_existing>(run);
    }
#endif

#ifdef BENCHMARK_ITERATION
    for (std::size_t run = 0; run < RUN_COUNT; ++run) {
        run_benchmark<BluePrint, HashMap, id_iteration>(run);
    }
#endif
}

// Function for benchmarking a hashmap against all blueprints.
template <typename BluePrint>
void run_blueprint_benchmarks()
{
#ifdef HASHMAP_1
    run_benchmarks<BluePrint, HASHMAP_1>();
#endif
#ifdef HASHMAP_2
    run_benchmarks<BluePrint, HASHMAP_2>();
#endif
#ifdef HASHMAP_3
    run_benchmarks<BluePrint, HASHMAP_3>();
#endif
#ifdef HASHMAP_4
    run_benchmarks<BluePrint, HASHMAP_4>();
#endif
#ifdef HASHMAP_5
    run_benchmarks<BluePrint, HASHMAP_5>();
#endif
#ifdef HASHMAP_6
    run_benchmarks<BluePrint, HASHMAP_6>();
#endif
#ifdef HASHMAP_7
    run_benchmarks<BluePrint, HASHMAP_7>();
#endif
#ifdef HASHMAP_8
    run_benchmarks<BluePrint, HASHMAP_8>();
#endif
#ifdef HASHMAP_9
    run_benchmarks<BluePrint, HASHMAP_9>();
#endif
#ifdef HASHMAP_10
    run_benchmarks<BluePrint, HASHMAP_10>();
#endif
#ifdef HASHMAP_11
    run_benchmarks<BluePrint, HASHMAP_11>();
#endif
#ifdef HASHMAP_12
    run_benchmarks<BluePrint, HASHMAP_12>();
#endif
#ifdef HASHMAP_13
    run_benchmarks<BluePrint, HASHMAP_13>();
#endif
#ifdef HASHMAP_14
    run_benchmarks<BluePrint, HASHMAP_14>();
#endif
#ifdef HASHMAP_15
    run_benchmarks<BluePrint, HASHMAP_15>();
#endif
#ifdef HASHMAP_16
    run_benchmarks<BluePrint, HASHMAP_16>();
#endif
}

int main(int argc, char * argv[])
{
    // Get UTC time string with colons replaced by underscores.
    auto utc_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream ss;
    ss << std::put_time(::gmtime(&utc_time), "%Y-%m-%dT%H:%M:%S");
    auto time_str = ss.str();
    std::replace(time_str.begin(), time_str.end(), ':', '_');

#ifdef BLUEPRINT_1
    run_blueprint_benchmarks<BLUEPRINT_1>();
#endif
#ifdef BLUEPRINT_2
    run_blueprint_benchmarks<BLUEPRINT_2>();
#endif
#ifdef BLUEPRINT_3
    run_blueprint_benchmarks<BLUEPRINT_3>();
#endif
#ifdef BLUEPRINT_4
    run_blueprint_benchmarks<BLUEPRINT_4>();
#endif
#ifdef BLUEPRINT_5
    run_blueprint_benchmarks<BLUEPRINT_5>();
#endif
#ifdef BLUEPRINT_6
    run_blueprint_benchmarks<BLUEPRINT_6>();
#endif
#ifdef BLUEPRINT_7
    run_blueprint_benchmarks<BLUEPRINT_7>();
#endif
#ifdef BLUEPRINT_8
    run_blueprint_benchmarks<BLUEPRINT_8>();
#endif
#ifdef BLUEPRINT_9
    run_blueprint_benchmarks<BLUEPRINT_9>();
#endif
#ifdef BLUEPRINT_10
    run_blueprint_benchmarks<BLUEPRINT_10>();
#endif
#ifdef BLUEPRINT_11
    run_blueprint_benchmarks<BLUEPRINT_11>();
#endif
#ifdef BLUEPRINT_12
    run_blueprint_benchmarks<BLUEPRINT_12>();
#endif
#ifdef BLUEPRINT_13
    run_blueprint_benchmarks<BLUEPRINT_13>();
#endif
#ifdef BLUEPRINT_14
    run_blueprint_benchmarks<BLUEPRINT_14>();
#endif
#ifdef BLUEPRINT_15
    run_blueprint_benchmarks<BLUEPRINT_15>();
#endif
#ifdef BLUEPRINT_16
    run_blueprint_benchmarks<BLUEPRINT_16>();
#endif

    std::cout << "Outputting results\n";

    //html_out(time_str);

    //csv_out(time_str);

    std::cout << "Optimization preventer: " << do_not_optimize << "\n";
    std::cout << "Done\n";
}
