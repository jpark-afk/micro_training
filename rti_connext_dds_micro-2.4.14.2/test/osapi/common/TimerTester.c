/*
 * FILE: TimerTester.c - Implementation of Timer unit-test
 *
 * (c) Copyright, Real-Time Innovations, 2008-2021
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
 * 29jul2014,tk MICRO-839/PR#9645: Added tests
 * 11jun2013,tk Added tests for:
 *                - MICRO-240/PR#1412
 *                - MICRO-223/PR#1403
 *                - MICRO-221/PR#1401
 *                - MICRO-217/PR#1099
 * 22mar2012,tk Written
 */
/*ce
 * \file
 * \brief Timer tester
 */
#include "test/test_setting.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_timer.h"
#include "osapi/osapi_system.h"
#include "osapi/osapi_log.h"

#include "System.h"
#include "TimerTester.h"

#define OSAPI_TIMERTESTER_MAX_TIMERS (4)

RTI_PRIVATE OSAPI_Timer_T OSAPI_TimerTester_fv_Timers[OSAPI_TIMERTESTER_MAX_TIMERS];
RTI_PRIVATE OSAPI_TimerTickHandlerFunction OSAPI_TimerTester_fv_TimerTick[OSAPI_TIMERTESTER_MAX_TIMERS];

#ifndef RTI_CERT
RTI_PRIVATE RTI_INT32
OSAPI_TimerTester_ms_ticks(RTI_INT64 ms)
{
    RTI_INT64 us_res;
    RTI_INT64 ticks;

    /* OSAPI_System_get_timer_resolution() returns ns, work on ms resolution
     * in the test.
     */
    us_res = OSAPI_System_get_timer_resolution();

    ticks = (ms * 1000000) / us_res;
    if ((ms * 1000000) % us_res)
    {
        ++ticks;
    }

    if (ticks == 0)
    {
        ++ticks;
    }

    return (RTI_INT32)ticks;
}
#endif

#ifndef RTI_CERT
#ifndef RTI_ARINC653
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_manual_initialize(void)
{
    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_manual_finalize(void)
{
    return RTI_TRUE;
}
#endif /* !RTI_ARINC653 */
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_manual_start_timer(OSAPI_Timer_T self,
                               OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;

    for (i = 0; i < OSAPI_TIMERTESTER_MAX_TIMERS; ++i)
    {
        if (OSAPI_TimerTester_fv_Timers[i] == NULL)
        {
            break;
        }
    }

    if (i == OSAPI_TIMERTESTER_MAX_TIMERS)
    {
        return RTI_FALSE;
    }

    OSAPI_TimerTester_fv_Timers[i] = self;
    OSAPI_TimerTester_fv_TimerTick[i] = tick_handler;

    return RTI_TRUE;
}
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_manual_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    for (i = 0; i < OSAPI_TIMERTESTER_MAX_TIMERS; ++i)
    {
        if (OSAPI_TimerTester_fv_Timers[i] == self)
        {
            OSAPI_TimerTester_fv_Timers[i] = NULL;
        }
    }

    return RTI_TRUE;
}
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_manual_ticks(RTI_INT32 count)
{
    RTI_INT32 i,j;

    for (j = 0; j < count; ++j)
    {
        for (i = 0; i < OSAPI_TIMERTESTER_MAX_TIMERS; ++i)
        {
            if (OSAPI_TimerTester_fv_Timers[i] != NULL)
            {
                OSAPI_TimerTester_fv_TimerTick[i](OSAPI_TimerTester_fv_Timers[i]);
            }
        }
    }

    return RTI_TRUE;
}
#endif

RTI_PRIVATE struct OSAPI_SystemI OSAPI_TimerTester_fv_SysIntf;
RTI_PRIVATE struct OSAPI_SystemI OSAPI_TimerTester_fv_SysIntfSave;

#ifndef RTI_CERT

RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_initialize_manual(struct UTEST_Context *setting)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 i;

    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    for (i = 0; i < OSAPI_TIMERTESTER_MAX_TIMERS; ++i)
    {
        OSAPI_TimerTester_fv_Timers[i] = NULL;
    }

    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntf);
    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntfSave);

#ifndef RTI_ARINC653
    OSAPI_TimerTester_fv_SysIntf.initialize = OSAPI_TimerTester_manual_initialize;
    OSAPI_TimerTester_fv_SysIntf.finalize = OSAPI_TimerTester_manual_finalize;
#endif /* !RTI_ARINC653 */
    OSAPI_TimerTester_fv_SysIntf.start_timer = OSAPI_TimerTester_manual_start_timer;
    OSAPI_TimerTester_fv_SysIntf.stop_timer = OSAPI_TimerTester_manual_stop_timer;
    /* NOTE: Native resolution is used. Thus, use all calculations
     * must be based on OSAPI_System_get_timer_resolution().
     */

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntf),
                "failed to set timer functions",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                goto done);
#endif
    retval = RTI_TRUE;

done:

    return retval;
}
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_finalize_manual(struct UTEST_Context *setting)
{
    RTI_BOOL retval = RTI_FALSE;

#ifndef RTI_CERT
    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);
#endif

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntfSave),
                "failed to reset timer functions",
                goto done);
#ifndef RTI_ARINC653
    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);
#endif /* RTI_ARINC653 */

#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                goto done);
#endif

    retval = RTI_TRUE;

done:

    return retval;

}
#endif

#ifndef RTI_CERT
RTI_PRIVATE OSAPI_TimeoutOp_t
OSAPI_TimerTester_timeout_manual(struct OSAPI_TimeoutUserData *storage)
{
    UTEST_Log_debug("OSAPI_TimerTester_timeout_manual\n");

    (*(RTI_INT32*)storage->field[0])++;

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}
#endif

/*e \defgroup OSAPI_TimerUnitTest OSAPI_Timer Unit Test
 *   \ingroup OSAPIModuleTest
 *   This module tests OSAPI Timer
 */
struct OSAPI_NtpTime cb_start_time,cb_end_time;

/* Macros to simplify conversion of existing tests to the new OSAPI timer API */
#define sec_from_ms(ms_)    ((ms_)/1000)
#define ns_from_sec(ms_)    (((ms_) - ((ms_)/1000)*1000)*1000000)
#define ms_to_sec_nsec(ms_) sec_from_ms(ms_),ns_from_sec(ms_)

