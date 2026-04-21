/*
 * FILE: UDPInterfaceTester.c - UDP transport unit-test implementation.
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
 * All rights reserved. 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 05oct2015,tk  MICRO-1489 Added tests for is_valid_hostname()
 * 17apr2014,tk  MICRO-715  send/receive tests more robust to timing
 * 17apr2014,tk  MICRO-101  test for replaced strrchr with UDP_Interface_strrchr
 * 03feb2014,eh  MICRO-714  change ack to acknack
 * 12jun2013,tk  MICRO-417  Added unit-test
 * 27apr2012,tk  Written
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_time.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_system.h"
#include "rt/rt_rt.h"
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

#include "UDPInterface.h"
#include "UDPInterfaceTester.h"

#define SLEEP_SEND_TIME 1
#define MAX_DATA_LENGTH 200

/* Determine if multicast can be enabled */
#ifdef RTI_FREERTOS
#if NETIO_CONFIG_ENABLE_MULTICAST
#if !defined(IP_MULTICAST_LOOP) || !defined(IP_MULTICAST_IF)   || \
    !defined(IP_MULTICAST_TTL)  || !defined(IP_ADD_MEMBERSHIP) || \
    !defined(IP_DROP_MEMBERSHIP)
#undef NETIO_CONFIG_ENABLE_MULTICAST
#define NETIO_CONFIG_ENABLE_MULTICAST 0
#endif
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */
#endif /* RTI_FREERTOS */

struct UDPInterfaceTesterData {
    int epoch;
    int sn;
    char msg[MAX_DATA_LENGTH];
};

struct UDPTestInterface
{
    struct NETIO_Interface _parent;
    RTI_INT32 index;
    volatile RTI_UINT32 packet_tx;
    volatile RTI_UINT32 packet_rx;
    volatile RTI_UINT32 packet_rx_ok;
    RTI_UINT32 packet_drop;
    struct UDP_InterfaceFactory *factory;
    struct NETIO_InterfaceListener listener;
    RTI_INT32 expected_epoch;
    RTI_INT32 expected_sn;
};

struct UDPTestInterfaceFactory;

RTI_PRIVATE struct NETIO_InterfaceI UDPTestInterface_g_intf;

RTI_PRIVATE RTI_BOOL
UDPTestInterface_initialize(struct UDPTestInterface *udp_intf,
                            struct UDPTestInterfaceFactory *factory,
                            const struct NETIO_InterfaceProperty *const property,
                            const struct NETIO_InterfaceListener *const listener)
{
    UNUSED_ARG(factory);

    if (!NETIO_Interface_initialize(&udp_intf->_parent,
                                    &UDPTestInterface_g_intf,
                                    property,listener))
    {
        return RTI_FALSE;
    }

    udp_intf->packet_rx = 0;
    udp_intf->packet_tx = 0;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_send(NETIO_Interface_T *self,
                      NETIO_Interface_T *src_intf,
                      struct NETIO_Address *destination,
                      NETIO_Packet_T *packet)
{
    UNUSED_ARG(self);
    UNUSED_ARG(src_intf);
    UNUSED_ARG(destination);
    UNUSED_ARG(packet);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_receive(NETIO_Interface_T *netio_intf,
                         struct NETIO_Address *src_addr,
                         struct NETIO_Address *dst,
                         NETIO_Packet_T *packet)
{
    struct UDPTestInterface *intf = (struct UDPTestInterface*)netio_intf;
    struct UDPInterfaceTesterData *data;
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst);

    data = (struct UDPInterfaceTesterData *)NETIO_Packet_get_head(packet);
    if ((data->epoch == intf->expected_epoch) &&
        (data->sn == intf->expected_sn))
    {
        ++intf->packet_rx;
        --intf->packet_rx_ok;
    }

    return RTI_TRUE;
}

#if 0
RTI_PRIVATE RTI_BOOL
UDPTestInterface_acknack(NETIO_Interface_T *self,
                         struct NETIO_Address *source,
                         NETIO_PacketId_T packet_id,
                         RTI_BOOL nack)
{
    UNUSED_ARG(self);
    UNUSED_ARG(source);
    UNUSED_ARG(packet_id);
    UNUSED_ARG(nack);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_request(NETIO_Interface_T *self,
                         struct NETIO_Address *source,
                         NETIO_Packet_T *packet,
                         NETIO_PacketId_T packet_id)
{
    UNUSED_ARG(self);
    UNUSED_ARG(source);
    UNUSED_ARG(packet);
    UNUSED_ARG(packet_id);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_return_loan(NETIO_Interface_T *self,
                             struct NETIO_Address *source,
                             NETIO_Packet_T **packet,
                             NETIO_PacketId_T packet_id)
{
    UNUSED_ARG(self);
    UNUSED_ARG(source);
    UNUSED_ARG(packet);
    UNUSED_ARG(packet_id);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_xmit_remove(NETIO_Interface_T *self,
                             struct NETIO_Address *destination,
                             NETIO_PacketId_T packet_id)
{
    UNUSED_ARG(self);
    UNUSED_ARG(destination);
    UNUSED_ARG(packet_id);

    return RTI_FALSE;
}
#endif

RTI_PRIVATE RTI_BOOL
UDPTestInterface_add_route(NETIO_Interface_T *self,
                           struct NETIO_Address *dst_addr,
                           NETIO_Interface_T *via_intf,
                           struct NETIO_Address *via_addr,
                           struct NETIORouteProperty *property,
                           RTI_BOOL *existed)
{
    UNUSED_ARG(self);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_delete_route(NETIO_Interface_T *self,
                              struct NETIO_Address *dst_addr,
                              NETIO_Interface_T *via_intf,
                              struct NETIO_Address *via_addr,
                              RTI_BOOL *existed)
{
    UNUSED_ARG(self);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_bind(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *src_addr,
                      struct NETIOBindProperty *property,
                      RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_unbind(NETIO_Interface_T *netio_intf,
                        struct NETIO_Address *src_addr,
                        NETIO_Interface_T *dst_intf,
                        RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_intf);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_bind_external(NETIO_Interface_T *dst_intf,
                               struct NETIO_Address *dst_addr,
                               NETIO_Interface_T *src_intf,
                               struct NETIO_Address *src_addr,
                               struct NETIOBindProperty *property,
                               RTI_BOOL *existed)
{
    UNUSED_ARG(dst_intf);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(src_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_unbind_external(NETIO_Interface_T *src_intf,
                                 struct NETIO_Address *src_addr,
                                 NETIO_Interface_T *dst_intf,
                                 struct NETIO_Address *dst_addr,
                                 RTI_BOOL *existed)
{
    UNUSED_ARG(src_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_intf);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(existed);

    return RTI_FALSE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_get_external_interface(NETIO_Interface_T *netio_intf,
                                        struct NETIO_Address *src_addr,
                                        NETIO_Interface_T **dst_intf,
                                        struct NETIO_Address *dst_addr)
{
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_addr);

    *dst_intf = netio_intf;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
UDPTestInterface_set_state(NETIO_Interface_T *src_intf,
                        NETIO_InterfaceState_T state)
{

    struct UDPTestInterface *intf = (struct UDPTestInterface*)src_intf;

    intf->_parent.state = state;

    return RTI_FALSE;
}

#ifndef RTI_AUTOSAR
RTI_PRIVATE struct NETIO_InterfaceI UDPTestInterface_g_intf =
{
    RT_COMPONENTI_BASE,
    UDPTestInterface_send, 
    NULL, /* acknack */
    NULL, /* request */
    NULL, /* return_loan */
    NULL, /* xmit_remove */
    UDPTestInterface_add_route, 
    UDPTestInterface_delete_route, 
    NULL, /* get_global_address */
    UDPTestInterface_bind,
    UDPTestInterface_unbind,
    UDPTestInterface_receive,
    UDPTestInterface_get_external_interface,
    UDPTestInterface_bind_external,
    UDPTestInterface_unbind_external,
    UDPTestInterface_set_state,
    NULL, /* release_address */
    NULL, /* resolve_address */
    NULL, /* get_route_table */
    NULL,  /* post_event */
    NULL
};
#endif /* RTI_AUTOSAR */

/* Create factory with max_message_size > send and receive buffer sizes */
RTI_PRIVATE unsigned char
UDPInterfaceTester_msg_size(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    RT_Registry_T *registry;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    const char* intfname = NULL;
    RTI_INT32 max_send_buffer_size;
    RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

    TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);

    registry = RT_Registry_get_instance();

    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                "failed to get property",return RTI_FALSE);

    rt_property.db = db;
    rt_property.max_factories = 2;
    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "filed to set rt property",
                goto done);

    intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

    REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
    REDA_StringSeq_set_length(&udpf_property.allow_interface,1);

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",(int*)&max_send_buffer_size))
    {
        udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
        udpf_property.max_send_message_size = max_send_buffer_size;
#endif
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",(int*)&max_receive_buffer_size))
    {
        udpf_property.max_receive_buffer_size = max_receive_buffer_size;
        udpf_property.max_message_size = max_receive_buffer_size;
    }

    if (intfname == NULL)
    {
#if defined(RTI_DARWIN) 
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo0");
#elif defined(RTI_WIN32)
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("Local Area Connection 2");
#elif defined(RTI_LINUX) 
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo");
#endif
    }
    else
    {
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);
    }


    /* Test 1: max_message_size > max_send_buffer_size,
             max_message_size > max_receive_buffer_size */
    udpf_property.max_message_size = 9001;
    udpf_property.max_receive_buffer_size = 9000;
    udpf_property.max_send_buffer_size = 9000;

    TEST_ASSERT(!RT_Registry_register(registry,
                                "udp0",
                                UDP_InterfaceFactory_get_interface(),
                                &udpf_property._parent._parent,
                                NULL),
                "registered UDP with max_message_size > send and receive",
                goto done);

    /* Test 2: max_message_size = max_send_buffer_size,
             max_message_size > max_receive_buffer_size */
    udpf_property.max_message_size = 9001;
    udpf_property.max_receive_buffer_size = 9000;
    udpf_property.max_send_buffer_size = 9001;

    TEST_ASSERT(!RT_Registry_register(registry,
                                "udp0",
                                UDP_InterfaceFactory_get_interface(),
                                &udpf_property._parent._parent,
                                NULL),
                "registered UDP with max_message_size > receive",
                goto done);


    /* Test 3: max_message_size = max_send_buffer_size,
             max_message_size  max_receive_buffer_size */
    udpf_property.max_message_size = 9000;
    udpf_property.max_receive_buffer_size = 9000;
    udpf_property.max_send_buffer_size = 9000;

    TEST_ASSERT(RT_Registry_register(registry,
                                "udp0",
                                UDP_InterfaceFactory_get_interface(),
                                &udpf_property._parent._parent,
                                NULL),
                "registered UDP with max_message_size > receive",
                goto done);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

#ifndef RTI_CERT
    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */

    retval = RTI_TRUE;

done:

#ifndef RTI_CERT
    REDA_StringSeq_finalize(&udpf_property.allow_interface);
    REDA_StringSeq_finalize(&udpf_property.deny_interface);
#endif

    return retval;

}

/*****************************************************************************/

/* Basic test:
 *
 * SCENARIO: 1-1 DDS communication
 *
 * This test emulates the common 1-1 DDS communication path:
 *
 *       DDS DataWriter   - - - ->  DDS DataReader
 *
 *             |                        /\
 *             V                        |
 *
 *        RTPS Writer     - - - ->  RTPS Reader
 *
 *             |                       /\
 *             V                        |
 *
 *            UDP                      UDP
 *             |                        |
 *             X------------------------X
 *
 * - - - > denotes two interfaces not directly connected, but communicating
 *
 *   |     Denotes a local, synchronous tx path
 *   V
 *
 *   /\    Denotes a local, synchronous rx path
 *   |
 *
 *   |   |
 *   X---X Denotes a cross connection
 */

RTI_PRIVATE unsigned char
UDPInterfaceTester_route(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *factory;
    NETIO_Interface_T *udp0;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct NETIO_AddressSeq routes = NETIO_AddressSeq_INITIALIZER;
    struct NETIO_NetmaskSeq netmasks = NETIO_NetmaskSeq_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    const char* intfname = NULL;
    RTI_INT32 max_send_buffer_size;
    RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

    TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);

    registry = RT_Registry_get_instance();
    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                "get property",
                goto done);

    rt_property.db = db;
    rt_property.max_factories = 2;
    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "filed to set rt property",
                goto done);

    intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

#if 0
    UTEST_Stdio_printf("intfname(%s)\n",intfname);
#endif

#if UDP_TRANSFORMS_ENABLED
    udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

    REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
    REDA_StringSeq_set_length(&udpf_property.allow_interface,1);

    if (intfname == NULL)
    {
#if defined(RTI_DARWIN) 
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo0");
#elif defined(RTI_WIN32) 
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("Local Area Connection 2");
#elif defined(RTI_LINUX) 
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo");
#endif
    }
    else
    {
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",(int*)&max_send_buffer_size))
    {
        udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
        udpf_property.max_send_message_size = max_send_buffer_size;
#endif
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",(int*)&max_receive_buffer_size))
    {
        udpf_property.max_receive_buffer_size = max_receive_buffer_size;
        udpf_property.max_message_size = max_receive_buffer_size;
    }

    TEST_ASSERT(RT_Registry_register(registry,
                                "udp0",
                                UDP_InterfaceFactory_get_interface(),
                                &udpf_property._parent._parent,
                                NULL),
                "failed to register UDP",
                goto done);

    factory = RT_Registry_lookup(registry,"udp0");

    TEST_ASSERT(factory != NULL,"unknown factory",goto done);

    udp_property.max_binds = 32;
    udp_property._parent.db = db;

    udp0 = NETIO_InterfaceFactory_create_component(factory,
                                    &udp_property._parent,
                                    NULL);
    TEST_ASSERT(udp0 != NULL,"udp_if_tx == NULL",goto done);


    NETIO_AddressSeq_set_maximum(&routes,16);
    NETIO_AddressSeq_set_length(&routes,0);
    NETIO_NetmaskSeq_set_maximum(&netmasks,16);
    NETIO_NetmaskSeq_set_length(&netmasks,0);

    TEST_ASSERT(NETIO_Interface_get_route_table(udp0,&routes,&netmasks),
                "failed to get route table",
                goto done);
#ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp0);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */
    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&routes);
    NETIO_NetmaskSeq_finalize(&netmasks);
    REDA_StringSeq_finalize(&udpf_property.allow_interface);
    REDA_StringSeq_finalize(&udpf_property.deny_interface);
#endif /* !RTI_CERT */
    return retval;

#undef MAX_NETIO_INTERACES
}

char UDPInterfaceTester_g_packet_buffer[1024];

volatile struct UDPTestInterface udp_test_if;

/* There is a very limited amount of memory in Threadx/NetX. If this value is
 * too high all available NetX UDP packet slots will be used for tx packets
 * so incoming packets are lost */
#if defined(RTI_THREADX)
#define MAX_NETIO_SEND 10
#else
#define MAX_NETIO_SEND 50
#endif

#define MC_ADDRESS_0 0xefffff01
#define MC_ADDRESS_1 0xefffff02
#define MC_ADDRESS_2 0xefffff03

#define UDPInterfaceTester_get_unicast_port(_setting) \
(7400 + (250 * (_setting)->domain_id) + 11)

#define UDPInterfaceTester_get_multicast_port(_setting) \
(7400 + (250 * (_setting)->domain_id) + 1)

#define UDPInterfaceTester_get_disc_unicast_port1(_setting) \
(7400 + (250 * (_setting)->domain_id) + 10)

#define UDPInterfaceTester_get_disc_unicast_port2(_setting) \
(7400 + (250 * (_setting)->domain_id) + 12)

#define MC_PORT_0 ((RTI_UINT32)UDPInterfaceTester_get_disc_unicast_port1(setting))
#define MC_PORT_1 ((RTI_UINT32)UDPInterfaceTester_get_unicast_port(setting))
/* NetX does not support reusing the same port in different sockets */
#ifndef RTI_THREADX
#define MC_PORT_2 ((RTI_UINT32)UDPInterfaceTester_get_unicast_port(setting))
#else
#define MC_PORT_2 ((RTI_UINT32)UDPInterfaceTester_get_disc_unicast_port2(setting))
#endif

RTI_PRIVATE unsigned char
UDPInterfaceTester_unicast(struct UTEST_Context *setting)
{
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    UTEST_VAR RT_Registry_T *registry;
    UTEST_VAR struct RT_ComponentFactory *factory;
    UTEST_VAR NETIO_Interface_T *udp_if_tx;
    UTEST_VAR NETIO_Interface_T *udp_if_rx;
    UTEST_VAR NETIO_Interface_T *netio_ul;
    UTEST_VAR RTI_UINT32 count;
    UTEST_VAR struct NETIO_Address src_address;
    UTEST_VAR struct NETIO_Address dst_address;
    UTEST_VAR DB_Database_T db = NULL;
    UTEST_VAR struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    UTEST_VAR OSAPI_NtpTime start_time,stop_time,run_time;
    UTEST_VAR RTI_INT32 sec;
    UTEST_VAR RTI_UINT32 msec;
    UTEST_VAR NETIO_Packet_T test_packet;
    UTEST_VAR struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    UTEST_VAR struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq dsts = REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_Address);
    UTEST_VAR struct UDPInterfaceTesterData data;
    UTEST_VAR const char* intfname = NULL;
    UTEST_VAR RTI_UINT32 port;
#if !NETIO_CONFIG_HAVE_IFCONF
    UTEST_VAR const char* mcifname;
    UTEST_VAR RTI_UINT32 address;
    UTEST_VAR RTI_UINT32 netmask;
    UTEST_VAR RTI_INT32 int_property;
    UTEST_VAR RTI_UINT32 flags = 0;
#ifndef RTI_CERT
    UTEST_VAR struct NETIO_AddressSeq address_seq = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_NetmaskSeq netmask_seq = NETIO_NetmaskSeq_INITIALIZER;
#endif /* !RTI_CERT */
#endif
    UTEST_VAR RTI_INT32 max_send_buffer_size;
    UTEST_VAR RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        OSAPI_Memory_zero(&data,sizeof(data));

        db_property.max_tables = 32;
        db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

        TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                    "failed to create db",
                    goto done);

        TEST_ASSERT(UDPTestInterface_initialize((struct UDPTestInterface *)&udp_test_if,
                                                NULL,NULL,NULL),
                    "failed to initialize UDP",
                    goto done);

        registry = RT_Registry_get_instance();
        TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                    "RT_Registry_get_property failed",
                    goto done);

        rt_property.db = db;
        rt_property.max_factories = 1;
        TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                    "failed to set rt property",
                    goto done);

#if NETIO_CONFIG_HAVE_IFCONF
        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);

#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        if (intfname == NULL)
        {
#if defined(RTI_DARWIN) || defined(RTI_VXWORKS)
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo0");
#elif defined(RTI_WIN32)
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("Local Area Connection 2");
#elif defined(RTI_LINUX) 
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo");
#endif
        }
        else
        {
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);
        }
