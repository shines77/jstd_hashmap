
#ifndef JSTD_BASIC_VLD_H
#define JSTD_BASIC_VLD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

//
// The switch of Visual Leak Detector(vld) 'JSTD_ENABLE_VLD' is defined in the 'vld_def.h' file.
// And 'vld_def.h' file will not be updated or uploaded to the git remote repository.
// You can edit the value of 'JSTD_ENABLE_VLD' and enable the Visual Leak Detector(vld).
//
#include "jstd/basic/vld_def.h"

/**********************************************************
 *
 *  Use Visual Leak Detector(vld) for Visual C++,
 *  Homepage: http://vld.codeplex.com/
 *
 **********************************************************/
#if defined(_MSC_VER)
#if defined(_DEBUG)

// If you have not installed VLD (visual leak detector), please comment out this statement.

#if defined(JSTD_ENABLE_VLD) && (JSTD_ENABLE_VLD != 0)
#include <vld.h>
#endif

#endif // _DEBUG
#endif // _MSC_VER

#endif // JSTD_BASIC_VLD_H
