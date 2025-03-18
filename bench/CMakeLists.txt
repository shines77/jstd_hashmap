
##
## benchmark
##
target_include_directories(benchmark
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/benchmark"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

set(BENCHMARK_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/benchmark/benchmark.cpp"
)

add_executable(benchmark ${BENCHMARK_SOURCE_FILES})

target_link_libraries(benchmark
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(benchmark
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(benchmark PUBLIC /W3 /WX)
endif()

##
## time_hash_map
##
target_include_directories(time_hash_map
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

set(TIME_HASH_MAP_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map/time_hash_map.cpp"
)

add_executable(time_hash_map ${TIME_HASH_MAP_SOURCE_FILES})

target_link_libraries(time_hash_map
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(time_hash_map
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(time_hash_map PUBLIC /W3 /WX)
endif()

##
## time_hash_map_new
##
target_include_directories(time_hash_map_new
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map_new"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

set(TIME_HASH_MAP_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map_new/time_hash_map_new.cpp"
)

add_executable(time_hash_map_new ${TIME_HASH_MAP_SOURCE_FILES})

target_link_libraries(time_hash_map_new
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(time_hash_map_new
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(time_hash_map_new PUBLIC /W3 /WX)
endif()

##
## cardinal_bench
##
target_include_directories(cardinal_bench
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/cardinal_bench"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

set(CARDINAL_BENCH_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/cardinal_bench/cardinal_bench.cpp"
)

add_executable(cardinal_bench ${CARDINAL_BENCH_SOURCE_FILES})

target_link_libraries(cardinal_bench
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(cardinal_bench
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(cardinal_bench PUBLIC /W3 /WX)
endif()

##
## jackson_bench
##
target_include_directories(jackson_bench
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/jackson_bench"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

set(JACKSON_BENCH_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/jackson_bench/jackson_bench.cpp"
)

add_executable(jackson_bench ${JACKSON_BENCH_SOURCE_FILES})

target_link_libraries(jackson_bench
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(jackson_bench
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(jackson_bench PUBLIC /W3 /WX)
endif()
