
#include "jstd/system/getchar.h"

#include <stdio.h>

#if defined(_MSC_VER) || defined(_ICL) || defined(__INTEL_COMPILER) || defined(__MINGW32__)

#include <conio.h>

/* Read 1 character without echo */
int jstd_getch(void)
{
    return _getch();
}

/* Read 1 character with echo */
int jstd_getche(void)
{
    int ch = _getch();
    if (ch != EOF)
        printf("%c", (char)ch);
    else
        printf("EOF: (%d)", ch);
    return ch;
}

#elif defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER)) || defined(__linux__)

#include <termios.h>

// <comment>
//
// What is equivalent to getch() & getche() in Linux?
//
// See: http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-in-linux
//
// See: http://blog.sina.com.cn/s/blog_49f9ea930100nyqc.html
//
// </comment>
//
//
// clear terminator screen: "clear" command or "reset" command or printf("%s",   "\033[1H\033[2J");
//

static struct termios s_term_old;

/* Initialize new terminal i/o settings */
static void init_terminal_os(int echo)
{
    struct termios term_new;
    tcgetattr(0, &s_term_old);                  /* grab old terminal i/o settings */
    term_new = s_term_old;                      /* make new settings same as old settings */
    term_new.c_lflag &= ~ICANON;                /* disable buffered i/o */
    term_new.c_lflag &= echo ? ECHO : ~ECHO;    /* set echo mode */
    term_new.c_cc[VTIME] = 0;
    term_new.c_cc[VMIN]  = 1;
    tcsetattr(0, TCSANOW, &term_new);           /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
static void reset_terminal_os(void)
{
    tcsetattr(0, TCSANOW, &s_term_old);
}

/* Read 1 character - echo defines echo mode */
int jstd_getch_term(int echo)
{
    int ch;
    if (!echo) {
        init_terminal_os(echo);
        ch = getchar();
        reset_terminal_os();
    }
    else {
        ch = getchar();
    }
    return ch;
}

/* Read 1 character without echo */
int jstd_getch(void)
{
    return jstd_getch_term(0);
}

/* Read 1 character with echo */
int jstd_getche(void)
{
    return jstd_getch_term(1);
}

#else /* other unknown os */

/* Read 1 character without echo */
int jstd_getch(void)
{
    return (int)-1;
}

/* Read 1 character with echo */
int jstd_getche(void)
{
    return (int)-1;
}

#endif /* __GNUC__ */