#ifndef RTI_CERT
RTI_PRIVATE OSAPI_TimeoutOp_t
/* Test:appends lua output?
 * Expect: Success
 */
OSAPI_TimerTester_timeout(struct OSAPI_TimeoutUserData *storage)
{
    struct OSAPI_NtpTime diff_time;
    RTI_INT32 sec_end;
    RTI_UINT32 ms_end;

    --storage->count[0];

    if (storage->count[0] == 0)
    {
        if ((storage->field[0] != NULL) &&
            !OSAPI_System_get_time((struct OSAPI_NtpTime*)storage->field[0]))
        {
        }

        if ((storage->field[1] != NULL) &&
            !OSAPI_Semaphore_give(storage->field[1]))
        {
        }
    }

    if (!OSAPI_System_get_time(&cb_end_time))
    {

    }

    OSAPI_NtpTime_subtract(&diff_time,&cb_end_time,&cb_start_time);
    OSAPI_NtpTime_to_millisec(&sec_end,&ms_end,&diff_time);
    cb_start_time = cb_end_time;

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_INT32
OSAPI_TimerTester_get_timeout_comp(RTI_INT32 timeout_val)
{
    RTI_INT32 timer_res;
    RTI_INT32 timeout_comp;

    timer_res = OSAPI_System_get_timer_resolution() / 1000;
    timeout_comp = (timeout_val*1000);

    if (timeout_comp % timer_res)
    {
        timeout_comp = ((timeout_comp/timer_res)+1)*timer_res;
    }

    return timeout_comp / 1000;
}
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_check_timeout(struct OSAPI_NtpTime *start_time,
                                struct OSAPI_NtpTime *end_time,
                                RTI_INT32 timeout,
                                RTI_INT32 count)

{
    RTI_INT32 timer_res;
    struct OSAPI_NtpTime diff_time;
    RTI_INT32 sec_end;
    RTI_UINT32 ms_end;
    RTI_INT32 timeout_comp;
    RTI_INT32 delta;
    RTI_UINT32 time_ms;
    RTI_BOOL result;

    timeout_comp = count*OSAPI_TimerTester_get_timeout_comp(timeout);
    timer_res = OSAPI_System_get_timer_resolution() / 1000;
    delta = (20*timer_res)/1000;

    OSAPI_NtpTime_subtract(&diff_time,end_time,start_time);
    OSAPI_NtpTime_to_millisec(&sec_end,&ms_end,&diff_time);

    time_ms = ((RTI_UINT32)sec_end * 1000U) + ms_end;

    result = ((RTI_INT32)time_ms >= (timeout_comp - delta)) &&
             ((RTI_INT32)time_ms <= (timeout_comp + delta));

    if (!result)
    {
        UTEST_Stdio_printf("time_ms = %d,timeout_comp=%d,delta=%d\n",
                            time_ms,timeout_comp,delta);
    }

    return result;
}
#endif

RTI_PRIVATE unsigned char
OSAPI_TimerTester_3s(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR OSAPI_Timer_T timer_2;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_2;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_2;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;

#if LOG_PRECONDITION_ENABLED
    UTEST_VAR struct OSAPI_TimeoutUserData out;
#endif
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR RTI_INT32 fr = 0;
    UTEST_VAR struct OSAPI_NtpTime start_time,end_time;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 32;
        mutex_1 = OSAPI_Mutex_new();
        mutex_2 = OSAPI_Mutex_new();
        wait_sem = OSAPI_Semaphore_new();

        timeout_val = 1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
        timer_2 = OSAPI_Timer_new(&property,mutex_2);
    }
    UTEST_SETUP_END

    test_result.field[0] = NULL;
    test_result.field[1] = NULL;
    test_result.count[0] = 10;

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 1s periodic timeout
     */
    if (!OSAPI_System_get_time(&cb_start_time))
    {

    }

#define OSAPI_MS_TO_NS(ms_) (ms_*1000000)
    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

#if LOG_PRECONDITION_ENABLED
    /*e \test
     * Test OSAPI_TimeoutHandle_get_user_data
     */
    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,NULL),
               "Test should failed",
               goto done);

    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,&timeout_1),
               "Test should failed",
               goto done);

    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(&out,NULL),
               "Test should failed",
               goto done);
#endif

    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_2,
                              &timeout_2,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 2",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start time",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,60*timeout_val,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,10),
                "incorrect sleep time",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 2s periodic timeout
     */
    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_2,&timeout_2),
                "failed to stop timer 2",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_2),
                "failed to delete timer 2",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 3s periodic timeout
     */

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete mutex_1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_2),
                "failed to delete mutex_1",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
#else
    CHECK_DO_RUN_TEST(setting);
    return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_3s_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR OSAPI_Timer_T timer_2;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_2;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_2;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 ticks;
    UTEST_VAR RTI_INT32 counter1 = 0;
    UTEST_VAR RTI_INT32 counter2 = 0 ;

#if LOG_PRECONDITION_ENABLED
    struct OSAPI_TimeoutUserData out = OSAPI_TimeoutUserData_INITIALIZER;
#endif

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 32;
        mutex_1 = OSAPI_Mutex_new();
        mutex_2 = OSAPI_Mutex_new();

        timeout_val = 1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
        timer_2 = OSAPI_Timer_new(&property,mutex_2);
    }
    UTEST_SETUP_END

    test_result.field[0] = &counter1;

#define OSAPI_MS_TO_NS(ms_) (ms_*1000000)
    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

#if LOG_PRECONDITION_ENABLED
    /*e \test
     * Test OSAPI_TimeoutHandle_get_user_data
     */
    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,NULL),
               "Test should failed",
               goto done);

    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,&timeout_1),
               "Test should failed",
               goto done);

    TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(&out,NULL),
               "Test should failed",
               goto done);
