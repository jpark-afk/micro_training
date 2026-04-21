/*
 * FILE: UTEST_Runner.c - Unit-test execution support
 *
 * (c) Copyright, Real-Time Innovations, 2004-2020
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
 * 31dec2013,tk  Refactored from Setting.c
 * 01aug2012,tk  Many enhancements
 * 01dec2004,cc  Created, based on Waveworks tree.
 */
/*ce
 * \file UTEST_Runner.h
 * \brief Unit-test execution support
 */
#ifndef UTEST_Runner_h
#define UTEST_Runner_h

#ifndef test_setting_h
#include "test/test_setting.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#if !defined(__vxworks) && !defined(WIN32) && !defined(RTI_THREADX) && !defined(RTI_AUTOSAR) && !defined(RTI_DEOS)
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "UTEST_Property.h"

#endif
