/*
 * FILE: OSAPITester.c
 *
 * (c) Copyright, Real-Time Innovations, 2011-2015.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief OSAPITester
 */
#include <math.h>
#include <stdlib.h>

#include "test/test_setting.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_config.h"
#include "osapi/osapi_log.h"
#include "MutexTester.h"
#include "SemaphoreTester.h"
#include "ThreadTester.h"
#include "SystemTester.h"
#include "TimerTester.h"
#include "OSAPITypeTester.h"
#include "ProcessTester.h"
#include "HeapTester.h"
#include "OSAPITester.h"

RTI_PRIVATE struct UTEST_TestEntry OSAPITester_tests[]=
{
    RTITestCase("system",
                OSAPI_SystemTester_run,
                TEST_ENABLED),

#ifndef RTI_AUTOSAR
    RTITestCase("mutex",
                OSAPI_MutexTester_run,
                TEST_ENABLED),
#endif

    RTITestCase("semaphore",
                OSAPI_SemaphoreTester_run,
                TEST_ENABLED),

#ifndef RTI_AUTOSAR
    /* It is not possible to create threads on Autosar.
     */
    RTITestCase("thread",
                OSAPI_ThreadTester_run,
                TEST_ENABLED),
#endif

    RTITestCase("timer",
                OSAPI_TimerTester_run,
                TEST_ENABLED),

    RTITestCase("process",
                OSAPI_ProcessTester_run,
                TEST_ENABLED),

    RTITestCase("OSAPIType",
                OSAPI_OSAPITypeTester_run,
                TEST_ENABLED),

    RTITestCase("heap",
                OSAPI_HeapTester_run,
                TEST_ENABLED)
};

UT_DEFINE_TEST(OSAPITester,"osapi","rti_me_heap.osapi={};\n")


