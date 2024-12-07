
#ifndef CPU_CORES_WIN32_H
#define CPU_CORES_WIN32_H

#if defined(__WIN32__) || defined(__WIN64__) || defined(__CYGWIN32__) || defined(__CYGWIN64__) \
 || defined(__MINGW32__) || defined(__MINGW64__) || defined(_WIN32) || defined(_WIN64)
#define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

//
// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx
//
typedef BOOL (WINAPI * LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

// Helper function to count set bits in the processor mask.
static inline
int countBitsSet(ULONG_PTR bitMask)
{
    int result = 0;
    while (bitMask != 0) {
        result += (int)(bitMask & 1);
        bitMask >>= 1;
    }
    return result;
}

static inline
bool getCpuCoresInfo(int & cpus, int & cores, int & logicalCores, double & clockSpeed)
{
    cpus = 0;
    cores = 0;
    logicalCores = 0;
    clockSpeed = 0;

    // Clock speed
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_EXECUTE, &hKey) == ERROR_SUCCESS) {
        DWORD type = REG_DWORD;
        DWORD val;
        DWORD cbData = sizeof(val);
        if (RegQueryValueEx(hKey, TEXT("~MHz"), NULL, &type, (LPBYTE)&val, &cbData) == ERROR_SUCCESS) {
            if (type == REG_DWORD && cbData == sizeof(DWORD)) {
                clockSpeed = val / 1000.0;
            }
        }

    }
    if (clockSpeed == 0) {
        // Can't access registry, try QueryPerformanceFrequency (nearly always same speed as CPU)
        LARGE_INTEGER f;
        if (!QueryPerformanceFrequency(&f)) {
            return false;
        }
        clockSpeed = f.QuadPart / 1000.0 / 1000.0;
    }

    // Everything else
    LPFN_GLPI glpi;
    glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
    if (glpi == NULL) {
        return false;
    }

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    DWORD bufferLength = 0;
    if (glpi(buffer, &bufferLength) == TRUE) {
        return false;
    }

    while (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        if (buffer != NULL) {
            free(buffer);
        }
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(bufferLength);
        if (buffer == NULL) {
            return false;
        }
        if (glpi(buffer, &bufferLength) == TRUE) {
            if (bufferLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) * sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) != bufferLength) {
                // sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) must have changed (different from at compile time)
                free(buffer);
                return false;
            }

            auto end = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)((char*)buffer + bufferLength);
            for (auto ptr = buffer; ptr != end; ++ptr) {
                switch (ptr->Relationship) {
                case RelationProcessorCore:
                    ++cores;
                    logicalCores += countBitsSet(ptr->ProcessorMask);
                    break;
                case RelationProcessorPackage:
                    ++cpus;
                    break;
                default:
                    break;
                }
            }

            free(buffer);
            return true;
        }
    }
    if (buffer != NULL) {
        free(buffer);
    }
    return false;
}

#else

// TODO: 
static inline
bool getCpuCoresInfo(int & cpus, int & cores, int & logicalCores, double & clockSpeed)
{
    cpus = 0;
    cores = 0;
    logicalCores = 0;
    clockSpeed = 0;

    return false;
}

#endif // OS_WINDOWS

#endif // CPU_CORES_WIN32_H
