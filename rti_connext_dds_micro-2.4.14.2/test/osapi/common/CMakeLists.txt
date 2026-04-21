###############################################################################
# (c) Copyright, Real-Time Innovations 2020-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

IF ((RTIME_SHIP_SOURCE) OR ((DEFINED RTIME_DEVTREE) AND (NOT ${RTIME_DEVTREE})))
    # LogTester, TimeTester  and internal tests are not needed for the port validation
    SET(RTI_SUBMODULE_TEST_SRC SemaphoreTester.c MutexTester.c 
                               SystemTester.c ThreadTester.c
                               TimerTester.c OSAPITypeTester.c
                               ProcessTester.c HeapTester.c)
                     
    SET(RTI_SUBMODULE_TEST_HDR SemaphoreTester.h MutexTester.h
                               SystemTester.h ThreadTester.h
                               TimerTester.h OSAPITypeTester.h
                               ProcessTester.h HeapTester.h)
ELSE()
    SET(RTI_SUBMODULE_TEST_SRC SemaphoreTester.c MutexTester.c LogTester.c 
                               SystemTester.c ThreadTester.c TimerTester.c
                               TimeTester.c OSAPITypeTester.c ProcessTester.c HeapTester.c
                               SemaphoreTesterInternal.c MutexTesterInternal.c
                               SystemTesterInternal.c ThreadTesterInternal.c TimerTesterInternal.c
                               HeapTesterInternal.c)
                     
    SET(RTI_SUBMODULE_TEST_HDR SemaphoreTester.h MutexTester.h LogTester.h
                               SystemTester.h ThreadTester.h TimerTester.h 
                               TimeTester.h OSAPITypeTester.h ProcessTester.h HeapTester.h
                               SemaphoreTesterInternal.h MutexTesterInternal.h
                               SystemTesterInternal.h ThreadTesterInternal.h TimerTesterInternal.h
                               HeapTesterInternal.h)
ENDIF()