#endif

   test_result.field[0] = &counter2;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_2,
                              &timeout_2,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 2",
            goto done);

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    /* First timeout is + 1 */
    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter1 == 0,"Unexpected timeout",goto done);
    TEST_ASSERT(counter2 == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter1 == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter1); goto done);
    TEST_ASSERT(counter2 == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter2); goto done);

    /* The rest should be exact */
    OSAPI_TimerTester_manual_ticks(ticks * 9);
    TEST_ASSERT(counter1 == 10,"Unexpected timeout",goto done);
    TEST_ASSERT(counter2 == 10,"Unexpected timeout",goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 2s periodic timeout
     */
    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_2,&timeout_2),
                "failed to stop timer 2",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_2),
                "failed to delete timer 2",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 3s periodic timeout
     */

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete mutex_1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_2),
                "failed to delete mutex_1",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
            "failed to finalize manual timer",
            return RTI_FALSE);

    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_1s(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;

#if LOG_PRECONDITION_ENABLED
    UTEST_VAR struct OSAPI_TimeoutUserData out;
#endif
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR RTI_INT32 fr = 0;
    UTEST_VAR struct OSAPI_NtpTime start_time,end_time;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 32;
        mutex_1 = OSAPI_Mutex_new();
        wait_sem = OSAPI_Semaphore_new();

        timeout_val = 1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 1s periodic timeout
     */
    if (!OSAPI_System_get_time(&cb_start_time))
    {

    }

#define OSAPI_MS_TO_NS(ms_) (ms_*1000000)
    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

#if LOG_PRECONDITION_ENABLED
    /*e \test
     * Test OSAPI_TimeoutHandle_get_user_data
     */
   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,NULL),
               "Test should failed",
               goto done);

   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,&timeout_1),
               "Test should failed",
               goto done);

   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(&out,NULL),
               "Test should failed",
               goto done);
#endif

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start time",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,60*timeout_val,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,10),
                "incorrect sleep time",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 2s periodic timeout
     */
    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 3s periodic timeout
     */

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete mutex_1",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_1s_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 counter = 0;
    UTEST_VAR RTI_INT32 ticks;

#if LOG_PRECONDITION_ENABLED
    struct OSAPI_TimeoutUserData out = OSAPI_TimeoutUserData_INITIALIZER;
#endif

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 32;
        mutex_1 = OSAPI_Mutex_new();

        timeout_val = 1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    test_result.field[0] = &counter;

#define OSAPI_MS_TO_NS(ms_) (ms_*1000000)
    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

#if LOG_PRECONDITION_ENABLED
    /*e \test
     * Test OSAPI_TimeoutHandle_get_user_data
     */
   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,NULL),
               "Test should failed",
               goto done);

   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(NULL,&timeout_1),
               "Test should failed",
               goto done);

   TEST_ASSERT(!OSAPI_TimeoutHandle_get_user_data(&out,NULL),
               "Test should failed",
               goto done);
#endif

   ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

   /* First timeout is + 1 */
   OSAPI_TimerTester_manual_ticks(ticks);
   TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

   OSAPI_TimerTester_manual_ticks(1);
   TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",test_result.count[0]); goto done);

   /* The rest should be exact */
   OSAPI_TimerTester_manual_ticks(ticks * 9);
   TEST_ASSERT(counter == 10,"Unexpected timeout",goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 2s periodic timeout
     */
    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    /*e \test
     * \addtogroup OSAPI_TimerUnitTest
     * Test a 3s periodic timeout
     */

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete mutex_1",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
                "failed to finalize manual timer",
                return RTI_FALSE);

    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_221(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timer_res;
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR RTI_INT32 fr = 0;
    UTEST_VAR struct OSAPI_NtpTime start_time,end_time;
    UTEST_VAR RTI_INT32 timeout_val;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_res = OSAPI_System_get_timer_resolution() / 1000;
        timer_1 = OSAPI_Timer_new(&property,mutex_1);
        wait_sem = OSAPI_Semaphore_new();
    }
    UTEST_SETUP_END

    /* \test
     * Test that timeouts with period equal to integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;

    /* timeout_val is in ms */
    timeout_val = (property.max_slots * timer_res)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start time",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,
                            20*timeout_val,&fr),
                "failed to take semaphore",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "semaphore timed out",
                goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,10),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test that timeouts with period less than integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;
    timeout_val = ((property.max_slots-1) * timer_res)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start time",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,
                            20*timeout_val,&fr),
                "failed to take semaphore",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "semaphore timed out",
                goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,10),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test that timeouts with period greater than integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;
    timeout_val = ((property.max_slots+1) * timer_res) / 1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start time",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,
                            20*timeout_val,&fr),
                "failed to take semaphore",
                goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
                "semaphore timed out",
                goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,10),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_221_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timer_res;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 counter = 0;
    UTEST_VAR RTI_INT32 ticks;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_res = OSAPI_System_get_timer_resolution() / 1000;
        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    /* \test
     * Test that timeouts with period equal to integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    counter = 0;
    test_result.field[0] = &counter;

    /* timeout_val is in ms */
    timeout_val = (property.max_slots * timer_res)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    OSAPI_TimerTester_manual_ticks(10*ticks + 1);
    TEST_ASSERT(counter == 10,"Unexpected timeout",goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test that timeouts with period less than integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    counter = 0;
    test_result.field[0] = &counter;
    timeout_val = ((property.max_slots-1) * timer_res)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    OSAPI_TimerTester_manual_ticks(10*ticks + 1);
    TEST_ASSERT(counter == 10,"Unexpected timeout",goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test that timeouts with period greater than integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    counter = 0;
    test_result.field[0] = &counter;
    timeout_val = ((property.max_slots+1) * timer_res) / 1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    OSAPI_TimerTester_manual_ticks(10*ticks + 1);
    TEST_ASSERT(counter == 10,"Unexpected timeout",goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
                "failed to finalize manual timer",
                return RTI_FALSE);

    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_240(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR RTI_INT32 fr = 0;
    UTEST_VAR struct OSAPI_NtpTime start_time,end_time;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        wait_sem = OSAPI_Semaphore_new();

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
        timeout_val = 500;
    }
    UTEST_SETUP_END

    /* \test
     * Test that timeouts with period equal to integer multiple of timer
     * resolution and number of slots are scheduled correctly.
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 20;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,60 * timeout_val,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,20),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_240_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 counter = 0;
    UTEST_VAR RTI_INT32 ticks;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
        timeout_val = 500;
    }
    UTEST_SETUP_END

    /* \test
        * Test that timeouts with period equal to integer multiple of timer
        * resolution and number of slots are scheduled correctly.
        */
    counter = 0;
    test_result.field[0] = &counter;

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",test_result.count[0]); goto done);

    /* The rest should be exact */
    OSAPI_TimerTester_manual_ticks(ticks * 19);
    TEST_ASSERT(counter == 20,"Unexpected timeout",goto done);


    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
            "failed to finalize manual timer",
            return RTI_FALSE);

    return retval;
#else
    CHECK_DO_RUN_TEST(setting);
    return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_839(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
#define count_between(low_,high_) ((count_1 >= (low_)) && (count_1 <= (high_)))
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timer_res;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR struct OSAPI_NtpTime start_time,end_time;
    UTEST_VAR RTI_INT32 fr = 0;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();
        wait_sem = OSAPI_Semaphore_new();

        /* Timeouts are in ms */
        timer_res = OSAPI_System_get_timer_resolution()/1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    /* \test
     * Test timeout = 0
     */
    test_result.field[0] = &end_time;
    timeout_val = 0;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 1;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,1000,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,1),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout < resolution
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 1;

    timeout_val = ((timer_res)-1)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,timeout_val*10,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,1),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout != X*resolution
     */
    test_result.field[0] = &end_time;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 1;

    timeout_val = ((10*timer_res)-1)/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,timeout_val*10,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,1),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout == X*resolution
     */
    test_result.field[0] = &end_time;
    timeout_val = (10*(timer_res))/1000;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 1;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,5000,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,1),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);


    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;

