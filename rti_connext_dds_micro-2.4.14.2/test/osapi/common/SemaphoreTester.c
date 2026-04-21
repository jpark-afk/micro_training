/*
 * FILE: SemaphoreTester.c - Semaphore unit-test implementation
 *
 * (c) Copyright, Real-Time Innovations, 2011-2021.
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
 * 18mar2011,tk  Updated
 */
/*ce
 * \file
 * \brief Semaphore tester
 */
#include "test/test_setting.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_thread.h"

#include "SemaphoreTester.h"

#define NUM_OF_GIVES 10
#define THREAD_PRIORITY 100
#define THREAD_STACK_SIZE 16384

typedef struct
{
    RTI_BOOL result;
    struct OSAPI_Semaphore* sem1;
    struct OSAPI_Semaphore* sem2;
    struct UTEST_Context* setting;
} OSAPI_SemaphoreTester_testBasicStruct;

#ifndef RTI_AUTOSAR
RTI_PRIVATE RTI_BOOL
OSAPI_SemaphoreTester_testBasicThreadWakeup(struct OSAPI_ThreadInfo* thread_info)
{
    OSAPI_SemaphoreTester_testBasicStruct* basicStruct =
            (OSAPI_SemaphoreTester_testBasicStruct*)thread_info->user_data;

    return OSAPI_Semaphore_give(basicStruct->sem1);
}

RTI_PRIVATE RTI_BOOL
OSAPI_SemaphoreTester_testBasicThread(struct OSAPI_ThreadInfo* thread_info)
{
    RTI_INT32 fr;
    OSAPI_SemaphoreTester_testBasicStruct* basicStruct =
            (OSAPI_SemaphoreTester_testBasicStruct*)thread_info->user_data;

    while (!thread_info->stop_thread)
    {
        /* Test: taking semaphore
         * Expect: Success
         */
        if (!OSAPI_Semaphore_take(basicStruct->sem1, OSAPI_SEMAPHORE_TIMEOUT_INFINITE,
                &fr))
        {
            UTEST_Stdio_snprintf(basicStruct->setting->test_last_error,50,"failed to take sem");
            basicStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }
        /* Test: giving semaphore
         * Expect: Success
         */
        if (!OSAPI_Semaphore_give(basicStruct->sem2))
        {
            UTEST_Stdio_snprintf(basicStruct->setting->test_last_error,50,"failed to give sem");
            basicStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE unsigned char
OSAPI_SemaphoreTester_testBasic(struct UTEST_Context* setting)
{
    UTEST_VAR int i,c;
    UTEST_VAR unsigned char result = RTI_FALSE;
    UTEST_VAR struct OSAPI_Thread* basicThread = NULL;
    UTEST_VAR OSAPI_SemaphoreTester_testBasicStruct basicStruct;
    UTEST_VAR struct OSAPI_ThreadProperty threadProps = OSAPI_THREAD_PROPERTY_DEFAULT;
    UTEST_VAR RTI_INT32 fr;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        basicStruct.result = RTI_TRUE;
        basicStruct.sem1 = OSAPI_Semaphore_new();
        basicStruct.sem2 = OSAPI_Semaphore_new();
        basicStruct.setting = setting;

        /* Test: create 2 new semaphores
            * Expect: Success
            */
        TEST_ASSERT(basicStruct.sem1 && basicStruct.sem2,
                    "could not create semaphores",
                    goto fail);

        basicThread = OSAPI_Thread_create("basic", &threadProps,
                OSAPI_SemaphoreTester_testBasicThread, (void *)&basicStruct,
                OSAPI_SemaphoreTester_testBasicThreadWakeup);

        /* Test: create new thread
            * Expect: Success
            */
        TEST_ASSERT(basicThread,
                    "could not create thread!",
                    goto fail);

        /* Test: start thread
            * Expect: Success
            */
        TEST_ASSERT(OSAPI_Thread_start(basicThread),
                    "could not start thread!",
                    goto fail);
    }
    UTEST_SETUP_END

    /* Test:  everyime we give semaphore 1, we can take semaphore 2 
     * Expect: Success
     */
    TEST_ASSERT(OSAPI_Semaphore_give(basicStruct.sem1),
                "could not give semaphore 1",
                goto fail);

    TEST_ASSERT(OSAPI_Semaphore_take(basicStruct.sem2,
                OSAPI_SEMAPHORE_TIMEOUT_INFINITE, &fr),
                "could not take semaphore 2",
                goto fail);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto fail);

    /* Test: try taking semaphore 2 without giving sem 1 first
     * Expect: Failure
     */
    TEST_ASSERT(OSAPI_Semaphore_take(basicStruct.sem2, 1000, &fr),
                "took semaphore 2 w/o giving semaphore 1",
                goto fail);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto fail);

     /* Test: giving semaphore 1 multiple times
     * Expect: Success
     */
    for (i = 0; i < NUM_OF_GIVES; i++)
    {
        TEST_ASSERT(OSAPI_Semaphore_give(basicStruct.sem1),
                    "could not give semaphore 1",
                    goto fail);
    }

    /* Test: take semaphore 2 - only the first take should be successful
     * Expect: Success
     */
    TEST_ASSERT(OSAPI_Semaphore_take(basicStruct.sem2,
                OSAPI_SEMAPHORE_TIMEOUT_INFINITE, &fr),
                "could not take semaphore 2",
                goto fail);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto fail);

    /* Test:  a condition where there MAY be a 2nd give on semaphore 2 depending on
     * when thread switching occurs. Because of this we allow at _most_
     * one additional take to succeed on semaphore 2
     * Expect: Success
     */
    c = 0;
    for (i = 0; i < NUM_OF_GIVES; i++)
    {
        if (c == 0)
        {
            if (OSAPI_Semaphore_take(basicStruct.sem2, 0, &fr))
            {
                ++c;
            }
        }
        else
        {
            TEST_ASSERT(OSAPI_Semaphore_take(basicStruct.sem2, 0, &fr),
                        "took semaphore 2 more than once "
                        "after giving semaphore 1 many times!",
                        goto fail);

            TEST_ASSERT(fr != OSAPI_SEMAPHORE_RESULT_ERROR,
                        "fail reason not OK or TIMEOUT",
                        goto fail);
        }
    }

    /* Test: normal give/take again to make sure everything still works
     * Expect: Success
     */
    TEST_ASSERT(OSAPI_Semaphore_give(basicStruct.sem1),
                "could not give semaphore 1",
                goto fail);

    TEST_ASSERT(OSAPI_Semaphore_take(basicStruct.sem2,
                OSAPI_SEMAPHORE_TIMEOUT_INFINITE, &fr),
                "could not take semaphore 2",
                goto fail);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto fail);

    result = RTI_TRUE;

