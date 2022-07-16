
#ifndef JSTD_TEST_PROCESS_MEMINFO_H
#define JSTD_TEST_PROCESS_MEMINFO_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif // _WIN32

#if defined(_MSC_VER) || defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)
#include <io.h>
#include <process.h>
#include <psapi.h>
#else
// defined(__linux__) || defined(__clang__) || defined(__FreeBSD__) || (defined(__GNUC__) && defined(__cygwin__))
#include <unistd.h>
#endif // _MSC_VER

#include "jstd/basic/inttypes.h"

#include <stdio.h>
#include <stdlib.h>     // For ::atoll()
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>

namespace jtest {

#if defined(_MSC_VER) || defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)

//
// See: https://www.cnblogs.com/talenth/p/9762528.html
// See: https://blog.csdn.net/springontime/article/details/80625850
//

static inline
int get_mem_info(std::string & mem_size)
{
    HANDLE handle = ::GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS_EX pmc = { 0 };
    if (!::GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc))) {
        DWORD errCode = ::GetLastError();
        printf("GetProcessMemoryInfo() failed, lastErrorCode: %d\n\n", errCode);
        return -1;
    }

    char buf[256];

    // Physical memory currently occupied
    // WorkingSetSize: %d (KB)
    ::snprintf(buf, sizeof(buf), "%" PRIuPTR " KB\n", pmc.WorkingSetSize / 1024);
    mem_size = buf;

    return 0;
}

static inline
std::size_t GetCurrentMemoryUsage()
{
    HANDLE handle = ::GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS_EX pmc = { 0 };
    if (!::GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc))) {
        DWORD errCode = ::GetLastError();
        printf("GetProcessMemoryInfo() failed, lastErrorCode: %d\n\n", errCode);
        return 0;
    }

    //
    // Physical memory currently occupied
    // WorkingSetSize: %d (KB)
    //
    std::size_t memory_usage = static_cast<std::size_t>(pmc.WorkingSetSize);
    return memory_usage;
}

#else

//
// See: https://blog.csdn.net/weiyuefei/article/details/52281312
//

static inline
int get_mem_info(std::string & str_mem_size)
{
    pid_t pid = getpid();

    char filename[128];
    ::snprintf(filename, sizeof(filename) - 1, "/proc/self/status");

    std::ifstream ifs;
    try {
        ifs.open(filename, std::ios::in);
        if (!ifs.good()) {
            std::cout << "open " << filename << " error!" << std::endl << std::endl;
            return (-1);
        }

        char buf[512];
        char mem_size[64];
        char mem_unit[64];

        while (!ifs.eof()) {
            buf[0] = '\0';
            ifs.getline(buf, sizeof(buf) - 1);
            if (::strncmp(buf, "VmRSS:", 6) == 0) {
                ::sscanf(buf + 6, "%s%s", mem_size, mem_unit);
                str_mem_size = std::string(mem_size) + " " + std::string(mem_unit);
                break;
            }
        }

        ifs.close();
    } catch (const std::exception & ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        ifs.close();
    }
    return 0;
}

static inline
std::size_t GetCurrentMemoryUsage()
{
    pid_t pid = getpid();

    char filename[128];
    ::snprintf(filename, sizeof(filename) - 1, "/proc/self/status");

    std::size_t memory_usage = 0;
    std::ifstream ifs;
    try {
        ifs.open(filename, std::ios::in);
        if (!ifs.good()) {
            std::cout << "open " << filename << " error!" << std::endl << std::endl;
            return 0;
        }

        char buf[512];
        char mem_size[64];
        char mem_unit[64];

        while (!ifs.eof()) {
            buf[0] = '\0';
            ifs.getline(buf, sizeof(buf) - 1);
            if (::strncmp(buf, "VmRSS:", 6) == 0) {
                ::sscanf(buf + 6, "%s %s", mem_size, mem_unit);
                std::size_t memory_usage = static_cast<std::size_t>(::atoll(mem_size));
                std::string memory_uint = mem_unit;
                if (memory_uint == "kB" || memory_uint == "KB")
                    memory_usage *= 1024;
                else if (memory_uint == "mB" || memory_uint == "MB")
                    memory_usage *= (1024 * 1024);
                break;
            }
        }

        ifs.close();
    } catch (const std::exception & ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        ifs.close();
    }
    return memory_usage;
}

#endif // _MSC_VER

} // namespace jtest

#endif // JSTD_TEST_PROCESS_MEMINFO_H
