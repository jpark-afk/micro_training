/*
 * FILE: ThreadTester.c - Thread unit-test implementation
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
 * \brief Thread tester
 */
#include "test/test_setting.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_semaphore.h"

#include "ThreadTester.h"

/* ThreadX only have memory for OSAPI_PLATFORM_THREADX_MAX_THREADS thread 
 * stacks. The system implementation already creates 1 thread so this test 
 * will use only OSAPI_PLATFORM_THREADX_MAX_THREADS - 1 threads in ThreadX
 */
#ifndef RTI_THREADX
#define NUM_THREADS 10
#else
#define NUM_THREADS (OSAPI_PLATFORM_THREADX_MAX_THREADS - 1)
#endif
#define THREAD_PRIORITY 100
#define THREAD_STACK_SIZE 16384
#define THREAD_DELAY 3000
#define THREAD_WAKEUP_TIMES 10

RTI_PRIVATE RTI_INT32
OSAPI_ThreadTester_testBasicThread(struct OSAPI_ThreadInfo* thread_info)
{
    UNUSED_ARG(thread_info);

    return RTI_TRUE;
}

RTI_PRIVATE unsigned char
/* Test: threads created, spawned and then destroyed
 * Return: Success
 */
OSAPI_ThreadTester_testBasic(struct UTEST_Context* setting)
{
    UTEST_VAR int i;
    UTEST_VAR char name[32];
    UTEST_VAR unsigned char result = RTI_FALSE;
    UTEST_VAR struct OSAPI_Thread* threads[NUM_THREADS];
    UTEST_VAR struct OSAPI_ThreadProperty threadProps = OSAPI_THREAD_PROPERTY_DEFAULT;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        for (i = 0; i < NUM_THREADS; i++)
        {
            threads[i] = NULL;
        }

        /* we spawn many threads */
        for (i = 0; i < NUM_THREADS; i++)
        {
            UTEST_Stdio_snprintf(name,32, "thread_%d", i);
            threads[i] = OSAPI_Thread_create(name,
                                            &threadProps,
                                            OSAPI_ThreadTester_testBasicThread,
                                            NULL,/* masquerade i as ptr */
                                            NULL);
            TEST_ASSERT(threads[i] != NULL,
                        "could not spawn thread",
                        goto fail);
        }
    }
    UTEST_SETUP_END

    /* then we start all of them */
    for (i = 0; i < NUM_THREADS; i++)
    {
        TEST_ASSERT(OSAPI_Thread_start(threads[i]),
                    "could not start thread",
                    goto fail);
    }

    result = RTI_TRUE;

fail:
    /* and we finally make sure we can destroy them as well */
    for (i = 0; i < NUM_THREADS; i++)
    {
        if (!OSAPI_Thread_destroy(threads[i]))
        {
            UTEST_Stdio_printf("could not destroy thread %d!\n", i);
            return RTI_FALSE;
        }
    }

    return result;
}

typedef struct
{
    RTI_BOOL result;
    struct OSAPI_Semaphore* aliveSem;
    struct OSAPI_Semaphore* statusSem;
    struct OSAPI_Semaphore* wakeupSem;
    struct UTEST_Context* setting;
} OSAPI_ThreadTester_testAdvancedStruct;


/* Test: wakeup an advanced thread
 * Return: Success
 */
RTI_PRIVATE RTI_INT32
OSAPI_ThreadTester_testAdvancedThreadWakeup(struct OSAPI_ThreadInfo* threadInfo)
{
    OSAPI_ThreadTester_testAdvancedStruct* advancedStruct =
            (OSAPI_ThreadTester_testAdvancedStruct*)threadInfo->user_data;

    /* give semaphore to wakeup the thead */
    return OSAPI_Semaphore_give(advancedStruct->wakeupSem);
}

/* Test: AdvancedThreads given, woken up and get status
 * Return: Success
 */