#else

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
        /* Disable automatic configuration for this test */
        udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        flags = UDP_INTERFACE_INTERFACE_UP_FLAG;

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_address",&address),
                "failed to get interface address",
                goto done);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_netmask",&netmask),
                "failed to get interface netmask",
                goto done);

        TEST_ASSERT(UTEST_Property_lookup_int_property(setting,"netio.udp.allow_interface_multicast",&int_property),
                "failed to get interface flags",
                goto done);

        if (int_property)
        {
            flags |= UDP_INTERFACE_INTERFACE_MULTICAST_FLAG;
        }

        mcifname = UTEST_Property_lookup_property(setting,"netio.udp.multicast_if");
        if (mcifname != NULL)
        {
            udpf_property.multicast_interface = REDA_String_dup(mcifname);
        }

        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                address,netmask,
                                                intfname,
                                                flags),
                    "failed to add if entry",
                    goto done);
#endif

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",&max_send_buffer_size))
        {
            udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
            udpf_property.max_send_message_size = max_send_buffer_size;
#endif
        }

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",&max_receive_buffer_size))
        {
            udpf_property.max_receive_buffer_size = max_receive_buffer_size;
            udpf_property.max_message_size = max_receive_buffer_size;
        }

        TEST_ASSERT(RT_Registry_register(registry,
                                    "udp0",
                                    UDP_InterfaceFactory_get_interface(),
                                    &udpf_property._parent._parent,
                                    NULL),
                    "failed to register UDP",
                    goto done);

        factory = RT_Registry_lookup(registry,"udp0");

        TEST_ASSERT(factory != NULL,"unknown factory",goto done);

        udp_property.max_binds = 32;
        udp_property._parent.db = db;

        /* This is the source address the receive interface is listening on */
        NETIO_Address_init(&src_address,0);
        NETIO_Address_init(&dst_address,0);

        udp_if_tx = NETIO_InterfaceFactory_create_component(factory,
                                        &udp_property._parent,
                                        NULL);
        TEST_ASSERT(udp_if_tx != NULL,"udp_if_tx == NULL",goto done);

        udp_if_rx = NETIO_InterfaceFactory_create_component(factory,
                                    &udp_property._parent,
                                    NULL);
        TEST_ASSERT(udp_if_rx != NULL,"udp_if_rx == NULL",goto done);

        TEST_ASSERT(NETIO_Interface_get_external_interface(
        (struct NETIO_Interface *)&udp_test_if._parent,
                            NETIO_AddressSeq_get_reference(&dsts,0),
                            &netio_ul,
                            &dst_address),
                    "NETIO_Interface_get_external_interface failed",
                    goto done);

        NETIO_AddressSeq_set_maximum(&dsts,3);
        NETIO_AddressSeq_set_length(&dsts,0);
        TEST_ASSERT(NETIO_Packet_initialize(&test_packet,&data,sizeof(data),0,&dsts),
                    "failed to init packet",
                    goto done);

        TEST_ASSERT(NETIO_Packet_set_head(&test_packet,0-(RTI_INT32)sizeof(data)),
                    "failed to init packet",
                    goto done);

        UTEST_Stdio_snprintf(data.msg,200,"Hello world");
        data.epoch = 0;
        data.sn = 0;
        NETIO_AddressSeq_set_maximum(&public_address,3);
        NETIO_AddressSeq_set_length(&public_address,0);
        NETIO_AddressSeq_set_maximum(&req_address,1);
        NETIO_AddressSeq_set_length(&req_address,1);
        NETIO_Address_init(NETIO_AddressSeq_get_reference(&req_address,0),0);

        /* Set port according to RTPS's formula for unicast user traffic */
        port = (RTI_UINT32)UDPInterfaceTester_get_unicast_port(setting);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),port,0);

        TEST_ASSERT(NETIO_Interface_reserve_address(udp_if_rx,
                                                        &req_address,
                                                        &public_address,
                                                        NULL),
                    "failed to get public addresses",
                    goto done);

        TEST_ASSERT(NETIO_AddressSeq_get_length(&public_address) == 1,
                    "failed to reserve unicast address",
                    goto done);