fail:

    TEST_ASSERT(OSAPI_Thread_destroy(basicThread),
                "could not destory thread",
                return RTI_FALSE);

#ifndef RTI_CERT
    OSAPI_Semaphore_delete(basicStruct.sem1);
    OSAPI_Semaphore_delete(basicStruct.sem2);
#endif
    return (unsigned char)(result && basicStruct.result == RTI_TRUE);
}
#endif /* !RTI_AUTOSAR */

RTI_PRIVATE unsigned char
OSAPI_SemaphoreTester_testSingle(struct UTEST_Context *setting)
{
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct OSAPI_Semaphore *sem = NULL;
    UTEST_VAR RTI_INT32 fr;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        sem = OSAPI_Semaphore_new();

        /* Test: create single semaphore
            * Expect: Success
            */
        TEST_ASSERT(sem != NULL,"failed to create semaphore", goto done);
    }
    UTEST_SETUP_END

    /* Test: take semaphores
     * Expect: Success
     */
    TEST_ASSERT(OSAPI_Semaphore_give(sem),
                "could not give semaphore",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem),
                "could not give semaphore",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem),
                "could not give semaphore",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 0, &fr),
                "could not take semaphore",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 0, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 0, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

#ifndef RTI_CERT
    TEST_ASSERT(OSAPI_Semaphore_delete(sem),
                "failed to delete semaphore", goto done);
#endif

    retval = RTI_TRUE;
done:
    return retval;
}

