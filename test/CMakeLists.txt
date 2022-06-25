
##
## power2_test
##
set(POWER2_TEST_SOURCE_FILES
    ../test/power2_test.cpp
)

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

##
## class_trivial
##
set(CLASS_TRIVIAL_SOURCE_FILES
    ../test/class_trivial.cpp
)

add_executable(class_trivial ${CLASS_TRIVIAL_SOURCE_FILES})

if (NOT MSVC)
    # For gcc or clang warning setting
    target_compile_options(class_trivial
        PUBLIC
            -Wall -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-variable -Wno-deprecated -faligned-new
    )
else()
    # Warning level 3 and all warnings as errors
    target_compile_options(class_trivial PUBLIC /W3 /WX)
endif()

target_link_libraries(class_trivial
PUBLIC
    ${EXTRA_LIBS}
)

target_include_directories(class_trivial
PUBLIC
    "${PROJECT_BINARY_DIR}/test"
    "${PROJECT_BINARY_DIR}"
    ${EXTRA_INCLUDES}
)
