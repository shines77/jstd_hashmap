CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

##
## CMake is v3.10.2 under Ubuntu 18.04.2, is v3.16.3 under Ubuntu 20.04.
##

set(JSTD_MAJOR_VERSION "1")
set(JSTD_MINOR_VERSION "0")
set(JSTD_PATCH_VERSION "2")
set(JSTD_VERSION_STRING "${JSTD_MAJOR_VERSION}.${JSTD_MINOR_VERSION}.${JSTD_PATCH_VERSION}")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.0")
    # Fixed CMP0048: https://github.com/Tencent/rapidjson/issues/1154
    cmake_policy(SET CMP0048 NEW)
    project(jstd_hashmap VERSION "${JSTD_VERSION_STRING}")
else()
    project(jstd_hashmap)
    set(JSTD_VERSION_MAJOR "${JSTD_MAJOR_VERSION}")
    set(JSTD_VERSION_MINOR "${JSTD_MINOR_VERSION}")
    set(JSTD_VERSION_PATCH "${JSTD_PATCH_VERSION}")
    set(JSTD_VERSION       "${JSTD_VERSION_STRING}")
endif()

message("")
message(STATUS "-----------------------------------------------")
message(STATUS "  project: ${PROJECT_NAME}, version: ${JSTD_VERSION_STRING}")
message(STATUS "-----------------------------------------------")
message("")

message(STATUS "------------------ Build Env -------------------")
message(STATUS "CMAKE_SOURCE_DIR   : ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR   : ${CMAKE_BINARY_DIR}")
message(STATUS "PROJECT_SOURCE_DIR : ${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR : ${PROJECT_BINARY_DIR}")
message(STATUS "------------------------------------------------")
message("")

message(STATUS "------------------ Current Build Env -------------------")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR   : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR   : ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_LIST_DIR     : ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "--------------------------------------------------------")
message("")

if (UNIX)
    # Adhere to GNU filesystem layout conventions
    include(GNUInstallDirs)
endif()

include(CMakePackageConfigHelpers)

configure_file("${PROJECT_SOURCE_DIR}/src/jstd/version.h.in" "${PROJECT_SOURCE_DIR}/src/jstd/config/version.h")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

##
## See: https://blog.csdn.net/u012487272/article/details/12882283
##

# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

## Configuration value are DEBUG, RELEASE, MINSIZEREL and RELWITHDEBINFO.

set(CMAKE_RELESE_POSTFIX "")
set(CMAKE_DEBUG_POSTFIX  "d")

