/*
 * FILE: autosarSocketTester.c - AUTOSAR socket unit tests implementation
 *
 * (c) Copyright, Real-Time Innovations, 2020-2021
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
 * 17mar2020,fmt Written
 */
/*ce
 * \file
 * \brief NETIO Autosar socket unit-tests
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_time.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_system.h"

#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#include "test/test_setting.h"

#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif

#if defined(RTI_AUTOSAR)

#include "autosarSocketTester.h"
#include "autosarSocket.h"

/*ci
 * \brief This function is actually never called. Tests create their sockets
 * to send UDP packets. This function is needed to ensure that the socket
 * configuration is consistent.
 *
 * \param[in]  data_ptr        Pointer to data to send.
 * \param[in]  length          Length in bytes of data to send.
 * \param[in]  remote_addr_ptr Remote address to send data to.
 */
RTI_PRIVATE Std_ReturnType
autosarSocketTester_send_dyn_data
    (P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) data_ptr,
     uint16 length,
     P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC) remote_addr_ptr)
{
    UNUSED_ARG(data_ptr);
    UNUSED_ARG(length);
    UNUSED_ARG(remote_addr_ptr);

    return E_OK;
}

/*ci
 * \brief Test callback function that is configured to be called when a UDP
 * pàcket is received.
 *
 * \param[in]  user_data  User data configured to be used with this callback.
 * \param[in]  buffer     Pointer to incoming data.
 * \param[in]  rx_len     Length of incoming packet.
 * \param[in]  ip_src     Pointer to source IP.
 */
RTI_PRIVATE void
autosarSocketTester_interface_receive(
    void *user_data,
    char *buffer,
    RTI_INT32 rx_len,
    const struct sockaddr_in *ip_src)
{
    unsigned int *number_of_calls = (unsigned int *)user_data;

    (*number_of_calls)++;
}

/*ci
 * \brief Simulates the reception of a UDP packet. Calls the Autosar
 * Socket functions that are called when a UDP packet is received
 * by the Autosar protocol stack.
 *
 * \param[in]  setting           UTEST Context settings
 * \param[in]  socket            Socket ID to be used.
 * \param[in]  use_socket_owner  Whether SocketOwner is configured or not.
 *
 * \return RTI_TRUE if success. RTI_FALSE if error.
 */
RTI_PRIVATE RTI_BOOL
autosarSocketTester_call_rx_indication(struct UTEST_Context *setting,
                                       TcpIp_SocketIdType socket,
                                       boolean use_socket_owner)
{
    uint8 buf_ptr[10];

    if (use_socket_owner)
    {
        TcpIp_SockAddrType remote_addr_ptr =
#if defined(RTIME_AUTOSAR_MICROSAR)
        {TCPIP_AF_INET};
#else
        {TCPIP_AF_INET, {0}};
#endif

        NETIO_Autosar_TcpIp_udp_rx_indication((TcpIp_SocketIdType)socket,
                                              &remote_addr_ptr,
                                              buf_ptr,
                                              sizeof(buf_ptr));
    }
    else
    {
        PduInfoType pdu_info = {buf_ptr, sizeof(buf_ptr)};

        TEST_ASSERT(NETIO_Autosar_TcpIp_pdu_callout(0, &pdu_info) == TRUE,
                    "error in call to NETIO_Autosar_TcpIp_pdu_callout()",
                    goto done);
    }

    return RTI_TRUE;

done:
    return RTI_FALSE;
}