#undef count_between
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_839_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
#define count_between(low_,high_) ((count_1 >= (low_)) && (count_1 <= (high_)))
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timer_res;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 counter = 0;
    UTEST_VAR RTI_INT32 ticks;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        /* Timeouts are in ms */
        timer_res = OSAPI_System_get_timer_resolution()/1000;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    /* \test
     * Test timeout = 0
     */
    counter = 0;
    test_result.field[0] = &counter;
    timeout_val = 0;

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("ticks=%d,count = %d\n",ticks,counter); goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("ticks=%d,count = %d\n",ticks,counter); goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout < resolution
     */
    counter = 0;
    test_result.field[0] = &counter;
    timeout_val = ((timer_res)-1)/1000;

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 2,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout != X*resolution
     */
    counter = 0;
    test_result.field[0] = &counter;

    timeout_val = ((10*timer_res)-1)/1000;

    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 2,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* \test
     * Test timeout == X*resolution
     */
    counter = 0;
    test_result.field[0] = &counter;
    timeout_val = (10*(timer_res))/1000;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    OSAPI_TimerTester_manual_ticks(9*ticks);
    TEST_ASSERT(counter == 10,"Unexpected timeout",UTEST_Log_error("count = %d\n",counter); goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
                "failed to finalize manual timer",
                return RTI_FALSE);

    return retval;

#undef count_between
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_secnsec(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
#define count_between(low_,high_) ((count_1 >= (low_)) && (count_1 <= (high_)))
    unsigned char retval = RTI_FALSE;
    OSAPI_Timer_T timer_1;
    struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    OSAPI_TimeoutHandle_T timeout_1;
    struct OSAPI_Mutex *mutex_1;
    struct OSAPI_TimeoutUserData test_result = OSAPI_TimeoutUserData_INITIALIZER;
    RTI_INT32 timeout_val;
    OSAPI_Semaphore_T *wait_sem;
    struct OSAPI_NtpTime start_time,end_time;
    RTI_INT32 fr = 0;

    CHECK_DO_RUN_TEST(setting);

    property.max_entries = 128;
    property.max_slots = 3;
    mutex_1 = OSAPI_Mutex_new();
    wait_sem = OSAPI_Semaphore_new();

    timer_1 = OSAPI_Timer_new(&property,mutex_1);

    /* \test
     * Test timeout = 0
     */
    test_result.field[0] = &end_time;
    timeout_val = 5129;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 1;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout,
                              &test_result),
            "failed to start timer 1",
            goto done);

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get start timer",
                goto done);

    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,5*timeout_val,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_TimerTester_check_timeout(&start_time,&end_time,
                                                timeout_val,1),
                "incorrect sleep time",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);


    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;

#undef count_between
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_secnsec_manual(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
#define count_between(low_,high_) ((count_1 >= (low_)) && (count_1 <= (high_)))
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 ticks;
    UTEST_VAR RTI_INT32 counter = 0;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_TimerTester_initialize_manual(setting),
                    "failed to initialize manual timer",
                    goto done);

        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    /* \test
     * Test timeout = 0
     */
    test_result.field[0] = &counter;
    timeout_val = 5129;
    ticks = OSAPI_TimerTester_ms_ticks(timeout_val);

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_manual,
                              &test_result),
            "failed to start timer 1",
            goto done);

    /* First timeout is + 1 */
    OSAPI_TimerTester_manual_ticks(ticks);
    TEST_ASSERT(counter == 0,"Unexpected timeout",goto done);

    OSAPI_TimerTester_manual_ticks(1);
    TEST_ASSERT(counter == 1,"Unexpected timeout",UTEST_Log_error("count = %d\n",test_result.count[0]); goto done);

    /* The rest should be exact */
    OSAPI_TimerTester_manual_ticks(ticks * 9);
    TEST_ASSERT(counter == 10,"Unexpected timeout",goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);

    retval = RTI_TRUE;

done:

    TEST_ASSERT(OSAPI_TimerTester_finalize_manual(setting),
                "failed to finalize manual timer",
                return RTI_FALSE);

    return retval;

#undef count_between
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

#ifndef RTI_CERT
RTI_PRIVATE OSAPI_TimeoutOp_t
OSAPI_TimerTester_timeout_NNN(struct OSAPI_TimeoutUserData *storage)
{
    --storage->count[0];

    if (storage->count[0] == 0)
    {
        if ((storage->field[1] != NULL) &&
            !OSAPI_Semaphore_give(storage->field[1]))
        {

        }
    }


    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}
#endif

