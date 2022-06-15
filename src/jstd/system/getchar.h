
#ifndef JSTD_SYSTEM_GETCHAR_H
#define JSTD_SYSTEM_GETCHAR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>

#if defined(_MSC_VER) && !defined(__clang__)
#include <conio.h>
#elif defined(__GNUC__) || defined(__clang__) || defined(__linux__)
#include <termios.h>
#endif // _MSC_VER && !__clang__

namespace jstd {

#if defined(_MSC_VER) && !defined(__clang__)

/* Read 1 character without echo */
static inline
int getch(void)
{
    return _getch();
}

/* Read 1 character with echo */
static inline
int getche(void)
{
    int ch = _getch();
    if (ch != EOF)
        printf("%c", (char)ch);
    else
        printf("EOF: (%d)", ch);
    return ch;
}

#elif defined(__GNUC__) || defined(__clang__) || defined(__linux__)

//
// What is equivalent to getch() & getche() in Linux?
//
// See: http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
// See: http://blog.sina.com.cn/s/blog_49f9ea930100nyqc.html
//
// clear terminator screen: "clear" command or "reset" command or printf("%s",   "\033[1H\033[2J");
//

/* Initialize new terminal or restore old terminal i/o settings */
static inline
void set_terminal_os(int echo, int reset)
{
    static struct termios s_term_old;
    static bool s_inited = false;

    if (!reset) {        
        if (!s_inited) {
            tcgetattr(0, &s_term_old);              /* grab old terminal i/o settings */
            s_inited = true;
        }
        struct termios term_new;
        term_new = s_term_old;                      /* make new settings same as old settings */
        term_new.c_lflag &= ~ICANON;                /* disable buffered i/o */
        term_new.c_lflag &= echo ? ECHO : ~ECHO;    /* set echo mode */
        term_new.c_cc[VTIME] = 0;
        term_new.c_cc[VMIN]  = 1;
        tcsetattr(0, TCSANOW, &term_new);           /* use these new terminal i/o settings now */
    } else {
        if (s_inited) {
            tcsetattr(0, TCSANOW, &s_term_old);
        }
    }
}

/* Read 1 character - echo defines echo mode */
static inline
int getch_term(int echo)
{
    int ch;
    if (!echo) {
        set_terminal_os(echo, 0);
        ch = getchar();
        set_terminal_os(echo, 1);
    } else {
        ch = getchar();
    }
    return ch;
}

/* Read 1 character without echo */
static inline
int getch(void)
{
    return getch_term(0);
}

/* Read 1 character with echo */
static inline
int getche(void)
{
    return getch_term(1);
}

#else /* other unknown os */

/* Read 1 character without echo */
static inline
int getch(void)
{
    return getchar();
}

/* Read 1 character with echo */
static inline
int getche(void)
{
    return getchar();
}

#endif // _MSC_VER && !__clang__

} // namespace jstd

#endif // JSTD_SYSTEM_GETCHAR_H
