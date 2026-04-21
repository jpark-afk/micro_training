/*
 * FILE: OSAPITypeTester.c - OSAPI Type unit-test
 *
 * (c) Copyright, Real-Time Innovations, 2021-2021.
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
 * 8feb2021,francisco  Created
 */
/*ce
 * \file
 * \brief OSAPITypeTester
 */
#ifndef test_setting_h
#include "test/test_setting.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "System.h"
#include "OSAPITypeTester.h"

/*ci
 * \brief An enum that can fit in 1 byte
 */
typedef enum
{
    OSAPI_OSAPITYPETESTER_ENUM_1BYTE_1,
    OSAPI_OSAPITYPETESTER_ENUM_1BYTE_2
} OSAPI_OSAPITypeTester_enum_1byte;

/*ci
 * \brief An enum that can fit in 2 bytes
 */
typedef enum
{
    OSAPI_OSAPITYPETESTER_ENUM_2BYTE_1 = 0x0,
    OSAPI_OSAPITYPETESTER_ENUM_2BYTE_2 = 0xFF,
    OSAPI_OSAPITYPETESTER_ENUM_2BYTE_3 = 0x1FF
} OSAPI_OSAPITypeTester_enum_2byte;

/*ci
 * \brief An enum that can fit in 4 bytes
 */
typedef enum
{
    OSAPI_OSAPITYPETESTER_ENUM_4BYTE_1 = 0x0,
    OSAPI_OSAPITYPETESTER_ENUM_4BYTE_2 = 0xFF,
    OSAPI_OSAPITYPETESTER_ENUM_4BYTE_3 = 0x1FF,
    OSAPI_OSAPITYPETESTER_ENUM_4BYTE_4 = 0xFFFF,
    OSAPI_OSAPITYPETESTER_ENUM_4BYTE_5 = 0x1FFFF
} OSAPI_OSAPITypeTester_enum_4byte;

RTI_PRIVATE unsigned char
OSAPI_OSAPITypeTester_check_types_size(struct UTEST_Context* setting)
{
    unsigned char result = RTI_FALSE;
    char buffer[150];

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(sizeof(RTI_INT8) == 1,
                "Wrong size of RTI_INT8",
                goto done);

    TEST_ASSERT(sizeof(RTI_UINT8) == 1,
                "Wrong size of RTI_UINT8",
                goto done);

    TEST_ASSERT(sizeof(RTI_INT16) == 2,
                "Wrong size of RTI_INT16",
                goto done);

    TEST_ASSERT(sizeof(RTI_UINT16) == 2,
                "Wrong size of RTI_UINT16",
                goto done);

    TEST_ASSERT(sizeof(RTI_INT32) == 4,
                "Wrong size of RTI_INT32",
                goto done);

    TEST_ASSERT(sizeof(RTI_UINT32) == 4,
                "Wrong size of RTI_UINT32",
                goto done);

    TEST_ASSERT(sizeof(RTI_INT64) == 8,
                "Wrong size of RTI_INT64",
                goto done);

    TEST_ASSERT(sizeof(RTI_UINT64) == 8,
                "Wrong size of RTI_UINT64",
                goto done);

    TEST_ASSERT(sizeof(RTI_FLOAT32) == 4,
                "Wrong size of RTI_FLOAT32",
                goto done);

    TEST_ASSERT(sizeof(RTI_DOUBLE64) == 8,
                "Wrong size of RTI_DOUBLE64",
                goto done);

    TEST_ASSERT(sizeof(RTI_DOUBLE128) == 16,
                "Wrong size of RTI_DOUBLE64",
                goto done);

    UTEST_Stdio_snprintf(buffer, sizeof(buffer),
                         "\nSize of OSAPI_OSAPITypeTester_enum_1byte (enum that fits in 1 byte) %d bytes\n",
                         sizeof(OSAPI_OSAPITypeTester_enum_1byte));
    UTEST_Log_debug(buffer);

    UTEST_Stdio_snprintf(buffer, sizeof(buffer),
                         "Size of OSAPI_OSAPITypeTester_enum_2byte (enum that fits in 2 bytes) %d bytes\n",
                         sizeof(OSAPI_OSAPITypeTester_enum_2byte));
    UTEST_Log_debug(buffer);

    UTEST_Stdio_snprintf(buffer, sizeof(buffer),
                         "Size of OSAPI_OSAPITypeTester_enum_4byte (enum that fits in 4 bytes) %d bytes\n",
                         sizeof(OSAPI_OSAPITypeTester_enum_4byte));
    UTEST_Log_debug(buffer);

    result = RTI_TRUE;

done:

    return result;
}