RTI_PRIVATE unsigned char
OSAPI_SemaphoreTester_testMultiple(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct OSAPI_Semaphore *sem1 = NULL;
    struct OSAPI_Semaphore *sem2 = NULL;
    RTI_INT32 fr;

    CHECK_DO_RUN_TEST(setting);

    sem1 = OSAPI_Semaphore_new();
    sem2 = OSAPI_Semaphore_new();

    /* Test: create multiple semaphore
     * Expect: Success
     */
    TEST_ASSERT(sem1 != NULL,"failed to create semaphore1", goto done);
    TEST_ASSERT(sem2 != NULL,"failed to create semaphore2", goto done);

    /* Test: give/take semaphores
     * Expect: Success
     */
    TEST_ASSERT(OSAPI_Semaphore_give(sem1),
                "could not give semaphore1",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem2),
                "could not give semaphore2",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem1),
                "could not give semaphore1",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem2),
                "could not give semaphore2",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem1),
                "could not give semaphore1",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_give(sem2),
                "could not give semaphore2",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem1, 0, &fr),
                "could not take semaphore1",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem2, 0, &fr),
                "could not take semaphore2",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem1, 0, &fr),
                "could not take semaphore1 (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem2, 0, &fr),
                "could not take semaphore2 (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem1, 0, &fr),
                "could not take semaphore1 (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem2, 0, &fr),
                "could not take semaphore2 (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

#ifndef RTI_CERT
    TEST_ASSERT(OSAPI_Semaphore_delete(sem2),
                "failed to delete semaphore2", goto done);
    TEST_ASSERT(OSAPI_Semaphore_delete(sem1),
                "failed to delete semaphore1", goto done);
#endif

    retval = RTI_TRUE;
done:
    return retval;
}

