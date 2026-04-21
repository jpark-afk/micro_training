/*
 * FILE: UTEST_File.h - Test file support
 *
 * (c) Copyright, Real-Time Innovations, 2011-2020.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 31dec2013,tk Refactored from UTEST_Property.c
 * 08nov2013,tk Written
 */
/*ce
 * \file
 * \brief Unit-test file support.
 */
#ifndef UTEST_File_h
#define UTEST_File_h

#ifndef test_setting_h
#include "test/test_setting.h"
#endif
#include <fcntl.h>
#if defined(_MSC_VER) || defined(WIN32)
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || \
    defined(__linux__) || defined(RTI_VXWORKS)
#include <unistd.h>
#endif
#if (defined(__APPLE__) && defined(__MACH__)) || \
     defined(__linux__)
#include <sys/uio.h>
#endif

#if defined(_MSC_VER) || defined(WIN32)
#define read _read
#define close _close
#define write _write
#endif

#if HAVE_TEST_RESULTS_FILE
/*e \brief
 * Open a file
 * 
 * \param[in] filename File name to open
 * \param[in] oflag    Flags
 * \param[in] shflag   Mode
 * 
 * \return File descriptor if success or -1 in case of failure.
 */
RTITestDllExport int
UTEST_File_open_file(const char *filename, int oflag, int shflag);
#endif /* HAVE_TEST_RESULTS_FILE */

#endif