RTI_PRIVATE unsigned char
OSAPI_TimerTester_MICRO_1617(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
#define count_between(low_,high_) ((count_1 >= (low_)) && (count_1 <= (high_)))
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_2;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR struct OSAPI_TimeoutUserData test_result;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR OSAPI_Semaphore_T *wait_sem;
    UTEST_VAR RTI_INT32 fr = 0;
    UTEST_VAR RTI_INT32 timer_res;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_res = OSAPI_System_get_timer_resolution() / 1000;
        wait_sem = OSAPI_Semaphore_new();

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    /* \test
     * Test timeout = 0
     */
    timeout_val = (property.max_slots * timer_res)/1000;
    test_result.field[1] = wait_sem;
    test_result.count[0] = 10;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_NNN,
                              &test_result),
            "failed to start timer 1",
            goto done);


    test_result.field[1] = NULL;

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_2,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_NNN,
                              &test_result),
            "failed to start timer 1",
            goto done);


    TEST_ASSERT(OSAPI_Semaphore_take(wait_sem,OSAPI_SEMAPHORE_TIMEOUT_INFINITE,&fr),
            "failed to take semaphore",
            goto done);

    TEST_ASSERT(fr == OSAPI_SEMAPHORE_RESULT_OK,
            "semaphore timed out",
            goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_2),
                "failed to stop timer 1",
                goto done);

    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);

    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "failed to delete timer mutex",
                goto done);


    TEST_ASSERT(OSAPI_Semaphore_delete(wait_sem),
                "failed to delete timer semaphore",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;

#undef count_between
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_gettimeofday(struct UTEST_Context *setting)
{
#if !defined(RTI_CERT) && defined(RTI_LINUX)
    unsigned char retval = RTI_FALSE;
    RTI_INT32 i;
    RTI_INT32 rval;
    struct OSAPI_NtpTime start_time,end_time,diff;
    RTI_INT32 sec;
    RTI_UINT32 msec;
    struct timespec this_iter_start_time, next_iter_start_time;
    RTI_UINT32 max_diff=0,min_diff=0xffffffff;

    CHECK_DO_RUN_TEST(setting);

    if (clock_gettime(CLOCK_REALTIME, &this_iter_start_time) == -1)
    {
        return RTI_FALSE;
    }

    for (i = 0; i < 1000; ++i)
    {
        next_iter_start_time = this_iter_start_time;
        next_iter_start_time.tv_nsec += 100000000;

        /* tv_nsec is 0 - 999999999 */
        next_iter_start_time.tv_nsec %= 1000000000;

        /* If nsec wrapped around, increment sec */
        if (next_iter_start_time.tv_nsec < this_iter_start_time.tv_nsec)
        {
            ++next_iter_start_time.tv_sec;
        }

        TEST_ASSERT(OSAPI_System_get_time(&start_time),
                    "failed to get start_time",
                    goto done);

        do
        {
            rval = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,
                                   &next_iter_start_time, NULL);
        } while (((rval == -1) && (errno == EINTR)));

        TEST_ASSERT(OSAPI_System_get_time(&end_time),
                    "failed to get end_time",
                    goto done);
        OSAPI_NtpTime_subtract(&diff,&end_time,&start_time);
        OSAPI_NtpTime_to_millisec(&sec,&msec,&diff);

        if (max_diff < msec)
        {
            max_diff = msec;
        }

        if (min_diff > msec)
        {
            min_diff = msec;
        }

        this_iter_start_time = next_iter_start_time;
    }

    UTEST_Stdio_printf("min_diff = %d,min_diff = %d\n",min_diff,max_diff);

    retval = RTI_TRUE;

done:
    return retval;

#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

#ifndef RTI_CERT
RTI_PRIVATE OSAPI_TimeoutOp_t
OSAPI_TimerTester_timeout_resolution(struct OSAPI_TimeoutUserData *storage)
{
    UNUSED_ARG(storage);
    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}
#endif

RTI_PRIVATE unsigned char
OSAPI_TimerTester_resolution(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;
    UTEST_VAR RTI_INT32 timeout_val;
    UTEST_VAR RTI_INT32 timer_res;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 128;
        property.max_slots = 3;
        mutex_1 = OSAPI_Mutex_new();

        timer_res = OSAPI_System_get_timer_resolution();

        timeout_val = timer_res;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              ms_to_sec_nsec(timeout_val),
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_resolution,
                              NULL),
            "failed to start timer 1",
            goto done);


    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);
    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);


    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "Failed to delete mutex",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;

#else
   CHECK_DO_RUN_TEST(setting);

   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_PRIVATE OSAPI_TimeoutOp_t
OSAPI_TimerTester_timeout_1000min(struct OSAPI_TimeoutUserData *storage)
{
    UNUSED_ARG(storage);
    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}

RTI_PRIVATE unsigned char
OSAPI_TimerTester_boundary(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR OSAPI_Timer_T timer_1;
    UTEST_VAR struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    UTEST_VAR OSAPI_TimeoutHandle_T timeout_1;
    UTEST_VAR struct OSAPI_Mutex *mutex_1;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        property.max_entries = 60;
        property.max_slots = 30;
        mutex_1 = OSAPI_Mutex_new();

        timer_1 = OSAPI_Timer_new(&property,mutex_1);
    }
    UTEST_SETUP_END

    TEST_ASSERT(OSAPI_Timer_create_timeout(timer_1,
                              &timeout_1,
                              1000 * 600,
                              0,
                              OSAPI_TIMER_PERIODIC,
                              OSAPI_TimerTester_timeout_1000min,
                              NULL),
            "failed to start timer 1",
            goto done);


    OSAPI_Thread_sleep(5000);

    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                "failed to stop timer 1",
                goto done);
    /* Cleanup test */
    TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                "failed to delete timer 1",
                goto done);


    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "Failed to delete mutex",
                goto done);

    retval = RTI_TRUE;

done:
    return retval;

#else
   CHECK_DO_RUN_TEST(setting);

   return RTI_TRUE;
#endif /* !RTI_TRUE */
}

RTI_INT32 OSAPI_TimerTester_fv_TimerResolution = 33000000;

/* NOTE: This structure is identical in layout to the one in Timer.c, but
 * since it is not in a header-file this one was created instead in order
 * test without adding new APIs.
 */
struct OSAPI_TimerTesterEntry
{
    void *ptr1;
    void *ptr2;

    RTI_INT32 start_wheel_count;

    RTI_INT32 start_ticks;

    /* times aound the wheel */
    RTI_INT32 wheel_count;

    /* User-data stored with the time-out */
    struct OSAPI_TimeoutUserData user_data;

    /* In case an antry is re-used */
    RTI_INT32 epoch;

    /* user callback */
    OSAPI_TimeoutFunction_T timeout;

    RTI_INT32 flags;
};