/*ci
 * \brief Test that if OSAPI_SystemProperty.use_udp_thread is TRUE
 * the notification callback configured in the socket is called asynchronously
 * when a UDP packet is received. The packet is queued instead of notifying
 * the upper layer immediately. Packet may be lost if the queue fills up faster
 * then packets are processed.
 *
 * \param[in]  setting           UTEST Context settings
 * \param[in]  use_socket_owner  RTI_TRUE if SocketOwner is configured.
 *                               OTherwise RTI_FALSE.
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_task_received_packets(struct UTEST_Context *setting,
                                          boolean use_socket_owner)
{
    struct OSAPI_SystemProperty system_property;
    unsigned char retval = RTI_FALSE;
    int socket;
    unsigned int number_of_calls = 0;
    int i;

    CHECK_DO_RUN_TEST(setting);

#define NUMBER_OF_RCV_BUFFERS 4

    TEST_ASSERT(OSAPI_System_get_property(&system_property),
                "failed to get system properties\n",
                goto done);

    NETIO_Autosar_finalize();
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
    TEST_ASSERT(OSAPI_System_finalize(), "failed to finalize system", goto done);

    system_property.port_property.use_socket_owner = use_socket_owner;
    system_property.port_property.max_receive_sockets = 2;
    system_property.port_property.number_of_rcv_buffers = NUMBER_OF_RCV_BUFFERS;
    system_property.port_property.rcv_buffer_size = 1500;

    system_property.port_property.use_udp_thread = TRUE;
    system_property.port_property.udp_receive_task_id =
        NETIO_Autosar_udp_receive_task_id;
    system_property.port_property.udp_packet_received_event =
        RTIME_UDP_Receive_Event_id;

    if (use_socket_owner)
    {
        system_property.port_property.send_data = NULL;
    }
    else
    {
        system_property.port_property.send_data = autosarSocketTester_send_dyn_data;
    }

    TEST_ASSERT(OSAPI_System_set_property(&system_property),
                "failed to set system properties\n",
                goto done);
    TEST_ASSERT(OSAPI_System_initialize(), 
                "failed to initialize system",
                goto done);
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize", goto done);

    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                goto done);

    /* allow the UDP receive task to run */
    OSAPI_Thread_sleep(1);

    /* create a socket */
    socket = NETIO_AutosarSocket_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    TEST_ASSERT(socket >= 0, "failed to create socket", goto done);

    /* configure receive callback */
    TEST_ASSERT(NETIO_AutosarSocket_recvnotify(
                    socket,
                    autosarSocketTester_interface_receive,
                    &number_of_calls) == 0,
                "call to NETIO_AutosarSocket_recvnotify() failed",
                goto done);

    /* Call the udp receive packet callback.
     */
    for (i = 0; i < NUMBER_OF_RCV_BUFFERS; i++)
    {
        TEST_ASSERT(autosarSocketTester_call_rx_indication(
                        setting, (TcpIp_SocketIdType)socket,
                        use_socket_owner) == RTI_TRUE,
                    "failed to call rx indication",
                    goto done);

        /* Packet is not received until we give the UDP receive task a chance
         * to run
         */
        TEST_ASSERT(number_of_calls == 0,
                    "incorrect number of calls (> 0)",
                    goto done);
    }
    OSAPI_Thread_sleep(1);
    TEST_ASSERT(number_of_calls == NUMBER_OF_RCV_BUFFERS,
                "incorrect number of calls",
                goto done);

    /* send only one packet */
    number_of_calls = 0;
    TEST_ASSERT(autosarSocketTester_call_rx_indication(
                    setting, (TcpIp_SocketIdType)socket,
                    use_socket_owner) == RTI_TRUE,
                "failed to call rx indication",
                goto done);

    TEST_ASSERT(number_of_calls == 0,
                "incorrect number of calls (> 0)",
                 goto done);
    OSAPI_Thread_sleep(1);
    TEST_ASSERT(number_of_calls == 1,
                "incorrect number of calls",
                goto done);

    /* send more packets than the number of receive buffers,
     * some of the packets will be dropped
     */
    number_of_calls = 0;
    for (i = 0; i < (NUMBER_OF_RCV_BUFFERS + 1); i++)
    {
        TEST_ASSERT(autosarSocketTester_call_rx_indication(
                        setting, (TcpIp_SocketIdType)socket,
                        use_socket_owner) == RTI_TRUE,
                    "failed to call rx indication",
                    goto done);

        /* Packet is not received until we give the UDP receive task a chance
         * to run
         */
        TEST_ASSERT(number_of_calls == 0,
                    "incorrect number of calls (> 0)",
                    goto done);
    }
    OSAPI_Thread_sleep(1);
    TEST_ASSERT(number_of_calls == NUMBER_OF_RCV_BUFFERS,
                "incorrect number of calls",
                goto done);

    /* send only one packet again */
    number_of_calls = 0;
    TEST_ASSERT(autosarSocketTester_call_rx_indication(
                    setting, (TcpIp_SocketIdType)socket,
                    use_socket_owner) == RTI_TRUE,
                "failed to call rx indication",
                goto done);

    TEST_ASSERT(number_of_calls == 0,
                "incorrect number of calls (> 0)",
                 goto done);
    OSAPI_Thread_sleep(1);
    TEST_ASSERT(number_of_calls == 1,
                "incorrect number of calls",
                goto done);

    retval = RTI_TRUE;

#undef NUMBER_OF_RCV_BUFFERS

done:
    return retval;
}

