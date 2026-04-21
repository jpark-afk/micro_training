/*
 * FILE: HeapTester.c - Heap unit-test
 *
 * (c) Copyright, Real-Time Innovations, 2016-2021.
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
 * \brief HeapTester
 */

#include "test/test_setting.h"
#include "osapi/osapi_heap.h"

#include "HeapTester.h"

#define BUFF_SIZE 10

/*e
 * \brief Test OSAPI allocations.
 *
 * \param[in] setting UTEST context setting
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_HeapTester_allocate_buffer(struct UTEST_Context *setting)
{
    unsigned char result = RTI_FALSE;
    RTI_UINT32 size = BUFF_SIZE;
    int i;
    char* buffer;
    RTI_INT32 alignment = sizeof(void*);

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    /* Test: allocate memory for buffer
     * Expect: Success
     */

    buffer = OSAPI_Heap_allocate(1,size);

    TEST_ASSERT(buffer != NULL,
                "failed to allocate",
                goto done);

    /* Test: alignment
     * Expect: Success
     */
    TEST_ASSERT(((buffer - (char*)OSAPI_ADDRESS_ZERO) % alignment) == 0,
                "Alignment of allocated memory is not correct",
                goto done);

    /* Test: store data in memory
     * Expect: Success
     */
    for (i = 0; i < BUFF_SIZE; i++)
    {
        buffer[i] = (char)i;
    }

    for (i = 0; i < BUFF_SIZE; i++)
    {
        TEST_ASSERT(buffer[i] == (char)i,
                    "failed to store data in memory",
                    goto done);
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_buffer(buffer);
#endif

    return RTI_TRUE;

done:

    return result;
}

RTI_PRIVATE struct UTEST_TestEntry OSAPI_HeapTester_tests[]=
{
    RTITestCase("heap_allocate",
                OSAPI_HeapTester_allocate_buffer,
                TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_HeapTester,"heap")
