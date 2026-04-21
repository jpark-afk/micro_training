/*
 * FILE: SystemTester.c - System unit-test implementation
 *
 * (c) Copyright, Real-Time Innovations, 2011-2024.
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
 * 19oct2020,tk MICRO-2582/PR#28143
 *     - Use OSAPI_PRECONDITION_ALWAYS for OSAPI_System_get_property
 *       and OSAPI_System_set_property
 * 18mar2011,tk  Updated
 */
/*ce
 * \file
 * \brief System tester
 */
#include "test/test_setting.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_time.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_system.h"
#include "osapi/osapi_string.h"

#include "System.h"

#include "SystemTester.h"

struct OSAPI_SystemTesterListenerData
{
    RTI_INT32 finalized_called_count;
    RTI_INT32 initialize_called_count;
    RTI_BOOL retvalue;
};

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_on_system_initialize(void *listener_data,
                                        struct OSAPI_System *system)
{
    struct OSAPI_SystemTesterListenerData *data =
                        (struct OSAPI_SystemTesterListenerData*)listener_data;
    UNUSED_ARG(system);

    ++data->initialize_called_count;

    return data->retvalue;
}

RTI_PRIVATE void
OSAPI_SystemTester_on_system_finalize(void *listener_data,
                                      struct OSAPI_System *system)
{
    struct OSAPI_SystemTesterListenerData *data =
                        (struct OSAPI_SystemTesterListenerData*)listener_data;
    UNUSED_ARG(system);

    ++data->finalized_called_count;
}
#endif