#define WAIT_FOR_RECEIVED()\
    {\
        RTI_INT32 max_wait_ = 50;\
        RTI_INT32 retry_ = 5;\
        udp_test_if.packet_rx_ok = 1;\
        do {\
            TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,\
                                            &src_address,&test_packet),\
                        "failed to send 1 uni-cast address",\
                        goto done);\
            while (udp_test_if.packet_rx_ok && max_wait_) \
            {\
                --max_wait_;\
                OSAPI_Thread_sleep(100);\
            }\
            --retry_;\
        } while (retry_ && udp_test_if.packet_rx_ok);\
        TEST_ASSERT((retry_ > 0) && (max_wait_ > 0), "UDP send/receive timed out",goto done);\
    }

        NETIO_AddressSeq_set_length(&dsts,1);
        *NETIO_AddressSeq_get_reference(&dsts,0) =
                            *NETIO_AddressSeq_get_reference(&public_address,0);
        udp_test_if.packet_rx = 0;
        udp_test_if.expected_epoch = 0;
        udp_test_if.expected_sn = 0;
    }
    UTEST_SETUP_END

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get time",
                goto done);

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,
                                         &src_address,&test_packet),
                    "failed to send 1 multi-cast address",
                    goto done);
    }

    OSAPI_Thread_sleep(1000);

    TEST_ASSERT(udp_test_if.packet_rx == 0,
                "sent count != received count",
                goto done);

    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                        NETIO_AddressSeq_get_reference(&dsts,0),
                        netio_ul,&dst_address,
                        NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    OSAPI_Thread_sleep(1000);


    udp_test_if.packet_rx = 0;
    data.epoch = 1;
    data.sn = 1;
    udp_test_if.expected_epoch = 1;
    udp_test_if.expected_sn = 1;
    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        WAIT_FOR_RECEIVED();
        ++data.sn;
        ++udp_test_if.expected_sn;
    }

    TEST_ASSERT(OSAPI_System_get_time(&stop_time),
                "failed to get time",
                goto done);

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",
                UTEST_Stdio_printf("sent/received: %d/%d\n",
                count,udp_test_if.packet_rx);goto done);

    OSAPI_NtpTime_subtract(&run_time,&stop_time,&start_time);
    OSAPI_NtpTime_to_microsec(&sec,&msec,&run_time);

#ifndef RTI_CERT
    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                            NETIO_AddressSeq_get_reference(&dsts,0),
                            netio_ul,&dst_address,NULL),
            "NETIO_Interface_unbind_external failed",
            goto done);

    TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                                     NETIO_AddressSeq_get_reference(&dsts,0)),
                "NETIO_Interface_release_address failed",
                goto done);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_tx);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_rx);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);

#endif /* !RTI_CERT */
    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&dsts);
    NETIO_AddressSeq_finalize(&req_address);
    NETIO_AddressSeq_finalize(&public_address);
#if !NETIO_CONFIG_HAVE_IFCONF
    NETIO_AddressSeq_finalize(&address_seq);
    NETIO_NetmaskSeq_finalize(&netmask_seq);
#endif
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
#endif /* !RTI_CERT */
    return retval;

#undef MAX_NETIO_INTERACES
#undef WAIT_FOR_RECEIVED
}

#if NETIO_CONFIG_ENABLE_MULTICAST
RTI_PRIVATE unsigned char
UDPInterfaceTester_multicast(struct UTEST_Context *setting)
{
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    UTEST_VAR RT_Registry_T *registry;
    UTEST_VAR struct RT_ComponentFactory *factory;
    UTEST_VAR NETIO_Interface_T *udp_if_tx;
    UTEST_VAR NETIO_Interface_T *udp_if_rx;
    UTEST_VAR NETIO_Interface_T *netio_ul;
    UTEST_VAR RTI_UINT32 count;
    UTEST_VAR struct NETIO_Address src_address;
    UTEST_VAR struct NETIO_Address dst_address;
    UTEST_VAR DB_Database_T db = NULL;
    UTEST_VAR struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    UTEST_VAR NETIO_Packet_T test_packet;
    UTEST_VAR struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    UTEST_VAR struct UDPInterfaceTesterData data;
    UTEST_VAR struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq dsts = REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_Address);
    UTEST_VAR const char* intfname = NULL;
#if !NETIO_CONFIG_HAVE_IFCONF
    UTEST_VAR const char* mcifname;
    UTEST_VAR RTI_UINT32 address;
    UTEST_VAR RTI_UINT32 netmask;
    UTEST_VAR RTI_INT32 int_property;
    UTEST_VAR RTI_UINT32 flags = 0;
#ifndef RTI_CERT
    UTEST_VAR struct NETIO_AddressSeq address_seq = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_NetmaskSeq netmask_seq = NETIO_NetmaskSeq_INITIALIZER;
#endif /* !RTI_CERT */
#endif
    UTEST_VAR RTI_INT32 max_send_buffer_size;
    UTEST_VAR RTI_INT32 max_receive_buffer_size;

#ifndef RTI_CERT
    UTEST_VAR struct NETIO_Address temp_addr_0;
    UTEST_VAR struct NETIO_Address temp_addr_1;
    UTEST_VAR RTI_INT32 sec;
    UTEST_VAR RTI_UINT32 msec;
    UTEST_VAR OSAPI_NtpTime start_time, stop_time, run_time;
#endif /* !RTI_CERT */
    UTEST_VAR RTI_UINT32 test_disabled = 0;

    if (UTEST_Property_lookup_uint_property(setting,"test.udp.multicast.disabled",&test_disabled)
        && test_disabled)
    {
        return 3;
    }

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        OSAPI_Memory_zero(&data,sizeof(data));

        db_property.max_tables = 32;
        db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

        TEST_ASSERT(DB_Database_create(&db,"udp_tester",
                                    &db_property,NULL) == DB_RETCODE_OK,
                    "failed to create db",
                    goto done);

        TEST_ASSERT(UDPTestInterface_initialize((struct UDPTestInterface *)&udp_test_if,
                                                NULL,NULL,NULL),
                    "failed to initialize UDP",
                    goto done);

        registry = RT_Registry_get_instance();
        TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                    "failed to get property",
                    goto done);
        rt_property.db = db;
        rt_property.max_factories = 1;
        TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                    "failed to set rt property",
                    goto done);

#if NETIO_CONFIG_HAVE_IFCONF
        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,2);

        if (intfname == NULL)
        {
            REDA_StringSeq_set_length(&udpf_property.allow_interface,2);

#if defined(RTI_DARWIN)
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo0");
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,1) = REDA_String_dup("en0");
#elif defined(RTI_WIN32)
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("Local Area Connection 2");
#elif defined(RTI_LINUX) 
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("eth0");
#endif
        }
        else
        {
            REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);
        }

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.multicast_if");
        if (intfname != NULL)
        {
            udpf_property.multicast_interface = REDA_String_dup(intfname);
        }
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

#else

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
        /* Disable automatic configuration for this test */
        udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        flags = UDP_INTERFACE_INTERFACE_UP_FLAG;

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_address",&address),
                "failed to get interface address",
                goto done);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_netmask",&netmask),
                "failed to get interface netmask",
                goto done);

        TEST_ASSERT(UTEST_Property_lookup_int_property(setting,"netio.udp.allow_interface_multicast",&int_property),
                "failed to get interface flags",
                goto done);

        if (int_property)
        {
            flags |= UDP_INTERFACE_INTERFACE_MULTICAST_FLAG;
        }

        mcifname = UTEST_Property_lookup_property(setting,"netio.udp.multicast_if");
        if (mcifname != NULL)
        {
            udpf_property.multicast_interface = REDA_String_dup(mcifname);
        }


        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                address,netmask,
                                                intfname,
                                                flags),
                    "failed to add if entry",
                    goto done);
#endif

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",&max_send_buffer_size))
        {
            udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
            udpf_property.max_send_message_size = max_send_buffer_size;
#endif
        }

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",&max_receive_buffer_size))
        {
            udpf_property.max_receive_buffer_size = max_receive_buffer_size;
            udpf_property.max_message_size = max_receive_buffer_size;
        }

        TEST_ASSERT(RT_Registry_register(registry,
                            "udp0",
                            UDP_InterfaceFactory_get_interface(),
                            &udpf_property._parent._parent,
                            NULL),
                    "failed to register UDP",
                    goto done);

        factory = RT_Registry_lookup(registry,"udp0");

        TEST_ASSERT(factory != NULL,"unknown factory",goto done);

        udp_property.max_binds = 32;
        udp_property._parent.db = db;

        /* This is the source address the receive interface is listening on */
        NETIO_Address_init(&src_address,0);
        NETIO_Address_init(&dst_address,0);

        udp_if_tx = NETIO_InterfaceFactory_create_component(factory,
                                        &udp_property._parent,
                                        NULL);
        TEST_ASSERT(udp_if_tx != NULL,"udp_if_tx == NULL",goto done);

        udp_if_rx = NETIO_InterfaceFactory_create_component(factory,
                                    &udp_property._parent,
                                    NULL);
        TEST_ASSERT(udp_if_rx != NULL,"udp_if_rx == NULL",goto done);

        TEST_ASSERT(NETIO_Interface_get_external_interface((struct NETIO_Interface *)&udp_test_if._parent,
        &src_address,
        &netio_ul,
        &dst_address),
                    "NETIO_Interface_get_external_interface failed",
                    goto done);

        NETIO_AddressSeq_set_maximum(&dsts,3);
        NETIO_AddressSeq_set_length(&dsts,0);

        TEST_ASSERT(NETIO_Packet_initialize(&test_packet,&data,sizeof(data),0,&dsts),
                    "failed to init packet",
                    goto done);

        TEST_ASSERT(NETIO_Packet_set_head(&test_packet,0-(RTI_INT32)sizeof(data)),
                    "failed to init packet",
                    goto done);

        UTEST_Stdio_snprintf(data.msg,200,"Hello world");
        data.epoch = 3;
        data.sn = 0;
        NETIO_AddressSeq_set_maximum(&public_address,3);
        NETIO_AddressSeq_set_length(&public_address,0);
        NETIO_AddressSeq_set_maximum(&req_address,3);

#ifndef RTI_CERT
        NETIO_AddressSeq_set_length(&req_address,3);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_0,MC_ADDRESS_0);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,1),MC_PORT_1,MC_ADDRESS_1);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,2),MC_PORT_2,MC_ADDRESS_2);
#else /* RTI_CERT: 1 addr only */
        NETIO_AddressSeq_set_length(&req_address,1);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_0,MC_ADDRESS_0);
#endif

        TEST_ASSERT(NETIO_Interface_reserve_address(udp_if_rx,
                                                    &req_address,
                                                    &public_address,
                                                    NULL),
                    "failed to get public addresses",
                    goto done);
    }
    UTEST_SETUP_END

    /*e \test
     * Test that an interface can send to one multicast address, no receivers
     */
    TEST_EXECUTE("send to one multicast address with no receivers");
    NETIO_AddressSeq_set_length(&dsts,1);
    NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&dsts,0),MC_PORT_0,MC_ADDRESS_0);
    udp_test_if.packet_rx = 0;
    data.epoch = 3;
    ++data.sn;
    ++udp_test_if.expected_sn;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,&src_address,&test_packet),
                    "failed to send 1 multi-cast address",
                    goto done);
    }

    OSAPI_Thread_sleep(1000);

    TEST_ASSERT(udp_test_if.packet_rx == 0,
                "sent count != received count",
                goto done);



    /*e \test
     * Test that an interface can receive from a multicast address
     */
    TEST_EXECUTE("send and receive to one multicast address");
    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                                    NETIO_AddressSeq_get_reference(&dsts,0),
                                    netio_ul,&dst_address,
                                    NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    OSAPI_Thread_sleep(5000);

#define WAIT_FOR_RECEIVED(count_) \
{\
    RTI_INT32 max_wait_ = 50;\
    RTI_INT32 retry_ = 5;\
    udp_test_if.packet_rx_ok = count_;\
    do {\
        TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,\
                                         &src_address,&test_packet),\
                    "failed to send 1 uni-cast address",\
                    goto done);\
        while (udp_test_if.packet_rx_ok && max_wait_) \
        {\
            --max_wait_;\
            OSAPI_Thread_sleep(100);\
        }\
        --retry_;\
    } while (retry_ && udp_test_if.packet_rx_ok);\
    TEST_ASSERT((retry_ > 0) && (max_wait_ > 0), "UDP send/receive timed out",goto done);\
}

    udp_test_if.packet_rx = 0;
    data.epoch = 4;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.expected_epoch = 4;
    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",UTEST_Stdio_printf("%d/%d\n",udp_test_if.packet_rx,count);
                goto done);

