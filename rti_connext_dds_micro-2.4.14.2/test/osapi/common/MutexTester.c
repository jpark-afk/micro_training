/*
 * FILE: MutexTester.c - Mutex unit-test
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
 * modification history
 * --------------------
 * 18mar2013,tk  Updated
 * 26oct2011,yy  Created
 */
/*ce
 * \file
 * \brief MutexTester
 */
#include "test/test_setting.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_mutex.h"

#include "MutexTester.h"

/* ThreadX only have memory for OSAPI_PLATFORM_THREADX_MAX_THREADS thread 
 * stacks. The system implementation already creates 1 thread so this test 
 * will use only OSAPI_PLATFORM_THREADX_MAX_THREADS - 1 threads in ThreadX
 */
#ifdef RTI_THREADX
#define NUM_THREADS (OSAPI_PLATFORM_THREADX_MAX_THREADS - 1)
#else
#define NUM_THREADS 10
#endif
#define NUM_TRIES 50
#define THREAD_PRIORITY 100
#define THREAD_STACK_SIZE 16384

#ifndef RTI_AUTOSAR
typedef struct
{
    unsigned char result;
    struct OSAPI_Mutex* mutex;
} OSAPI_MutexTester_testBasicStruct;

RTI_PRIVATE RTI_BOOL
OSAPI_MutexTester_testBasicThread(struct OSAPI_ThreadInfo* threadInfo)
{
    static int shouldBeZero = 0;
    int i;
    OSAPI_MutexTester_testBasicStruct* basicStruct =
            (OSAPI_MutexTester_testBasicStruct*)threadInfo->user_data;

    for (i = 0; i < NUM_TRIES; i++)
    {
        /* enter critical section */
        if (!OSAPI_Mutex_take(basicStruct->mutex))
        {
            basicStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }

        /* since shouldBeZero is only changed in the critical section */
        if (shouldBeZero != 0)
        {
            basicStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }
        shouldBeZero++;
        shouldBeZero--;

        /* exit critical section */
        if (!OSAPI_Mutex_give(basicStruct->mutex))
        {
            basicStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE unsigned char
OSAPI_MutexTester_testBasic(struct UTEST_Context* setting)
{
    UTEST_VAR int i;
    UTEST_VAR char name[32];
    UTEST_VAR unsigned char result = RTI_FALSE;
    UTEST_VAR struct OSAPI_Thread* threads[NUM_THREADS];
    UTEST_VAR OSAPI_MutexTester_testBasicStruct basicStruct;
    UTEST_VAR struct OSAPI_ThreadProperty threadProps = OSAPI_THREAD_PROPERTY_DEFAULT;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        basicStruct.result = RTI_TRUE;
        basicStruct.mutex = OSAPI_Mutex_new();

        /* Test: mutex created
        * Expect: Success
        */
        TEST_ASSERT(basicStruct.mutex,"could not create mutex",goto fail);

        for (i = 0; i < NUM_THREADS; i++)
        {
            threads[i] = NULL;
        }

        /* create all threads */
        for (i = 0; i < NUM_THREADS; i++)
        {
            UTEST_Stdio_snprintf(name,32,"thread_%d", i);
            threads[i] = OSAPI_Thread_create(name, &threadProps,
                    OSAPI_MutexTester_testBasicThread, (void *)&basicStruct, NULL);

            /* Test:threads spawned
            * Expect: Success
            */
            TEST_ASSERT(threads[i],"could not spawn thread",goto fail);
        }
        /* start all threads */
        for (i = 0; i < NUM_THREADS; i++)
        {
            /* Test:threads started
            * Expect: Success
            */
            TEST_ASSERT(OSAPI_Thread_start(threads[i]),"could not start thread",goto fail);
        }
    }
    UTEST_SETUP_END

    result = RTI_TRUE;

fail:
    /* kill all threads */
    for (i = 0; i < NUM_THREADS; i++)
    {
        /* Test:threads destroyed
         * Expect: Success
         */
        TEST_ASSERT(OSAPI_Thread_destroy(threads[i]),
                    "could not destroy thread",
                    return RTI_FALSE);
    }

#ifndef RTI_CERT
    OSAPI_Mutex_delete(basicStruct.mutex);
#endif

    return (unsigned char)(result && basicStruct.result);
}
#endif /* !RTI_AUTOSAR */

#ifndef RTI_AUTOSAR
RTI_PRIVATE struct UTEST_TestEntry OSAPI_MutexTester_tests[]=
{
    /* Not possible to create threads on Autosar. We would need to update
     * the test app to create 32 tasks in order to enable that test.
     */
    RTITestCase("basic",
                OSAPI_MutexTester_testBasic,
                TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_MutexTester,"mutex")
#endif
