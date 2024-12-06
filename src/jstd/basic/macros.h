
#ifndef JSTD_BASIC_MARCOS_H
#define JSTD_BASIC_MARCOS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define JSTD_DEFINED(X)         ((##X) && (##X != 0))

#define JSTD_STRINGIFY(Text)    JSTD_TO_STRING(Text)
#define JSTD_TO_STRING(Text)    #Text

#define JSTD_GLUE(a, b)         a ## b
#define JSTD_JOIN(a, b)         JSTD_GLUE(a, b)

#define JSTD_TO_BYTE(n)         ((n) & 0xFFu)
#define JSTD_TO_WORD(n)         ((n) & 0xFFFFu)
#define JSTD_TO_DWORD(n)        ((n) & 0xFFFFFFFFu)

#define JSTD_TO_INT(a, b, c, d) \
                                ((JSTD_TO_BYTE(a) << 24) | (JSTD_TO_BYTE(b) << 16) | \
                                 (JSTD_TO_BYTE(c) <<  8) | (JSTD_TO_BYTE(d) <<  0))

#define JSTD_TO_INT2(a, b)      JSTD_TO_INT(0, 0, a, b)
#define JSTD_TO_INT3(a, b, c)   JSTD_TO_INT(0, a, b, c)

#define JSTD_TO_VER3(a, b, c)   ((JSTD_TO_BYTE(a) << 24) | (JSTD_TO_BYTE(b) << 16) | \
                                 (JSTD_TO_WORD(c) <<  0))

#define JSTD_TO_VER4(a, b, c, d) \
                                JSTD_TO_INT(a, b, c, d)

#define JSTD_TO_DOT2(a, b, c)   a ##.## b
#define JSTD_TO_DOT3(a, b, c)   a ##.## b ##.## c
#define JSTD_TO_DOT4(a, b, c, d) \
                                a ##.## b ##.## c ##.## d

//                               0 ~ 8191 (9999)
#define JSTD_TO_DEC(n)          ((n) & 0x1FFFu)

//                               0 ~ 99
#define JSTD_TO_DEC1(n)         ((n) & 0x7Fu)
#define JSTD_TO_DEC2(n)         (((n) & 0x7Fu) * 100)
#define JSTD_TO_DEC3(n)         (((n) & 0x7Fu) * 10000)
#define JSTD_TO_DEC4(n)         (((n) & 0x7Fu) * 1000000)
#define JSTD_TO_DEC5(n)         (((n) & 0x7Fu) * 100000000)

#define JSTD_MAKE_DEC4(a, b, c, d) \
                                (JSTD_TO_DEC4(a) + JSTD_TO_DEC3(b) + \
                                 JSTD_TO_DEC2(c) + JSTD_TO_DEC1(d))
#define JSTD_MAKE_DEC4_(a, b, c, d) \
                                (JSTD_TO_DEC5(a) + JSTD_TO_DEC4(b) + \
                                 JSTD_TO_DEC3(c) + JSTD_TO_DEC (d))

#define JSTD_MAKE_DEC2(a, b)    JSTD_MAKE_DEC4(0, 0, a, b)

#define JSTD_MAKE_DEC3(a, b, c) JSTD_MAKE_DEC4(0, a, b, c)

#define JSTD_MAKE_DEC3_(a, b, c) \
                                JSTD_MAKE_DEC4_(0, a, b, c)

#endif // JSTD_BASIC_MARCOS_H
