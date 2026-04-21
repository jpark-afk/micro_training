/*
 * FILE: UTEST_System.h - Unit-test System information
 *
 * (c) Copyright, Real-Time Innovations, 2013-2015.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file UTEST_System.h - Unit-test system support
 */
#ifndef UTEST_System_h
#define UTEST_System_h

#ifndef test_dll_h
#include "test/test_dll.h"
#endif

/*e \brief
 * Fill a UTEST_SystemInfo struct.
 *
 * \param[out]  sysinfo  System information struct to fill.
 *
 * \return Returns one if success of zero if error.
 */
RTITestDllExport char
UTEST_System_get_info(struct UTEST_SystemInfo *sysinfo);

#endif