#ifndef RTI_ARINC653
RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_initialize(void)
{
    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_finalize(void)
{
    return RTI_TRUE;
}
#endif /* !RTI_ARINC653 */

RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_start_timer(OSAPI_Timer_T self,
                               OSAPI_TimerTickHandlerFunction tick_handler)
{
    OSAPI_TimerTester_fv_Timers[0] = self;
    OSAPI_TimerTester_fv_TimerTick[0] = tick_handler;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_TimerTester_stop_timer(OSAPI_Timer_T self)
{
    UNUSED_ARG(self);
    OSAPI_TimerTester_fv_Timers[0] = NULL;
    OSAPI_TimerTester_fv_TimerTick[0] = NULL;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_INT32
OSAPI_TimerTester_get_timer_resolution(void)
{
    return OSAPI_TimerTester_fv_TimerResolution;
}

struct OSAPI_TimerTesterData
{
    RTI_INT32 retval;

    RTI_INT32 max_slots;

    RTI_INT32 resolution;

    RTI_INT32 seconds;

    RTI_INT32 nanosec;

    RTI_INT32 wheel_count;

    RTI_INT32 tick_count;
};

#if RTIME_USE_32BIT_TIMER_MATH
RTI_PRIVATE struct OSAPI_TimerTesterData OSAPI_TimerTesterData_fv_TimerData[]=
{
    {
        RTI_TRUE,4,33000000,35,129000000,266,1
    },
    {
        RTI_TRUE,32,33000000,1,0,0,31
    },
    /* retval,slots,resolution,sec,nsec,expected wheel,expected ticks */
    {
        RTI_TRUE,4,2033000000,500000,3,61485,2
    },
        /* retval,slots,resolution,sec,nsec,expected wheel,expected ticks */
    {
        RTI_TRUE,32,33000000,1,0,0,31
    },
    {
        RTI_TRUE,32,10000000,1,0,3,4
    },
    {
        RTI_TRUE,32,10000000,600000,0,1875000,0
    },
    {
        RTI_TRUE,159,10000000,600000,0,377358,78
    },
    {
        RTI_TRUE,159,10000000,60000000,0,37735849,9
    },
    {
        RTI_TRUE,159,1000000,60000000,0,377358490,90
    },
    {
        RTI_FALSE,159,1000000,600000000,0,377358490,90
    },
    {
        RTI_TRUE,10,10000000,0,238000000,2,4
    },
    {
        RTI_TRUE,10,1000000,0,238000000,23,8
    },
    {
        RTI_TRUE,10,1000000000,0,238000000,0,1
    },
    /* 10 */
    {
        RTI_TRUE,10,1000000000,5,0,0,5
    },
    {
        RTI_TRUE,1,1000000000,100,0,100,0
    },
    {
        RTI_TRUE,10,10000000,31540000,0,315400000,0
    },
    {
        RTI_TRUE,159,10000000,31540000,0,19836477,157
    },
    {
        RTI_TRUE,3,9998480,5,129000000,171,0
    },
    {
        RTI_TRUE,3,16666666,5,129000000,102,2
    },
    {
        RTI_TRUE,3,9998480,1,0,33,2
    },
    {
        RTI_TRUE,21,9998480,31536000,0,150194258,3
    }
};

RTI_INT32 OSAPI_TimerTesterData_fv_TimerDataCount =
        sizeof(OSAPI_TimerTesterData_fv_TimerData) / sizeof(struct OSAPI_TimerTesterData);

RTI_PRIVATE unsigned char
OSAPI_TimerTester_calculation(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    OSAPI_Timer_T timer_1;
    struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    struct OSAPI_Mutex *mutex_1;
    struct OSAPI_TimerTesterEntry *entry;
    OSAPI_TimeoutHandle_T timeout_1;
    RTI_INT32 j;

    CHECK_DO_RUN_TEST(setting);

#ifndef RTI_CERT
    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
            "failed to set system properties",
            goto done);
#endif

    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntf);
    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntfSave);

#ifndef RTI_ARINC653
    OSAPI_TimerTester_fv_SysIntf.initialize = OSAPI_TimerTester_initialize;
    OSAPI_TimerTester_fv_SysIntf.finalize = OSAPI_TimerTester_finalize;
#endif /* !RTI_ARINC653 */
    OSAPI_TimerTester_fv_SysIntf.start_timer = OSAPI_TimerTester_start_timer;
    OSAPI_TimerTester_fv_SysIntf.stop_timer = OSAPI_TimerTester_stop_timer;
    OSAPI_TimerTester_fv_SysIntf.get_timer_resolution = OSAPI_TimerTester_get_timer_resolution;

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntf),
                "failed to set timer functions",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                goto done);
#endif

    mutex_1 = OSAPI_Mutex_new();
    property.max_entries = 128;

    for (j = 0; j < OSAPI_TimerTesterData_fv_TimerDataCount; ++j)
    {
        property.max_slots = OSAPI_TimerTesterData_fv_TimerData[j].max_slots;
        OSAPI_TimerTester_fv_TimerResolution = OSAPI_TimerTesterData_fv_TimerData[j].resolution;

        timer_1 = OSAPI_Timer_new(&property,mutex_1);

        TEST_ASSERT(OSAPI_TimerTesterData_fv_TimerData[j].retval == OSAPI_Timer_create_timeout(
                                   timer_1,
                                   &timeout_1,
                                   OSAPI_TimerTesterData_fv_TimerData[j].seconds,
                                   OSAPI_TimerTesterData_fv_TimerData[j].nanosec,
                                   OSAPI_TIMER_PERIODIC,
                                   OSAPI_TimerTester_timeout_1000min,
                                   NULL),
                    "failed to start timer 1",
                    UTEST_Stdio_printf("j = %d\n",j);
                    goto done);

        if (OSAPI_TimerTesterData_fv_TimerData[j].retval)
        {

            entry = (struct OSAPI_TimerTesterEntry*)timeout_1._entry;

            TEST_ASSERT((OSAPI_TimerTesterData_fv_TimerData[j].wheel_count == entry->start_wheel_count) &&
                        (OSAPI_TimerTesterData_fv_TimerData[j].tick_count == entry->start_ticks),
                        "incorrect wheel/tick count",
                        UTEST_Log_error("j=%d, wheel_count/expected = %d/%d, ticks/expected = %d/%d\n",
                                        j,
                                        entry->start_wheel_count,OSAPI_TimerTesterData_fv_TimerData[j].wheel_count,
                                        entry->start_ticks,OSAPI_TimerTesterData_fv_TimerData[j].tick_count);
                        goto done);

            UTEST_Log_debug("j=%d wheel_count/expected = %d/%d, ticks/expected = %d/%d\n",
                            j,
                            entry->start_wheel_count,OSAPI_TimerTesterData_fv_TimerData[j].wheel_count,
                            entry->start_ticks,OSAPI_TimerTesterData_fv_TimerData[j].tick_count);

            TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                    "failed to stop timer 1",
                    goto done);

        }

