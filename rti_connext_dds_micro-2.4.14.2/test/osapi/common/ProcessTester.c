/*
 * FILE: ProcessTester.c - Process unit-test
 *
 * (c) Copyright, Real-Time Innovations, 2016-2021
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
 * 15sep2016,eh  Written
 */
/*ce
 * \file
 * \brief ProcessTester
 */

#include "test/test_setting.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"

#include "ProcessTester.h"

/*e
 * \brief Test OSAPI OSAPI_Process_pid_as_string().
 *
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_ProcessTester_test_pid_as_string(struct UTEST_Context *setting)
{
    unsigned char result = RTI_FALSE;
    char buffer[7];

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY
    
    /* Test: max length > pid's strlen + 1
     * Expect: Success, return pid's strlen
     */
    buffer[0] = 0;
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,7,(OSAPI_ProcessId)0xdeadc) == 5,
            "failed string length 5",
            goto done);

    TEST_ASSERT(!OSAPI_String_cmp(buffer,"deadc"),
            "failed conversion deadc",
            goto done);  

    /* Test: max length = pid's strlen - 2
     * Expect: Failure, return max length, truncated string with NUL terminator
     */
    buffer[0] = 0;
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,3,(OSAPI_ProcessId)0xdeadc) == 3,
            "failed string length 3",
            goto done);
    TEST_ASSERT(!OSAPI_String_cmp(buffer,"de"),
            "failed conversion de",
            goto done);
    
    /* Test: max length = pid's strlen - 1
     * Expect: Failure, return max length, truncated string with NUL terminator
     */
    buffer[0] = 0;
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,4,(OSAPI_ProcessId)0xdeadc) == 4,
            "failed string length 4",
            goto done);
    TEST_ASSERT(!OSAPI_String_cmp(buffer,"dea"),
            "failed conversion dead",
            goto done);

    /* Test: max length = pid's strlen
     * Expect: Failure, return max length, truncated string with NUL terminator
     */
    buffer[0] = 0;
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,5,(OSAPI_ProcessId)0xdeadc) == 5,
            "failed string length 5",
            goto done);
    TEST_ASSERT(!OSAPI_String_cmp(buffer,"dead"),
            "failed conversion dead",
            goto done);


    /* Test: max length = pid's strlen + 1
     * Expect: Success, return pid's str len
     */
    buffer[0] = 0;
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,6,(OSAPI_ProcessId)0xdeadc) == 5,
            "failed string length 5",
            goto done);

    TEST_ASSERT(!OSAPI_String_cmp(buffer,"deadc"),
            "failed conversion deadc",
            goto done);

    /* Test: pid = 0
     * Expect: Success, return strlen of 1, buffer = '0'
     */
    TEST_ASSERT(OSAPI_Process_pid_as_string(buffer,2,0) == 1,
            "failed string length 1",
            goto done);
    
    TEST_ASSERT(!OSAPI_String_cmp(buffer,"0"),
            "failed conversion 0",
            goto done);  
    
    result = RTI_TRUE;

done:

    return result;
}

/*e
 * \brief Test OSAPI OSAPI_Process_getpid().
 *
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_ProcessTester_test_get_pid(struct UTEST_Context *setting)
{
    unsigned char result = RTI_FALSE;
    OSAPI_ProcessId pid;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    /* Test: Call OSAPI_Process_getpid()
     * Expect: Returns
     */
    pid = OSAPI_Process_getpid();
    IGNORE_RETVAL(pid);

    result = RTI_TRUE;

    return result;
}


RTI_PRIVATE struct UTEST_TestEntry OSAPI_ProcessTester_tests[]=
{
    RTITestCase("pid_as_string",
                OSAPI_ProcessTester_test_pid_as_string,
                TEST_ENABLED),

    RTITestCase("getpid",
            OSAPI_ProcessTester_test_get_pid,
            TEST_ENABLED)
};


UT_DEFINE_SUBMODULE_RUNNER(OSAPI_ProcessTester,"process")

