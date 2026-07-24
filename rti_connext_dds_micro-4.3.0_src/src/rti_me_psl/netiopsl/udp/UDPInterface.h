/*
 * FILE: UDPInterface.h - Exported UDP functions
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc.
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
 * 28jun2016,tk MICRO-1548 Cleaned up system header file use
 *                         Moved system dependencies from header to source file
 */
 /*ci
  * \addtogroup NETIO_UDPInterfaceClass
  * @{
  */
#ifndef UDPInterface_h
#define UDPInterface_h

#ifndef rti_me_psl_dll_h
#include "rti_me_psl/rti_me_psl_dll.h"
#endif

#include "rti_me_psl.h"

#ifndef netio_config_h
#include "netio/netio_config.h"
#endif

#ifndef netio_interface_h
#include "netio/netio_interface.h"
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

#if RTI_WIN32
typedef SOCKET RTI_SOCKET;
#else
 typedef int RTI_SOCKET;
#endif

/*ci \brief Structure representing a UDP socket and its priority */
typedef struct RTI_SEND_SOCKET
{
    /*ci \brief The underlying socket handle */
    RTI_SOCKET sock;
    /*ci \brief priority value for the socket, used to avoid redundant setsockopt calls */
    RTI_INT32 priority;
} RTI_SEND_SOCKET;


#define UDP_INVALID_SOCKET -1
#define UDP_INVALID_PRIORITY -1

/*ci \brief Initializer for RTI_SEND_SOCKET structure */
#define RTI_SEND_SOCKET_INITIALIZER \
{ \
    UDP_INVALID_SOCKET, UDP_INVALID_PRIORITY \
}


#if UDP_TRANSFORMS_ENABLED
#include "UDPTransform.h"
#endif

/*ci \brief Maximum number of unicast sockets that will be allowed to be created */
#define UDP_UNICAST_SOCKETS_MAX 256

/*ci \brief Maximum number of transport priority values that can be mapped
 *          Value is based on the fact that IPv4 TOS value is 8 bits. This value is used to size the static
 *          precomputed priority mapping array in the UDP_TransportPriorityMap structure.
 */
#define UDP_PRIORITY_MAP_MAX 256

/*ci
 * \brief Precomputed priority mapping value used when calculating the DSCP values.
 */
struct UDP_TransportPriorityMap
{
    /*ci
     * \brief mask shifted to the least significatn 16 bytes
     */
    RTI_UINT32 mask_shifted;
    /*ci
     * \brief Number of bits to shift the priority value to align with the mask
     */
    RTI_UINT32 shift_nibbles;
    /*ci
     * \brief Range of transport priorities
     */
    RTI_UINT32 range;

    /*ci
     * \brief Precomputed mapping from transport priority values to socket indices
     *        for efficient lookup during packet transmission. The array is sized
     *        to cover all possible priority values (0-255) since TOS values are 8 bits.
     */
    RTI_UINT8 map[UDP_PRIORITY_MAP_MAX];
};

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

    /*ci
     * \brief Bind to specific interface
     */
    RTI_BOOL use_interface_bind;

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

#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)
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
     *        are used. In that case only one UDP thread is created, so UDP packets
     *        are not processed in parallel, which allows the usage of only one
     *        receive buffer.
     *        RTI have done some performance tests and setting this variable in
     *        internal ram would increase performance 0.5%-1.0%.
     */
    char *receive_buffer;
#endif /* defined(RTI_USE_NONBLOCKING_SOCKET) && defined(RTI_THREADX) */

    /* PSL related code */

    /*ci \brief upstream interface
     */
    struct NETIO_Interface *upstream_intf;

    /*ci \brief temporary sequences to call UDP_Interface_reserve_address with
     */
    struct NETIO_AddressSeq req_addr;

    /*ci \brief temporary sequences to call UDP_Interface_reserve_address with
     */
    struct NETIO_AddressSeq resvd_addr;

    /*ci Sequence of  allocated strings that are freed in delete_instance.
     */
    struct REDA_StringSeq if_name_seq;

#if NETIO_CONFIG_ENABLE_MULTICAST
    /*ci \brief Sequence of outgoing multicast interfaces
     */
    struct NETIO_AddressSeq multicast_interfaces;

    /*ci an array of multicast send sockets, one for each allowed multicast interface
     * allocated upto UDP_MAX_MULTICAST_INTERFACES, but only the first 'multicast_interface._length' entries are used
     */
    RTI_SEND_SOCKET *multicast_sockets;
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

    /*ci
     * \brief Priority mapping
     * \details Precaulculated mapping from transport_priority_mask and transport priority range.
     *            Only allocated when the transport_priority_mask is non-zero.
     */

    struct UDP_TransportPriorityMap *priority_map;

    /*ci an array of unicast send sockets */
    RTI_SEND_SOCKET *unicast_send_sockets;
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
    /*ciß
     * \brief Thread blocked on receive call
     */
    struct OSAPI_Thread *_rx_thread;
#endif

    /*ci
     * \brief The number of listeners to this portß
     */
    RTI_UINT32 _ref_count;

    /*ci
     * \brief The number of listeners to this port
     */
    RTI_UINT32 _shared_ref_count;

    /*ci
     * \brief Back reference to the UDP receive interface that created this port
     */
    struct UDP_Interface *_udp_intf;

    /*ci
     * \brief the socket descriptor passed to the socket receive call
     */
    RTI_SOCKET _sock;

#if defined(RTI_AUTOSAR) && !defined(RTI_CERT)
    /*ci
    * \brief Indicates whether the UDP port is open or not.
    */
    RTI_BOOL is_open;
#endif
};

#endif

#ifndef RTI_CERT
#if defined(RTI_AUTOSAR)
/*ci
 * \brief MICRO-12928: Recover UDP receive sockets on all registered AUTOSAR
 *        UDP interfaces after a NoCom/FullCom transition (Non-Cert,
 *        socket-owner mode only).
 *
 * Iterates over all UDP interfaces registered during creation and for
 * each one:
 *  - refreshes the stale send socket,
 *  - deregisters each stale receive socket from the recvnotify table,
 *  - acquires a fresh socket ID from TcpIp,
 *  - re-binds the socket to the same local address/port,
 *  - re-registers the original receive callback with the new socket ID.
 *
 * Best-effort: if one interface fails, recovery continues for the rest.
 *
 * \return RTI_TRUE if all interfaces recovered successfully, RTI_FALSE if
 *         any interface failed.
 */
extern RTI_BOOL
UDP_Interface_recover_sockets(TcpIp_LocalAddrIdType local_addr_id);

/*ci
 * \brief Change the state of the socket to closed
 *
 * \param[in] udp_intf The interface to search for the socket on
 * \param[in] socket_id The socket ID of the socket to close.
 * \param[out] socket_found RTI_TRUE if the socket was found
 *
 * \return RTI_TRUE if the function succeeded, RTI_FALSE on failure. If the socket was
 *         found, socket_found is TRUE on return, otherwise FALSE.
 */
extern RTI_BOOL
UDP_Interface_close_socket(TcpIp_SocketIdType socket_id);

#endif /* RTI_AUTOSAR */
#endif /* !RTI_CERT */

/*ci @} */