#ifndef RTI_CERT

    /*e \test
     * Test that a 2nd multicast address can be added to the send path
     */
    TEST_EXECUTE("send to two multicast addresses, listener on one");
    NETIO_AddressSeq_set_length(&dsts,2);
    NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&dsts,1),MC_PORT_1,MC_ADDRESS_1);

    udp_test_if.packet_rx = 0;
    data.epoch = 5;
    udp_test_if.expected_epoch = 5;
    data.sn = 0;
    udp_test_if.expected_sn = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that a 2nd multicast address can be added to the receive path
     */
    TEST_EXECUTE("send to two multicast addresses,listner on two");
    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                                    NETIO_AddressSeq_get_reference(&dsts,1),
                                    netio_ul,&dst_address,
                                    NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    OSAPI_Thread_sleep(1000);

    udp_test_if.packet_rx = 0;
    data.epoch = 6;
    udp_test_if.expected_epoch = 6;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that a 3rd multicast address can be added to the receive path
     */
    TEST_EXECUTE("send to two multicast addresses, listener on three");
    NETIO_AddressSeq_set_length(&dsts,3);
    NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&dsts,2),MC_PORT_2,MC_ADDRESS_2);
    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                                    NETIO_AddressSeq_get_reference(&dsts,2),
                                    netio_ul,&dst_address,
                                    NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    OSAPI_Thread_sleep(1000);

    data.epoch = 7;
    udp_test_if.expected_epoch = 7;
    NETIO_AddressSeq_set_length(&dsts,2);
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;
    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",UTEST_Stdio_printf("%d/%d\n",udp_test_if.packet_rx,2*MAX_NETIO_SEND);
                goto done);

    /*e \test
     * Test that a 3rd multicast address can be added to the send path
     */
    TEST_EXECUTE("send to three multicast addresses, listener on three");
    NETIO_AddressSeq_set_length(&dsts,3);
    data.epoch = 8;
    udp_test_if.expected_epoch = 8;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(3);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 3*MAX_NETIO_SEND,
                "sent count != received count",UTEST_Stdio_printf("%d/%d\n",3*MAX_NETIO_SEND,udp_test_if.packet_rx);
                goto done);

    /*e \test
     * Test that the 2nd address can be removed from the send path
     */
    TEST_EXECUTE("remove 2nd multi-cast address from send path");
    data.epoch = 9;
    udp_test_if.expected_epoch = 9;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    temp_addr_1 = *NETIO_AddressSeq_get_reference(&dsts,1);
    *NETIO_AddressSeq_get_reference(&dsts,1) = *NETIO_AddressSeq_get_reference(&dsts,2);
    NETIO_AddressSeq_set_length(&dsts,2);

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that the 2nd address can be removed from the receive path
     */
    TEST_EXECUTE("remove listener on 2nd multi-cast address");
    data.epoch = 10;
    udp_test_if.expected_epoch = 10;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,&temp_addr_1,
                                                netio_ul,&dst_address,NULL),
                "NETIO_Interface_unbind_external failed",
                goto done);

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that the 1st address can be removed from the receive path
     */
    TEST_EXECUTE("remove listener on 1st multi-cast address");
    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                            NETIO_AddressSeq_get_reference(&dsts,0),
                            netio_ul,&dst_address,NULL),
                "NETIO_Interface_unbind_external failed",
                goto done);

    data.epoch = 11;
    udp_test_if.expected_epoch = 11;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that the 1st address can be removed from the send path
     */
    TEST_EXECUTE("remove 1st multi-cast address from sent list");
    temp_addr_0 = *NETIO_AddressSeq_get_reference(&dsts,0);
    *NETIO_AddressSeq_get_reference(&dsts,0) = *NETIO_AddressSeq_get_reference(&dsts,1);
    NETIO_AddressSeq_set_length(&dsts,1);

    data.epoch = 12;
    udp_test_if.expected_epoch = 12;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that a 2nd multicast address can be added to the send path
     */
    TEST_EXECUTE("Add 2nd multi-cast address to sent list");
    NETIO_AddressSeq_set_length(&dsts,2);
    *NETIO_AddressSeq_get_reference(&dsts,1) = temp_addr_1;

    data.epoch = 13;
    udp_test_if.expected_epoch = 13;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(udp_test_if.packet_rx == count,
                "sent count != received count",
                goto done);

    /*e \test
     * Test that a 2nd multicast address can be added to the receive path
     */
    TEST_EXECUTE("Add 2nd multi-cast address to receiver");
    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                                    &temp_addr_1,
                                    netio_ul,&dst_address,
                                    NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    data.epoch = 14;
    udp_test_if.expected_epoch = 14;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",
                goto done);

    /*e \test
     * Test time to send to 2 multi-cast
     */
    TEST_EXECUTE("time sending to two multi-cast");
    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get time",
                goto done);

    data.epoch = 15;
    udp_test_if.expected_epoch = 15;
    data.sn = 0;
    udp_test_if.expected_sn = 0;
    udp_test_if.packet_rx = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(2);
    }

    TEST_ASSERT(OSAPI_System_get_time(&stop_time),
                "failed to get time",
                goto done);

    TEST_ASSERT(udp_test_if.packet_rx == 2*MAX_NETIO_SEND,
                "sent count != received count",UTEST_Stdio_printf("%d/%d\n",udp_test_if.packet_rx,2*MAX_NETIO_SEND);
                goto done);

    OSAPI_NtpTime_subtract(&run_time,&stop_time,&start_time);
    OSAPI_NtpTime_to_microsec(&sec,&msec,&run_time);

    /*e \test
     * Test that the interface is cleanly deleted
     */

    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                        NETIO_AddressSeq_get_reference(&dsts,0),
                        netio_ul,&dst_address,NULL),
                "NETIO_Interface_unbind_external failed",
                goto done);

    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                        NETIO_AddressSeq_get_reference(&dsts,1),
                        netio_ul,&dst_address,NULL),
                "NETIO_Interface_unbind_external failed",
                goto done);

    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                        &temp_addr_0,
                        netio_ul,&dst_address,NULL),
                "NETIO_Interface_unbind_external failed",
                goto done);

    TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                                        NETIO_AddressSeq_get_reference(&req_address,0)),
               "failed to release public address",
               goto done);

    TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                                        NETIO_AddressSeq_get_reference(&req_address,1)),
               "failed to release public address",
               goto done);

    TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                                        NETIO_AddressSeq_get_reference(&req_address,2)),
               "failed to release public address",
               goto done);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_tx);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_rx);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);

#endif /* !RTI_CERT */
    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&dsts);
    NETIO_AddressSeq_finalize(&req_address);
    NETIO_AddressSeq_finalize(&public_address);
#if !NETIO_CONFIG_HAVE_IFCONF
    NETIO_AddressSeq_finalize(&address_seq);
    NETIO_NetmaskSeq_finalize(&netmask_seq);
#endif
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
#endif /* !RTI_CERT */
    return retval;

#undef WAIT_FOR_RECEIVED
}
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

#if NETIO_CONFIG_ENABLE_MULTICAST
RTI_PRIVATE unsigned char
UDPInterfaceTester_multicast_reserve(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *factory;
    NETIO_Interface_T *udp_if_rx;
    struct NETIO_Address src_address;
    struct NETIO_Address dst_address;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    struct UDPInterfaceTesterData data;
    struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    const char* intfname = NULL;
    RTI_INT32 i;
    RTI_INT32 max_send_buffer_size;
    RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    /* Need two interfaces to run this test */
    if ((UTEST_Property_lookup_property(setting,"netio.udp.allow_interface_1") == NULL) ||
        (UTEST_Property_lookup_property(setting,"netio.udp.allow_interface_2") == NULL))
    {
        return RTI_TRUE;
    }

    OSAPI_Memory_zero(&data,sizeof(data));

    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

    TEST_ASSERT(DB_Database_create(&db,"udp_tester",
                                   &db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);

    TEST_ASSERT(UDPTestInterface_initialize((struct UDPTestInterface *)&udp_test_if,
                                            NULL,NULL,NULL),
                "failed to initialize UDP",
                goto done);

    registry = RT_Registry_get_instance();
    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                "failed to get property",
                goto done);
    rt_property.db = db;
    rt_property.max_factories = 1;
    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "failed to set rt property",
                goto done);


    REDA_StringSeq_set_maximum(&udpf_property.allow_interface,2);
    REDA_StringSeq_set_length(&udpf_property.allow_interface,2);

#if UDP_TRANSFORMS_ENABLED
    udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

    intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface_1");
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);

    intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface_2");
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,1) = REDA_String_dup(intfname);

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",&max_send_buffer_size))
    {
        udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
        udpf_property.max_send_message_size = max_send_buffer_size;
#endif
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",&max_receive_buffer_size))
    {
        udpf_property.max_receive_buffer_size = max_receive_buffer_size;
        udpf_property.max_message_size = max_receive_buffer_size;
    }

    TEST_ASSERT(RT_Registry_register(registry,
                         "udp0",
                         UDP_InterfaceFactory_get_interface(),
                         &udpf_property._parent._parent,
                         NULL),
                "failed to register UDP",
                goto done);

    factory = RT_Registry_lookup(registry,"udp0");

    TEST_ASSERT(factory != NULL,"unknown factory",goto done);

    udp_property.max_binds = 32;
    udp_property._parent.db = db;

    /* This is the source address the receive interface is listening on */
    NETIO_Address_init(&src_address,0);
    NETIO_Address_init(&dst_address,0);

    udp_if_rx = NETIO_InterfaceFactory_create_component(factory,
                                   &udp_property._parent,
                                   NULL);
    TEST_ASSERT(udp_if_rx != NULL,"udp_if_rx == NULL",goto done);

    NETIO_AddressSeq_set_maximum(&public_address,3);
    NETIO_AddressSeq_set_length(&public_address,0);
    NETIO_AddressSeq_set_maximum(&req_address,1);
    NETIO_AddressSeq_set_length(&req_address,1);
    NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_0,MC_ADDRESS_0);

    TEST_ASSERT(NETIO_Interface_reserve_address(udp_if_rx,
                                            &req_address,&public_address,NULL),
                "failed to get public addresses",
                goto done);

    TEST_ASSERT(NETIO_AddressSeq_get_length(&public_address) == 1,
            "reserved duplicate multi-address when more than 1 nic is allowed",
            goto done);

    for (i = 0; i < NETIO_AddressSeq_get_length(&public_address); ++i)
    {
        TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                NETIO_AddressSeq_get_reference(&public_address,i)),
                "failed to release public address",
                goto done);
    }

#ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp_if_rx);

    TEST_ASSERT(RT_Registry_unregister(registry,"udp0",NULL,NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                "failed to finalize",
                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */
    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&req_address);
    REDA_StringSeq_finalize(&udpf_property.allow_interface);
    REDA_StringSeq_finalize(&udpf_property.deny_interface);
    NETIO_AddressSeq_finalize(&public_address);
#endif /* !RTI_CERT */
    return retval;

#undef WAIT_FOR_RECEIVED
}
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

#if ((!defined(RTI_CERT)) && (!defined(RTI_AUTOSAR)))
RTI_PRIVATE unsigned char
UDPInterfaceTester_nat(struct UTEST_Context *setting)
{
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    UTEST_VAR RT_Registry_T *registry;
    UTEST_VAR struct RT_ComponentFactory *factory;
    UTEST_VAR NETIO_Interface_T *udp_if_tx;
    UTEST_VAR NETIO_Interface_T *udp_if_rx;
    UTEST_VAR NETIO_Interface_T *netio_ul;
    UTEST_VAR RTI_UINT32 count;
    UTEST_VAR struct NETIO_Address src_address;
    UTEST_VAR struct NETIO_Address dst_address;
    UTEST_VAR DB_Database_T db = NULL;
    UTEST_VAR struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    UTEST_VAR OSAPI_NtpTime start_time,stop_time,run_time;
    UTEST_VAR RTI_INT32 sec;
    UTEST_VAR RTI_UINT32 msec;
    UTEST_VAR NETIO_Packet_T test_packet;
    UTEST_VAR struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    UTEST_VAR struct RT_RegistryProperty rt_property =
                                RT_RegistryProperty_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq dsts = REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_Address);
    UTEST_VAR struct UDPInterfaceTesterData data;
    UTEST_VAR RTI_UINT32 port0, port1;
    UTEST_VAR const char* intfname = NULL;
    UTEST_VAR RTI_UINT32 address;