/*i
 * \brief Test OSAPI OSAPI_SystemI interface listener.
 * 
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_SystemTester_testListener(struct UTEST_Context* setting)
{
#ifndef RTI_CERT
    unsigned char result = RTI_FALSE;
    struct OSAPI_SystemListener syslist = OSAPI_SystemListener_INITIALIZER;
    struct OSAPI_SystemListener syslist2 = OSAPI_SystemListener_INITIALIZER;
    struct OSAPI_SystemTesterListenerData test_data;

    CHECK_DO_RUN_TEST(setting);

    test_data.initialize_called_count = 0;
    test_data.finalized_called_count = 0;
    test_data.retvalue = RTI_TRUE;

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    syslist.listener_data = &test_data;

    /*i \test
     * Test that the OSAPI_System_on_initialize listener is called
     */
    syslist.on_system_initialize = OSAPI_SystemTester_on_system_initialize;

    TEST_ASSERT(OSAPI_System_set_listener(&syslist),
                "failed to install listener",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 1,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 0,
                "on_system_finalize called",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 1,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 0,
                "on_system_finalize called",
                goto done);

    TEST_ASSERT(OSAPI_System_get_listener(&syslist2),
                "failed to get listener",
                goto done);

    TEST_ASSERT(syslist2.listener_data == &test_data,
                "incorrect listener data",
                goto done);

    TEST_ASSERT(syslist2.on_system_finalize == NULL,
                "incorrect on_system_finalize",
                goto done);

    TEST_ASSERT(syslist2.on_system_initialize ==
                OSAPI_SystemTester_on_system_initialize,
                "incorrect on_system_initialize",
                goto done);

    TEST_ASSERT(!OSAPI_System_set_listener(&syslist),
                "install listener should have failed",
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "system finalize failed",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    /*i \test
     * Test that OSAPI_System_on_finalize listener is called
     */
    syslist.listener_data = &test_data;
    syslist.on_system_finalize = OSAPI_SystemTester_on_system_finalize;
    syslist.on_system_initialize = NULL;
    test_data.initialize_called_count = 0;
    test_data.finalized_called_count = 0;

    TEST_ASSERT(OSAPI_System_set_listener(&syslist),
                "failed to install listener",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 0,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 0,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 0,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 0,
                "on_system_finalize called",
                goto done);

    TEST_ASSERT(OSAPI_System_get_listener(&syslist2),
                "failed to get listener",
                goto done);

    TEST_ASSERT(syslist2.listener_data == &test_data,
                "incorrect listener data",
                goto done);

    TEST_ASSERT(syslist2.on_system_finalize ==
                OSAPI_SystemTester_on_system_finalize,
                "incorrect on_system_finalize",
                goto done);

    TEST_ASSERT(syslist2.on_system_initialize == NULL,
                "incorrect on_system_initialize",
                goto done);

    TEST_ASSERT(!OSAPI_System_set_listener(&syslist),
                "install listener should have failed",
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "system finalize failed",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 0,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 1,
                "on_system_finalize called",
                goto done);

    TEST_ASSERT(!OSAPI_System_finalize(),
                "system finalize failed",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 0,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 1,
                "on_system_finalize called",
                goto done);

    /*i \test
     * Test that if OSAPI_System_on_initialize returns FALSE,
     * OSAPI_System_initialize also return FALSE;
     */
    syslist.listener_data = &test_data;
    syslist.on_system_finalize = NULL;
    syslist.on_system_initialize = OSAPI_SystemTester_on_system_initialize;
    test_data.initialize_called_count = 0;
    test_data.finalized_called_count = 0;
    test_data.retvalue = RTI_FALSE;

    TEST_ASSERT(OSAPI_System_set_listener(&syslist),
                "failed to install listener",
                goto done);

    TEST_ASSERT(!OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(test_data.initialize_called_count == 1,
                "on_system_intialize not called",
                goto done);

    TEST_ASSERT(test_data.finalized_called_count == 0,
                "on_system_finalize called",
                goto done);

    TEST_ASSERT(!OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "system finalize failed",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    TEST_ASSERT(OSAPI_System_set_listener(NULL),
                "failed to install listener",
                goto done);

    TEST_ASSERT(OSAPI_System_set_interface(NULL),
                "should have failed to set interface",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to finalize interface",
                goto done);

    result = RTI_TRUE;

 done:

   return result;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_CERT */
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_fv_start_timer_called = RTI_FALSE;

RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_fv_stop_timer_called = RTI_FALSE;

RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_fv_get_timer_res_called = RTI_FALSE;
#endif

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_start_timer_impl(OSAPI_Timer_T self,
                                    OSAPI_TimerTickHandlerFunction tick_handler)
{
    UNUSED_ARG(self);
    UNUSED_ARG(tick_handler);
    OSAPI_SystemTester_fv_start_timer_called = RTI_TRUE;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_stop_timer_impl(OSAPI_Timer_T self)
{
    UNUSED_ARG(self);
    OSAPI_SystemTester_fv_stop_timer_called = RTI_TRUE;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_INT32
OSAPI_SystemTester_get_timer_resolution(void)
{
    OSAPI_SystemTester_fv_get_timer_res_called = RTI_TRUE;

    return 1000;
}

RTI_PRIVATE RTI_BOOL
OSAPI_SystemTester_intf_on_system_initialize(void *listener_data,
                                             struct OSAPI_System *system)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(system);

    return RTI_TRUE;
}
#endif

/*i
 * \brief Test OSAPI OSAPI_SystemI interface.
 * 
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_SystemTester_testInterface(struct UTEST_Context* setting)
{
#ifndef RTI_CERT
    unsigned char result = RTI_FALSE;
    struct OSAPI_SystemI sysintf = OSAPI_SystemI_INITIALIZER;
    struct OSAPI_SystemI sysintf2 = OSAPI_SystemI_INITIALIZER;
    struct OSAPI_SystemListener syslist = OSAPI_SystemListener_INITIALIZER;
    RTI_INT32 sec;
    RTI_UINT32 nanosec;

    CHECK_DO_RUN_TEST(setting);

    syslist.on_system_initialize = OSAPI_SystemTester_intf_on_system_initialize;

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to initialize interface",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    TEST_ASSERT(OSAPI_System_set_listener(&syslist),
                "failed to set listener",
                goto done);
    /*i \test
     * Test that a NULL system interface cannot be installed
     */
    TEST_ASSERT(!OSAPI_System_set_interface(&sysintf),
                "should have failed to set interface",
                goto done);

    sysintf.get_timer_resolution = OSAPI_SystemTester_get_timer_resolution;

    TEST_ASSERT(!OSAPI_System_set_interface(&sysintf),
                "should have failed to set interface",
                goto done);

    sysintf.start_timer = OSAPI_SystemTester_start_timer_impl;

    TEST_ASSERT(!OSAPI_System_set_interface(&sysintf),
                "should have failed to set interface",
                goto done);

    sysintf.stop_timer = OSAPI_SystemTester_stop_timer_impl;

    TEST_ASSERT(OSAPI_System_set_interface(&sysintf),
                "should not have failed to set interface",
                goto done);

    /*i \test
     * Test that an interface cannot be installed after the system has
     * been initialized
     */
    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize interface",
                goto done);

    TEST_ASSERT(!OSAPI_System_set_interface(&sysintf),
                "should have failed to set interface",
                goto done);

    /*i \test
     * Test that OSAPI_System_get_interface works
     */
    TEST_ASSERT(OSAPI_System_get_interface(&sysintf2),
                "failed to get interface",
                goto done);

    TEST_ASSERT((sysintf2.get_timer_resolution == OSAPI_SystemTester_get_timer_resolution) &&
                (sysintf2.start_timer == OSAPI_SystemTester_start_timer_impl) &&
                (sysintf2.stop_timer == OSAPI_SystemTester_stop_timer_impl),
                "incorrect system interface returned",
                goto done);

    /*i \test
     * Test that the installed interface is called
     */
    TEST_ASSERT(OSAPI_System_start_timer(0,0),
                "OSAPI_System_start_timer failed",
                goto done);

    TEST_ASSERT(OSAPI_System_stop_timer(0),
                "OSAPI_System_stop_timer failed",
                goto done);

    TEST_ASSERT(OSAPI_System_get_timer_resolution() == 1000,
                "OSAPI_System_get_timer_resolution failed",
                goto done);

    TEST_ASSERT(OSAPI_System_get_ticktime(&sec, &nanosec),
                "OSAPI_System_get_ticktime failed",
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize interface",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    /* NOTE: Reset the interface to NULL so that he internal interfaces are
     * used.
     */
    TEST_ASSERT(OSAPI_System_set_interface(NULL),
                "should have failed to set interface",
                goto done);

    TEST_ASSERT(OSAPI_System_set_listener(NULL),
                "should have failed to set interface",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to finalize interface",
                goto done);

    result = RTI_TRUE;


 done:

   return result;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_CERT */
}

/*i
 * \brief Test OSAPI OSAPI_System_get_hostname().
 * 
 * \details MICRO-735 tests. FT-01 and FT-02
 *
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_SystemTester_hostname(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    const char *hostname_property;
    unsigned char result = RTI_FALSE;
    struct OSAPI_SystemProperty sys_property = OSAPI_SystemProperty_INITIALIZER;
    char hostname[OSAPI_SYSTEM_MAX_HOSTNAME];
    char test_hostname[] = "my_computer";

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system interface",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    /*\test
     * MICRO-735/FT-01:
     * Verify that the hostname is resolved correctly when it specified manually
     */
    TEST_EXECUTE("MICRO-735/FT-01-NNN")

    TEST_ASSERT(OSAPI_System_get_property(&sys_property),
                "failed to get system property",
                goto done);

    TEST_ASSERT(UTEST_Stdio_snprintf(sys_property.hostname,
                                     OSAPI_SYSTEM_MAX_HOSTNAME,
                                     test_hostname) < OSAPI_SYSTEM_MAX_HOSTNAME,
                 "failed to set hostname property",
                 goto done);

    TEST_ASSERT(OSAPI_System_set_property(&sys_property),
                "failed to set system property",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(OSAPI_System_get_hostname(hostname),
                "failed to get system hostname",
                goto done);

    TEST_ASSERT(!OSAPI_String_ncmp(test_hostname,hostname,OSAPI_SYSTEM_MAX_HOSTNAME-1),
                "test_hostname not equal to actual hostname",
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system interface",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    /*\test
     * MICRO-735/FT-02:
     * Verify that the hostname is resolved correctly when resolved automatically
     * This test relies on a specific system property being specified.
     */
    TEST_EXECUTE("MICRO-735/FT-01-NNN")

    hostname_property = UTEST_Property_lookup_property(setting,
                                                   "osapi.system.my_hostname");

    TEST_ASSERT(hostname_property != NULL,
                "osapi.system.my_hostname not set",
                goto done);

    /* Empty hostname means detect it automatically */
    TEST_ASSERT(UTEST_Stdio_snprintf(sys_property.hostname,
                    OSAPI_SYSTEM_MAX_HOSTNAME,"") < OSAPI_SYSTEM_MAX_HOSTNAME,
                "failed to set hostname property",
                 goto done);

    TEST_ASSERT(OSAPI_System_set_property(&sys_property),
                "failed to set system property",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize system",
                goto done);

    TEST_ASSERT(OSAPI_System_get_hostname(hostname),
                "failed to get system hostname",
                goto done);

    TEST_ASSERT(!OSAPI_String_ncmp(hostname_property,hostname,OSAPI_SYSTEM_MAX_HOSTNAME-1),
                "hostname_property not equal to actual hostname",
                UTEST_Stdio_printf("property=%s,hostname=%s\n",hostname_property,hostname);
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system interface",
                goto done);
    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize interface",
                goto done);

    result = RTI_TRUE;

 done:

   return result;
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_CERT */
}

/*i
 * \brief Test OSAPI OSAPI_System_get_ticktime().
 *
 * \param[in] setting UTEST context settings.
 *
 * \return RTI_TRUE if the test was successful. Otherwise RTI_FALSE.
 */
RTI_PRIVATE unsigned char
OSAPI_SystemTester_get_ticktime(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char result = RTI_FALSE;
    UTEST_VAR RTI_INT32 sec_before, sec_after;
    UTEST_VAR RTI_UINT32 nanosec_before, nanosec_after;
    const RTI_UINT32 wait_time = 500u;
    RTI_UINT32 max_try_count = 120000u / wait_time;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        TEST_ASSERT(OSAPI_System_finalize(),
                    "failed to finalize system interface",
                    goto done);
        TEST_ASSERT(UTEST_system_port_properties_callout(),
                "failed to set system properties",
                    goto done);
        TEST_ASSERT(OSAPI_System_initialize(),
                    "failed to initialize system",
                    goto done);
    }
    UTEST_SETUP_END

    TEST_ASSERT(OSAPI_System_get_ticktime(&sec_before, &nanosec_before),
                "failed to get ticktime before sleep",
                goto done);

    while (max_try_count-- > 0) 
    {
        OSAPI_Thread_sleep(wait_time);

        TEST_ASSERT(OSAPI_System_get_ticktime(&sec_after, &nanosec_after),
                  "failed to get ticktime after sleep", goto done);

        if (sec_after > sec_before) 
        {
            break;
        }
    }

    TEST_ASSERT(sec_after > sec_before,
                "Seconds after sleeping is not more than before",
                UTEST_Stdio_printf("sec_before=%d,sec_after=%d\n",sec_before, sec_after);
                goto done);

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system interface",
                goto done);

    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

#ifndef RTI_ARINC653
    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize interface",
                goto done);
#endif /* !RTI_ARINC653 */
    result = RTI_TRUE;

done:
    return result;

#else
    CHECK_DO_RUN_TEST(setting);
    return RTI_TRUE;
#endif /* !RTI_CERT */
}