RTI_PRIVATE unsigned char
OSAPI_SemaphoreTester_test_timeout(struct UTEST_Context *setting)
{
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct OSAPI_Semaphore *sem = NULL;
    UTEST_VAR RTI_INT32 fr;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        sem = OSAPI_Semaphore_new();
        /* Test: create single semaphore
            * Expect: Success
            */
        TEST_ASSERT(sem != NULL,"failed to create semaphore", goto done);
    }
    UTEST_SETUP_END

    /* Test: take semaphore
     * Expect: Failure(timeout)
     */
    TEST_ASSERT(OSAPI_Semaphore_take(sem, 0, &fr),
                "could not take semaphore (expect timeout)",
                goto done);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    /* Test: take semaphore
     * Expect: Failure(timeout)
     */
    TEST_ASSERT(OSAPI_Semaphore_take(sem, 1000, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 2000, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 5000, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(sem, 5500, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

#ifndef RTI_CERT
    TEST_ASSERT(OSAPI_Semaphore_delete(sem),
                "failed to delete semaphore", goto done);
#endif

    retval = RTI_TRUE;
done:
    return retval;
}

struct OSAPI_SemaphoreTesterSemThread
{
    struct OSAPI_Thread *osapi_thread;
    struct OSAPI_Semaphore *my_sem1;
    struct UTEST_Context *setting;
    RTI_BOOL test_ok;
};

#ifndef RTI_AUTOSAR
RTI_PRIVATE RTI_BOOL
OSAPI_SemaphoreTester_timeout_thread_wakeup(struct OSAPI_ThreadInfo* threadInfo)
{
    UNUSED_ARG(threadInfo);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_SemaphoreTester_timeout_thread(struct OSAPI_ThreadInfo* threadInfo)
{
    struct OSAPI_SemaphoreTesterSemThread* basicStruct =
                (struct OSAPI_SemaphoreTesterSemThread*)threadInfo->user_data;
    struct UTEST_Context *setting = basicStruct->setting;
    RTI_INT32 fr;
    struct OSAPI_Semaphore *sem = basicStruct->my_sem1;
    RTI_INT32 loops;

    basicStruct->test_ok = RTI_FALSE;

    for (loops = 0; loops < 8; ++loops)
    {
        /* Test: timeout of taking threads
         * Expect: Failure(timeout)
         */
        TEST_ASSERT(OSAPI_Semaphore_take(sem, 0, &fr),
                    "could not take semaphore (expect timeout)",
                    goto done);

        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

        TEST_ASSERT(OSAPI_Semaphore_take(sem, 100, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

        TEST_ASSERT(OSAPI_Semaphore_take(sem, 600, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

        TEST_ASSERT(OSAPI_Semaphore_take(sem, 950, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);

        TEST_ASSERT(OSAPI_Semaphore_take(sem, 1000, &fr),
                "could not take semaphore (expect timeout)",
                goto done);

        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_TIMEOUT,
                "fail reason not TIMEOUT",
                goto done);
    }

    basicStruct->test_ok = RTI_TRUE;

    return RTI_TRUE;

done:
    return RTI_FALSE;
}

RTI_PRIVATE unsigned char
OSAPI_SemaphoreTester_test_timeout_mt(struct UTEST_Context *setting)
{
/* ThreadX only have memory for OSAPI_PLATFORM_THREADX_MAX_THREADS thread 
 * stacks. The system implementation already creates 1 thread so this test 
 * will use only OSAPI_PLATFORM_THREADX_MAX_THREADS - 1 threads in ThreadX
 */
#ifndef RTI_THREADX
#define MAX_SEM_THREAD 8
#else
#define MAX_SEM_THREAD (OSAPI_PLATFORM_THREADX_MAX_THREADS - 1)
#endif
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct OSAPI_SemaphoreTesterSemThread sem_threads[MAX_SEM_THREAD];
    UTEST_VAR struct OSAPI_ThreadProperty thread_props = OSAPI_THREAD_PROPERTY_DEFAULT;
    UTEST_VAR RTI_INT32 i;
    UTEST_VAR struct OSAPI_SemaphoreTesterSemThread *sem_thread;
    UTEST_VAR RTI_INT32 thread_complete = 0;
    UTEST_VAR RTI_INT32 max_wait_count = 45;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        for (i = 0; i < MAX_SEM_THREAD; ++i)
        {
            sem_thread = &sem_threads[i];
            sem_thread->test_ok = RTI_FALSE;
            sem_thread->osapi_thread = OSAPI_Thread_create("sem_thread",&thread_props,
                    OSAPI_SemaphoreTester_timeout_thread, sem_thread,
                    OSAPI_SemaphoreTester_timeout_thread_wakeup);

            sem_thread->my_sem1 = OSAPI_Semaphore_new();
            TEST_ASSERT((sem_thread->osapi_thread != NULL) &&
                        (sem_thread->my_sem1 != NULL),
                        "failed to create thread resources",
                        goto done);

            TEST_ASSERT(OSAPI_Thread_start(sem_thread->osapi_thread),
                        "could not start sem_thread",
                        goto done);
        }
    }
    UTEST_SETUP_END

    OSAPI_Thread_sleep(1000);

    while (max_wait_count && (thread_complete < MAX_SEM_THREAD))
    {
        OSAPI_Thread_sleep(1000);
        for (i = 0; (i < MAX_SEM_THREAD); i++)
        {
            if (sem_threads[i].test_ok)
            {
                thread_complete++;
            }
        }
        --max_wait_count;
    }

    retval = (unsigned char)(thread_complete == MAX_SEM_THREAD);

    for (i = 0; i < MAX_SEM_THREAD; ++i)
    {
#ifndef RTI_CERT
        /* Test: threads destroyed
         * Expect: Success
         */
        TEST_ASSERT(OSAPI_Thread_destroy(sem_threads[i].osapi_thread),
                    "could not destory thread",
                    return RTI_FALSE);
        /* Test: threads deleted
         * Expect: Success
         */
        TEST_ASSERT(OSAPI_Semaphore_delete(sem_threads[i].my_sem1),
                    "failed to delete semaphore", goto done);
#endif
    }

    retval = RTI_TRUE;

done:

    return retval;
}
#endif /* RTI_AUTOSAR */

RTI_PRIVATE struct UTEST_TestEntry OSAPI_SemaphoreTester_tests[]=
{
    RTITestCase("basic",
                OSAPI_SemaphoreTester_testSingle,
                TEST_ENABLED),

    RTITestCase("basic_multiple",
                OSAPI_SemaphoreTester_testMultiple,
                TEST_ENABLED),

#ifndef RTI_AUTOSAR
    /* This test needs to create a thread, 
     * which is not supported at the moment in Autosar
     */
    RTITestCase("thread",
                OSAPI_SemaphoreTester_testBasic,
                TEST_ENABLED),
#endif

    RTITestCase("timeout",
                OSAPI_SemaphoreTester_test_timeout,
                TEST_ENABLED),

#ifndef RTI_AUTOSAR
    /* This test needs to create a thread, 
     * which is not supported at the moment in Autosar
     */
    RTITestCase("timeout_mt",
                OSAPI_SemaphoreTester_test_timeout_mt,
                TEST_ENABLED)
#endif
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_SemaphoreTester,"semaphore")