/*ci
 * \brief Test that if SocketOwnwer is configured in AUTOSAR and
 * OSAPI_SystemProperty.use_udp_thread is TRUE the notification callback
 * configured in the socket is called asynchronously when a UDP packet is
 * received. The packet is queued instead of notifying the upper layer
 * immediately. Packet may be lost if the queue fills up faster then packets
 * are processed.
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_socketownwer_task_received_packets(
    struct UTEST_Context *setting)
{
    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                return RTI_FALSE);
    return autosarSocketTester_task_received_packets(setting, TRUE);
}

/*ci
 * \brief Test that if SocketOwnwer is not configured in AUTOSAR and
 * OSAPI_SystemProperty.use_udp_thread is TRUE the notification callback
 * configured in the socket is called asynchronously when a UDP packet is
 * received. The packet is queued instead of notifying the upper layer
 * immediately. Packet may be lost if the queue fills up faster then packets
 * are processed.
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_nosocketownwer_task_received_packets(
    struct UTEST_Context *setting)
{
    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                return RTI_FALSE);
    return autosarSocketTester_task_received_packets(setting, FALSE);
}

/*ci
 * \brief Test that if OSAPI_SystemProperty.use_udp_thread is FALSE
 * the notification callback configured in the socket is called synchronously
 * when a UDP packet is received.
 *
 * \param[in]  setting           UTEST Context settings
 * \param[in]  use_socket_owner  RTI_TRUE if SocketOwner is configured.
 *                               OTherwise RTI_FALSE.
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_notask_received_packets(struct UTEST_Context *setting,
                                            boolean use_socket_owner)
{
    struct OSAPI_SystemProperty system_property;
    unsigned char retval = RTI_FALSE;
    int socket;
    unsigned int number_of_calls = 0;
    int i;

    CHECK_DO_RUN_TEST(setting);

    TEST_ASSERT(OSAPI_System_get_property(&system_property),
                "failed to get system properties\n",
                goto done);

    NETIO_Autosar_finalize();
    TEST_ASSERT(OSAPI_Log_finalize(), "failed log finalize", goto done);
    TEST_ASSERT(OSAPI_System_finalize(), "failed to finalize system", goto done);

    system_property.port_property.use_socket_owner = use_socket_owner;
    system_property.port_property.max_receive_sockets = 2;
    system_property.port_property.number_of_rcv_buffers = 1;
    system_property.port_property.rcv_buffer_size = 1500;

    system_property.port_property.use_udp_thread = FALSE;
    system_property.port_property.udp_receive_task_id = 0;
    system_property.port_property.udp_packet_received_event = 0;

    if (use_socket_owner)
    {
        system_property.port_property.send_data = NULL;
    }
    else
    {
        system_property.port_property.send_data = autosarSocketTester_send_dyn_data;
    }

    TEST_ASSERT(OSAPI_System_set_property(&system_property),
                "failed to set system properties\n",
                goto done);
    TEST_ASSERT(OSAPI_System_initialize(), 
                "failed to initialize system",
                goto done);
    TEST_ASSERT(OSAPI_Log_initialize(), "failed log initialize", goto done);

    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                goto done);

    /* create a socket */
    socket = NETIO_AutosarSocket_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    TEST_ASSERT(socket >= 0, "failed to create socket", goto done);

    /* configure receive callback */
    TEST_ASSERT(NETIO_AutosarSocket_recvnotify(socket,
                                               autosarSocketTester_interface_receive,
                                               &number_of_calls) == 0,
                "call to NETIO_AutosarSocket_recvnotify() failed",
                goto done);

    /* Call the udp receive packet callback. The packet must be received
     * immediately
     */
    for (i = 0; i < 100; i++)
    {
        TEST_ASSERT(autosarSocketTester_call_rx_indication(
                        setting, (TcpIp_SocketIdType)socket,
                        use_socket_owner) == RTI_TRUE,
                    "failed to call rx indication",
                    goto done);

        TEST_ASSERT(number_of_calls == (i + 1),
                    "incorrect number of calls",
                    goto done);
    }

    retval = RTI_TRUE;

done:
    return retval;
}

/*ci
 * \brief Test that if SocketOwnwer is configured in AUTOSAR and
 * OSAPI_SystemProperty.use_udp_thread is FALSE the notification callback
 * configured in the socket is called synchronously when a UDP packet is
 * received.
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_socketownwer_notask_received_packets(
    struct UTEST_Context *setting)
{
    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                return RTI_FALSE);
    return autosarSocketTester_notask_received_packets(setting, TRUE);
}

/*ci
 * \brief Test that if SocketOwnwer is not configured in AUTOSAR and
 * OSAPI_SystemProperty.use_udp_thread is FALSE the notification callback
 * configured in the socket is called synchronously when a UDP packet is
 * received.
 *
 * \param[in]  setting       UTEST Context settings
 *
 * \return RTI_TRUE if the test passed correctly. RTI_FALSE if the test failed.
 */
RTI_PRIVATE unsigned char
autosarSocketTester_nosocketownwer_notask_received_packets(
    struct UTEST_Context *setting)
{
    TEST_ASSERT(NETIO_Autosar_initialize() == RTI_TRUE,
                "failed to initilize NETIO",
                return RTI_FALSE);
    return autosarSocketTester_notask_received_packets(setting, FALSE);
}

RTI_PRIVATE struct UTEST_TestEntry autosarSocketTester_tests[]=
{
    RTITestCase("SocketOwner/notask_received_packets",
                autosarSocketTester_socketownwer_notask_received_packets,
                TEST_ENABLED),
    RTITestCase("SocketOwner/task_received_packets",
                autosarSocketTester_socketownwer_task_received_packets,
                TEST_ENABLED),
    RTITestCase("NoSocketOwner/notask_received_packets",
                autosarSocketTester_nosocketownwer_notask_received_packets,
                TEST_ENABLED),
    RTITestCase("NoSocketOwner/task_received_packets",
                autosarSocketTester_nosocketownwer_task_received_packets,
                TEST_ENABLED)
};

UT_DEFINE_SUBMODULE_RUNNER(autosarSocketTester, "autosar")

#endif /* defined(RTI_AUTOSAR) */
