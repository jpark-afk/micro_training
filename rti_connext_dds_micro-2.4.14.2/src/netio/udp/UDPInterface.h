/*
 * FILE: UDPInterface.h - Exported UDP functions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020
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
 * 14dec2020,tk MICRO-2679/PR.28247 Removed use_interface_bind for RTI_CERT.
 * 28jun2016,tk MICRO-1548 Cleaned up system header file use
 *                         Moved system dependencies from header to source file
 */
 /*ci
  * \addtogroup NETIO_UDPInterfaceClass
  * @{
  */
#ifndef UDPInterface_h
#define UDPInterface_h

#include "osapi/osapi_config.h"

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

#ifndef netio_config_h
#include "netio/netio_config.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif

#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif

#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

#ifndef db_api_h
#include "db/db_api.h"
#endif

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef netio_log_h
#include "netio/netio_log.h"
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

#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif

#ifdef RTI_WIN32
#include <winsock2.h>
typedef SOCKET RTI_SOCKET;
#else
typedef int RTI_SOCKET;
#endif

#if UDP_TRANSFORMS_ENABLED
#include "UDPTransform.h"
#endif

/*ci
 * \brief Derived UDP route entry
 */
struct UDPRouteEntry
{
    /*ci
     * \brief Parent route entry
     */
    struct NETIORouteEntry _parent;
};

/*ci
 * \brief Implementation of the UDP interface
 */
struct UDP_Interface
{
    /*ci
     * \brief Base NETIO class
     */
    struct NETIO_Interface _parent;

    /*ci
     * \brief The properties the UDP interface was created with
     */
    struct NETIO_InterfaceProperty property;

    /*ci
     * \brief The factory that created the UDP interface
     */
    struct UDP_InterfaceFactory *factory;

    /*ci
     * \brief Socket used for all outgoing traffic, shared for all destinations
     */
    RTI_SOCKET send_socket;

    /*ci
     * \brief The listener the UDP interface was created with
     */
    struct NETIO_InterfaceListener listener;

    /*ci
     * \brief Table with UDP port entries, once for each unique port listened to
     */
    DB_Table_T rx_thread_table;

    /*ci
     * \brief The network interface to use for multicast destinations. 0
     *        means the interface assigned by the OS
     */
    struct NETIO_Address mc_bind_address;

    /*ci
     * \brief Flag to indicate if multicast is enabled on any of the allowed
     *        interfaces.
     */
    RTI_BOOL multicast_enabled;

#if UDP_TRANSFORMS_ENABLED
    /*ci
     * \brief The rules table for transformations
     */
    struct UDP_TransformTable transform_rules;

    /*ci
     * \brief At least one source rule was found
     */
    RTI_BOOL src_transforms_enabled;

    /*ci
     * \brief At least one destination rule was found
     */
    RTI_BOOL dst_transforms_enabled;

    /*ci
     * \brief At least one destination or source rule was found
     */
    RTI_BOOL transforms_enabled;

    /*ci
     * \brief Which locator to use when announcing transformed UDP
     */
    RTI_INT32 transform_locator_kind;

    /*ci
     * \brief When regular UDP can be used.
     */
    UDP_TransformUdpMode_T transform_udp_mode;

#endif

#if UDP_INTERFACE_BIND_ENABLED
    /*ci
     * \brief Bind to specific interface
     */
    RTI_BOOL use_interface_bind;
#endif

#ifndef RTI_CERT
    /* NAT */
    /*ci \brief The NAT table
     */
    DB_Table_T nat;
    
    /*ci \brief An index of local addresses in the NAT table
     */
    DB_Index_T nat_local_idx;

    /*ci \brief An index of public addresses in the NAT table
     */
    DB_Index_T nat_public_idx;
#endif /* !RTI_CERT */

#if defined(RTI_USE_NONBLOCKING_SOCKET) && defined(RTI_THREADX)
    /*ci
     * \brief Pointer to UDP thread structure used when only one thread is created
     *        for all receiving sockets.
     */
    struct OSAPI_Thread *udp_thread;

    /*ci
     * \brief Queue used to send a message to UDP thread when a new packet is
     * received
     */
    TX_QUEUE *udp_queue;

    /*ci
     * brief Memory used by \ref udp_queue
     */
    #define UDP_QUEUE_SIZE          100
    struct UDPPortEntry *udp_queue_memory[UDP_QUEUE_SIZE];

    /*ci
     * \brief Variable to use as unique receive buffer in case non blocking sockets
     *        are used. In that case it is not needed to process UDP packets
     *        in parallel, which allows the usage of only one receive buffer.
     */
    char *receive_buffer;
#endif /* defined(RTI_USE_NONBLOCKING_SOCKET) && defined(RTI_THREADX) */
};

/*ci
 * \brief Implementation of the UDP interface factory derived from the
 *        \ref RT_ComponentFactory
 */
struct UDP_InterfaceFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief Counter used to differentiate between different UDP interfaces
     *        created from the same factory
     */
    RTI_INT32 instance_counter;

    /*ci
     * \brief Pointer to properties for the factory. This pointer must be
     *        valid for as long as the factory is registered
     */
    struct UDP_InterfaceFactoryProperty *property;

    /*ci
     * \brief The listener on the UDP factory
     */
    struct RT_ComponentFactoryListener listener;
};

/*ci
 * \brief UDP bind entry
 */
struct UDPBindEntry
{
    /*ci
     * \brief base-class
     */
    struct NETIOBindEntry _parent;

    /*ci
     * \brief The number of binds to the port
     */
    RTI_INT32 ref_count;
};

/*ci
 * \brief UDP port entry
 */
struct UDPPortEntry
{
    /*ci
     * \brief The address to bind to and receive data from
     */
    struct NETIO_Address source;

    /*ci
     * \brief Receive buffer passed to the socket receive call
     */
    NETIO_Packet_T _rx_buffer;

#ifndef RTI_USE_NONBLOCKING_SOCKET
    /*ci
     * \brief Thread blocked on receive call
     */
    struct OSAPI_Thread *_rx_thread;
#endif /* RTI_USE_NONBLOCKING_SOCKET */

    /*ci
     * \brief The number of listeners to this port
     */
    RTI_UINT32 _ref_count;

    /*ci
     * \brief Back reference to the UDP receive interface that created this port
     */
    struct UDP_Interface *_udp_intf;

    /*ci
     * \brief the socket descriptor passed to the socket receive call
     */
    RTI_SOCKET _sock;
};

#endif

/*ci @} */