message(STATUS "--------------- Build Output ------------------")
message(STATUS "EXECUTABLE_OUTPUT_PATH : ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "LIBRARY_OUTPUT_PATH    : ${LIBRARY_OUTPUT_PATH}")
message(STATUS "-----------------------------------------------")
message(STATUS "CMAKE_EXECUTABLE_OUTPUT_DIRECTORY : ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY    : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY    : ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY    : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "-----------------------------------------------")
message("")

set(CMAKE_VERBOSE_MAKEFILE ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include("${PROJECT_SOURCE_DIR}/cmake/utils.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/system.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/DetectCPUArchitectures.cmake")

## include(CMakeASM_NASMInformation)
include(DetectCPUArchitectures)

if (0)

enable_language(ASM_NASM)

if (CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CAN_USE_ASSEMBLER TRUE)
endif()

if (CMAKE_CL_64)
    set(CMAKE_CL_ARCH x64)
else()
    set(CMAKE_CL_ARCH x86)
endif()

endif()

##
## See: https://stackoverflow.com/questions/39258250/how-to-detect-if-64-bit-msvc-with-cmake
##
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(CMAKE_PLATFORM_ARCH Arch64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(CMAKE_PLATFORM_ARCH Arch32)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(JSTD_CMAKE_SHOW_DETAIL ON)

include(CMakeUtils)
include(GetOSEnv)
include(CheckComplierEnv)
include(GetCompilerToolset)
include(DetectCXXCompiler)

GetOSEnv()
CheckCompilerEnvironment()
GetCompilerToolset(CMAKE_C_TOOLSET   "c")
GetCompilerToolset(CMAKE_CXX_TOOLSET "cxx")

set(ipo_supported NO)
set(CMakeIPOEnable FALSE)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.9")
    ##
    ## Supported INTERPROCEDURAL_OPTIMIZATION since CMake 3.9, support for Visual Studio generators since CMake 3.13.
    ##
    cmake_policy(SET CMP0069 NEW)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        cmake_policy(SET CMP0138 NEW)
    endif()

    ##
    ## https://stackoverflow.com/questions/31355692/how-do-i-enable-link-time-optimization-lto-with-cmake
    ##
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)

    if (ipo_supported)
        ## set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        set(CMakeIPOEnable FALSE)
    else()
        message(STATUS "IPO/LTO is not supported: <${error}>")
    endif()
else()
    message(STATUS "CMake version is < 3.9, is not supported IPO/LTO.")
endif()

message(STATUS "-------------- Compiler Toolset ---------------")
message(STATUS "CMAKE_C_TOOLSET   : ${CMAKE_C_TOOLSET}")
message(STATUS "CMAKE_CXX_TOOLSET : ${CMAKE_CXX_TOOLSET}")
message(STATUS "-----------------------------------------------")
message("")

message(STATUS "--------------- Architectures -----------------")
message(STATUS "CMAKE_BUILD_TYPE         : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CL_ARCH            : ${CMAKE_CL_ARCH}")
message(STATUS "CMAKE_PLATFORM_ARCH      : ${CMAKE_PLATFORM_ARCH}")
message(STATUS "CMAKE_CPU_ARCHITECTURES  : ${CMAKE_CPU_ARCHITECTURES}")
message(STATUS "CMAKE_IPO_SUPPORTED      : ${ipo_supported}")
message(STATUS "-----------------------------------------------")
message("")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -D_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
else()
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

if (NOT MSVC)
    ## For C_FLAGS
    ## -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4a -msse4.1 -msse4.2 -mavx -mavx2 -mavx512vl -mavx512f
    if (CMakeIPOEnable)
        set(CMAKE_C_FLAGS_DEFAULT       "${CMAKE_C_FLAGS} -std=c99 -march=native -mtune=native -finput-charset=utf-8 -flto -ffat-lto-objects -fPIC -fpic")
    else()
        set(CMAKE_C_FLAGS_DEFAULT       "${CMAKE_C_FLAGS} -std=c99 -march=native -mtune=native -finput-charset=utf-8 -fPIC -fpic")
    endif()
    set(CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEFAULT} -g -pg -D_DEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL        "${CMAKE_C_FLAGS_DEFAULT} -Os -DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_DEFAULT} -O2 -g")

    ## For CXX_FLAGS
    ## -Wall -Werror -Wextra -Wno-format -Wno-unused-function
    ## -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4a -msse4.1 -msse4.2 -mavx -mavx2 -mavx512vl -mavx512f
    # -fexec-charset=gbk -finput-charset=gbk
    if (CMakeIPOEnable)
        set(CMAKE_CXX_FLAGS_DEFAULT     "${CMAKE_CXX_FLAGS} -std=c++17 -march=native -mtune=native -finput-charset=utf-8 -flto -ffat-lto-objects -fPIC -fpic")
    else()
        set(CMAKE_CXX_FLAGS_DEFAULT     "${CMAKE_CXX_FLAGS} -std=c++17 -march=native -mtune=native -finput-charset=utf-8 -fPIC -fpic")
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_DEFAULT} -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEFAULT} -g -pg -D_DEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL      "${CMAKE_CXX_FLAGS_DEFAULT} -Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_DEFAULT} -O2 -g")

    set(CMAKE_AR        "/usr/bin/gcc-ar")
    set(CMAKE_LINKER    "/usr/bin/ld")
    set(CMAKE_NM        "/usr/bin/gcc-nm")
    set(CMAKE_OBJDUMP   "/usr/bin/objdump")
    set(CMAKE_RANLIB    "/usr/bin/gcc-ranlib")

    ## -faligned-new: GCC >= 7.1, CLANG >= 4.0
    ## See: https://trac.wildfiregames.com/ticket/5139
    if (CMAKE_COMPILER_IS_CLANGCXX)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.0")
            message("")
            message(STATUS "Clang >= 4.0, Added CXX_FLAGS: -faligned-new")
            message("")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
        endif()
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.1")
            message("")
            message(STATUS "GCC >= 7.1, Added CXX_FLAGS: -faligned-new")
            message("")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
        endif()
    endif()