#if !NETIO_CONFIG_HAVE_IFCONF
#if NETIO_CONFIG_ENABLE_MULTICAST    
    UTEST_VAR const char* mcifname;
    UTEST_VAR RTI_INT32 int_property;
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */    
    UTEST_VAR RTI_UINT32 netmask;
    UTEST_VAR RTI_INT32 flags = 0;
#endif /* !NETIO_CONFIG_HAVE_IFCONF */
    UTEST_VAR RTI_INT32 max_send_buffer_size;
    UTEST_VAR RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        OSAPI_Memory_zero(&data,sizeof(data));

        db_property.max_tables = 32;
        db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

        TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                    "failed to create db",
                    goto done);

        TEST_ASSERT(UDPTestInterface_initialize((struct UDPTestInterface *)&udp_test_if,
                                                NULL,NULL,NULL),
                    "failed to initialize UDP",
                    goto done);

        registry = RT_Registry_get_instance();
        TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                    "failed to get property",
                    goto done);
        rt_property.db = db;
        rt_property.max_factories = 1;
        TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                    "failed to set rt property",
                    goto done);

#if NETIO_CONFIG_HAVE_IFCONF

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");
        if (intfname != NULL)
        {
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);
        }
        else
        {
#if defined(RTI_DARWIN) || defined(RTI_VXWORKS)
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo0");
#elif defined(RTI_WIN32)
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("Loopback Pseudo-Interface 1");
#elif defined(RTI_LINUX)
            *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("lo");
#endif
        }

#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_address",&address),
                "failed to get interface address",
                goto done);

#else

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
        /* Disable automatic configuration for this test */
        udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        flags = UDP_INTERFACE_INTERFACE_UP_FLAG;

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");

        TEST_ASSERT(intfname != NULL,"failed to lookup allow interface",goto done);

        TEST_ASSERT(REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1),
                    "failed to set maximum allow interface",
                    goto done);

        TEST_ASSERT(REDA_StringSeq_set_length(&udpf_property.allow_interface,1),
                    "failed to set length allow interface",
                    goto done);

        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_address",&address),
                "failed to get interface address",
                goto done);

        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_netmask",&netmask),
                "failed to get interface netmask",
                goto done);

#if NETIO_CONFIG_ENABLE_MULTICAST
        TEST_ASSERT(UTEST_Property_lookup_int_property(setting,"netio.udp.allow_interface_multicast",&int_property),
                "failed to get interface flags",
                goto done);

        if (int_property)
        {
            flags |= UDP_INTERFACE_INTERFACE_MULTICAST_FLAG;
        }

        mcifname = UTEST_Property_lookup_property(setting,"netio.udp.multicast_if");
        if (mcifname != NULL)
        {
            udpf_property.multicast_interface = REDA_String_dup(mcifname);
        }
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                address,
                                                netmask,
                                                intfname,
                                                flags),
                    "failed to add if entry",
                    goto done);

#endif

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",&max_send_buffer_size))
        {
            udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
            udpf_property.max_send_message_size = max_send_buffer_size;
#endif
        }

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",&max_receive_buffer_size))
        {
            udpf_property.max_receive_buffer_size = max_receive_buffer_size;
            udpf_property.max_message_size = max_receive_buffer_size;
        }

        /* Configure the NAT table */
        UDP_NatEntrySeq_set_maximum(&udpf_property.nat,1);
        UDP_NatEntrySeq_set_length(&udpf_property.nat,1);

        port0 = (RTI_UINT32)UDPInterfaceTester_get_unicast_port(setting);
        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    local_address.kind = NETIO_ADDRESS_KIND_UDPv4;

        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    local_address.port = port0;

        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    local_address.value.ipv4.address = address;

        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    public_address.kind = NETIO_ADDRESS_KIND_UDPv4;

        port1 = port0 + 10;
        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    public_address.port = port1;

        UDP_NatEntrySeq_get_reference(&udpf_property.nat,0)->
                                    public_address.value.ipv4.address =
                                                            NETIO_htonl(address);

        TEST_ASSERT(RT_Registry_register(registry,
                                    "udp0",
                                    UDP_InterfaceFactory_get_interface(),
                                    &udpf_property._parent._parent,
                                    NULL),
                    "failed to register UDP",
                    goto done);

        factory = RT_Registry_lookup(registry,"udp0");

        TEST_ASSERT(factory != NULL,"unknown factory",goto done);

        udp_property.max_binds = 32;
        udp_property._parent.db = db;

        /* This is the source address the receive interface is listening on */
        NETIO_Address_init(&src_address,0);
        NETIO_Address_init(&dst_address,0);

        udp_if_tx = NETIO_InterfaceFactory_create_component(factory,
                                        &udp_property._parent,
                                        NULL);
        TEST_ASSERT(udp_if_tx != NULL,"udp_if_tx == NULL",goto done);

        udp_if_rx = NETIO_InterfaceFactory_create_component(factory,
                                    &udp_property._parent,
                                    NULL);
        TEST_ASSERT(udp_if_rx != NULL,"udp_if_rx == NULL",goto done);

        TEST_ASSERT(NETIO_Interface_get_external_interface(
        (struct NETIO_Interface *)&udp_test_if._parent,
                            NETIO_AddressSeq_get_reference(&dsts,0),
                            &netio_ul,
                            &dst_address),
                    "NETIO_Interface_get_external_interface failed",
                    goto done);

        NETIO_AddressSeq_set_maximum(&dsts,3);
        NETIO_AddressSeq_set_length(&dsts,0);
        TEST_ASSERT(NETIO_Packet_initialize(&test_packet,&data,sizeof(data),0,&dsts),
                    "failed to init packet",
                    goto done);
        TEST_ASSERT(NETIO_Packet_set_head(&test_packet,0-(RTI_INT32)sizeof(data)),
                    "failed to set head",
                    goto done);
        UTEST_Stdio_snprintf(data.msg,200,"Hello world");
        NETIO_AddressSeq_set_maximum(&public_address,3);
        NETIO_AddressSeq_set_length(&public_address,0);
        NETIO_AddressSeq_set_maximum(&req_address,1);
        NETIO_AddressSeq_set_length(&req_address,1);

        NETIO_Address_init(NETIO_AddressSeq_get_reference(&req_address,0),0);

        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),port0,0);

        TEST_ASSERT(NETIO_Interface_reserve_address(udp_if_rx,
                                                        &req_address,
                                                        &public_address,
                                                        NULL),
                    "failed to get public addresses",
                    goto done);

        /* MICRO-1756 : try to reserve twice the same address and verify that
            * duplicated addresses are filtered
            */
        TEST_ASSERT(NETIO_Interface_reserve_address(udp_if_rx,
                                                        &req_address,
                                                        &public_address,
                                                        NULL),
                    "failed to get public addresses",
                    goto done);
    }
    UTEST_SETUP_END


    TEST_ASSERT(!((NETIO_AddressSeq_get_length(&public_address) == 2) &&
                  NETIO_AddressSeq_get_reference(&public_address,0)->value.ipv4.address ==
                  NETIO_AddressSeq_get_reference(&public_address,1)->value.ipv4.address),
                "non-filtered nat address detected",UTEST_Stdio_printf("length = %d\n",NETIO_AddressSeq_get_length(&public_address));
                goto done);

    TEST_ASSERT(NETIO_AddressSeq_get_length(&public_address) == 1,
                "failed to reserve unicast address",UTEST_Stdio_printf("length = %d\n",NETIO_AddressSeq_get_length(&public_address));
                goto done);

    TEST_ASSERT(NETIO_AddressSeq_get_reference(&public_address,0)->
                value.ipv4.address == NETIO_htonl(address),
                "incorrect unicast address reserved",
                goto done);

    TEST_ASSERT(NETIO_AddressSeq_get_reference(&public_address,0)->
                port == port1,
                "incorrect unicast address reserved",
                goto done);

    TEST_ASSERT(NETIO_Address_get_kind(NETIO_AddressSeq_get_reference(&public_address,0))
                 == NETIO_ADDRESS_KIND_UDPv4,
                "incorrect unicast address reserved",
                goto done);

#define WAIT_FOR_RECEIVED(count_)\
{\
    RTI_INT32 max_wait_ = 50;\
    RTI_INT32 retry_ = 5;\
    udp_test_if.packet_rx_ok = count_;\
    do {\
        TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,\
                                         &src_address,&test_packet),\
                    "failed to send 1 uni-cast address",\
                    goto done);\
        while (udp_test_if.packet_rx_ok && max_wait_) \
        {\
            --max_wait_;\
            OSAPI_Thread_sleep(100);\
        }\
        --retry_;\
    } while (retry_ && udp_test_if.packet_rx_ok);\
    TEST_ASSERT((retry_ > 0) && (max_wait_ > 0), "UDP send/receive timed out",goto done);\
}

    /* IMPORTANT: Althought the reserved address is translated, we are still
     * listenting to the local address. Because we are behind the firewall,
     * and we don't have any NAT device we must send to the local address,
     * not the public address.
     *
     * Furthermore, bind_external and unbind_external both work behind the
     * firewall and should thus work on the local addresses.
     */
    NETIO_AddressSeq_set_length(&dsts,1);
    NETIO_AddressSeq_get_reference(&dsts,0)->kind = NETIO_ADDRESS_KIND_UDPv4;
    NETIO_AddressSeq_get_reference(&dsts,0)->port = port0;
    NETIO_AddressSeq_get_reference(&dsts,0)->value.ipv4.address = address;
    udp_test_if.packet_rx = 0;
    udp_test_if.expected_epoch = 50;
    data.epoch = 50;
    data.sn = 0;

    TEST_ASSERT(OSAPI_System_get_time(&start_time),
                "failed to get time",
                goto done);

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        TEST_ASSERT(NETIO_Interface_send(udp_if_tx,udp_if_tx,
                                         &src_address,&test_packet),
                    "failed to send 1 multi-cast address",
                    goto done);
    }

    OSAPI_Thread_sleep(1000);

    TEST_ASSERT(udp_test_if.packet_rx == 0,
                "sent count != received count",
                goto done);

    /* Listen on the UDP interface that was advertized
     */
    TEST_ASSERT(NETIO_Interface_bind_external(udp_if_rx,
                        NETIO_AddressSeq_get_reference(&dsts,0),
                        netio_ul,&dst_address,
                        NULL,NULL),
                "NETIO_Interface_bind_external failed",
                goto done);

    OSAPI_Thread_sleep(1000);

    udp_test_if.packet_rx = 0;
    data.epoch = 51;
    udp_test_if.expected_epoch = 51;
    data.sn = 0;
    udp_test_if.expected_sn = 0;

    for (count = 0; count < MAX_NETIO_SEND; ++count)
    {
        ++data.sn;
        ++udp_test_if.expected_sn;
        WAIT_FOR_RECEIVED(1);
    }

    TEST_ASSERT(OSAPI_System_get_time(&stop_time),
                "failed to get time",
                goto done);

    TEST_ASSERT(udp_test_if.packet_rx == MAX_NETIO_SEND,
                "sent count != received count",
                UTEST_Stdio_printf("sent/received: %d/%d\n",
                count,udp_test_if.packet_rx);goto done);

    OSAPI_NtpTime_subtract(&run_time,&stop_time,&start_time);
    OSAPI_NtpTime_to_microsec(&sec,&msec,&run_time);

    TEST_ASSERT(NETIO_Interface_unbind_external(udp_if_rx,
                            NETIO_AddressSeq_get_reference(&dsts,0),
                            netio_ul,&dst_address,NULL),
            "NETIO_Interface_unbind_external failed",
            goto done);

    TEST_ASSERT(NETIO_Interface_release_address(udp_if_rx,
                         NETIO_AddressSeq_get_reference(&public_address,0)),
                "NETIO_Interface_release_address failed",
                goto done);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_tx);

    NETIO_InterfaceFactory_delete_component(factory,udp_if_rx);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);


    retval = RTI_TRUE;

done:

    NETIO_AddressSeq_finalize(&dsts);
    NETIO_AddressSeq_finalize(&req_address);
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
    NETIO_AddressSeq_finalize(&public_address);

    return retval;

