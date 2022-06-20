
set(POWER2_TEST_SOURCE_FILES
    ../test/power2_test.cpp
)

## power2_test
add_executable(power2_test ${POWER2_TEST_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(power2_test
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated -faligned-new
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(power2_test PUBLIC /W3 /WX)
endif()

target_link_libraries(power2_test
PUBLIC
    ${EXTRA_LIBS}
)

target_include_directories(power2_test
PUBLIC
    "${PROJECT_BINARY_DIR}/test"
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
)
