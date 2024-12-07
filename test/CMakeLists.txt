
##
## power2_test
##
set(POWER2_TEST_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/power2_test.cpp
)

add_executable(power2_test ${POWER2_TEST_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(power2_test
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(power2_test PUBLIC /W3 /WX)
endif()

target_link_libraries(power2_test
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(power2_test
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

##
## class_trivial
##
set(CLASS_TRIVIAL_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/class_trivial.cpp
)

add_executable(class_trivial ${CLASS_TRIVIAL_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(class_trivial
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(class_trivial PUBLIC /W3 /WX)
endif()

target_link_libraries(class_trivial
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(class_trivial
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)

##
## feature_test
##
set(FEATURE_TEST_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/feature_test.cpp
)

add_executable(feature_test ${FEATURE_TEST_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(feature_test
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(feature_test PUBLIC /W3 /WX)
endif()

target_link_libraries(feature_test
PUBLIC
    ${EXTRA_LIBS}
    ${JSTD_HASHMAP_LIBNAME}
)

target_include_directories(feature_test
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    ${EXTRA_INCLUDES}
)
