/*
 * FILE: UTEST_Stdio.c - Unit-test stdio support
 *
 * (c) Copyright, Real-Time Innovations, 2013-2020
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
 * 31dec2013,tk Refactored from UT_Property.c
 * 08nov2013,tk Written
 */
/*ce
 * \file UTEST_Stdio.c
 * \brief Unit-test stdio support
 */

#include "UTEST_Stdio.h"

#if defined(__ANDROID__)
#include <android/log.h>
#endif

/*** SOURCE_BEGIN ***/

void
UTEST_Stdio_printf(const char *format,...)
{
    va_list arglist;
    va_start(arglist, format);

#if defined(__ANDROID__)
    (void)__android_log_vprint(ANDROID_LOG_DEBUG,"com.rti.test",format,arglist);
#elif defined(RTI_AUTOSAR)
#  if defined(RTIME_AUTOSAR_MICROSAR) && defined(__TASKING__)
    (void)standalonevprintf(format,arglist);
#  elif defined(RTIME_AUTOSAR_MICROSAR) && defined(_MSC_VER)
    (void)vprintf(format, arglist);
#  elif defined(RTIME_AUTOSAR_WINCORE)
    (void)vprintf(format, arglist);
#  else
#    error "vprintf must be provided"
#  endif
#elif defined(RTI_THREADX)
    (void)standalonevprintf(format,arglist);
#else
    (void)vprintf(format,arglist);
#endif

    va_end(arglist);
}

int
UTEST_Stdio_snprintf(char *s, size_t n, const char *format, ...)
{
    int len = 0;

    va_list arglist;

    va_start(arglist, format);

#if defined(_MSC_VER) || defined(WIN32)
    len = vsnprintf_s(s,n,_TRUNCATE,format,arglist);
#elif defined(_WRS_VXWORKS_5_X) || ((__GNUC__ == 3) && (__GNUC_MINOR__ == 3))
    (void)n;
    len = vsprintf(s,format,arglist);
#else
    len = vsnprintf(s,n,format,arglist);
#endif

    va_end(arglist);

    return len;
}