#ifndef RTI_CERT
RTI_PRIVATE unsigned char
OSAPI_SystemTester_precondition_always(struct UTEST_Context *setting)
{
    unsigned char result = RTI_FALSE;
    struct OSAPI_SystemProperty property = OSAPI_SystemProperty_INITIALIZER;
    struct OSAPI_SystemListener listener = OSAPI_SystemListener_INITIALIZER;
    struct OSAPI_SystemI sysintf = OSAPI_SystemI_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    TEST_ASSERT(OSAPI_System_finalize(),
                "failed to finalize system interface",
                goto done);

    TEST_ASSERT(UTEST_system_port_properties_callout(),
               "failed to set system properties",
                goto done);

    TEST_ASSERT(!OSAPI_System_get_property(NULL),
                "Should have failed to get system property",
                goto done);

    TEST_ASSERT(OSAPI_System_get_property(&property),
                "Failed to get system property",
                goto done);

    TEST_ASSERT(!OSAPI_System_set_property(NULL),
                "Should have failed to set system property",
                goto done);

    TEST_ASSERT(OSAPI_System_set_property(&property),
                "Failed to set system property",
                goto done);

    TEST_ASSERT(!OSAPI_System_get_listener(NULL),
                "Should have failed to get system listener",
                goto done);

    TEST_ASSERT(OSAPI_System_get_listener(&listener),
                "Failed to get system listener",
                goto done);

    TEST_ASSERT(OSAPI_System_set_listener(&listener),
                "Failed to set system listener",
                goto done);

    TEST_ASSERT(!OSAPI_System_get_interface(NULL),
                "Should have failed to get system listener",
                goto done);

    TEST_ASSERT(OSAPI_System_get_interface(&sysintf),
                "Failed to get system interface",
                goto done);

    OSAPI_System_get_native_interface(&sysintf);

    TEST_ASSERT(OSAPI_System_set_interface(&sysintf),
                "Failed to set system interface",
                goto done);

    TEST_ASSERT(OSAPI_System_initialize(),
                "failed to initialize interface",
                goto done);

    result = RTI_TRUE;

done:
    return result;

}
#endif

RTI_PRIVATE struct UTEST_TestEntry OSAPI_SystemTester_tests[]=
{
    RTITestCase("listener",
                OSAPI_SystemTester_testListener,
                TEST_DISABLED),

    RTITestCase("interface",
                OSAPI_SystemTester_testInterface,
                TEST_DISABLED),

    RTITestCaseTags("hostname",
                    OSAPI_SystemTester_hostname,
                    TEST_ENABLED,"MICRO-735"),

    RTITestCase("get_ticktime",
                OSAPI_SystemTester_get_ticktime,
                TEST_ENABLED)

#ifndef RTI_CERT
    , RTITestCase("precondition_always",
                 OSAPI_SystemTester_precondition_always,
                 TEST_ENABLED)
#endif
};

UT_DEFINE_SUBMODULE_RUNNER(OSAPI_SystemTester,"system")

