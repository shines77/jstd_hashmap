
#ifndef JSTD_STRING_DEF_H
#define JSTD_STRING_DEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace jstd {

struct JSTD_DLL CompareResult {
    enum {
        IsSmaller = -1,
        IsEqual = 0,
        IsBigger = 1
    };
};

} // namespace jstd

#endif // JSTD_STRING_DEF_H
