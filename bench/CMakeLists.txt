
##
## benchmark
##
set(BENCHMARK_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/benchmark/benchmark.cpp
)

add_executable(benchmark ${BENCHMARK_SOURCE_FILES})

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

target_link_libraries(benchmark
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(benchmark
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/benchmark"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

##
## time_hash_map
##
set(TIME_HASH_MAP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/time_hash_map/time_hash_map.cpp
)

add_executable(time_hash_map ${TIME_HASH_MAP_SOURCE_FILES})

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

target_link_libraries(time_hash_map
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(time_hash_map
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

##
## time_hash_map_new
##
set(TIME_HASH_MAP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/time_hash_map_new/time_hash_map_new.cpp
)

add_executable(time_hash_map_new ${TIME_HASH_MAP_SOURCE_FILES})

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

target_link_libraries(time_hash_map_new
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(time_hash_map_new
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/time_hash_map_new"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

##
## cardinal_bench
##
set(CARDINAL_BENCH_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cardinal_bench/cardinal_bench.cpp
)

add_executable(cardinal_bench ${CARDINAL_BENCH_SOURCE_FILES})

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

target_link_libraries(cardinal_bench
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(cardinal_bench
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/cardinal_bench"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)
