/*
 * FILE: NETIOTester.c - NETIO Unit-test
 *
 * (c) Copyright, Real-Time Innovations, 2005-2020.
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
 * 07mar05,rh  Created
 *
 */
/*ce
 * \file
 * \brief Entrypoint for all netio unit tests
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_log.h"
#include "test/test_setting.h"
#include "netio/netio_config.h"
#include "UDPInterfaceTester.h"
#include "UDPTransformTester.h"
#if defined(RTI_AUTOSAR)
#include "autosarSocketTester.h"
#endif

#include "NETIOTester.h"

RTI_PRIVATE struct UTEST_TestEntry NETIOTester_tests[]=
{
        RTITestCase("udp",
                    UDPInterfaceTester_run,
                    TEST_ENABLED)

#if UDP_TRANSFORMS_ENABLED
        , RTITestCase("udp/transform",
                      UDPTransformTester_run,
                      TEST_ENABLED)
#endif

#if defined(RTI_AUTOSAR)
        , RTITestCase("autosarSocket",
                      autosarSocketTester_run,
                      TEST_ENABLED)
#endif
};

UT_DEFINE_TEST(NETIOTester,"netio","rti_me_heap.netio={};\n")
