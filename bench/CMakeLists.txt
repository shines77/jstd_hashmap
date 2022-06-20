
##
## benchmark
##
set(BENCHMARK_SOURCE_FILES
    ../bench/benchmark/benchmark.cpp
)

add_executable(benchmark ${BENCHMARK_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(benchmark
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated -faligned-new
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(benchmark PUBLIC /W3 /WX)
endif()

target_link_libraries(benchmark
PUBLIC
    ${EXTRA_LIBS}
)

target_include_directories(benchmark
PUBLIC
    "${PROJECT_BINARY_DIR}/bench/benchmark"
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
)

##
## time_hash_map
##
set(TIME_HASH_MAP_SOURCE_FILES
    ../bench/time_hash_map/time_hash_map.cpp
)

add_executable(time_hash_map ${TIME_HASH_MAP_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(time_hash_map
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated -faligned-new
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(time_hash_map PUBLIC /W3 /WX)
endif()

target_link_libraries(time_hash_map
PUBLIC
    ${EXTRA_LIBS}
)

target_include_directories(time_hash_map
PUBLIC
    "${PROJECT_BINARY_DIR}/bench/time_hash_map"
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
)

##
## cardinal_bench
##
set(CARDINAL_BENCH_SOURCE_FILES
    ../bench/cardinal_bench/cardinal_bench.cpp
)

add_executable(cardinal_bench ${CARDINAL_BENCH_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(cardinal_bench
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated -faligned-new
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(cardinal_bench PUBLIC /W3 /WX)
endif()

target_link_libraries(cardinal_bench
PUBLIC
    ${EXTRA_LIBS}
)

target_include_directories(cardinal_bench
PUBLIC
    "${PROJECT_BINARY_DIR}/bench/cardinal_bench"
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
)