endif()

if (WIN32)
    ## add_compile_options(-D__SSE3__ -D__SSE4A__ -D__SSE4_1__ -D__SSE4_2__)
    add_compile_options("-D_WIN32_WINNT=0x0601 -D_CRT_SECURE_NO_WARNINGS")
    ## set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32 mswsock)
endif()

include(With_MT_If_MSVC)

if (MSVC AND MSVC_STATIC_CRT)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
    )
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()

if (UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

set(JSTD_HASHMAP_ENABLED ON)

if (JSTD_HASHMAP_ENABLED)

set(JSTD_HASHMAP_LIBNAME jstd_hashmap${SUFFIX64_UNDERSCORE})

# Set default library output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
if (MSVC)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/lib/msvc/Debug)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/lib/msvc/Release)
endif()

# get obj vars into format that add_library likes: $<TARGET_OBJS:objlib>
# (see http://www.cmake.org/cmake/help/v3.0/command/add_library.html)
set(TARGET_OBJS "")
foreach (SUBDIR ${SUBDIRS})
    add_subdirectory(${SUBDIR})
    string(REPLACE "/" "_" subdir_obj ${SUBDIR})
    list(APPEND TARGET_OBJS "$<TARGET_OBJECTS:${subdir_obj}>")
endforeach()

# Only generate .def for dll on MSVC and always produce pdb files for debug and release
if (MSVC)
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 3.4)
        set(JSTD_HASHMAP_DEF_FILE "${PROJECT_BINARY_DIR}/jstd_hashmap.def")
    endif()
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# Handle MSVC exports
if (MSVC AND BUILD_SHARED_LIBS)
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 3.4)
        include("${PROJECT_SOURCE_DIR}/cmake/Export.cmake")
    else()
        # Creates verbose .def file (51KB vs 18KB)
        set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS true)
    endif()
endif()