RTI_PRIVATE unsigned char
OSAPI_OSAPITypeTester_check_math(struct UTEST_Context* setting)
{
    unsigned char result = RTI_FALSE;

    /* volatile so the compiler can not apply any optimization,
     * which might have the effect of the CPU not actually performing
     * any mathematical operation.
     */
    volatile const RTI_UINT8 var1_8bits = 0x53U;
    volatile const RTI_UINT8 var2_8bits = 0x15U;
    volatile const RTI_UINT8 var3_8bits = 0x7U;

    volatile const RTI_UINT16 var1_16bits = 0x5348U;
    volatile const RTI_UINT16 var2_16bits = 0x1526U;

    volatile const RTI_UINT32 var1_32bits = 0x53481a79U;
    volatile const RTI_UINT32 var2_32bits = 0x152634bcU;

    volatile const RTI_UINT64 var1_64bits = 0x53481a79fdb04834ULL;
    volatile const RTI_UINT64 var2_64bits = 0x152634bccb3792afULL;

    volatile RTI_UINT8 result_8bits;
    volatile RTI_UINT16 result_16bits;
    volatile RTI_UINT32 result_32bits;
    volatile RTI_UINT64 result_64bits;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    result_8bits = (RTI_UINT8)(var1_8bits + var2_8bits);
    TEST_ASSERT(result_8bits == 0x68U,
                "Wrong addition of RTI_UINT8",
                goto done);

    result_8bits = (RTI_UINT8)(var2_8bits * var3_8bits);
    TEST_ASSERT(result_8bits == 0x93U,
                "Wrong multiplication of RTI_UINT8",
                goto done);

    result_16bits = (RTI_UINT16)(var1_16bits + var2_16bits);
    TEST_ASSERT(result_16bits == 0x686EU,
                "Wrong addition of RTI_UINT16",
                goto done);

    result_16bits = (RTI_UINT16)(var1_8bits * var2_8bits);
    TEST_ASSERT(result_16bits == 0x6CFU,
                "Wrong multiplication of RTI_UINT16",
                goto done);

    result_32bits = var1_32bits + var2_32bits;
    TEST_ASSERT(result_32bits == 0x686E4F35U,
                "Wrong addition of RTI_UINT32",
                goto done);

    result_32bits = (RTI_UINT32)var1_16bits * (RTI_UINT32)var2_16bits;
    TEST_ASSERT(result_32bits == 0x6E144B0U,
                "Wrong multiplication of RTI_UINT32",
                goto done);

    result_64bits = var1_64bits + var2_64bits;
    TEST_ASSERT(result_64bits == 0x686E4F36C8E7DAE3ULL,
                "Wrong addition of RTI_UINT64",
                goto done);

    result_64bits = (RTI_UINT64)var1_32bits * (RTI_UINT64)var2_32bits;
    TEST_ASSERT(result_64bits == 0x6E15807A94A04DCULL,
                "Wrong multiplication of RTI_UINT64",
                goto done);

    result = RTI_TRUE;

done:

    return result;
}

RTI_PRIVATE struct UTEST_TestEntry OSAPI_OSAPITypeTester_tests[]=
{
    RTITestCase("size",
                OSAPI_OSAPITypeTester_check_types_size,
                TEST_ENABLED),

    RTITestCase("math",
                OSAPI_OSAPITypeTester_check_math,
                TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_OSAPITypeTester,"OSAPIType")