#ifndef RTI_CERT
        /* Cleanup test */
        TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                    "failed to delete timer 1",
                    goto done);
#endif
    }

#ifndef RTI_CERT
    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "Failed to delete mutex",
                goto done);
#endif

    retval = RTI_TRUE;

done:

#ifndef RTI_CERT
    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", ;);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);
#endif

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntfSave),
                "failed to reset timer functions",
                ;);
    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                ;);
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                ;);
#endif
    return retval;
}
#endif

RTI_PRIVATE const RTI_INT32 OSAPI_TimerTester_fv_Resolutions[]=
{
    /* us resolutions */
    100000,900000,999999,

    /* ms resolutions */
    1000000,3000000,9998480,10000000,33000000,98000000,100000000,133000000,

    /* sec resolutions */
    1000000000,2000000000,0x7fffffffL
};

#define MAX_RES_INDEX (sizeof(OSAPI_TimerTester_fv_Resolutions) / sizeof(RTI_INT32))

RTI_PRIVATE const struct OSAPI_TimerTesterData OSAPI_TimerTesterData_fv_TimerData2[]=
{
    /* retval,slots,resolution,sec,nsec,expected wheel,expected ticks */
    {
        RTI_TRUE,4,2033000000,500000,3,61485,2
    },
        /* retval,slots,resolution,sec,nsec,expected wheel,expected ticks */
    {
        RTI_TRUE,32,33000000,1,0,0,31
    },
    {
        RTI_TRUE,32,10000000,1,0,3,4
    },
    {
        RTI_TRUE,32,10000000,600000,0,1875000,0
    },
    {
        RTI_TRUE,159,10000000,600000,0,377358,78
    },
    {
        RTI_TRUE,159,10000000,60000000,0,37735849,9
    },
    {
        RTI_TRUE,159,1000000,60000000,0,377358490,90
    },
#if 0
    {
        RTI_TRUE,159,1000000,600000000,0,377358490,90
    },
#endif

    {
        RTI_TRUE,10,10000000,0,238000000,2,4
    },
    {
        RTI_TRUE,10,1000000,0,238000000,23,8
    },
    {
        RTI_TRUE,10,1000000000,0,238000000,0,1
    },
    /* 10 */
    {
        RTI_TRUE,10,1000000000,5,0,0,5
    },
    {
        RTI_TRUE,1,1000000000,100,0,100,0
    },
    {
        RTI_TRUE,10,10000000,31540000,0,315400000,0
    },
    {
        RTI_TRUE,159,10000000,31540000,0,19836477,157
    },
    {
        RTI_TRUE,3,9998480,5,129000000,171,0
    },
    {
        RTI_TRUE,3,16666666,5,129000000,102,2
    },
    {
        RTI_TRUE,3,9998480,1,0,33,2
    },
    {
        RTI_TRUE,21,9998480,31536000,0,150194531,19
    }
};

RTI_INT32 OSAPI_TimerTesterData_fv_TimerDataCount2 =
        sizeof(OSAPI_TimerTesterData_fv_TimerData2) / sizeof(struct OSAPI_TimerTesterData);

RTI_PRIVATE unsigned char
OSAPI_TimerTester_calculation2(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    OSAPI_Timer_T timer_1;
    struct OSAPI_TimerProperty property = OSAPI_TimerProperty_INITIALIZER;
    struct OSAPI_Mutex *mutex_1;
    struct OSAPI_TimerTesterEntry *entry;
    OSAPI_TimeoutHandle_T timeout_1;
    RTI_INT32 j;
    RTI_UINT32 res_index;
    RTI_INT32 local_wheel_count;
    RTI_INT32 local_start_ticks;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

#ifndef RTI_CERT
    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);
#endif

    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntf);
    OSAPI_System_get_native_interface(&OSAPI_TimerTester_fv_SysIntfSave);

#ifndef RTI_ARINC653
    OSAPI_TimerTester_fv_SysIntf.initialize = OSAPI_TimerTester_initialize;
    OSAPI_TimerTester_fv_SysIntf.finalize = OSAPI_TimerTester_finalize;
#endif /* !RTI_ARINC653 */
    OSAPI_TimerTester_fv_SysIntf.start_timer = OSAPI_TimerTester_start_timer;
    OSAPI_TimerTester_fv_SysIntf.stop_timer = OSAPI_TimerTester_stop_timer;
    OSAPI_TimerTester_fv_SysIntf.get_timer_resolution = OSAPI_TimerTester_get_timer_resolution;

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntf),
                "failed to set timer functions",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                goto done);
