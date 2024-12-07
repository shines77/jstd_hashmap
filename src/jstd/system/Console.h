
#ifndef JSTD_SYSTEM_CONSOLE_H
#define JSTD_SYSTEM_CONSOLE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_WIN32) || defined(__cygwin__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif // _WIN32

#include <stdio.h>
#include <stdarg.h>

#ifdef _MSC_VER
#include <conio.h>
#endif // _MSC_VER

#include "jstd/basic/stdint.h"
#include "jstd/system/getchar.h"
#include "jstd/test/CPUWarmUp.h"

namespace jstd {

#if defined(_MSC_VER) && !defined(__clang)

/********************************************************

  fgColor:

    0  - BLACK
    1  - BLUE
    2  - GREEN
    3  - CYAN
    4  - RED
    5  - MAGENTA
    6  - BROWN
    7  - LIGHTGRAY

    8  - DARKGRAY
    9  - LIGHTBLUE
    10 - LIGHTGREEN
    11 - LIGHTCYAN
    12 - LIGHTRED
    13 - LIGHTMAGENTA
    14 - YELLOW
    15 - WHITE

********************************************************/

struct FgColor {
    enum {
        Black           = 0x00,
        Blue            = 0x01,
        Green           = 0x02,
        Cyan            = Blue | Green,
        Red             = 0x04,
        Magenta         = Red | Blue,
        Brown           = Red | Green,
        LightGray       = Red | Green | Blue,

        Intensity       = 0x08,

        Darkgray        = Intensity | Black,
        LightBlue       = Intensity | Blue,
        LightGreen      = Intensity | Green,
        LightCyan       = Intensity | Blue | Green,
        LightRed        = Intensity | Red,
        LightMagenta    = Intensity | Red | Blue,
        Yellow          = Intensity | Red | Green,
        White           = Intensity | Red | Green | Blue,
        Last
    };
};

struct BgColor {
    enum {
        Black           = 0x00,
        Blue            = 0x10,
        Green           = 0x20,
        Cyan            = Blue | Green,
        Red             = 0x40,
        Magenta         = Red | Blue,
        Brown           = Red | Green,
        LightGray       = Red | Green | Blue,

        Intensity       = 0x80,

        Darkgray        = Intensity | Black,
        LightBlue       = Intensity | Blue,
        LightGreen      = Intensity | Green,
        LightCyan       = Intensity | Blue | Green,
        LightRed        = Intensity | Red,
        LightMagenta    = Intensity | Red | Blue,
        Yellow          = Intensity | Red | Green,
        White           = Intensity | Red | Green | Blue,
        Last
    };
};

struct StaticConsoleAttr {
    static WORD getOrSetAttr(bool isSet, WORD newValue = 0) {
        static WORD s_wConsoleAttributes = 0;
        if (isSet) {
            s_wConsoleAttributes = newValue;
        }
        return s_wConsoleAttributes;
    }
};

//
// Set windows console text foreground color
//
static inline
BOOL SetConsoleTextFgColor(DWORD fgColor)
{
    BOOL result = FALSE;

    // We will need this handle to get the current background attribute
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        // We use csbi for the wAttributes word.
        if (::GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
            // Mask out all but the background attribute, and add in the forgournd color
            WORD wColor = (csbi.wAttributes & 0xF0) | (fgColor & 0x0F);
            result = ::SetConsoleTextAttribute(hStdOut, wColor);
            if (result) {
                //s_wConsoleAttributes = csbi.wAttributes;
                StaticConsoleAttr::getOrSetAttr(true, csbi.wAttributes);
            }
        }
    }

    return result;
}

//
// Set windows console text foreground color
//
static inline
BOOL SetConsoleTextBgColor(DWORD bgColor)
{
    BOOL result = FALSE;

    // We will need this handle to get the current background attribute
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        // We use csbi for the wAttributes word.
        if (::GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
            // Mask out all but the background attribute, and add in the forgournd color
            WORD wColor = (bgColor & 0xF0) | (csbi.wAttributes & 0x0F);
            result = ::SetConsoleTextAttribute(hStdOut, wColor);
            if (result) {
                //s_wConsoleAttributes = csbi.wAttributes;
                StaticConsoleAttr::getOrSetAttr(true, csbi.wAttributes);
            }
        }
    }

    return result;
}

//
// Set windows console text foreground and background color
//
static inline
BOOL SetConsoleTextColor(DWORD fgColor, DWORD bgColor)
{
    BOOL result = FALSE;

    // We will need this handle to get the current background attribute
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        // We use csbi for the wAttributes word.
        if (::GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
            // Mask out all but the background attribute, and add in the forgournd color
            WORD wColor = (bgColor & 0xF0) | (fgColor & 0x0F);
            result = ::SetConsoleTextAttribute(hStdOut, wColor);
            if (result) {
                //s_wConsoleAttributes = csbi.wAttributes;
                StaticConsoleAttr::getOrSetAttr(true, csbi.wAttributes);
            }
        }
    }

    return result;
}

static inline
BOOL RecoverConsoleTextColor()
{
    BOOL result = FALSE;

    // We will need this handle to get the current background attribute
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut != INVALID_HANDLE_VALUE) {
        WORD wConsoleAttributes = StaticConsoleAttr::getOrSetAttr(false);
        result = ::SetConsoleTextAttribute(hStdOut, wConsoleAttributes);
    }

    return result;
}

#endif // _MSC_VER && !__clang__

class JSTD_DLL Console
{
public:
    Console() = default;
    ~Console() = default;

    static void Write(const char * fmt, ...) {
        va_list arg_list;
        va_start(arg_list, fmt);
        ::vprintf(fmt, arg_list);
        va_end(arg_list);
    }

    static void WriteLine(const char * fmt = nullptr, ...) {
        va_list arg_list;
        if (fmt != nullptr) {
            va_start(arg_list, fmt);
            ::vprintf(fmt, arg_list);
            va_end(arg_list);
        }
        printf("\n");
    }

    static int ReadKey(bool displayTips = true, bool echoInput = false,
                       bool newLine = true, bool enabledCpuWarmup = false) {
        int keyCode;
        if (displayTips) {
            printf("Press any key to continue ...");

            keyCode = jstd::getch();
            printf("\n");
        }
        else {
            keyCode = jstd::getch();
            if (echoInput) {
                if (keyCode != EOF)
                    printf("%c", (char)keyCode);
                else
                    printf("EOF: (%d)", keyCode);
            }
        }

        if (newLine) {
            printf("\n");
        }

        // After call jstd_getch(), warm up the CPU again, at least 500 ms.
        if (enabledCpuWarmup) {
            jtest::CPU::warm_up(800);
        }
        return keyCode;
    }

    static int ReadKeyLine(bool displayTips = true, bool echoInput = false,
                           bool enabledCpuWarmup = false) {
        return ReadKey(displayTips, echoInput, true, enabledCpuWarmup);
    }

    static int ReadInput(const char * fmt, ...) {
        va_list arg_list;
        va_start(arg_list, fmt);
        ::vprintf(fmt, arg_list);
        va_end(arg_list);

        int result = ::vscanf(fmt, arg_list);
        return result;
    }
};

} // namespace jstd

#endif // JSTD_SYSTEM_CONSOLE_H