if (UNIX)
    set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
    if (USE_THREAD)
        # Add threading library to linker
        find_package(Threads)
        if (THREADS_HAVE_PTHREAD_ARG)
            set_property(TARGET ${JSTD_HASHMAP_LIBNAME} PROPERTY COMPILE_OPTIONS "-pthread")
            set_property(TARGET ${JSTD_HASHMAP_LIBNAME} PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
        endif()
    endif()
else()
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

##
## libjstd_hashmap
##
include_directories(include)
include_directories(src)
include_directories(test)

set(JSTD_HASHMAP_SOURCE_FILES
    src/jstd/dll_main.cpp
    src/jstd/basic/assert.c
)

# add_subdirectory(main EXCLUDE_FROM_ALL src/main/asm)

##
## See: https://stackoverflow.com/questions/39258250/how-to-detect-if-64-bit-msvc-with-cmake
##
if (0)
if ("amd64" STREQUAL "${CMAKE_CPU_ARCHITECTURES}" OR
    "x64" STREQUAL "${CMAKE_CPU_ARCHITECTURES}" OR
    "${CMAKE_PLATFORM_ARCH}" STREQUAL "Arch64")
    set(UTF8_ENCODING_SOURCE_FILES ${UTF8_ENCODING_SOURCE_FILES}
        src/jstd/asm/asmlib.h
        src/jstd/asm/InstructionSet_x64.asm
        src/jstd/asm/utf8_decode_sse41_x64.asm
    )
elseif ("x86" STREQUAL "${CMAKE_CPU_ARCHITECTURES}" OR
    "${CMAKE_PLATFORM_ARCH}" STREQUAL "Arch32")
    set(UTF8_ENCODING_SOURCE_FILES ${UTF8_ENCODING_SOURCE_FILES}
        src/jstd/asm/asmlib.h
        src/jstd/asm/InstructionSet_x86.asm
        src/jstd/asm/utf8_decode_sse41_x86.asm
    )
else()
    message("This platform does not support our asm code.")
endif()
endif()

## list(LENGTH ASM_SOURCE_FILES asm_files_length)
## if (${asm_files_length} GREATER 0)
##     ## STATIC, SHARED
##     add_library(asmlib STATIC ${ASM_SOURCE_FILES})
##     set(EXTRA_LIBS ${EXTRA_LIBS} asmlib)
##     set(EXTRA_INCLUDES ${EXTRA_INCLUDES} "${PROJECT_SOURCE_DIR}/src")
## endif()

add_library(${JSTD_HASHMAP_LIBNAME} STATIC ${JSTD_HASHMAP_SOURCE_FILES} ${TARGET_OBJS} ${JSTD_HASHMAP_DEF_FILE})

target_include_directories(${JSTD_HASHMAP_LIBNAME}
INTERFACE
    $<INSTALL_INTERFACE:jstd/all${SUFFIX64}>
)

# set(EXTRA_LIBS ${EXTRA_LIBS} jstd_hashmap)
set(EXTRA_INCLUDES ${EXTRA_INCLUDES} "${PROJECT_SOURCE_DIR}/src")

# Android needs to explicitly link against libm
if (ANDROID)
    target_link_libraries(${JSTD_HASHMAP_LIBNAME} m)
endif()

set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES
    VERSION ${JSTD_MAJOR_VERSION}.${JSTD_MINOR_VERSION}
    SOVERSION ${JSTD_MAJOR_VERSION}
)

# Set output for libjstd_hashmap
set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES LIBRARY_OUTPUT_NAME_DEBUG "${JSTD_HASHMAP_LIBNAME}_debug")
set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES EXPORT_NAME "jstd_hashmap")

foreach (OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG})
    set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG})
    set_target_properties(${JSTD_HASHMAP_LIBNAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG})
endforeach()

if (BUILD_SHARED_LIBS)
    if (NOT MSVC)
        target_link_libraries(${JSTD_HASHMAP_LIBNAME} ${EXTRA_LIBS} "-Wl,-allow-multiple-definition")
    else()
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
        target_link_libraries(${JSTD_HASHMAP_LIBNAME} ${EXTRA_LIBS})
    endif()
else ()
    target_link_libraries(${JSTD_HASHMAP_LIBNAME} ${EXTRA_LIBS})
endif()

if (BUILD_SHARED_LIBS AND NOT ${SYMBOLPREFIX}${SYMBOLSUFFIX} STREQUAL "")
    if (NOT DEFINED ARCH)
        set(ARCH_IN "x86_64")
    else()
        set(ARCH_IN ${ARCH})
    endif()

    if (NOT ${SYMBOLPREFIX} STREQUAL "")
        message(STATUS "adding prefix ${SYMBOLPREFIX} to names of exported symbols in ${JSTD_HASHMAP_LIBNAME}")
    endif()
    if (NOT ${SYMBOLSUFFIX} STREQUAL "")
        message(STATUS "adding suffix ${SYMBOLSUFFIX} to names of exported symbols in ${JSTD_HASHMAP_LIBNAME}")
    endif()
endif()

endif(JSTD_HASHMAP_ENABLED)

add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)

add_subdirectory(bench)

add_subdirectory(test)