#endif

    mutex_1 = OSAPI_Mutex_new();
    property.max_entries = 128;

    for (property.max_slots = 32 /*OSAPI_TIMER_MIN_SLOTS*/; property.max_slots <= OSAPI_TIMER_MAX_SLOTS; property.max_slots++)
    {
        for (res_index = 0; res_index < MAX_RES_INDEX; res_index++)
        {
            for (j = 7; j < OSAPI_TimerTesterData_fv_TimerDataCount2; ++j)
            {
                OSAPI_TimerTester_fv_TimerResolution = OSAPI_TimerTester_fv_Resolutions[res_index];

                timer_1 = OSAPI_Timer_new(&property,mutex_1);

                UTEST_Log_debug("Test %u/%u/%d: slots = %d, res =%d,timeout=%d.%d\n",
                       property.max_slots,res_index,j,
                       property.max_slots,OSAPI_TimerTester_fv_TimerResolution,
                       OSAPI_TimerTesterData_fv_TimerData2[j].seconds,
                       OSAPI_TimerTesterData_fv_TimerData2[j].nanosec);

                TEST_ASSERT(OSAPI_TimerTesterData_fv_TimerData2[j].retval ==
                        OSAPI_Timer_create_timeout(
                                                timer_1,
                                                &timeout_1,
                                                OSAPI_TimerTesterData_fv_TimerData2[j].seconds,
                                                OSAPI_TimerTesterData_fv_TimerData2[j].nanosec,
                                                OSAPI_TIMER_PERIODIC,
                                                OSAPI_TimerTester_timeout_1000min,
                                                NULL),
                           "Failed to create timer_1",
                           UTEST_Log_error("Test %u/%u/%d: slots = %d, res =%d,timeout=%d.%d\n",
                                  property.max_slots,res_index,j,
                                  property.max_slots,OSAPI_TimerTester_fv_TimerResolution,
                                  OSAPI_TimerTesterData_fv_TimerData2[j].seconds,
                                  OSAPI_TimerTesterData_fv_TimerData2[j].nanosec);
                            goto done);

                if (OSAPI_TimerTesterData_fv_TimerData2[j].retval)
                {
                    RTI_UINT64 total_ticks;
                    RTI_INT32 max_slots;

                    total_ticks =
                            (((RTI_UINT64)OSAPI_TimerTesterData_fv_TimerData2[j].seconds * 1000000000LL) +
                              (RTI_UINT64)OSAPI_TimerTesterData_fv_TimerData2[j].nanosec) / (RTI_UINT64)OSAPI_TimerTester_fv_TimerResolution;

                    if ((((RTI_UINT64)OSAPI_TimerTesterData_fv_TimerData2[j].seconds * 1000000000LL) +
                          (RTI_UINT64)OSAPI_TimerTesterData_fv_TimerData2[j].nanosec) % (RTI_UINT64)OSAPI_TimerTester_fv_TimerResolution)
                    {
                        total_ticks++;
                    }

                    max_slots = property.max_slots;
                    if ((OSAPI_TimerTester_fv_TimerResolution < 1000000LL) ||
                        (max_slots >  OSAPI_TIMER_MAX_SLOTS))
                    {
                        max_slots = OSAPI_TIMER_MAX_SLOTS;
                    }
                    else if (max_slots < OSAPI_TIMER_MIN_SLOTS)
                    {
                        max_slots = OSAPI_TIMER_MIN_SLOTS;
                    }

                    /* The test would not get here if this would cause a 32-bit
                     * overflow.
                     */
                    local_wheel_count = (RTI_INT32)(total_ticks / (RTI_UINT64)max_slots);
                    local_start_ticks = (RTI_INT32)(total_ticks % (RTI_UINT64)max_slots);

                    entry = (struct OSAPI_TimerTesterEntry*)timeout_1._entry;

                    TEST_ASSERT((local_wheel_count == entry->start_wheel_count) &&
                                (local_start_ticks == entry->start_ticks),
                                "incorrect wheel/tick count",
                                UTEST_Log_error("j=%d, wheel_count/expected = %d/%d, ticks/expected = %d/%d\n",
                                                j,
                                                entry->start_wheel_count,local_wheel_count,
                                                entry->start_ticks,local_start_ticks);
                                goto done);

                    UTEST_Log_debug("NNN j=%d, wheel_count/expected = %d/%d, ticks/expected = %d/%d\n",
                                    j,
                                    entry->start_wheel_count,local_wheel_count,
                                    entry->start_ticks,local_start_ticks);

                    TEST_ASSERT(OSAPI_Timer_delete_timeout(timer_1,&timeout_1),
                            "failed to stop timer 1",
                            goto done);

                }

#ifndef RTI_CERT

                /* Cleanup test */
                TEST_ASSERT(OSAPI_Timer_delete(timer_1),
                            "failed to delete timer 1",
                            goto done);
#endif
            }
        }
    }

#ifndef RTI_CERT
    TEST_ASSERT(OSAPI_Mutex_delete(mutex_1),
                "Failed to delete mutex",
                goto done);
#endif

    retval = RTI_TRUE;

done:

#ifndef RTI_CERT
    OSAPI_System_finalize();
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", ;);
#endif
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);
#endif

    TEST_ASSERT(OSAPI_System_set_interface(&OSAPI_TimerTester_fv_SysIntfSave),
                "failed to reset timer functions",
                ;);
    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                ;);
#if defined(RTI_AUTOSAR)
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize",
                ;);
#endif

    return retval;
}

RTI_PRIVATE struct UTEST_TestEntry OSAPI_TimerTester_tests[]=
{
    RTITestCase("1s",
                OSAPI_TimerTester_1s,
                TEST_DISABLED),

    RTITestCase("1s_manual",
                OSAPI_TimerTester_1s_manual,
                TEST_ENABLED),

    RTITestCase("3s",
                OSAPI_TimerTester_3s,
                TEST_DISABLED),

    RTITestCase("3s_manual",
                OSAPI_TimerTester_3s_manual,
                TEST_ENABLED),

    RTITestCaseTags("MICRO-221",
                    OSAPI_TimerTester_MICRO_221,
                    TEST_DISABLED,"MICRO-221"),

    RTITestCaseTags("MICRO-221_manual",
                    OSAPI_TimerTester_MICRO_221_manual,
                    TEST_ENABLED,"MICRO-221"),

    RTITestCaseTags("MICRO-240",
                    OSAPI_TimerTester_MICRO_240,
                    TEST_DISABLED,"MICRO-240"),

    RTITestCaseTags("MICRO-240_manual",
                    OSAPI_TimerTester_MICRO_240_manual,
                    TEST_ENABLED,"MICRO-240"),

    RTITestCaseTags("MICRO-839",
                    OSAPI_TimerTester_MICRO_839,
                    TEST_DISABLED,"MICRO-839"),

#ifndef RTI_FREERTOS
    RTITestCaseTags("MICRO-839_manual",
                    OSAPI_TimerTester_MICRO_839_manual,
                    TEST_ENABLED,"MICRO-839"),
#endif

    RTITestCaseTags("MICRO-1617",
                    OSAPI_TimerTester_MICRO_1617,
                    TEST_ENABLED,"MICRO-1617"),

    RTITestCase("sec_nsec",
                OSAPI_TimerTester_secnsec,
                TEST_DISABLED),

    RTITestCase("sec_nsec_manual",
                OSAPI_TimerTester_secnsec_manual,
                TEST_ENABLED),

    RTITestCase("gettimeofday",
                OSAPI_TimerTester_gettimeofday,
                TEST_DISABLED),

    RTITestCase("resolution",
                OSAPI_TimerTester_resolution,
                TEST_ENABLED),

#if RTIME_USE_32BIT_TIMER_MATH
    RTITestCase("calculation",
                OSAPI_TimerTester_calculation,
                TEST_ENABLED),
#endif

#ifndef RTI_AUTOSAR
    RTITestCase("calculation2",
                OSAPI_TimerTester_calculation2,
                TEST_ENABLED),
#endif

#ifndef RTI_FREERTOS
    RTITestCase("boundary",
                OSAPI_TimerTester_boundary,
                TEST_ENABLED)
#endif /* RTI_FREERTOS */
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_TimerTester,"timer")