#undef MAX_NETIO_INTERACES
#undef WAIT_FOR_RECEIVED
}
#endif /* !RTI_CERT && !RTI_AUTOSAR */

#ifndef RTI_CERT
RTI_PRIVATE struct UDP_InterfaceTableEntry UDP_g_InterfaceTesterIfTable[]=
{
        {UDP_INTERFACE_INTERFACE_UP_FLAG,0x7f000001,0xffffff00,"eth0"},
        {UDP_INTERFACE_INTERFACE_UP_FLAG,0x7f000002,0xffffff00,"eth0"},
        {
                UDP_INTERFACE_INTERFACE_UP_FLAG |
                UDP_INTERFACE_INTERFACE_MULTICAST_FLAG,
                0x7f000003,0xffffff00,"eth1"
        },
        {
                UDP_INTERFACE_INTERFACE_MULTICAST_FLAG,
                0x7f000004,0xffffff00,"eth2"
        }
};
#endif /* !RTI_CERT */

RTI_PRIVATE unsigned char
UDPInterfaceTester_iftable(struct UTEST_Context *setting)
{
#ifndef RTI_CERT
    UTEST_VAR unsigned char retval = RTI_FALSE;
    UTEST_VAR RT_Registry_T *registry;
    UTEST_VAR struct RT_ComponentFactory *factory;
    UTEST_VAR NETIO_Interface_T *udp_if;
    UTEST_VAR DB_Database_T db = NULL;
    UTEST_VAR RTI_INT32 i,len;
    UTEST_VAR struct NETIO_InterfaceProperty udp_property =
                                    NETIO_InterfaceProperty_INITIALIZER;
    UTEST_VAR struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    UTEST_VAR struct UDP_InterfaceFactoryProperty udpf_property =
                                    UDP_InterfaceFactoryProperty_INITIALIZER;
    UTEST_VAR struct RT_RegistryProperty rt_property =
                                    RT_RegistryProperty_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR char long_name[UDP_INTERFACE_MAX_IFNAME+1];
    UTEST_VAR struct UDP_InterfaceTableEntry *if_entry;
    UTEST_VAR const char* intfname;
    UTEST_VAR const char* mcifname;
    UTEST_VAR RTI_UINT32 address;
    UTEST_VAR RTI_UINT32 netmask;
    UTEST_VAR RTI_INT32 int_property;
    UTEST_VAR RTI_UINT32 flags = 0;
    UTEST_VAR struct NETIO_Address *netio_address;
    UTEST_VAR struct NETIO_Netmask *netio_netmask;
    UTEST_VAR struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_AddressSeq address_seq = NETIO_AddressSeq_INITIALIZER;
    UTEST_VAR struct NETIO_NetmaskSeq netmask_seq = NETIO_NetmaskSeq_INITIALIZER;
    UTEST_VAR RTI_INT32 max_send_buffer_size;
    UTEST_VAR RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);

    UTEST_SETUP_BEGIN
    {
        db_property.max_tables = 32;
        db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

        TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                    "failed to create db",
                    goto done);

        registry = RT_Registry_get_instance();
        TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                    "failed to get property",
                    goto done);
        rt_property.db = db;
        rt_property.max_factories = 1;
        TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                    "failed to set rt property",
                    goto done);

        /* Disable automatic configuration for this test */
#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
        udpf_property.disable_auto_interface_config = RTI_FALSE;
#endif
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif


#ifdef RTI_CERT
        /* Cert cannot grow sequence maximum, must allocate maximum up-front */
        TEST_ASSERT(UDP_InterfaceTableEntrySeq_set_maximum(&udpf_property.if_table,
                                                        6),
                    "failed to set max udp if_table",
                    goto done);
#endif /* RTI_CERT */

        /*\test
            * Add invalid interface
            */
        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0x40000000,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed to add if entry",
                    goto done);

        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0x00001,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed to add if entry",
                    goto done);

        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0x80000001,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed to add if entry",
                    goto done);

        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0x80001000,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed to add if entry",
                    goto done);

        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0x80000000,
                                                "eth0",
                                                0xffff),
                    "should have failed to add if entry",
                    goto done);

        /*\test
            * Add valid interface
            */
        TEST_EXECUTE("Add valid interface");
        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0xffffff00,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "failed to add if entry",
                    goto done);

        /*\test
            * call with NULL table
            */
        TEST_EXECUTE("Add interface with NULL if_table");
        TEST_ASSERT(!UDP_InterfaceTable_add_entry(NULL,
                                                0x7f000001,0xffffff00,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed, if_table = NULL",
                    goto done);

        /*\test
            * call with NULL name
            */
        TEST_EXECUTE("Add interface with NULL name");
        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0xffffff00,
                                                NULL,
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed, name = NULL",
                    goto done);

        /*\test
            * call with name > UDP_INTERFACE_MAX_IFNAME
            *
            * UDP_INTERFACE_MAX_IFNAME includes zero, so must take that
            * into account
            */
        for (i = 0; i < (UDP_INTERFACE_MAX_IFNAME); i++)
        {
            long_name[i] = 'a';
        }
        long_name[i] = 0;

        TEST_EXECUTE("Add interface where name > UDP_INTERFACE_MAX_IFNAME");
        TEST_ASSERT(!UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000001,0xffffff00,
                                                long_name,
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "should have failed, name > UDP_INTERFACE_MAX_IFNAME",
                    goto done);

        /*\test
            * Add another valid interface with an existing name
            */
        TEST_EXECUTE("Add another valid interface with an existing name");
        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                0x7f000002,0xffffff00,
                                                "eth0",
                                                UDP_INTERFACE_INTERFACE_UP_FLAG),
                    "failed to add if entry",
                    goto done);

        /*\test
            * Add 3rd valid interface with multi-cast enabled
            */
        TEST_EXECUTE("Add another valid interface with multi-cast enabled");
        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                        0x7f000003,0xffffff00,
                                        "eth1",
                                        UDP_INTERFACE_INTERFACE_UP_FLAG |
                                        UDP_INTERFACE_INTERFACE_MULTICAST_FLAG),
                    "failed to add if entry",
                    goto done);

        /*\test
            * Add 4th valid interface with multi-cast enabled, but not up
            */
        TEST_EXECUTE("Add another valid interface with an existing name");
        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                        0x7f000004,0xffffff00,
                                        "eth2",
                                        UDP_INTERFACE_INTERFACE_MULTICAST_FLAG),
                    "failed to add if entry",
                    goto done);

        /*\test
            * verify that sequence has the correct number of elements and that each
            * element is correct
            */
        TEST_EXECUTE("Verify if_table sequence is correct");

        len = UDP_InterfaceTableEntrySeq_get_length(&udpf_property.if_table);
        TEST_ASSERT(len == 4,"if_table length is not correct",goto done);

        /* Table will be in reverse order of entries since an entry is added to
            * the end of the sequence
            */
        for (i = 0; i < len; ++i)
        {
            if_entry = UDP_InterfaceTableEntrySeq_get_reference(
                                                        &udpf_property.if_table,i);
            TEST_ASSERT(if_entry != NULL,"if_entry == NULL",goto done);
            TEST_ASSERT(if_entry->flags == UDP_g_InterfaceTesterIfTable[i].flags,
                        "if_entry->flags incorrect",
                        goto done);
            TEST_ASSERT(if_entry->address == UDP_g_InterfaceTesterIfTable[i].address,
                        "if_entry->address incorrect",
                        goto done);
            TEST_ASSERT(if_entry->netmask == UDP_g_InterfaceTesterIfTable[i].netmask,
                        "if_entry->netmask incorrect",
                        goto done);
            TEST_ASSERT(!REDA_String_compare(if_entry->ifname,
                                            UDP_g_InterfaceTesterIfTable[i].ifname),
                        "if_entry->ifname incorrect",
                        goto done);
        }

        /*\test
            * Reserve addresses set of addresses based on the static configuration. We
            * currently only test the equivalent of reading out the interface list.
            */
        TEST_EXECUTE("Add valid interface");
        UDP_InterfaceFactoryProperty_finalize(&udpf_property);
        UDP_InterfaceFactoryProperty_initialize(&udpf_property);

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
        /* Disable automatic configuration for this test */
        udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
#if UDP_TRANSFORMS_ENABLED
        udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

        flags = UDP_INTERFACE_INTERFACE_UP_FLAG;

        intfname = UTEST_Property_lookup_property(setting,"netio.udp.allow_interface");
        TEST_ASSERT(intfname != NULL,
                    "netio.udp.allow_interface property not found",
                    goto done);

        REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
        REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
        *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup(intfname);

        address = 0;
        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_address",(unsigned int*)&address),
                "failed to get interface address",
                goto done);

        netmask = 0;
        TEST_ASSERT(UTEST_Property_lookup_uint_property(setting,"netio.udp.allow_interface_netmask",(unsigned int*)&netmask),
                "failed to get interface netmask",
                goto done);

        int_property = 0;
        TEST_ASSERT(UTEST_Property_lookup_int_property(setting,"netio.udp.allow_interface_multicast",(int*)&int_property),
                "failed to get interface flags",
                goto done);

        if (int_property)
        {
            flags |= UDP_INTERFACE_INTERFACE_MULTICAST_FLAG;
        }

        mcifname = UTEST_Property_lookup_property(setting,"netio.udp.multicast_if");
        if (mcifname != NULL)
        {
            udpf_property.multicast_interface = REDA_String_dup(mcifname);
        }

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",(int*)&max_send_buffer_size))
        {
            udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
            udpf_property.max_send_message_size = max_send_buffer_size;
#endif
        }

        if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",(int*)&max_receive_buffer_size))
        {
            udpf_property.max_receive_buffer_size = max_receive_buffer_size;
            udpf_property.max_message_size = max_receive_buffer_size;
        }

        TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                                address,netmask,
                                                intfname,
                                                flags),
                    "failed to add if entry",
                    goto done);

        TEST_ASSERT(RT_Registry_register(registry,
                                    "udp0",
                                    UDP_InterfaceFactory_get_interface(),
                                    &udpf_property._parent._parent,
                                    NULL),
                    "failed to register UDP",
                    goto done);

        factory = RT_Registry_lookup(registry,"udp0");
        TEST_ASSERT(factory != NULL,"failed to lookup udp0 factory",goto done);

        udp_property.max_binds = 32;
        udp_property._parent.db = db;

        udp_if = NETIO_InterfaceFactory_create_component(factory,
                                        &udp_property._parent,
                                        NULL);

        TEST_ASSERT(udp_if != NULL,"udp_if == NULL",goto done);

#if NETIO_CONFIG_ENABLE_MULTICAST
        NETIO_AddressSeq_set_maximum(&req_address,2);
        NETIO_AddressSeq_set_length(&req_address,2);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_0,MC_ADDRESS_0);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,1),MC_PORT_1,address);
        NETIO_AddressSeq_set_maximum(&public_address,3);
        NETIO_AddressSeq_set_length(&public_address,0);
#else
        NETIO_AddressSeq_set_maximum(&req_address,1);
        NETIO_AddressSeq_set_length(&req_address,1);
        NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_1,address);
        NETIO_AddressSeq_set_maximum(&public_address,3);
        NETIO_AddressSeq_set_length(&public_address,0);
#endif

        TEST_ASSERT(NETIO_Interface_reserve_address(
                                        udp_if,&req_address,&public_address,NULL),
                    "failed to get public addresses",
                    goto done);

        len = NETIO_AddressSeq_get_length(&public_address);
#if NETIO_CONFIG_ENABLE_MULTICAST
        TEST_ASSERT(len == 2,"public_address length is not correct",goto done);
#else
        TEST_ASSERT(len == 1,"public_address length is not correct",goto done);