RTI_PRIVATE RTI_INT32
OSAPI_ThreadTester_testAdvancedThread(struct OSAPI_ThreadInfo* threadInfo)
{
    OSAPI_ThreadTester_testAdvancedStruct* advancedStruct =
            (OSAPI_ThreadTester_testAdvancedStruct*)threadInfo->user_data;

    /* give semaphore to indicate we have started */
    if (!OSAPI_Semaphore_give(advancedStruct->statusSem))
    {
        UTEST_Stdio_printf("could not give status semaphore!\n");
    }

    while (!threadInfo->stop_thread)
    {
        /* wakeup routine gives this semaphore to wake us up */
        if (!OSAPI_Semaphore_take(advancedStruct->wakeupSem,
                OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
        {
            UTEST_Stdio_snprintf(advancedStruct->setting->test_last_error,50,"could not take wakeup semaphore");
            advancedStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }

        /* give semaphore to indicate that we're alive and well */
        if (!OSAPI_Semaphore_give(advancedStruct->aliveSem))
        {
            UTEST_Stdio_snprintf(advancedStruct->setting->test_last_error,50,"could not give status semaphore!");
            advancedStruct->result = RTI_FALSE;
            return RTI_FALSE;
        }
    }

    /* give semaphore to indicate we have exited */
    if (!OSAPI_Semaphore_give(advancedStruct->statusSem))
    {
        UTEST_Stdio_snprintf(advancedStruct->setting->test_last_error,50,"could not give status semaphore!");
        advancedStruct->result = RTI_FALSE;
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE unsigned char
OSAPI_ThreadTester_testAdvanced(struct UTEST_Context* setting)
{
    UTEST_VAR int i;
    UTEST_VAR unsigned char result = RTI_FALSE;
    UTEST_VAR struct OSAPI_Thread* advancedThread;
    UTEST_VAR OSAPI_ThreadTester_testAdvancedStruct advancedStruct;
    UTEST_VAR struct OSAPI_ThreadProperty threadProps = OSAPI_THREAD_PROPERTY_DEFAULT;
    UTEST_VAR RTI_INT32 fr;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        /* create 3 new semaphores - aliveSem, statusSem,wakeupSem */
        advancedStruct.result = RTI_TRUE;
        advancedStruct.aliveSem = OSAPI_Semaphore_new();
        advancedStruct.statusSem = OSAPI_Semaphore_new();
        advancedStruct.wakeupSem = OSAPI_Semaphore_new();
        advancedStruct.setting = setting;

        /* Test: semaphores created
            * Expect: Success
            */
        TEST_ASSERT(advancedStruct.statusSem && advancedStruct.wakeupSem,
                    "could not create semaphores",
                    return RTI_FALSE);

        /* create the thread */
        advancedThread = OSAPI_Thread_create("advanced", &threadProps,
                OSAPI_ThreadTester_testAdvancedThread, (void *)&advancedStruct,
                OSAPI_ThreadTester_testAdvancedThreadWakeup);

        /* Test: threads created
            * Expect: Success
            */
        TEST_ASSERT(advancedThread,
                    "could not create thread!",
                    return RTI_FALSE);

        /* Test: threads started
            * Expect: Success
            */
        TEST_ASSERT(OSAPI_Thread_start(advancedThread),
                    "could not start thread!",
                    goto fail);
    }
    UTEST_SETUP_END

    /* Test: semaphore is given when the thread has started
     * Expect: Success 
     */
    TEST_ASSERT(OSAPI_Semaphore_take(advancedStruct.statusSem, THREAD_DELAY, &fr),
                "thread hasn't started after we told it to",
                goto fail);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                goto fail);

    /* we wake the thread up several times */
    for (i = 0; i < THREAD_WAKEUP_TIMES; i++)
    {
        /* Test: thread woken up
         * Expect: Success 
         */
        TEST_ASSERT(OSAPI_Thread_wakeup(advancedThread),
                    "could not wakeup thread",
                    goto fail);

        /* Test: semaphore is given when the thread wakes up
         * Expect: Success 
         */
        TEST_ASSERT(OSAPI_Semaphore_take(advancedStruct.aliveSem, THREAD_DELAY, &fr),
                    "thread hasn't woken up after we told it to",
                    goto fail);
        TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                    "fail reason not OK",
                    goto fail);
    }

    result = RTI_TRUE;

fail:
    /* Test:  kill the thread
     * Expect: Success 
     */
    TEST_ASSERT(OSAPI_Thread_destroy(advancedThread),
                "could not destory thread",
                return RTI_FALSE);

    /* Test: semaphore is given when the thread has exited
     * Expect: Success 
     */
    TEST_ASSERT(OSAPI_Semaphore_take(advancedStruct.statusSem, THREAD_DELAY, &fr),
                "thread hasn't exited after we told it to!",
                return RTI_FALSE);
    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "fail reason not OK",
                return RTI_FALSE);

#ifndef RTI_CERT
    OSAPI_Semaphore_delete(advancedStruct.aliveSem);
    OSAPI_Semaphore_delete(advancedStruct.statusSem);
    OSAPI_Semaphore_delete(advancedStruct.wakeupSem);
#endif

    return (unsigned char)(result && advancedStruct.result == RTI_TRUE);
}

#ifndef RTI_CERT

RTI_PRIVATE RTI_INT32
OSAPI_ThreadTester_testBasicThreadWPriorty(struct OSAPI_ThreadInfo* thread_info)
{
    RTI_INT32 count = 0;

#if 0
    /* This test is commented out because it is unique to Linux and
     * needs root access to run properly
     */
    int policy;
    struct sched_param param;
    UTEST_Stdio_printf("started OSAPI_ThreadTester_testBasicThreadWPriorty\n");
    pthread_getschedparam(pthread_self(),&policy,&param);
    UTEST_Stdio_printf("my priorty is %d\n",param.sched_priority);
#endif

    while (!thread_info->stop_thread && (count++ < 20))
    {
    }

    return RTI_TRUE;
}

#endif

RTI_PRIVATE unsigned char
OSAPI_ThreadTester_test_priority(struct UTEST_Context *setting)
{
#if !defined(RTI_CERT)
    UTEST_VAR struct OSAPI_Thread *thread = NULL;
    UTEST_VAR struct OSAPI_ThreadProperty thread_props = OSAPI_THREAD_PROPERTY_DEFAULT;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        UTEST_Output_lua_write(setting,
                "rti_me_heap.osapi.thread={};\n");

        /* create the thread */
#if 0
        thread_props.priority = 20;
        /* This requires root access */
        thread_props.options |= OSAPI_THREAD_REALTIME_PRIORITY;
#endif
        thread = OSAPI_Thread_create("advanced", &thread_props,
                OSAPI_ThreadTester_testBasicThreadWPriorty, NULL,
                NULL);

        if (!OSAPI_Thread_start(thread))
        {
            return RTI_FALSE;
        }
    }
    UTEST_SETUP_END

    if (!OSAPI_Thread_destroy(thread))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */

}

RTI_PRIVATE struct UTEST_TestEntry OSAPI_ThreadTester_tests[]=
{
    RTITestCase("basic",
                OSAPI_ThreadTester_testBasic,
                TEST_ENABLED),

    RTITestCase("advanced",
                OSAPI_ThreadTester_testAdvanced,
                TEST_ENABLED),

    RTITestCase("priority",
                OSAPI_ThreadTester_test_priority,
                TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_ThreadTester,"thread")

