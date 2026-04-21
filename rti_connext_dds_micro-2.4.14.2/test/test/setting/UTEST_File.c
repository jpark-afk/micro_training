/*
 * FILE: UTEST_File.c - Test file support
 *
 * (c) Copyright, Real-Time Innovations, 2013-2020.
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
 * \file UTEST_File.c
 * \brief Unit-test file support 
 */

#include "UTEST_File.h"

const char*
UTEST_File_get_basename(const char *filename)
{
    const char *c_ptr = filename + strlen(filename);

    /* begining (c_ptr==filename) will not check for preceding '/' */
    while ((c_ptr != filename) && (*(c_ptr-1) != '/') && (*(c_ptr-1) != '\\'))
    {
        --c_ptr;
    }

    return c_ptr;
}

#if HAVE_TEST_RESULTS_FILE
int
UTEST_File_open_file(const char *filename,int oflag,int shflag)
{
#if defined(_MSC_VER) || defined(WIN32)
    int fd;
    errno_t ec;
    ec = _sopen_s(&fd,filename,oflag,_SH_DENYNO,_S_IREAD | _S_IWRITE);
    if (ec)
    {
        return -1;
    }
    else
    {
        return fd;
    }
#else
    return open(filename,oflag,shflag);
#endif
}
#endif