#endif

        /* Table will be in reverse order of entries since an entry is added to
        * the end of the sequence
        */
        for (i = 0; i < len; ++i)
        {
            netio_address = NETIO_AddressSeq_get_reference(&public_address,i);
            TEST_ASSERT(netio_address != NULL,"netio_address == NULL",goto done);
            TEST_ASSERT(NETIO_Address_get_kind(netio_address) == NETIO_ADDRESS_KIND_UDPv4,
                        "netio_address->kind",
                        goto done);
#if NETIO_CONFIG_ENABLE_MULTICAST
            if (i == 0)
            {
                TEST_ASSERT(netio_address->port == MC_PORT_0,
                        "netio_address->port incorrect",
                        goto done);
                TEST_ASSERT(netio_address->value.ipv4.address == MC_ADDRESS_0,
                        "netio_address->value.ipv4.address incorrect",
                        goto done);
            }
            else
#endif
            {
                TEST_ASSERT(netio_address->port == MC_PORT_1,
                        "netio_address->port incorrect",
                        goto done);
                TEST_ASSERT(netio_address->value.ipv4.address == address,
                        "netio_address->value.ipv4.address incorrect",
                        goto done);
            }
        }

#if NETIO_CONFIG_ENABLE_MULTICAST
        NETIO_AddressSeq_set_maximum(&address_seq,4);
        NETIO_NetmaskSeq_set_maximum(&netmask_seq,4);
#else
        NETIO_AddressSeq_set_maximum(&address_seq,3);
        NETIO_NetmaskSeq_set_maximum(&netmask_seq,3);
#endif

        TEST_ASSERT(NETIO_Interface_get_route_table(udp_if,&address_seq,&netmask_seq),
                    "failed to get route table",
                    goto done);

        TEST_ASSERT(NETIO_AddressSeq_get_length(&address_seq) == 3,
                    "route table address length incorrect",
                    goto done);

        TEST_ASSERT(NETIO_NetmaskSeq_get_length(&netmask_seq) == 3,
                    "route table netmask length incorrect",
                    goto done);

        len = NETIO_AddressSeq_get_length(&address_seq);
        for (i = 0; i < len; ++i)
        {
            netio_address = NETIO_AddressSeq_get_reference(&address_seq,i);
            TEST_ASSERT(netio_address != NULL,"netio_address == NULL",goto done);

            netio_netmask = NETIO_NetmaskSeq_get_reference(&netmask_seq,i);
            TEST_ASSERT(netio_netmask != NULL,"netio_netmask == NULL",goto done);

            TEST_ASSERT(NETIO_Address_get_kind(netio_address) == NETIO_ADDRESS_KIND_UDPv4,
                        "netio_address->kind",
                        goto done);

            if (i == 0)
            {
                TEST_ASSERT(netio_address->port == 0,
                        "netio_address->port incorrect",
                        goto done);
                TEST_ASSERT(netio_address->value.ipv4.address == 0,
                        "netio_address->value.ipv4.address incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->bits == 0,
                        "netio_netmask->bits incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->mask[0] == 0,
                        "netio_netmask->bits incorrect",
                        goto done);
            }
            else if (i == 1)
            {
                TEST_ASSERT(netio_address->port == 0,
                        "netio_address->port incorrect",
                        goto done);
                TEST_ASSERT(netio_address->value.ipv4.address == address,
                        "netio_address->value.ipv4.address incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->bits == 32,
                        "netio_netmask->bits incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->mask[0] == netmask,
                        "netio_netmask->netmask incorrect",
                        goto done);
            }
            else
            {
                TEST_ASSERT(netio_address->port == 0,
                        "netio_address->port incorrect",
                        goto done);
                TEST_ASSERT(netio_address->value.ipv4.address == 0xef000001,
                        "netio_address->value.ipv4.address incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->bits == 32,
                        "netio_netmask->bits incorrect",
                        goto done);

                TEST_ASSERT(netio_netmask->mask[0] == 0xe0000000,
                        "netio_netmask->bits incorrect",
                        goto done);

            }
        }
    }
    UTEST_SETUP_END

    len = NETIO_AddressSeq_get_length(&public_address);
    for (i = 0; i < len; ++i)
    {
        TEST_ASSERT(NETIO_Interface_release_address(udp_if,
                             NETIO_AddressSeq_get_reference(&public_address,i)),
                    "NETIO_Interface_release_address failed",
                    goto done);
    }

#ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp_if);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */

    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&req_address);
    NETIO_AddressSeq_finalize(&public_address);
    NETIO_AddressSeq_finalize(&address_seq);
    NETIO_NetmaskSeq_finalize(&netmask_seq);

    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
#endif
    return retval;

#undef MAX_NETIO_INTERACES
#undef WAIT_FOR_RECEIVED
#else
   CHECK_DO_RUN_TEST(setting);
   return RTI_TRUE;
#endif /* !RTI_CERT */
}

RTI_PRIVATE unsigned char
UDPInterfaceTester_invalid_ip(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct UDP_InterfaceFactoryProperty udpf_property =
                                    UDP_InterfaceFactoryProperty_INITIALIZER;
    struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    RTI_INT32 len;
    struct NETIO_InterfaceProperty udp_property =
                                    NETIO_InterfaceProperty_INITIALIZER;
    NETIO_Interface_T *udp_if;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *factory;
    struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;
    RTI_INT32 max_send_buffer_size;
    RTI_INT32 max_receive_buffer_size;
    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY
    
    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;
    TEST_ASSERT(DB_Database_create(&db,"udp_tester",&db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);
    
    registry = RT_Registry_get_instance();
    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                    "failed to get property",
                goto done);
    rt_property.db = db;
    rt_property.max_factories = 1;
    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "failed to set rt property",
                goto done);

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",(int*)&max_send_buffer_size))
    {
        udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
        udpf_property.max_send_message_size = max_send_buffer_size;
#endif
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",(int*)&max_receive_buffer_size))
    {
        udpf_property.max_receive_buffer_size = max_receive_buffer_size;
        udpf_property.max_message_size = max_receive_buffer_size;
    }

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
    /* Disable automatic configuration for this test */
    udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
    
    
    TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                                             0x00010101,0xffffff00,
                                             "invalid_ip",
                                             UDP_INTERFACE_INTERFACE_UP_FLAG),
                                            "failed to add if entry",
                                            goto done);
    
    TEST_ASSERT(RT_Registry_register(registry,
                                "udp0",
                                UDP_InterfaceFactory_get_interface(),
                                &udpf_property._parent._parent,
                                NULL),
                "failed to register UDP",
                goto done);
    
    factory = RT_Registry_lookup(registry,"udp0");
    TEST_ASSERT(factory != NULL,"failed to lookup udp0 factory",goto done);

    udp_property.max_binds = 32;
    udp_property._parent.db = db;

    udp_if = NETIO_InterfaceFactory_create_component(factory,
                                    &udp_property._parent,
                                    NULL);
    
    TEST_ASSERT(udp_if != NULL,"udp_if == NULL",goto done)

    NETIO_AddressSeq_set_length(&req_address,1);
    NETIO_AddressSeq_set_length(&public_address,0);
    TEST_ASSERT(NETIO_Interface_reserve_address(
                                    udp_if,&req_address,&public_address,NULL),
                "failed to get public addresses",
                goto done);
    len = NETIO_AddressSeq_get_length(&public_address);
    /* we requested 1 and added 1 to the if_table
     * but the invalid interface should be rejected*/
    TEST_ASSERT(len == 0, "Public address length in not correct", goto done);
    
 #ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp_if);

    TEST_ASSERT(RT_Registry_unregister(registry,
                                "udp0",
                                NULL,
                                NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                                "failed to finalize",
                                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif
   
    retval = RTI_TRUE;
    
done:
    
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&req_address);
    NETIO_AddressSeq_finalize(&public_address);
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
#endif
    return retval;
    
}

#if NETIO_CONFIG_ENABLE_MULTICAST
RTI_PRIVATE unsigned char
UDPInterfaceTester_mc_membership(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *factory;
    NETIO_Interface_T *udp_if_rx;
    struct NETIO_Address src_address;
    struct NETIO_Address dst_address;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct NETIO_AddressSeq public_address = NETIO_AddressSeq_INITIALIZER;
    struct NETIO_AddressSeq req_address = NETIO_AddressSeq_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    struct UDPInterfaceTesterData data;
    struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    OSAPI_Memory_zero(&data,sizeof(data));

    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

    TEST_ASSERT(DB_Database_create(&db,"udp_tester",
                                   &db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);

    TEST_ASSERT(UDPTestInterface_initialize(
                    (struct UDPTestInterface *)&udp_test_if,NULL,NULL,NULL),
                    "failed to initialize UDP",
                goto done);

    registry = RT_Registry_get_instance();
    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                "failed to get property",
                goto done);
    rt_property.db = db;
    rt_property.max_factories = 1;

    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "failed to set rt property",
                goto done);

    /* \test
     * Test that enabling multicast on a non-valid interface fails
     * MICRO-417
     */

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
    /* Use the static configuration to add an interface with a bogus IP
     * address and specify this interface as the multicast interface
     */
    udpf_property.disable_auto_interface_config = RTI_TRUE;
#endif
    udpf_property.multicast_interface = REDA_String_dup("invalid");
#if UDP_TRANSFORMS_ENABLED
    udpf_property.transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
#endif

    TEST_ASSERT(UDP_InterfaceTable_add_entry(&udpf_property.if_table,
                     0x00000001,0xffffff00,
                     "invalid",
                     UDP_INTERFACE_INTERFACE_MULTICAST_FLAG |
                     UDP_INTERFACE_INTERFACE_UP_FLAG),
                "failed to add if entry",
                goto done);


    REDA_StringSeq_set_maximum(&udpf_property.allow_interface,1);
    REDA_StringSeq_set_length(&udpf_property.allow_interface,1);
    *REDA_StringSeq_get_reference(&udpf_property.allow_interface,0) = REDA_String_dup("invalid");

    TEST_ASSERT(RT_Registry_register(registry,
                         "udp0",
                         UDP_InterfaceFactory_get_interface(),
                         &udpf_property._parent._parent,
                         NULL),
                "failed to register UDP",
                goto done);

    factory = RT_Registry_lookup(registry,"udp0");

    TEST_ASSERT(factory != NULL,"unknown factory",goto done);


    udp_property.max_binds = 32;
    udp_property._parent.db = db;

    /* This is the source address the receive interface is listening on */
    NETIO_Address_init(&src_address,0);
    NETIO_Address_init(&dst_address,0);

    udp_if_rx = NETIO_InterfaceFactory_create_component(factory,
                                   &udp_property._parent,
                                   NULL);
    TEST_ASSERT(udp_if_rx != NULL,"udp_if_rx == NULL",goto done);

    NETIO_AddressSeq_set_maximum(&public_address,3);
    NETIO_AddressSeq_set_length(&public_address,0);
    NETIO_AddressSeq_set_maximum(&req_address,1);
    NETIO_AddressSeq_set_length(&req_address,1);
    NETIO_Address_set_ipv4(NETIO_AddressSeq_get_reference(&req_address,0),MC_PORT_0,MC_ADDRESS_0);

    TEST_ASSERT(!NETIO_Interface_reserve_address(udp_if_rx,
                                                &req_address,&public_address,NULL),
                "should have failed to get public addresses",
                goto done);
#ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp_if_rx);

    TEST_ASSERT(RT_Registry_unregister(registry,"udp0",NULL,NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                "failed to finalize",
                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */
    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    NETIO_AddressSeq_finalize(&req_address);
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
    NETIO_AddressSeq_finalize(&public_address);
#endif /* !RTI_CERT */
    return retval;
}
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

MUST_CHECK_RETURN RTI_PRIVATE char*
UDPInterfaceTester_strrchr(const char *s,RTI_INT32 c)
{
    char *rval = NULL;

    if (s == NULL)
    {
        return NULL;
    }

    rval = (char*)(s + REDA_String_length(s));

    while ((rval >= s) && (*rval != c))
    {
        rval--;
    }

    if (rval < s)
    {
        return NULL;
    }

    return rval;
}

RTI_PRIVATE unsigned char
UDPInterfaceTester_test_strrchr(struct UTEST_Context *setting)
{
    unsigned char rval = RTI_FALSE;
    const char *string1 = "";
    const char *string2 = "010202:";
    const char *string3 = NULL;
    const char *string4 = "aa:bb::cc";
    const char *string5 = "aaabaabcc";
    char *ptr;

    CHECK_DO_RUN_TEST(setting);
    UTEST_SETUP_ONLY

    /*\test Test that NULL is returned for invalid string */
    ptr = UDPInterfaceTester_strrchr(NULL,0);
    TEST_ASSERT(ptr == NULL,"ptr != NULL",goto done);

    /*\test Test that \0 is returned */
    ptr = UDPInterfaceTester_strrchr(string1,0);
    TEST_ASSERT(*ptr == 0,"*ptr != NULL",goto done);

    /*\test Test that NULL is returned for char not found */
    ptr = UDPInterfaceTester_strrchr(string1,'a');
    TEST_ASSERT(ptr == NULL,"ptr != NULL",goto done);

    /*\test Test that ptr is returned for the right string 2 */
    ptr = UDPInterfaceTester_strrchr(string2,'0');
    TEST_ASSERT(!REDA_String_compare(ptr,"02:"),"ptr != 02:",goto done);

    ptr = UDPInterfaceTester_strrchr(string2,'2');
    TEST_ASSERT(!REDA_String_compare(ptr,"2:"),"ptr != 2:",goto done);

    ptr = UDPInterfaceTester_strrchr(string2,'1');
    TEST_ASSERT(!REDA_String_compare(ptr,"10202:"),"ptr != 10202:",goto done);

    /*\test Test that ptr is returned for the right string 3 */
    ptr = UDPInterfaceTester_strrchr(string3,'0');
    TEST_ASSERT(ptr == NULL,"ptr != NULL",goto done);

    /*\test Test that ptr is returned for the right string 4 */
    ptr = UDPInterfaceTester_strrchr(string4,'c');
    TEST_ASSERT(!REDA_String_compare(ptr,"c"),"ptr != c",UTEST_Stdio_printf("ptr=%s\n",ptr); goto done);

    ptr = UDPInterfaceTester_strrchr(string4,':');
    TEST_ASSERT(!REDA_String_compare(ptr,":cc"),"ptr != cc",goto done);

    ptr = UDPInterfaceTester_strrchr(string4,'b');
    TEST_ASSERT(!REDA_String_compare(ptr,"b::cc"),"ptr != b::cc",goto done);

    ptr = UDPInterfaceTester_strrchr(string4,'a');
    TEST_ASSERT(!REDA_String_compare(ptr,"a:bb::cc"),"ptr != a:bb::cc",goto done);

    /*\test Test that ptr is returned for the right string 5 "aaabaabcc"*/
    ptr = UDPInterfaceTester_strrchr(string5,'c');
    TEST_ASSERT(!REDA_String_compare(ptr,"c"),"ptr != c",goto done);

    ptr = UDPInterfaceTester_strrchr(string5,'b');
    TEST_ASSERT(!REDA_String_compare(ptr,"bcc"),"ptr != bcc",goto done);

    ptr = UDPInterfaceTester_strrchr(string5,'a');
    TEST_ASSERT(!REDA_String_compare(ptr,"abcc"),"ptr != abcc",goto done);

    rval = RTI_TRUE;

done:
    return rval;
}

struct UDPInterfaceTester_HostNameEntry
{
    char *hostname;
    RTI_BOOL expected_is_invalid;
    RTI_BOOL exptected_retval;
};

#ifndef RTI_ARINC653
#ifndef RTI_FREERTOS
#ifndef RTI_AUTOSAR
RTI_PRIVATE struct UDPInterfaceTester_HostNameEntry UDPInterfaceTester_fv_hostnames[]=
{
    /* hostname,expected_is_invalid,expected_retval */
    {"10.10.192.168",RTI_FALSE,RTI_TRUE},
    {"abc",RTI_FALSE,RTI_FALSE},
    {"a.b.c",RTI_FALSE,RTI_FALSE},
    {"a.b.c.",RTI_FALSE,RTI_FALSE},
    {"a.b.c-",RTI_FALSE,RTI_FALSE},
    {"a",RTI_FALSE,RTI_FALSE},
    {"ab",RTI_FALSE,RTI_FALSE},
    {"aaaaaaaaaaaaaaaaaaaa11111",RTI_FALSE,RTI_FALSE},
    {"aaaaaaaaaaaaaaaaaaaa1111",RTI_FALSE,RTI_FALSE},
    {"1a",RTI_FALSE,RTI_FALSE},
    {"support.rti.com",RTI_FALSE,RTI_TRUE},
    {"WWW.rti.CoM",RTI_FALSE,RTI_TRUE},
    {"WWW.rti.CoM-GW",RTI_FALSE,RTI_FALSE},
    {"WWW.rti.CoM-TAC",RTI_FALSE,RTI_FALSE},
    {"WWW.rti.CoM-GATEWAY",RTI_FALSE,RTI_FALSE},
    {".1a",RTI_FALSE,RTI_FALSE},
    {"-1a",RTI_FALSE,RTI_FALSE},
    {"$1a",RTI_FALSE,RTI_FALSE},
    {"@1a",RTI_FALSE,RTI_FALSE},
    {"aaa%^@*",RTI_FALSE,RTI_FALSE},
    {NULL,RTI_FALSE,RTI_FALSE}
};
#endif /* RTI_AUTOSAR */
#endif /* RTI_FREERTOS */
#endif /* RTI_ARINC653 */

#ifndef RTI_ARINC653
#ifndef RTI_FREERTOS
#ifndef RTI_AUTOSAR
/* Note : This function relies on RTIs NDS server to return IP address from host names.
 * Connection to RTI network is needed as function uses some machines names which
 * are located in RTI HQ.
 */
RTI_PRIVATE unsigned char
UDPInterfaceTester_test_hostname(struct UTEST_Context *setting)
{
    unsigned char retval = RTI_FALSE;
    struct NETIO_InterfaceProperty udp_property = NETIO_InterfaceProperty_INITIALIZER;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *factory;
    NETIO_Interface_T *udp_if;
    DB_Database_T db = NULL;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udpf_property = UDP_InterfaceFactoryProperty_INITIALIZER;
    struct RT_RegistryProperty rt_property = RT_RegistryProperty_INITIALIZER;
    struct NETIO_Address address = NETIO_Address_INITIALIZER;
    RTI_INT32 i;
    RTI_BOOL is_invalid;
    RTI_INT32 max_send_buffer_size;
    RTI_INT32 max_receive_buffer_size;

    CHECK_DO_RUN_TEST(setting);

    db_property.max_tables = 32;
    db_property.lock_mode = DB_LOCK_LEVEL_GLOBAL;

    TEST_ASSERT(DB_Database_create(&db,"udp_tester",
                                   &db_property,NULL) == DB_RETCODE_OK,
                "failed to create db",
                goto done);

    registry = RT_Registry_get_instance();
    TEST_ASSERT(RT_Registry_get_property(registry,&rt_property),
                "failed to get property",
                goto done);

    rt_property.db = db;
    rt_property.max_factories = 1;
    TEST_ASSERT(RT_Registry_set_property(registry,&rt_property),
                "failed to set rt property",
                goto done);

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_send_buffer_size",&max_send_buffer_size))
    {
        udpf_property.max_send_buffer_size = max_send_buffer_size;
#if UDP_TRANSFORMS_ENABLED
        udpf_property.max_send_message_size = max_send_buffer_size;
#endif
    }

    if (UTEST_Property_lookup_int_property(setting,"netio.udp.max_receive_buffer_size",&max_receive_buffer_size))
    {
        udpf_property.max_receive_buffer_size = max_receive_buffer_size;
        udpf_property.max_message_size = max_receive_buffer_size;
    }

    TEST_ASSERT(RT_Registry_register(registry,
                "udp",UDP_InterfaceFactory_get_interface(),
                &udpf_property._parent._parent,NULL),
                "failed to register UDP",
                goto done);

    factory = RT_Registry_lookup(registry,"udp");

    TEST_ASSERT(factory != NULL,"unknown factory",goto done);

    udp_property.max_binds = 32;
    udp_property._parent.db = db;

    udp_if = NETIO_InterfaceFactory_create_component(factory,
                                                   &udp_property._parent,NULL);

    TEST_ASSERT(udp_if != NULL,"udp_if == NULL",goto done);

    /* NOTE: This function only tests whether a hostname is valid or not
     *       and does not care if the name actually resolved to something.
     *       Thus, it only cares if is_invalid is TRUE/FALSE
     */

    for (i = 0; UDPInterfaceTester_fv_hostnames[i].hostname != NULL; ++i)
    {
        TEST_ASSERT(NETIO_Interface_resolve_address(udp_if,
                            UDPInterfaceTester_fv_hostnames[i].hostname,
                            &address,&is_invalid) ==
                        UDPInterfaceTester_fv_hostnames[i].exptected_retval,
                    "unexpected retval",
                    UTEST_Stdio_printf("hostname: %s",UDPInterfaceTester_fv_hostnames[i].hostname);
                    goto done);

        TEST_ASSERT(is_invalid == UDPInterfaceTester_fv_hostnames[i].expected_is_invalid,
                    "unexpected is_invalid",
                    UTEST_Stdio_printf("%s",UDPInterfaceTester_fv_hostnames[i].hostname);
                    goto done);
    }

#ifndef RTI_CERT
    NETIO_InterfaceFactory_delete_component(factory,udp_if);

    TEST_ASSERT(RT_Registry_unregister(registry,"udp",NULL,NULL),
                "failed to register UDP",
                goto done);

    TEST_ASSERT(RT_Registry_finalize(registry),
                "failed to finalize",
                goto done);

    TEST_ASSERT(DB_Database_delete(db) == DB_RETCODE_OK,
                "failed to delete db",
                goto done);
#endif /* !RTI_CERT */

    retval = RTI_TRUE;

done:
#ifndef RTI_CERT
    UDP_InterfaceFactoryProperty_finalize(&udpf_property);
#endif /* !RTI_CERT */

    return retval;
}
#endif /* !RTI_AUTOSAR */
#endif /* !RTI_FREERTOS */
#endif /* !RTI_ARINC653 */

RTI_PRIVATE struct UTEST_TestEntry UDPInterfaceTester_tests[]=
{
        RTITestCase("route",
                    UDPInterfaceTester_route,
                    TEST_ENABLED),

        RTITestCaseTags("iftable",
                UDPInterfaceTester_iftable,
                TEST_ENABLED,"MICRO-277"),

#ifndef RTI_AUTOSAR
        RTITestCase("unicast",
                    UDPInterfaceTester_unicast,
                    TEST_ENABLED),

#if NETIO_CONFIG_ENABLE_MULTICAST
        RTITestCaseTags("multicast",
                UDPInterfaceTester_multicast,
                TEST_ENABLED,"MICRO-158"),

        RTITestCaseTags("multicast_reserve",
                UDPInterfaceTester_multicast_reserve,
                TEST_ENABLED,
                "MICRO-144"),

        RTITestCaseTags("multicast_membership",
                UDPInterfaceTester_mc_membership,
                TEST_DISABLED,
                "MICRO-417"),
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */                

#if (!defined(RTI_CERT))
        RTITestCaseTags("nat",
                UDPInterfaceTester_nat,
                TEST_ENABLED,"MICRO-199,MICRO-1756"),
#endif /* !RTI_CERT */
#endif /* !RTI_AUTOSAR */

        RTITestCaseTags("max_message_size",
                UDPInterfaceTester_msg_size,
                TEST_ENABLED,"MICRO-352"),

        RTITestCaseTags("strrchr",
                UDPInterfaceTester_test_strrchr,
                TEST_ENABLED,"MICRO-101"),

#ifndef RTI_VX653
#ifndef RTI_FREERTOS
#ifndef RTI_AUTOSAR
        RTITestCaseTags("hostname",
                UDPInterfaceTester_test_hostname,
                TEST_ENABLED,"MICRO-1489,MICRO-1957"),
#endif /* !RTI_AUTOSAR */
#endif /* !RTI_FREERTOS */
#endif /* !RTI_VX653 */

        RTITestCaseTags("invalid_ip",
                UDPInterfaceTester_invalid_ip,
                TEST_ENABLED,"MICRO-1602"),
};

UT_DEFINE_SUBMODULE_RUNNER(UDPInterfaceTester,"udp")

