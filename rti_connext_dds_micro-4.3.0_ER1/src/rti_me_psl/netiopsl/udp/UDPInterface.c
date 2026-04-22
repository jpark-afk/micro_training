/*
 * FILE: UDPInterface.c - UDP transport
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 25aug2016,tk MICRO-1563 Removed is_valid_hostname(), it is not useful
 * 25aug2016,eh MICRO-1559 Added support for VxWorks 64-Bit compatibility
 * 28jun2016,tk MICRO-1548 Cleaned up system header file use
 *                         Moved system dependencies from header to source file
 * 05oct2015,tk MICRO-1153 Exclude multicast altogether when not enabled
 *                         at compile time when reading the interface
 *                         list
 * 05oct2015,tk MICRO-1489 Validate a hostname with is_valid_hostname
 *                         before trying to resolve it.
 * 03aug2015,tk MICRO-1469/PR#15621 Re-factored get_next_interface() to avoid
 *                                  deactivated code.
 * 27jul2015,tk MICRO-1465/PR#15598 Only compare ports for unicast addresses
 *                                  in the receive() call
 * 21jul2015,tk MICRO-1459/PR#15577 Removed inactive code for Cert in
 *                                  address_ipv4_string_to_address
 * 21jul2015,tk MICRO-1453/PR#15569 Decrease length in add_route_entry() in case
 *                                  of failure
 * 21jul2015,tk MICRO-1447/PR#15535 Call get_next_interface_done() on error
 *                                  get_route_table()
 * 29jun2015,tk MICRO-1334/PR#15092 Validate inputs in _add_entry()
 * 29jun2015,tk MICRO-1328/PR#15057 Added check to prevent exceeding rcvd_addr
 * 09jun2015,tk MICRO-1286/PR#14934 Updated use of source/local_source address
 * 27may2015,tk MICRO-1231/PR#14729 Release rx table entry on error
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 12feb2015,tk MICRO-1227/PR#14786 Use max_binds to initialize the port-table
 * 12feb2015,tk MICRO-1090/PR#14088 Fixed comment for UDP_Interface_strrchr
 * 25feb2015,tk MICRO-1065 Don't return failure if forwarding upstream fails
 * 22feb2015,eh MICRO-1065 Fix receive failure by returning DB cursor
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk  MICRO-915/PR#11276 Fixed spelling error in function name
 *                                  UDP_Interface_resolve_ipv4_adddress
 * 31jul2014,tk  MICRO-241/PR#1413  Removed superfluous DB fields
 * 31jul2014,tk  MICRO-842/PR#9683  Removed superfluous paramaters in DB
 *                                  compare function
 * 15may2014,as  MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 05may2014,tk  MICRO-72 - Updated based on CR-232
 * 28apr2014,as  MICRO-778: Add robustness check to
 *               UDP_InterfaceFactory_initialize
 * 17apr2014,tk  MICRO-101: replaced strrchr with UDP_Interface_strrchr
 * 03feb2014,eh  MICRO-714: change ack to acknack
 * 31jul2013,eh  MICRO-352: max_message_size
 * 12jun2013,tk  MICRO-417: IP_ADD_MEMBERSHIP segfault on error
 * 17may2013,eh  MICRO-385: remove unused fns/fields of packet
 * 13mar2013,eh  MICRO-352: (max message/send/receive sizes)
 * 06feb2013,eh  Save and restore packet in receive()
 * 27apr2012,tk  Written
 */
/*ci
 * \file
 * \brief UDPv4 Implementation
 *
 * \details
 * This file implements a UDP NETIO interface. It supports multiple platforms,
 * such as Win32 and UNIX, and care must be taken to maintain this
 * functionality.
 *
 * \addtogroup NETIO_UDPInterfaceClass
 * @{
 */
#include "rti_me_psl.h"

#if !UDP_EXCLUDE_BUILTIN

#ifdef HAVE_SOCKET_API

#if defined(RTI_QNX6)
/* This prevents inline functions for byte-swapping from being
 * included, as these are not used by RTI.
 */
#define _NET_NETBYTE_H_INCLUDED
#include <sys/sockio.h>
#ifdef __EXT_BSD
#undef __EXT_BSD
#endif
#define __EXT_BSD 1
typedef unsigned long u_long;
#endif

#if defined(RTI_VXWORKS)
#include <unistd.h>
#if (!defined(RTI_CERT)) || \
    (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE)
#include <netinet/in.h>
#include <net/if.h>
#include <sockLib.h>
#include <ioLib.h>
#include <ioctl.h>
#include <hostLib.h>
#else
#include <certStack/tStkIn.h>
#include <certStack/tStkSocket.h>
#include <certStack/tStkSockLib.h>
#include <certStack/tStkIf.h>
#endif

#elif defined(RTI_UNIX) /* POSIX */
#if defined(RTI_DARWIN)
#define _DARWIN_C_SOURCE
#undef _SYS_TYPES_H_
#include <sys/types.h>
#include <sys/ioctl.h>


#elif defined(RTI_LINUX)
/* NOTE:
 * defining _BSD_SOURCE should have been sufficient. However, this file
 * includes osapi_config.h which already includes features.h which defines
 * the supported APIs. Thus, force features.h to be included again. This is
 * done to limit BSD features to the UDP interface.
 */
#if OSAPI_ENABLE_STRICT_POSIX
#undef _FEATURES_H
#define _BSD_SOURCE
#endif /* !OSAPI_ENABLE_STRICT_POSIX */

#if !ENABLE_FACE_COMPLIANCE
#include <sys/ioctl.h>
#endif /* ENABLE_FACE_COMPLIANCE */
#endif /* RTI_LINUX */

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>

#ifdef RTI_VOS
#if !ENABLE_FACE_COMPLIANCE
#include <ioctl.h>
#endif
#include <net/if_flags.h>
#endif /* VOS */

#elif defined(RTI_WIN32) /* !RTI_UNIX */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>
#ifndef EADDRINUSE
#define EADDRINUSE WSAEADDRINUSE
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#elif defined(LWIP_SYS) /* !RTI_WIN32 */
#include "lwip/arch.h"
#if defined(HAVE_SOCKET_API)
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#endif
#endif /* RTI_VXWORKS */

#if !defined(IF_NAMESIZE)
#define IF_NAMESIZE 1024
#elif defined(IF_NAMESIZE) && defined(RTI_WIN32)
/* IF_NAMESIZE is not supported by all VS versions */
#undef IF_NAMESIZE
#define IF_NAMESIZE 1024
#endif /* !defined(IF_NAMESIZE) */

#include "UDPInterface.h"

/*ci \brief If IPPROTO_IP is not defined specify a protocol of 0 which
 *          causes socket() to use an unspecified default protocol appropriate
 *          for the requested socket type. Only SOCK_DGRAM is requested in
 *          this file.
 */
#ifndef IPPROTO_IP
#define IPPROTO_IP (0)
#endif

/*ci
 * \def RTI_OSAPI_SOCKET_UDPV4_ADDRESS_NAME_LENGTH_MAX
 * \brief The maximum length the hostname
 */
#define RTI_OSAPI_SOCKET_UDPV4_ADDRESS_NAME_LENGTH_MAX 64

/*ci
 * \brief Default properties for the UDP factory
 */
LINK_SECTION_DATA_SDRAM
struct UDP_InterfaceFactoryProperty UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT =
                                      UDP_InterfaceFactoryProperty_INITIALIZER;

/*ci
 *\brief Forward declaration of the NETIO UDP interface implementation
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI UDP_Interface_fv_Intf;

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_receive(NETIO_Interface_T *netio_intf,
                     struct NETIO_Address *source,
                     NETIO_Packet_T *packet);

/*ci
 * \brief UDP specific interface definition
 */
struct UDP_NetworkIfInfo
{
    /*ci
     * \brief RTI_TRUE if the interface supports multicast
     */
    RTI_BOOL mc_enabled;

    /*ci
     * \brief RTI_TRUE if the interface address defined by the OS
     */
    RTI_UINT32 intf_address;

    /*ci
     * \brief RTI_TRUE if the interface netmask defined by the OS
     */
    RTI_UINT32 intf_netmask;

    /*ci
     * \brief The name of the lastname read from the OS interface list
     */
    char lastname[IF_NAMESIZE];

    /*ci
     * \brief Some platforms may provide multiple names for the interface,
     *        such as on Windows, in addition to the system assigned name
     *        lets the user specify their own name, such as
     *        Local Area Connection
     */
    char fname[IF_NAMESIZE];
};

/*ci
 * \brief Constant to initialize UDP_NetworkIfInfo
 */
#define UDP_NetworkIfInfo_INITIALIZER \
{\
    RTI_FALSE,\
    0,\
    0,\
    {0},\
    {0}\
}

/*ci
 * \brief Structure to hold network interface specific information when
 *       iterating over an interface list
 */
#if defined(RTI_UNIX) || defined(RTI_VXWORKS) || defined(RTI_FREERTOS) || defined(RTI_THREADX) || defined(RTI_AUTOSAR)
#define NETIO_HOST_INTERFACE_BUFFER_INIT 4096
struct UDP_IfContext
{
    /*ci
     * \brief RTI_TRUE if the context has been initialized, check before
     *        an iteration can begin
     */
    RTI_BOOL initialized;

#if NETIO_CONFIG_HAVE_IFCONF
    /*ci
     * \brief The size of the buffer holding network interface information
     */
    RTI_INT32 interfaceBufferSize;

    /*ci
     * \brief Buffer containing network interface information
     */
    char interfaceBuffer[NETIO_HOST_INTERFACE_BUFFER_INIT];

    /*ci
     * \brief Pointer to the current network buffer
     */
    char *bufferPointer;

    /*ci
     * \brief Platform specific interface configuration structure
     */
    struct ifconf ifc;

    /*ci
     * \brief Socket descriptor to read interface information
     */
    RTI_SOCKET s;
#endif

    /*ci
     * \brief The current index in the manually specified interface table
     */
    RTI_INT32 if_table_index;

    /*ci
     * \brief The number of entries in the interface table
     */
    RTI_INT32 if_table_length;
};

/*ci
 * \brief Constant to initialize the UDP_IfContext
 */
#if NETIO_CONFIG_HAVE_IFCONF
#define UDP_IFCONTEXT_INITIALIZER \
{\
    RTI_FALSE,\
    NETIO_HOST_INTERFACE_BUFFER_INIT,\
    {0},\
    NULL,\
    {0,{NULL}},\
    0,\
    0,\
    0\
}
#else
#define UDP_IFCONTEXT_INITIALIZER \
{\
    RTI_FALSE,\
    0,\
    0\
}
#endif


#elif defined (RTI_WIN32)
/* Win32 specific network interface context information
 */
#define NETIO_HOST_INTERFACE_BUFFER_INIT 1024
#define MAX_FRIENDLY_NAME 255
typedef DWORD(WINAPI * GETADAPTERSADDRESSES_FUNC)
    (ULONG Family,
     DWORD Flags,
     PVOID Reserved,
     PIP_ADAPTER_ADDRESSES pAdapterAddresses, PULONG pOutBufLen);

struct UDP_IfContext
{
    RTI_BOOL initialized;
    RTI_INT32 interfaceBufferSize;
    HINSTANCE hinstIphlpapiLib;
    GETADAPTERSADDRESSES_FUNC myGetAdaptersAddresses;
    IP_ADAPTER_ADDRESSES *interfaceBuffer;
    IP_ADAPTER_ADDRESSES *ifbufIter;
    char addressString[INET6_ADDRSTRLEN];
    RTI_INT32 if_table_index;
    RTI_INT32 if_table_length;
};

#define UDP_IFCONTEXT_INITIALIZER \
{\
    RTI_FALSE,\
    NETIO_HOST_INTERFACE_BUFFER_INIT\
}
#else
#warning

#endif

#ifdef RTI_VXWORKS
#ifndef RTI_CERT
#define NETIO_ioctl(s, option, pointer) ioctl(s, option, (OSAPI_IoctlArg)(pointer))
#else
#define NETIO_ioctl(s, option, pointer)
#endif
#else
#define NETIO_ioctl ioctl
#endif /* RTI_VXWORKS */

#endif

/*ci
 * The minimum hostname length as defined by RFC-952
 */
#define UDP_MIN_HOSTNAME_LEN   (2)

/*ci
 * The maximum hostname length as defined by RFC-1123
 */
#define UDP_MAX_HOSTNAME_LEN   (255)

/*ci
 * The maximum name part of a hostname as defined by RFC-952
 */
#define UDP_MAX_NAME_LEN       (24)


#if defined(RTI_WIN32)
/*ci
 * \brief Get the last error code from the socket API.
 *
 * \details
 *
 * Get the last error code set.  To make sure you are not reading an
 * error code corresponding to a previous call, set the error code to
 * RTI_OSAPI_SOCKET_NOERROR because making a netio call. This call is not
 * thread-safe on some OSes at the moment.
 */
#define NETIO_Socket_get_error WSAGetLastError
#else
#ifdef RTI_VXWORKS
#ifndef errno
#define errnoGet() errno
#endif
#endif
#define NETIO_Socket_get_error() errno
#endif

/*
 * The following function are simple wrappers from IP specific calls to
 * the standard BSD socket functions.
 */
#if (defined(LWIP_SYS) && defined(HAVE_SOCKET_API))
#define netiosock_bind(a,b,c)           lwip_bind(a,b,c)
#define netiosock_connect(a,b,c)        lwip_connect(a,b,c)
#define netiosock_setsockopt(a,b,c,d,e) lwip_setsockopt(a,b,c,d,e)
#define netiosock_getsockopt(a,b,c,d,e) lwip_getsockopt(a,b,c,d,e)
#define netiosock_recv(a,b,c,d)         lwip_recv(a,b,c,d)
#define netiosock_recvfrom(a,b,c,d,e,f) lwip_recvfrom(a,b,c,d,e,f)
#define netiosock_send(a,b,c,d)         lwip_send(a,b,c,d)
#define netiosock_sendto(a,b,c,d,e,f)   lwip_sendto(a,b,c,d,e,f)
#define netiosock_socket(a,b,c)         lwip_socket(a,b,c)
#define netiosock_sendmsg(a,b,c)        lwip_sendmsg(a,b,c)
#define netiosock_close(s)              lwip_close(s)
#define netiosock_gethostname(a,b)      -1 /* unsupported */
#if LWIP_DNS
#define netiosock_gethostbyname(n)      lwip_gethostbyname(n)
#else
#define netiosock_gethostbyname(n)      -1 /* unsupported */
#endif
#elif (defined(RTI_THREADX))

#include "threadxSocket.h"

#define netiosock_bind(a,b,c)           OSAPI_ThreadXSocket_bind(a,b,c)
#define netiosock_connect(a,b,c)        -1 /* unsupported */
#define netiosock_setsockopt(a,b,c,d,e) OSAPI_ThreadXSocket_setsockopt(a,b,c,d,e)
#define netiosock_getsockopt(a,b,c,d,e) -1 /* unsupported */
#define netiosock_recv(a,b,c,d)         -1 /* unsupported */
#define netiosock_recvfrom(a,b,c,d,e,f) OSAPI_ThreadXSocket_recvfrom(a,b,c,d,e,f)
#ifdef RTI_USE_NONBLOCKING_SOCKET
#define netiosock_recvnotify(a,b,c)     OSAPI_ThreadXSocket_recvnotify(a,b,c)
#define netiosock_packets_queued(a)     OSAPI_ThreadXSocket_packets_queued(a)
#endif /* RTI_USE_NONBLOCKING_SOCKET */
#define netiosock_send(a,b,c,d)         -1 /* unsupported */
#define netiosock_sendto(a,b,c,d,e,f)   OSAPI_ThreadXSocket_sendto(a,b,c,d,e,f)
#define netiosock_sendmsg(a,b,c)        OSAPI_ThreadXSocket_sendmsg(a,b,c)
#define netiosock_socket(a,b,c)         OSAPI_ThreadXSocket_socket(a,b,c)
#define netiosock_close(s)              OSAPI_ThreadXSocket_close(s)
#define netiosock_gethostname(a,b)      -1 /* unsupported */

#elif (defined(RTI_AUTOSAR))

#include "autosar/autosarSocket.h"

#define netiosock_bind(a,b,c)           NETIO_AutosarSocket_bind(a,b,c)
#define netiosock_connect(a,b,c)        -1 /* unsupported */
#define netiosock_setsockopt(a,b,c,d,e) NETIO_AutosarSocket_setsockopt(a,b,c,d,e)
#define netiosock_getsockopt(a,b,c,d,e) -1 /* unsupported */
#define netiosock_recv(a,b,c,d)         -1 /* unsupported */
#define netiosock_recvfrom(a,b,c,d,e,f) -1 /* unsupported */
#ifdef RTI_USE_NONBLOCKING_SOCKET
#define netiosock_recvnotify(a,b,c)     NETIO_AutosarSocket_recvnotify(a,b,c)
#define netiosock_packets_queued(a)     -1 /* unsupported */
#endif
#define netiosock_send(a,b,c,d)         -1 /* unsupported */
#define netiosock_sendto(a,b,c,d,e,f)   NETIO_AutosarSocket_sendto(a,b,c,d,e,f)
#define netiosock_sendmsg(a,b,c)        NETIO_AutosarSocket_sendmsg(a,b,c)
#define netiosock_socket(a,b,c)         NETIO_AutosarSocket_socket(a,b,c)
#define netiosock_close(s)              NETIO_AutosarSocket_close(s)
#define netiosock_gethostname(a,b)      -1 /* unsupported */
#define netiosock_gethostbyname(n)      -1 /* unsupported */

#else
#define netiosock_bind(a,b,c)           bind(a,b,c)
#define netiosock_connect(a,b,c)        connect(a,b,c)
#define netiosock_setsockopt(a,b,c,d,e) setsockopt(a,b,c,d,e)
#define netiosock_getsockopt(a,b,c,d,e) getsockopt(a,b,c,d,e)
#define netiosock_recv(a,b,c,d)         recv(a,b,c,d)
#define netiosock_recvfrom(a,b,c,d,e,f) recvfrom(a,b,c,d,e,f)
#define netiosock_send(a,b,c,d)         send(a,b,c,d)
#define netiosock_sendto(a,b,c,d,e,f)   sendto(a,b,c,d,e,f)
#define netiosock_sendmsg(a,b,c)        sendmsg(a,b,c)
#define netiosock_socket(a,b,c)         socket(a,b,c)
#if defined(RTI_WIN32)
#define netiosock_close(s)              closesocket(s)
#else
#define netiosock_close(s)              close(s)
#endif
#define netiosock_gethostname(a,b)      gethostname(a,b)
#if defined(RTI_WIN32)
/* gethostbyname(n) is deprecated in Windows. getaddrinfo() is used instead */
#define netiosock_gethostbyname(n)      -1
#else
#define netiosock_gethostbyname(n)      gethostbyname(n)
#endif

#endif

/* Determine if multicast can be enabled */
#if NETIO_CONFIG_ENABLE_MULTICAST
#if !defined(IP_MULTICAST_LOOP) || !defined(IP_MULTICAST_IF)   || \
    !defined(IP_MULTICAST_TTL)  || !defined(IP_ADD_MEMBERSHIP) || \
    !defined(IP_DROP_MEMBERSHIP)
#undef NETIO_CONFIG_ENABLE_MULTICAST
#define NETIO_CONFIG_ENABLE_MULTICAST 0
#endif
#endif

#if defined(RTI_WIN32)
#define IOVEC_MSG    WSAMSG
#define IOVEC_MSG_BUFS lpBuffers
#define IOVEC_MSG_LEN dwBufferCount
#define IOVEC_MSGBUF WSABUF
#define IOVEC_MSGBUF_BUF buf
#define IOVEC_MSGBUF_LEN len
#else
#define IOVEC_MSG    struct msghdr
#define IOVEC_MSG_BUFS msg_iov
#define IOVEC_MSG_LEN msg_iovlen
#define IOVEC_MSGBUF struct iovec
#define IOVEC_MSGBUF_BUF iov_base
#define IOVEC_MSGBUF_LEN iov_len
#endif

/*ci \brief An IP address is considered invalid if the first two MSB are zero.
 *          This mask checks for the above condition
 */
#define NETIO_VALID_IPV4_ADDRESS_MASK 0x000000FF

/*ci \brief IP Address format OK.
 */
#define NETIO_UDP_INTERFACE_ADDR_FORMAT_OK 0

/*ci \brief IP Address out of range.
 */
#define NETIO_UDP_INTERFACE_ADDR_FORMAT_OUT_OF_RANGE 1

/*ci \brief IP Address invalid format.
 */
#define NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID 2

#if defined(RTI_USE_NONBLOCKING_SOCKET) && defined(RTI_THREADX)

/*ci
 * \brief Initialize the UDP thread
 *
 * \details
 * Initialize the UDP thread used for all receiving sockets and all
 * related resources.
 *
 * \param[in] udp_intf   Pointer to UDP receive interface
 * \param[in] udp_property The properties registered with the UDP interface
 *
 * \return RTI_TRUE if UDP thread can be created or otherwise RTI_FALSE
 *
 * \sa \ref UDP_Interface_udp_thread_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_udp_thread_initialize(
                               struct UDP_Interface *udp_intf,
                               struct UDP_InterfaceFactoryProperty *udp_property);

#ifndef RTI_CERT
/*ci
 * \brief Finalize the UDP thread
 *
 * \details
 * Finalize the UDP thread used for all receiving sockets
 * and all related resources.
 *
 * \param[in] udp_intf   Pointer to UDP receive interface
 *
 * \sa \ref UDP_Interface_udp_thread_initialize
 */
RTI_PRIVATE void
UDP_Interface_udp_thread_finalize(struct UDP_Interface *udp_intf);
#endif /* !RTI_CERT */
#endif /* defined(RTI_USE_NONBLOCKING_SOCKET) && defined(RTI_THREADX) */

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Locate the rightmost occurrence of a character in a NULL terminated
 *        string
 *
 * \param[in] s String to search in
 * \param[in] c Character to search for
 *
 * \return Pointer to character if it exists, otherwise NULL
 */
MUST_CHECK_RETURN RTI_PRIVATE char*
UDP_Interface_strrchr(const char *s,RTI_INT32 c)
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

/*ci \brief Determine if this is a valid IP address
 *
 * \param[in] if_data Interface structure with address to check
 *
 * \return TRUE if this if_data contains a valid IP address, FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_is_valid_address(struct UDP_NetworkIfInfo *if_data)
{
    OSAPI_PRECONDITION(if_data == NULL,
                        return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("if_data",if_data,RTI_TRUE);)

    return (if_data->intf_address & NETIO_VALID_IPV4_ADDRESS_MASK) ?
                                                    RTI_TRUE:RTI_FALSE;
}

/*ci
 * \brief Initialize the socket module
 *
 * \details
 *
 * Some platforms require that the socket API is explicitly initialized. To
 * ensure consistent behavior this function is always called and will call the
 * required OS specific initializing functions.
 *
 * \return RTI_TRUE on success, RTI_FALSE in failure
 */
#if defined(RTI_WIN32)
RTI_PRIVATE RTI_BOOL
NETIO_SocketModule_init(void)
{
    WSADATA wd;
    int rtcode;

    LINK_SECTION_BSS_SDRAM
    static RTI_BOOL already_called = RTI_FALSE;

    if (already_called)
    {
        return RTI_TRUE;
    }

    if (!(rtcode = WSAStartup(0x0202, &wd)) ||  /* 2.2 success */
        !(rtcode = WSAStartup(0x0002, &wd)))
    {                           /* 2.0 success */
    }
    else
    {
        UDP_LOG_WSA_STARTUP(OSAPI_LOGKIND_ERROR,rtcode)
        return RTI_FALSE;
    }

    /* Check version returned must not be older than 2.0 */
    if (HIBYTE(wd.wVersion) != 2)
    {
        UDP_LOG_WINSOCK_INCOMPATIBLE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    already_called = RTI_TRUE;

    return RTI_TRUE;
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Called to convert from a machine name to a numerical IP address
 *
 * \param[in]  address_out  Structure holding the numerical IP address in
 *                          host order
 *
 * \param[in]  hostName_in  The machine name to convert
 *
 * \return 0 on success -1 on failure
 */
RTI_PRIVATE RTI_INT32
NETIO_Socket_get_hostbyname(struct in_addr *address_out, const char *hostname_in)
{
    RTI_INT32 result = -1;
    char hostname[RTI_OSAPI_SOCKET_UDPV4_ADDRESS_NAME_LENGTH_MAX];
#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) || defined(RTI_WIN32)
    struct addrinfo addr_hints;
    struct addrinfo *addr_result = NULL;
    struct addrinfo *addr_list = NULL;
#elif (defined(RTI_UNIX) || (LWIP_DNS && LWIP_SOCKET))
    struct hostent *hostEntry = NULL;
#endif

    OSAPI_PRECONDITION(address_out == NULL,goto done,
            OSAPI_Log_entry_add_pointer("address_out",address_out,RTI_TRUE);)

#if defined(RTI_WIN32)
    if (!NETIO_SocketModule_init())
    {
        UDP_LOG_SOCKET_INIT(OSAPI_LOGKIND_ERROR,NETIO_Socket_get_error())
        goto done;
    }
#endif

    if (hostname_in == NULL)
    {
        if (netiosock_gethostname(
            hostname, RTI_OSAPI_SOCKET_UDPV4_ADDRESS_NAME_LENGTH_MAX - 1) != 0)
        {
            UDP_LOG_GETHOSTNAME(OSAPI_LOGKIND_ERROR,
                                "localhost",NETIO_Socket_get_error())
            goto done;
        }

        hostname_in = hostname;
    }

#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) || defined(RTI_WIN32)
    OSAPI_Memory_zero(&addr_hints,sizeof(addr_hints));
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = IPPROTO_UDP;
    result = getaddrinfo((char *)hostname_in,NULL,&addr_hints,&addr_result);
    if (result != 0)
    {
        UDP_LOG_GETHOSTNAME(OSAPI_LOGKIND_ERROR,hostname_in,result)
        goto done;
    }
    addr_list = addr_result;

    while (addr_result != NULL)
    {
        if (addr_result->ai_family == AF_INET)
        {
            OSAPI_Memory_copy(
                      address_out,
                      &((struct sockaddr_in*)addr_result->ai_addr)->sin_addr,
                      sizeof(struct in_addr));
            break;
        }
        else
        {
            addr_result = addr_result->ai_next;
        }
    }

    freeaddrinfo(addr_list);
#elif defined(RTI_UNIX) || (LWIP_DNS && LWIP_SOCKET)
    hostEntry = (struct hostent *)netiosock_gethostbyname((char *)hostname_in);
    if (!hostEntry)
    {
        UDP_LOG_GETHOSTNAME(OSAPI_LOGKIND_ERROR,
                            hostname_in,NETIO_Socket_get_error())
        goto done;
    }

    OSAPI_Memory_copy(address_out, hostEntry->h_addr_list[0],
                      (RTI_UINT32)hostEntry->h_length);

#elif defined(RTI_VXWORKS)
    address_out->s_addr = hostGetByName((char *)hostname_in);

    if (address_out->s_addr == (unsigned int)ERROR)
    {
        UDP_LOG_GETHOSTNAME(OSAPI_LOGKIND_ERROR,
                            hostname_in,NETIO_Socket_get_error())
        goto done;
    }
#elif defined(RTI_AUTOSAR)
    /* This is actually not supported in Autosar but we allow compilation without
     * RTI_CERT flag. This is only needed in case gcc extensions are disabled in
     * the compiler; in that case preprocessor directive #warning is not available
     */
    UDP_LOG_GETHOSTNAME(OSAPI_LOGKIND_ERROR,
                        hostname_in,NETIO_Socket_get_error())
    goto done;
#else
#warning "Micro Custom Port: gethostbyname is not supported, implement NETIO_Socket_get_hostbyname()"
#endif

#if !defined(RTI_AUTOSAR)
    result = 0;
#endif

done:
    return result;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
#define T struct UDP_NatEntry
#define TSeq UDP_NatEntrySeq
#include "reda/reda_sequence_defn.h"
#endif /* !RTI_CERT */

#define T struct UDP_InterfaceTableEntry
#define TSeq UDP_InterfaceTableEntrySeq
#include "reda/reda_sequence_defn.h"

#if UDP_TRANSFORMS_ENABLED
#define T struct UDP_TransformRule
#define TSeq UDP_TransformRuleSeq
#include "reda/reda_sequence_defn.h"
#endif

/*ci
 * \brief Check if an address is multicast or not
 *
 * \param[in] address NETIO_Address to check
 *
 * \return RTI_TRUE if the IP address is multicast, RTI_FALSE if not
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_is_multicast(const struct NETIO_Address *const address)
{
    return ((NETIO_ntohl(address->value.ipv4.address) >= 0xe0000000) &&
            (NETIO_ntohl(address->value.ipv4.address) <= 0xefffffff));
}

RTI_BOOL
UDP_InterfaceTable_add_entry(struct UDP_InterfaceTableEntrySeq *seq,
                             RTI_UINT32 address,
                             RTI_UINT32 netmask,
                             const char *ifname,
                             RTI_UINT32 flags)
{
    RTI_INT32 len,max;
    struct UDP_InterfaceTableEntry *entry = NULL;
    RTI_BOOL no_one = RTI_FALSE;
    RTI_UINT32 mask;

    OSAPI_PRECONDITION_ALWAYS((seq == NULL) || (ifname == NULL) ||
                              ((flags &
                               ~(UDP_INTERFACE_INTERFACE_UP_FLAG |
                                 UDP_INTERFACE_INTERFACE_MULTICAST_FLAG)) != 0),
                              return RTI_FALSE,
                      OSAPI_Log_entry_add_pointer("seq",seq,RTI_FALSE);
                      OSAPI_Log_entry_add_uint("flags",flags,RTI_FALSE);
                      OSAPI_Log_entry_add_pointer("ifname",ifname,RTI_TRUE);)

    OSAPI_PRECONDITION_ALWAYS(REDA_String_length(ifname) >
                                  (UDP_INTERFACE_MAX_IFNAME-1),
                                  return RTI_FALSE,
                      OSAPI_Log_entry_add_pointer("ifname",seq,RTI_FALSE);
                      OSAPI_Log_entry_add_uint("length",REDA_String_length(ifname),RTI_FALSE);
                      OSAPI_Log_entry_add_int("max",UDP_INTERFACE_MAX_IFNAME-1,RTI_TRUE);)

    /* Ensure the netmask is valid (0 or one more 1s followed by 0s
     * There is no check on the address
     */
    mask = 0x80000000;
    for (len = 0; len < 32; len++)
    {
        if (no_one && (netmask & mask))
        {
            break;
        }

        if (!(netmask & mask))
        {
            no_one = RTI_TRUE;
        }

        mask = mask >> 1;
    }

    if (len < 32)
    {
        /* The netmask was either on the form 0...1 or 10...1 */
        return RTI_FALSE;
    }

    len = UDP_InterfaceTableEntrySeq_get_length(seq);
    max = UDP_InterfaceTableEntrySeq_get_maximum(seq);

    if (len == max)
    {
        if (!UDP_InterfaceTableEntrySeq_set_maximum(seq,max+1))
        {
            return RTI_FALSE;
        }
    }

    if (!UDP_InterfaceTableEntrySeq_set_length(seq,len+1))
    {
        return RTI_FALSE;
    }

    entry = UDP_InterfaceTableEntrySeq_get_reference(seq,len);
    if (entry == NULL)
    {
        return RTI_FALSE;
    }

    entry->flags = flags;
    entry->address = address;
    entry->netmask = netmask;
    OSAPI_Memory_copy(entry->ifname,ifname,REDA_String_length(ifname)+1);

    return RTI_TRUE;
}

#ifdef HAVE_SOCKET_API

#ifndef RTI_CERT
/*\brief Check if two entries in the NAT table are equal
 *
 * \param[in] flags database flags for the comparison
 * \param[in[ op1 The left side of the comparison
 * \param[in] op2 THe right side of the comparison
 *
 * return If the entries are equal, a negative number of op1 < op2 and
 *        a positive number if op2 > op1.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_compare_nat_entry(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct UDP_NatEntry *left = (struct UDP_NatEntry*)op1;
    const struct UDP_NatEntry *right = (struct UDP_NatEntry*)op2;
    RTI_INT32 cval;
    UNUSED_ARG(flags);

    cval = NETIO_Address_compare(&left->local_address,&right->local_address);
    if (cval)
    {
        return cval;
    }

    return NETIO_Address_compare(&left->public_address,&right->public_address);
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*\brief Check if two local address the NAT table are equal
 *
 * \param[in] flags database flags for the comparison
 * \param[in[ op1 The left side of the comparison
 * \param[in] op2 THe right side of the comparison
 *
 * return If the entries are equal, a negative number of op1 < op2 and
 *        a positive number if op2 > op1.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_compare_nat_local(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct UDP_NatEntry *left = (struct UDP_NatEntry*)op1;
    const struct NETIO_Address *right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        right = (const struct NETIO_Address *)op2;
    }
    else
    {
        right = &((struct UDP_NatEntry*)op2)->local_address;
    }

    return NETIO_Address_compare(&left->local_address,right);
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*\brief Check if two public address the NAT tables are equal
 *
 * \param[in] flags database flags for the comparison
 * \param[in[ op1 The left side of the comparison
 * \param[in] op2 THe right side of the comparison
 *
 * return If the entries are equal, a negative number of op1 < op2 and
 *        a positive number if op2 > op1.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_compare_nat_public(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct UDP_NatEntry *left = (struct UDP_NatEntry*)op1;
    const struct NETIO_Address *right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        right = (const struct NETIO_Address *)op2;
    }
    else
    {
        right = &((struct UDP_NatEntry*)op2)->public_address;
    }

    return NETIO_Address_compare(&left->public_address,right);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Compare entries in the table of UDP port entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A UDPPortEntry already in the database
 * \param[in] op2   Either a UDPPortEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_compare_port(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct UDPPortEntry *record_left = (struct UDPPortEntry*)op1;
    const struct NETIO_Address *id_right;
    struct UDP_Interface *udp_intf;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const struct NETIO_Address *)op2;
    }
    else
    {
        id_right = &((struct UDPPortEntry*)op2)->source;
    }

    if (record_left->source.port > id_right->port)
    {
        return 1;
    }

    if (record_left->source.port < id_right->port)
    {
        return -1;
    }

    udp_intf = record_left->_udp_intf;

    /* NOTE: By default unicast ports are shared across all interface address,
     * and multicast addresses are bound to specific interface.
     *
     * However, it is possible to change this behavior:
     * - If use_interface_bind is TRUE, then unicast sockets are also bound
     *   to the interface address, otherwise to ANY.
     * - If disable_multicast_bind is FALSE (default), then multicast sockets
     *   are also bound to the multicast address, otherwise to ANY.
     */
    if (!udp_intf->use_interface_bind &&
        !UDP_Interface_is_multicast(&record_left->source))
    {
        return 0;
    }

    if (udp_intf->factory->property->disable_multicast_bind &&
        UDP_Interface_is_multicast(&record_left->source))
    {
        return 0;
    }

    /* Either unicast and use_interface_bind OR
     * multicast and not disable_multicast_bind
     */
    if (NETIO_ntohl(record_left->source.value.ipv4.address) >
        NETIO_ntohl(id_right->value.ipv4.address))
    {
        return 1;
    }

    if (NETIO_ntohl(record_left->source.value.ipv4.address) <
        NETIO_ntohl(id_right->value.ipv4.address))
    {
        return -1;
    }

    return 0;
}

/*ci
 * \brief Create a socket and set the socket properties based on the UDP
 *        factory properties
 *
 * \param[in] udp_intf The UDP interface to create the socket for
 * \param[out] sock The socket structure to update with the created socket
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_create_send_socket(const struct UDP_Interface *udp_intf, RTI_SEND_SOCKET *sock)
{
#if defined(RTI_FREERTOS)
    UNUSED_ARG(udp_intf);
#endif
    sock->priority = UDP_INVALID_PRIORITY;
    sock->sock = netiosock_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock->sock == UDP_INVALID_SOCKET)
    {
        UDP_LOG_SOCKET_CREATE(OSAPI_LOGKIND_ERROR,errno)
        return RTI_FALSE;
    }

#if defined(RTI_UNIX) && \
    ((ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
     (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE))
    if (fcntl(sock->sock, F_SETFD, FD_CLOEXEC) == -1)
    {
        UDP_LOG_SOCKET_SETFD(OSAPI_LOGKIND_ERROR,errno)
        netiosock_close(sock->sock);
        sock->sock = UDP_INVALID_SOCKET;
        return RTI_FALSE;
    }
#endif

#ifndef RTI_FREERTOS
    if (netiosock_setsockopt(sock->sock, SOL_SOCKET, SO_SNDBUF,
                 (char *)&udp_intf->factory->property->max_send_buffer_size,
                 sizeof(udp_intf->factory->property->max_send_buffer_size)) < 0)
    {
        UDP_LOG_SOCKET_SNDBUF(OSAPI_LOGKIND_ERROR,
                              udp_intf->factory->property->max_send_buffer_size,
                              errno)
        netiosock_close(sock->sock);
        sock->sock = UDP_INVALID_SOCKET;
        return RTI_FALSE;
    }
#endif

    return RTI_TRUE;
}

#if NETIO_CONFIG_ENABLE_MULTICAST
/*ci
 * \brief Set multicast option on a socket
 *
 * \param[in] sock     The socket to set the multicast option on
 * \param[in] option   The socket option to set:
 *                         0 = disable multicast loopback
 *                         1 = enable multicast loopback
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_set_multicast_loopback_option(RTI_SEND_SOCKET sock,RTI_UINT8 option)
{
    if (netiosock_setsockopt(sock.sock,IPPROTO_IP,
                             IP_MULTICAST_LOOP,(void*)&option,
                             sizeof(option)) < 0)
    {
        UDP_LOG_SOCKET_SET_MCAST(OSAPI_LOGKIND_ERROR,errno)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Enable multicast loopback on an socket
 *
 * \param[in] sock     The socket to enable loopback on
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_enable_multicast_loopback(RTI_SEND_SOCKET sock)
{
    return UDP_Interface_set_multicast_loopback_option(sock,1);
}

/*ci
 * \brief Disable  multicast loopback on socket
 *
 * \param[in] sock The socket to disable loopback on
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_disable_multicast_loopback(RTI_SEND_SOCKET sock)
{
    return UDP_Interface_set_multicast_loopback_option(sock,0);
}

/*ci
 * \brief Configure  multicast loopback on socket
 *
 * \param[in] udp_intf The UDP interface to configure the socket on
 * \param[in] sock     The socket to configure
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_configure_multicast_loopback(const struct UDP_Interface *udp_intf,
                                          RTI_SEND_SOCKET sock)
{
    /* Set the multicast loopback option,0 = disable, 1 = enable */
    if (udp_intf->factory->property->multicast_loopback_disabled)
    {
        OSAPI_TRACE_NET("disable multicast loopback",RTI_TRUE)
        return UDP_Interface_disable_multicast_loopback(sock);
    }

    OSAPI_TRACE_NET("enable multicast loopback",RTI_TRUE)
    return UDP_Interface_enable_multicast_loopback(sock);
}

/*ci
 * \brief Set bind properties for a socket used to receive multicast
 *
 * \param[in] udp_intf The UDP interface
 * \param[in] sock The socket to set the options on
 * \param[in] src_addr The multicast adddress
 * \param[in,out] src_addr The socket structure to update
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_bind_multicast_socket(const struct UDP_Interface *udp_intf,
                                    RTI_SOCKET sock,
                                    const struct NETIO_Address *src_addr,
                                    struct sockaddr_in *sock_addr)
{

#if !defined(RTI_FREERTOS)
    const int YES = 1;
    OSAPI_TRACE_NET("set multi-cast port reuse:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

#if defined(RTI_IRIX) || defined(RTI_LYNX) || \
    (defined(RTI_VXWORKS) && VXWORKS_VERSION_5_4_OR_BETTER) \
    || defined(RTI_QNX) || (defined(RTI_INTY) && defined(RTI_IPEAK)) || defined(RTI_DARWIN)
    if (netiosock_setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *)&YES, sizeof(int)) != 0)
#else
    if (netiosock_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&YES, sizeof(int)) != 0)
#endif
    {
        UDP_LOG_SOCKET_REUSE_PORT(OSAPI_LOGKIND_ERROR,errno)
        return RTI_FALSE;
    }
#ifdef RTI_WIN32
    sock_addr->sin_addr.s_addr = NETIO_htonl(INADDR_ANY);
#else
    if (udp_intf->factory->property->disable_multicast_bind)
    {
        sock_addr->sin_addr.s_addr = NETIO_htonl(INADDR_ANY);
    }
    else
    {
        sock_addr->sin_addr.s_addr = src_addr->value.ipv4.address;
    }
#endif
#else /* RTI_FREERTOS */
    UNUSED_ARG(udp_intf);
    UNUSED_ARG(sock);
    UNUSED_ARG(src_addr);
    sock_addr->sin_addr.s_addr = NETIO_htonl(INADDR_ANY);
#endif /* !RTI_FREERTOS */

    return RTI_TRUE;
}

/*ci
 * \brief Add the interface as an output multicast interface
 *
 * \param[in] src_intf The UDP interface
 * \param[in] if_data Network interface data
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_add_multicast_interface(struct UDP_Interface *src_intf,
                                      const struct UDP_NetworkIfInfo *if_data)
{
    RTI_INT32 index, length;
    struct NETIO_Address *an_address = NULL;
    RTI_SEND_SOCKET s = RTI_SEND_SOCKET_INITIALIZER;
    RTI_BOOL add_intf = RTI_TRUE;
    RTI_UINT8 option;

    if (src_intf->factory->property->disable_multicast_interface_select)
    {
        return RTI_TRUE;
    }

    if ((src_intf->factory->property->multicast_interface != NULL) &&
        (src_intf->mc_bind_address.value.ipv4.address != INADDR_ANY))
    {
        /* The multicast interface has already been added */
        return RTI_TRUE;
    }

    /* If src_intf->factory->property->multicast_interface is specified
     * then only add one outgoing interface.
     */
    if (src_intf->factory->property->multicast_interface != NULL)
    {
        if (!REDA_String_ncompare(if_data->lastname,
                                  src_intf->factory->property->multicast_interface,
                                  IF_NAMESIZE))
        {
            add_intf = RTI_TRUE;

           /* This is only to maintain backward compatability and
            * prevent the interface from being set multiple times.
            * mc_bind_address is not used for anything else, everything else is
            * using the multicast_interface sequence
            */
            src_intf->mc_bind_address.value.ipv4.address = if_data->intf_address;
        }
#ifdef RTI_WIN32
        else if (!REDA_String_compare(if_data->fname,
                             src_intf->factory->property->multicast_interface))
        {
            add_intf = RTI_TRUE;
            src_intf->mc_bind_address.value.ipv4.address = if_data->intf_address;
        }
#endif
        else
        {
            add_intf = RTI_FALSE;
        }
    }

    if (!add_intf)
    {
        return RTI_TRUE;
    }

    length = NETIO_AddressSeq_get_length(&src_intf->multicast_interfaces);
    for (index = 0; index < length; index++)
    {
        an_address = NETIO_AddressSeq_get_reference(
                                        &src_intf->multicast_interfaces,index);
        if (an_address == NULL)
        {
            continue;
        }
        if (an_address->value.ipv4.address == if_data->intf_address)
        {
            break;
        }
    }

    if (index < length)
    {
        /* The outgoing multicast interface has already been added */
        return RTI_TRUE;
    }

    if (!NETIO_AddressSeq_set_length(&src_intf->multicast_interfaces,index + 1))
    {
        return RTI_FALSE;
    }

    /* Add the interface as an outgoing multicast interface */
    an_address = NETIO_AddressSeq_get_reference(
                                &src_intf->multicast_interfaces,index);

    if (an_address == NULL)
    {
        /* This is not expected since set_length should already have failed */
        return RTI_FALSE;
    }

    NETIO_Address_init(an_address,0);

    /* This is the address of the NIC, _not_ the multicast address */
    an_address->value.ipv4.address = if_data->intf_address;

    if (!UDP_Interface_create_send_socket(src_intf, &s))
    {
        return RTI_FALSE;
    }

    src_intf->multicast_sockets[index] = s;
    if (netiosock_setsockopt(s.sock, IPPROTO_IP,
             IP_MULTICAST_IF,(char *)&if_data->intf_address,
             sizeof(if_data->intf_address)) < 0)
    {
        UDP_LOG_SOCKET_SET_MCASTIF(OSAPI_LOGKIND_ERROR,errno)
        return RTI_FALSE;
    }

    if (!UDP_Interface_configure_multicast_loopback(src_intf,s))
    {
        return RTI_FALSE;
    }

    option = (RTI_UINT8)src_intf->factory->property->multicast_ttl;
    if (netiosock_setsockopt(s.sock, IPPROTO_IP, IP_MULTICAST_TTL,
                             (void *)&option,sizeof(option)) < 0)
    {
        UDP_LOG_SOCKET_TTL(OSAPI_LOGKIND_ERROR,
                           src_intf->factory->property->multicast_ttl,errno)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Join the specified multicast group
 *
 * \param[in] src_intf The UDP interface
 * \param[in] mc_address The multicast group to join
 * \param[in] sock The socket to join the multicast group address
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_join_multicast_group(const struct UDP_Interface *src_intf,
                                   const struct NETIO_Address *mc_address,
                                   RTI_SOCKET sock)
{
    RTI_INT32 index, length;
    struct NETIO_Address *intf_address;
    struct ip_mreq imr;
    int rc;

    imr.imr_multiaddr.s_addr = mc_address->value.ipv4.address;

    length = NETIO_AddressSeq_get_length(&src_intf->multicast_interfaces);

    if (length == 0)
    {
        /* join multicast on the default interface if none are specified */
        imr.imr_interface.s_addr = NETIO_htonl(INADDR_ANY);
        rc = netiosock_setsockopt(sock,IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                  (void *)&imr, sizeof(imr));
        if (rc < 0)
        {
            UDP_LOG_SOCKET_ADDGRP(OSAPI_LOGKIND_ERROR,
                                  INADDR_ANY,
                                  mc_address->value.ipv4.address,errno)
            return RTI_FALSE;
        }
    }
    else
    {
        /* join multicast on each multicast interface */
        for (index = 0; index < length; index++)
        {
            intf_address = NETIO_AddressSeq_get_reference(
                                        &src_intf->multicast_interfaces,index);
            if (intf_address == NULL)
            {
                continue;
            }

            imr.imr_interface.s_addr = intf_address->value.ipv4.address;

            OSAPI_TRACE_NET("join multi-cast group:",RTI_FALSE)
            OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(mc_address),RTI_FALSE)
            OSAPI_TRACE_INT32("port",mc_address->port,RTI_FALSE)
            OSAPI_TRACE_INT32("address",mc_address->value.ipv4.address,RTI_FALSE)
            OSAPI_TRACE_INT32("interface",intf_address->value.ipv4.address,RTI_TRUE)

            rc = netiosock_setsockopt(sock,IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                      (void *)&imr, sizeof(imr));
            if (rc < 0)
            {
                UDP_LOG_SOCKET_ADDGRP(OSAPI_LOGKIND_ERROR,
                                      intf_address->value.ipv4.address,
                                      mc_address->value.ipv4.address,errno)
                return RTI_FALSE;
            }
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Leave the specified multicast group
 *
 * \param[in] src_intf The UDP interface
 * \param[in] mc_address The multicast group to leave
 * \param[in] sock The socket to leave the multicast group address
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_leave_multicast_group(const struct UDP_Interface *src_intf,
                                    const struct NETIO_Address *mc_address,
                                    RTI_SOCKET sock)
{
    struct ip_mreq imr;
    int rc;
    RTI_INT32 index, length;
    struct NETIO_Address *intf_address;

    imr.imr_multiaddr.s_addr = mc_address->value.ipv4.address;

    length = NETIO_AddressSeq_get_length(&src_intf->multicast_interfaces);

    if (length == 0)
    {
        /* join multicast on the default interface if none are specified */
        imr.imr_interface.s_addr = NETIO_htonl(INADDR_ANY);
        rc = netiosock_setsockopt(sock,IPPROTO_IP,
                              IP_DROP_MEMBERSHIP,(void *)&imr, sizeof(imr));
        if (rc < 0)
        {
            UDP_LOG_DROPGRP(OSAPI_LOGKIND_ERROR,rc)
            return RTI_FALSE;
        }
    }
    else
    {
        for (index = 0; index < length; index++)
        {
            intf_address = NETIO_AddressSeq_get_reference(
                                &src_intf->multicast_interfaces,index);
            if (intf_address == NULL)
            {
                continue;
            }

            imr.imr_interface.s_addr = intf_address->value.ipv4.address;

            OSAPI_TRACE_NET("leave multi-cast group:",RTI_FALSE)
            OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(mc_address),RTI_FALSE)
            OSAPI_TRACE_INT32("port",mc_address->port,RTI_FALSE)
            OSAPI_TRACE_INT32("address",mc_address->value.ipv4.address,RTI_FALSE)
            OSAPI_TRACE_INT32("interface",intf_address->value.ipv4.address,RTI_TRUE)

            rc = netiosock_setsockopt(sock,IPPROTO_IP,
                                  IP_DROP_MEMBERSHIP,(void *)&imr, sizeof(imr));
            if (rc < 0)
            {
                UDP_LOG_DROPGRP(OSAPI_LOGKIND_ERROR,rc)
                return RTI_FALSE;
            }
        }
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize all the outgoing multicast interfaces
 *
 * \param[in] src_intf The UDP interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_multicast_interface_finalize(struct UDP_Interface *src_intf)
{
    RTI_INT32 index, length;
    struct NETIO_Address *an_address;

    length = NETIO_AddressSeq_get_length(&src_intf->multicast_interfaces);
    for (index = 0; index < length; index++)
    {
        an_address = NETIO_AddressSeq_get_reference(
                                        &src_intf->multicast_interfaces,index);
        if (an_address == NULL)
        {
            continue;
        }
        netiosock_close(src_intf->multicast_sockets[index].sock);
        src_intf->multicast_sockets[index].sock = UDP_INVALID_SOCKET;
    }
    OSAPI_Heap_free_array(src_intf->multicast_sockets);
    return NETIO_AddressSeq_finalize(&src_intf->multicast_interfaces);
}
#endif

#endif

#if !defined(RTI_AUTOSAR)
/*ci
 * \brief UDP receive thread implementation
 *
 * \details
 *
 * UDP_Interface_receive_thread is the entry point for UDP thread. Each receive
 * thread blocks on a single socket descriptor until data is available, then
 * reads the data and passes it on for processing upstream. Since the thread
 * is blocked on a socket the wake up routine sends data to itself to wake the
 * thread up.
 *
 * \param[in] thread_info Thread information structure passed from OSAPI Thread
 * \param[in] port_entry Port entry to use
 */
RTI_PRIVATE void
UDP_Interface_receive_thread_impl(struct OSAPI_ThreadInfo *thread_info,
                                  struct UDPPortEntry *port_entry)
{
    struct sockaddr_in ip_src;
    struct NETIO_Packet packet;
    RTI_BOOL bretval;

#if (defined(RTI_VXWORKS) && !defined(RTI_CERT))
    int ip_len = sizeof(struct sockaddr_in);
#else
    socklen_t ip_len = sizeof(struct sockaddr_in);
#endif
#if defined(RTI_WIN32) || defined(RTI_WINCE)
    int flags = 0;              /* don't care about flag, but can't provide NULL */
#endif
#if defined(RTI_WIN32)
    WSABUF mbuf;
#endif
    RTI_INT32 rx_len;

#ifdef RTI_USE_NONBLOCKING_SOCKET
    UNUSED_ARG(thread_info);
#endif

    OSAPI_Memory_zero(&ip_src,ip_len);

#ifndef RTI_USE_NONBLOCKING_SOCKET
    while (!thread_info->stop_thread)
#else
    do
#endif
    {
#if defined(RTI_WIN32)
        mbuf.len = port_entry->_rx_buffer.max_length;
        mbuf.buf = port_entry->_rx_buffer.buffer;
        if (WSARecvFrom((SOCKET)port_entry->_sock,
                (WSABUF*)&mbuf, 1, &rx_len, &flags,
                (struct sockaddr *)&ip_src, &ip_len, NULL, NULL) != 0)
        {
            port_entry->_rx_buffer.max_length = -1;
        }
#else
        OSAPI_TRACE_NET("wait for data:",RTI_FALSE)
        OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_TRUE)

        rx_len = (RTI_INT32)netiosock_recvfrom(port_entry->_sock,
                                    port_entry->_rx_buffer.buffer,
                                    port_entry->_rx_buffer.max_length,
                                    0,(struct sockaddr*)&ip_src,&ip_len);
#endif

#ifndef RTI_USE_NONBLOCKING_SOCKET
        if (thread_info->stop_thread)
        {
            break;
        }
#endif

        OSAPI_TRACE_NET("wait for data:",RTI_FALSE)
        OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_FALSE)
        OSAPI_TRACE_INT32("length",rx_len,RTI_TRUE)

        if (rx_len <= 0)
        {
            if (rx_len < 0)
            {
                UDP_LOG_RECV_ERROR(OSAPI_LOGKIND_ERROR,errno)
            }
            continue;
        }

        /* Ensure that properties pointer is still valid. If UDP factory is
         * unregistered before participant is deleted that pointer could be NULL.
         * To avoid a run time error we check for a NULL pointer, but there is
         * nothing we can do with the received message.
         */
        if (port_entry->_udp_intf->factory->property == NULL)
        {
            OSAPI_TRACE_NET("UDP factory property NULL, dropping msg",RTI_TRUE)
            continue;
        }

        if (rx_len > port_entry->_udp_intf->factory->property->max_message_size)
        {
            OSAPI_TRACE_NET("received message larger then max message:",RTI_FALSE)
            OSAPI_TRACE_INT32("size",rx_len,RTI_TRUE)
            continue;
        }

        if (!NETIO_Packet_initialize(&packet,port_entry->_rx_buffer.buffer,
                                     (RTI_SIZE_T)rx_len, 0, NULL))
        {
            UDP_LOG_PACKET_INIT(OSAPI_LOGKIND_ERROR,
                                &packet,port_entry->_rx_buffer.buffer,
                                rx_len)
            continue;
        }

        if (!NETIO_Packet_set_head(&packet, 0 - rx_len))
        {
            UDP_LOG_PACKET_HEAD(OSAPI_LOGKIND_ERROR,&packet, 0 - rx_len)
            continue;
        }

        if (NETIO_Packet_is_ndds_ping(&packet))
        {
            continue;
        }

        OSAPI_TRACE_NET("process message",RTI_TRUE)


        NETIO_Address_set_ipv4(&packet.source,
                               (RTI_UINT32)NETIO_ntohs(ip_src.sin_port),
                               NETIO_ntohl(ip_src.sin_addr.s_addr));

        bretval = UDP_Interface_receive(&port_entry->_udp_intf->_parent,
                                        &port_entry->source,&packet);

#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            UDP_LOG_PACKET_FWD(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(bretval);
#endif
        OSAPI_TRACE_NET("message processed:",RTI_TRUE)
#ifndef RTI_USE_NONBLOCKING_SOCKET
    }
#else
    } while(netiosock_packets_queued(port_entry->_sock) > 0);

#endif
}

#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)
/*ci
 * \brief UDP receive thread
 *
 * \details
 *
 * UDP_Interface_ is the entry point for UDP thread. When a socket notifies
 * that there is data to read on it, a message is added in \ref udp_queue
 * with a pointer to the socket with data.
 *
 * \param[in] thread_info Thread information structure passed from OSAPI Thread
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_nonblocking_udp_thread(struct OSAPI_ThreadInfo *thread_param)
{
    struct UDPPortEntry *port_entry;
    struct UDP_Interface *udp_intf;

    OSAPI_TRACE_NET("started udp thread:",RTI_TRUE)

    udp_intf = (struct UDP_Interface*)thread_param->user_data;

    while (!thread_param->stop_thread)
    {
        if (tx_queue_receive(udp_intf->udp_queue,
                             &port_entry,
                             TX_WAIT_FOREVER) == TX_SUCCESS)
        {
            /* Note that a NULL pointer might be added to the queue in case
             * this thread needs to be awaken
             */
            if ((!thread_param->stop_thread) && (port_entry != NULL))
            {
                UDP_Interface_receive_thread_impl(thread_param, port_entry);
            }
        }
    }

    OSAPI_TRACE_NET("stopped received thread:",RTI_FALSE)

    return RTI_TRUE;
}

#elif !defined(RTI_USE_NONBLOCKING_SOCKET)

/*ci
 * \brief UDP receive thread
 *
 * \details
 *
 * UDP_Interface_receive_thread is the entry point for UDP thread. Each receive
 * thread blocks on a single socket descriptor until data is available, then
 * reads the data and passes it on for processing upstream. Since the thread
 * is blocked on a socket the wake up routine sends data to itself to wake the
 * thread up.
 *
 * \param[in] thread_info Thread information structure passed from OSAPI Thread
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_receive_thread(struct OSAPI_ThreadInfo *thread_info)
{
    struct UDPPortEntry *port_entry =
            (struct UDPPortEntry *)thread_info->user_data;

    OSAPI_TRACE_NET("started thread:",RTI_FALSE)
    OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_TRUE)

    UDP_Interface_receive_thread_impl(thread_info, port_entry);

    OSAPI_TRACE_NET("stopped received thread:",RTI_FALSE)
    OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_TRUE)

    return RTI_TRUE;
}
#endif /* !RTI_AUTOSAR */
#endif /* RTI_USE_NONBLOCKING_SOCKET */

/*ci
 * \brief Check if a network interface should be considered part of the
 *        UDP interface
 *
 * \details
 *
 * When the UDP interface is registered the \ref UDP_InterfaceFactoryProperty
 * is passed in with the optional \ref UDP_InterfaceFactoryProperty_allow_interface
 * and UDP_InterfaceFactoryProperty_deny_interface sequence of allowed
 * and denied interface names. This function checks if the interface names
 * should be part of the UDP interface. The function first checks if it is
 * allowed and if so returns true, if the allowed_interface has a length of zero
 * the check is skipped. If the deny_interface list has at least one element
 * it is checked next. If the interface name is in the deny list it is not
 * considered part of the network interfaces for any UDP interface
 * created from the factory.
 *
 * \param[in] src_intf UDP interface instance to check
 * \param[in] lastname Network interface name to check
 *
 * \return RTI_TRUE if the interface is allowed, RTI_FALSE if the interface is
 *         not allowed
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_interface_allowed(struct UDP_Interface *src_intf,
                                char *lastname)
{
    RTI_INT32 j;
    char *a_string;

    /* Check if interface appears in the allowed  list */
    if (REDA_StringSeq_get_length(
                        &src_intf->factory->property->allow_interface) > 0)
    {
        for (j = 0; j < REDA_StringSeq_get_length(
                    &src_intf->factory->property->allow_interface); ++j)
        {
            a_string = *REDA_StringSeq_get_reference(
                    &src_intf->factory->property->allow_interface,j);
            if (!REDA_String_ncompare(a_string,lastname,IF_NAMESIZE-1))
            {
                return RTI_TRUE;
            }
        }

        if (j == REDA_StringSeq_get_length(
                            &src_intf->factory->property->allow_interface))
        {
            return RTI_FALSE;
        }
    }

    /* Allowed, check if it is the deny list */
    if (REDA_StringSeq_get_length(
                            &src_intf->factory->property->deny_interface) > 0)
    {
        for (j = 0; j < REDA_StringSeq_get_length(
                           &src_intf->factory->property->deny_interface); ++j)
        {
            a_string = *REDA_StringSeq_get_reference(
                        &src_intf->factory->property->deny_interface,j);
            if (!REDA_String_ncompare(a_string,lastname,IF_NAMESIZE-1))
            {
                return RTI_FALSE;
            }
        }
        if (j == REDA_StringSeq_get_length(
                                &src_intf->factory->property->deny_interface))
        {
            return RTI_TRUE;
        }
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT

#ifdef IP_TOS
/*ci
 * \brief Finalize the priority map
 *
 * \param[in] udp_intf The UDP interface to finalize the priority map for
 */
RTI_PRIVATE void
UDP_Interface_finalize_priority_map(struct UDP_Interface *udp_intf)
{
    if (udp_intf->priority_map != NULL)
    {
        OSAPI_Heap_free_struct(udp_intf->priority_map);
        udp_intf->priority_map = NULL;
    }
}
#endif /* IP_TOS */

/*ci
 * \brief Finalize a UDP interface instance
 *
 * \param[in] netio_intf Interface to finalize
 *
 * \sa \ref UDP_Interface_initialize
 */
RTI_PRIVATE void
UDP_Interface_finalize(struct UDP_Interface *netio_intf)
{
    struct UDP_NatEntry nat_key;
    struct UDP_NatEntry *nat_record;
    struct UDP_NatEntrySeq *nat_seq;
    DB_ReturnCode_T dbrc;
    RTI_INT32 i,len;
    RTI_BOOL bretval = RTI_TRUE;

#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)
    UDP_Interface_udp_thread_finalize(netio_intf);
#endif

    len = UDP_NatEntrySeq_get_length(&netio_intf->factory->property->nat);
    if (len > 0)
    {
        nat_seq = &netio_intf->factory->property->nat;
        for (i = 0; i < len; i++)
        {
            struct UDP_NatEntry *nat_key_ref = NULL;

            nat_key_ref = UDP_NatEntrySeq_get_reference(nat_seq,i);
            if (nat_key_ref == NULL)
            {
                UDP_LOG_FINALIZE(OSAPI_LOGKIND_ERROR)
                return;
            }

            nat_key = *nat_key_ref;
            nat_key.public_address.value.ipv4.address =
                        NETIO_htonl(nat_key.public_address.value.ipv4.address);
            nat_key.local_address.value.ipv4.address =
                        NETIO_htonl(nat_key.local_address.value.ipv4.address);

            nat_record = NULL;
            dbrc = DB_Table_remove_record(netio_intf->nat,
                                          (DB_Record_T*)&nat_record,
                                          (DB_Key_T)&nat_key);
            if (dbrc != DB_RETCODE_OK)
            {
                UDP_LOG_NAT_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
                continue;
            }

            dbrc = DB_Table_delete_record(netio_intf->nat,nat_record);
#if OSAPI_ENABLE_LOG
            if (dbrc != DB_RETCODE_OK)
            {
                UDP_LOG_NAT_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
            }
#else
            IGNORE_RETVAL(dbrc);
#endif
        }

        dbrc = DB_Table_delete_index(netio_intf->nat,
                                     netio_intf->nat_local_idx);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            UDP_LOG_NAT_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif

        dbrc = DB_Table_delete_index(netio_intf->nat,
                                     netio_intf->nat_public_idx);
#if OSAPI_ENABLE_LOG
        if ( dbrc != DB_RETCODE_OK)
        {
            UDP_LOG_NAT_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }
    if (netio_intf->unicast_send_sockets != NULL)
    {
        for (RTI_UINT32 k = 0; k < netio_intf->factory->property->max_unicast_send_sockets; k++)
        {
            if (netio_intf->unicast_send_sockets[k].sock != UDP_INVALID_SOCKET)
            {
                netiosock_close(netio_intf->unicast_send_sockets[k].sock);
                netio_intf->unicast_send_sockets[k].sock = UDP_INVALID_SOCKET;
            }
        }
    }
    OSAPI_Heap_free_array(netio_intf->unicast_send_sockets);

#if NETIO_CONFIG_ENABLE_MULTICAST
    bretval = bretval && UDP_Interface_multicast_interface_finalize(netio_intf);
#endif

#if UDP_TRANSFORMS_ENABLED
    /* UDP_TransformTable_finalize logs the error */
    bretval = bretval && UDP_TransformTable_finalize(&netio_intf->transform_rules);
#endif
#ifdef IP_TOS
    UDP_Interface_finalize_priority_map(netio_intf);
#endif
    /* Ensure that in case there is already an error bretval has the right value
     * even if function call return RTI_TRUE.
     */
    bretval = bretval && NETIO_Interface_finalize(&netio_intf->_parent);

#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        UDP_LOG_FINALIZE(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Delete a UDP interface instance
 *
 * \param[in] netio_intf UDP Interface to delete
 *
 * \sa \ref UDP_Interface_create
 */
RTI_PRIVATE void
UDP_Interface_delete(struct UDP_Interface *netio_intf)
{
    if (netio_intf)
    {
        UDP_Interface_finalize(netio_intf);
        OSAPI_Heap_free_struct(netio_intf);
    }
}
#endif /* !RTI_CERT */

RTI_PRIVATE RTI_BOOL
UDP_Interface_send_multicast(const struct UDP_Interface *self,
                             const struct NETIO_Address *dst_addr,
                             const NETIO_Packet_T *packet,
                             struct sockaddr_in *sock_addr,
                             IOVEC_MSG *message);

RTI_PRIVATE RTI_BOOL
UDP_Interface_send_unicast(const struct UDP_Interface *self,
                           const struct NETIO_Address *dst_addr,
                           const NETIO_Packet_T *packet,
                           struct sockaddr_in *sock_addr,
                           IOVEC_MSG *message);


/*ci
 * \brief Send a packet or message to the specified destination
 *
 * \param[in] self         UDP interface
 * \param[in] dst_addr     Destination address
 * \param[in] packet       The packet to send unless message is NULL
 * \param[in] sock_addr    Destination socket filled in
 * \param[in,out] message  If not NULL, the message to send, otherwise send
 *                         the packet payload directly. Updated with sock_addr.
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_send_to_destination(const struct UDP_Interface *self,
                                  const struct NETIO_Address *dst_addr,
                                  const NETIO_Packet_T *packet,
                                  struct sockaddr_in *sock_addr,
                                  IOVEC_MSG *message)
{
    RTI_BOOL retval;

    if (UDP_Interface_is_multicast(dst_addr))
    {
        retval = UDP_Interface_send_multicast(self,
                               dst_addr,packet,sock_addr,message);
    }
    else
    {
        retval =  UDP_Interface_send_unicast(self,
                               dst_addr,packet,sock_addr,message);
    }

    return retval;
}

#ifndef RTI_USE_NONBLOCKING_SOCKET

#if NETIO_CONFIG_ENABLE_MULTICAST

/*ci
 * \brief Enable/Disable loopback to wakeup multicast threads
 *
 * \param[in] self The UDP interface
 * \param[in] enable Set to 1 to enable multicast wakeup, 0 to disable
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_set_multicast_wakeup(struct UDP_Interface *self,RTI_UINT8 enable)
{
    RTI_INT32 index, length;
    struct NETIO_Address *an_address;

    if (self->factory->property->disable_multicast_interface_select)
    {
        for (RTI_UINT32 i = 0 ; i < self->factory->property->max_unicast_send_sockets ; i++)
        {
            if (!UDP_Interface_set_multicast_loopback_option(self->unicast_send_sockets[i],enable))
            {
                return RTI_FALSE;
            }
        }
    }

    length = NETIO_AddressSeq_get_length(&self->multicast_interfaces);
    for (index = 0; index < length; index++)
    {
        an_address = NETIO_AddressSeq_get_reference(
                                &self->multicast_interfaces,index);
        if (an_address == NULL)
        {
            continue;
        }

        if (!UDP_Interface_set_multicast_loopback_option(self->multicast_sockets[index],enable))
        {
            return RTI_FALSE;
        }

    }

    return RTI_TRUE;
}
#endif

/*ci
 * \brief Force loopback to wakeup threads
 *
 * \param[in] self The UDP interface
 * \param[in] port_entry The port entry to wakeup
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_force_loopback_wakeup(struct UDP_Interface *self,
                                    struct UDPPortEntry *port_entry)
{
#if NETIO_CONFIG_ENABLE_MULTICAST
    /* Only multicast requires forcing loopback */
    if (UDP_Interface_is_multicast(&port_entry->source) &&
        self->factory->property->multicast_loopback_disabled)
    {
        return UDP_Interface_set_multicast_wakeup(self,1);
    }
#else
    UNUSED_ARG(self);
    UNUSED_ARG(port_entry);
#endif
    return RTI_TRUE;
}

/*ci
 * \brief Restore loopback setting based on port entry
 *
 * \param[in] self The UDP interface
 * \param[in] port_entry The port entry to restore
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_restore_loopback_wakeup(struct UDP_Interface *self,
                                      struct UDPPortEntry *port_entry)
{
#if NETIO_CONFIG_ENABLE_MULTICAST
    /* Only multicast requires restoring loopback */
    if (UDP_Interface_is_multicast(&port_entry->source) &&
        self->factory->property->multicast_loopback_disabled)
    {
        return UDP_Interface_set_multicast_wakeup(self,0);
    }
#else
    UNUSED_ARG(self);
    UNUSED_ARG(port_entry);
#endif
    return RTI_TRUE;
}

/*ci
 * \brief UDP wakeup a receive thread
 *
 * \details
 *
 * UDP receive threads calls blocking read calls. To unblock a receive
 * thread when a UDP receive thread is deleted this wakeup function is called
 * which sends data to the socket to unblock the thread.
 *
 * \param[in] thread_info Thread specific data passed from OSAPI
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE  RTI_BOOL
UDP_Interface_wakeup_receive_thread(struct OSAPI_ThreadInfo *thread_info)
{
    struct UDPPortEntry *port_entry = (struct UDPPortEntry *)
                                                        thread_info->user_data;
    struct sockaddr_in sock_addr;
    RTI_INT32 i;
    NETIO_Packet_T wakeup_packet = NETIO_Packet_INITIALIZER;
    RTI_BOOL retval = RTI_TRUE;
    struct UDP_Interface *self = NULL;

#define ping_count 20

    OSAPI_TRACE_NET("wake up receive thread:",RTI_FALSE)
    OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_TRUE)

    if (!NETIO_Packet_set_payload(&wakeup_packet,
                                  NETIO_INFO_TS_PING_MSG,
                                  NETIO_INFO_TS_PING_MSG_SIZE))
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(&sock_addr, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = port_entry->source.value.ipv4.address;
    sock_addr.sin_port = NETIO_htons((RTI_UINT16)port_entry->source.port);

    self = port_entry->_udp_intf;

    /* Temporarily enable loopback to be able to wakeup the receive thread */
    if (!UDP_Interface_force_loopback_wakeup(self,port_entry))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < ping_count; ++i)
    {
        retval = retval &&  UDP_Interface_send_to_destination(port_entry->_udp_intf,
        /* coverity[cert_pos39_c_violation:FALSE] */
                       &port_entry->source,&wakeup_packet,&sock_addr,NULL);
    }

    /* Restore multicast loopback again */
    if (!UDP_Interface_restore_loopback_wakeup(self,port_entry))
    {
        return RTI_FALSE;
    }

    return retval;

#undef ping_count
}
#else

#ifndef RTI_AUTOSAR
/*ci
 * \brief UDP unique receive thread wakeup
 *
 * \details
 *
 * Unique UDP receive thread blocks in a queue. To unblock the UDP receive
 * thread when it is deleted the wakeup which sends a NULL pointer to the queue
 * to unblock the thread.
 *
 * \param[in] thread_info Thread specific data passed to OSAPI
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_wakeup_nonblocking_udp_thread(struct OSAPI_ThreadInfo *thread_info)
{
    UINT rc;
    struct OSAPI_ThreadXSocket_callback *callback_info = NULL;
    struct UDP_Interface *udp_intf;

    udp_intf = (struct UDP_Interface*)thread_info->user_data;

    /* sends a NULL pointer to indicate thread should be finished */
    rc = tx_queue_front_send(udp_intf->udp_queue,
                             &callback_info,
                             TX_NO_WAIT);
    if (rc != TX_SUCCESS)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_AUTOSAR */
#endif /* !RTI_USE_NONBLOCKING_SOCKET */

#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)

#ifndef RTI_CERT
/*ci
 * \brief Finalize the UDP thread
 *
 * \details
 * Finalize the UDP thread used for all receiving sockets
 * and all related resources.
 *
 * \param[in] udp_intf   Pointer to UDP receive interface
 *
 * \sa \ref UDP_Interface_udp_thread_initialize
 */
RTI_PRIVATE void
UDP_Interface_udp_thread_finalize(struct UDP_Interface *udp_intf)
{
    if (udp_intf->udp_thread != NULL)
    {
        RTI_BOOL rb;

        rb = OSAPI_Thread_destroy(udp_intf->udp_thread);
        IGNORE_RETVAL(rb);
        udp_intf->udp_thread = NULL;
    }

    if (udp_intf->udp_queue != NULL)
    {
        UINT rc;

        rc = tx_queue_delete(udp_intf->udp_queue);
        IGNORE_RETVAL(rc);

        OSAPI_Heap_free_struct(udp_intf->udp_queue);
        udp_intf->udp_queue = NULL;
    }

    if (udp_intf->receive_buffer != NULL)
    {
        OSAPI_Heap_free_buffer(udp_intf->receive_buffer);
        udp_intf->receive_buffer = NULL;
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the UDP thread
 *
 * \details
 * Initialize the UDP thread used for all receiving sockets and all
 * related resources.
 *
 * \param[in] udp_intf   Pointer to UDP receive interface
 * \param[in] udp_property The properties registered with the UDP interface
 *
 * \return RTI_TRUE if UDP thread can be created or otherwise RTI_FALSE
 *
 * \sa \ref UDP_Interface_udp_thread_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_udp_thread_initialize(
                               struct UDP_Interface *udp_intf,
                               struct UDP_InterfaceFactoryProperty *udp_property)
{
    OSAPI_Heap_allocate_struct(&udp_intf->udp_queue, TX_QUEUE);
    if (udp_intf->udp_queue == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    if (tx_queue_create(udp_intf->udp_queue,
                        "udp_rx_queue",
                        sizeof(NX_UDP_SOCKET*)/sizeof(RTI_INT32),
                        udp_intf->udp_queue_memory,
                        sizeof(udp_intf->udp_queue_memory)) != TX_SUCCESS)
    {
        /* important to free the pointer as the queue is either allocated and
		 * and created or not allocated at all */
        OSAPI_Heap_free_struct(udp_intf->udp_queue);
        udp_intf->udp_queue = NULL;
        goto fail;
    }

    udp_intf->udp_thread = OSAPI_Thread_create(
                           "udp_rx_thread_unique",
                           &udp_property->recv_thread,
                           UDP_Interface_nonblocking_udp_thread,
                           (void*)udp_intf,
                           UDP_Interface_wakeup_nonblocking_udp_thread);
    if (udp_intf->udp_thread == NULL)
    {
        goto fail;
    }
    if (!OSAPI_Thread_start(udp_intf->udp_thread))
    {
        goto fail;
    }

    return RTI_TRUE;

fail:

#ifndef RTI_CERT
    UDP_Interface_udp_thread_finalize(udp_intf);
#endif /* !RTI_CERT */

    return RTI_FALSE;
}
#endif /* RTI_USE_NONBLOCKING_SOCKET */

/*ci
 * \brief Create a listener for a UDP port
 *
 * \details
 *
 * Each UDP port being listened on has its own receive buffer, receive
 * thread and receive socket. A UDP port-entry can be shared between
 * multiple NETIO interfaces upstream, for example multiple RTPS interfaces
 * can listen to the same UDP port.
 *
 * \param[in]  src_intf UDP interface to create entry on
 * \param[in]  src_addr The address to listen on
 * \param[in]  property The properties of the address to listen to
 * \param[out] existed  Whether the port already existed or not
 *
 * \return Pointer to new port entry on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct UDPPortEntry*
UDP_Interface_create_bind_entry(struct UDP_Interface *src_intf,
                                struct NETIO_Address *src_addr,
                                struct NETIOBindProperty *property,
                                RTI_BOOL *existed)
{
    struct UDPPortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct sockaddr_in sock_addr;
    RTI_SOCKET sock = UDP_INVALID_SOCKET;
    int rc;
#ifndef RTI_USE_NONBLOCKING_SOCKET
    const char *tname = "udp_rx_uc";
#endif /* RTI_USE_NONBLOCKING_SOCKET */
    RTI_UINT32 local_addr;

    UNUSED_ARG(property);

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    OSAPI_TRACE_NET("bind port:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

    dbrc = DB_Table_select_match(src_intf->rx_thread_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,(DB_Key_T)src_addr);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        OSAPI_TRACE_NET("port exists:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)
        port_entry->_shared_ref_count++;
        return port_entry;
    }

    sock = netiosock_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        OSAPI_TRACE_NET("socket create error:",RTI_FALSE)
        OSAPI_TRACE_INT32("errno",errno,RTI_TRUE)
        UDP_LOG_SOCKET_CREATE(OSAPI_LOGKIND_ERROR,errno)
        return NULL;
    }

#if (defined(RTI_UNIX) || defined(RTI_LYNX)) && \
    ((ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
     (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE))
    if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1)
    {
        UDP_LOG_SOCKET_SETFD(OSAPI_LOGKIND_ERROR,errno)
        rc = netiosock_close(sock);
#if OSAPI_ENABLE_LOG || OSAPI_ENABLE_TRACE
        if (rc < 0)
        {
            OSAPI_TRACE_NET("socket fcntl error:",RTI_FALSE)
            OSAPI_TRACE_INT32("errno",errno,RTI_TRUE)
            UDP_LOG_SOCKET_CLOSE(OSAPI_LOGKIND_ERROR,errno)
        }
#else
        IGNORE_RETVAL(rc);
#endif
        return NULL;
    }
#endif

    /* In case there are more than one UDP transport registered it is not
     * possible to bind to INADDR_ANY. In that case each transport will
     * allow a non-intersecting group of local ip addresses.
     *
     * Note : it would be better to use src_intf->factory->instance_counter but
     * as UDPInterfaceFactory is not a singleton src_intf->factory->instance_counter
     * is always 1 (even in case several UDP transports are registered)
     */
    if (!src_intf->use_interface_bind)
    {
        local_addr = NETIO_htonl(INADDR_ANY);
    }
    else
    {
        local_addr = src_addr->value.ipv4.address;
    }

    /* Check that the specified socket can be reserved */
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = NETIO_htons((RTI_UINT16)src_addr->port);

#if NETIO_CONFIG_ENABLE_MULTICAST
    if (UDP_Interface_is_multicast(src_addr))
    {
        if (!UDP_Interface_bind_multicast_socket(src_intf,sock,src_addr,&sock_addr))
        {
            goto close_and_release;
        }
    }
    else
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */
    {
        sock_addr.sin_addr.s_addr = local_addr;
    }

    if (netiosock_bind(sock,(struct sockaddr *)&sock_addr,
                            sizeof(struct sockaddr_in)) < 0)
    {
#if OSAPI_ENABLE_LOG
#ifdef RTI_WIN32
        if (WSAGetLastError() != WSAEADDRINUSE)
        {
            UDP_LOG_SOCKET_BIND(OSAPI_LOGKIND_ERROR,
                    src_addr->value.ipv4.address,
                    src_addr->port, WSAGetLastError())
        }
        else
        {
            UDP_LOG_SOCKET_BIND(OSAPI_LOGKIND_WARNING,
                    src_addr->value.ipv4.address,
                    src_addr->port, WSAGetLastError())
        }
#else
        if (errno != EADDRINUSE)
        {
            UDP_LOG_SOCKET_BIND(OSAPI_LOGKIND_ERROR,
                    src_addr->value.ipv4.address,
                    src_addr->port, errno)
        }
        else
        {
            UDP_LOG_SOCKET_BIND(OSAPI_LOGKIND_WARNING,
                    src_addr->value.ipv4.address,
                    src_addr->port, errno)
        }
#endif
#endif
        goto close_and_release;
    }

#if NETIO_CONFIG_ENABLE_MULTICAST
    if (UDP_Interface_is_multicast(src_addr))
    {
        if (!UDP_Interface_join_multicast_group(src_intf,src_addr,sock))
        {
            goto close_and_release;
        }
#ifndef RTI_USE_NONBLOCKING_SOCKET
        tname = "udp_rx_mc";
#endif /* RTI_USE_NONBLOCKING_SOCKET */
    }
#endif

    /* Create bind entry */
    dbrc = DB_Table_create_record(src_intf->rx_thread_table,
                              (DB_Record_T *)&port_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_PORT_ENTRY(OSAPI_LOGKIND_ERROR,dbrc)
        goto close_and_release;
    }

    OSAPI_Memory_zero(port_entry,sizeof(struct UDPPortEntry));
    port_entry->source = *src_addr;
    port_entry->_udp_intf = src_intf;
    port_entry->_ref_count = 0;
    port_entry->_shared_ref_count = 1;
    port_entry->_sock = sock;
#ifndef RTI_USE_NONBLOCKING_SOCKET
    port_entry->_rx_thread = NULL;
#endif
    /* Cannot have negative message size */
    port_entry->_rx_buffer.max_length = (RTI_UINT32)
            port_entry->_udp_intf->factory->property->max_message_size;

#ifndef RTI_USE_NONBLOCKING_SOCKET
    OSAPI_Heap_allocate_buffer(&port_entry->_rx_buffer.buffer,
                               port_entry->_rx_buffer.max_length,
                               OSAPI_ALIGNMENT_DEFAULT);
#else
#if !defined(RTI_AUTOSAR)
    if (src_intf->receive_buffer == NULL)
    {
        OSAPI_Heap_allocate_buffer(&src_intf->receive_buffer,
                                   port_entry->_rx_buffer.max_length,
                                   OSAPI_ALIGNMENT_DEFAULT);
    }
    port_entry->_rx_buffer.buffer = src_intf->receive_buffer;
#else
    port_entry->_rx_buffer.buffer = NULL;
#endif /* !RTI_AUTOSAR */
#endif
#if !defined(RTI_AUTOSAR)
    if (port_entry->_rx_buffer.buffer == NULL)
    {
        UDP_LOG_PORT_ALLOC(OSAPI_LOGKIND_ERROR)
        goto close_and_release;
    }
#endif /* !RTI_AUTOSAR */
    dbrc = DB_Table_insert_record(src_intf->rx_thread_table,
                                 (DB_Record_T)port_entry);
    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_EXISTS))
    {
        UDP_LOG_PORT_ADD(OSAPI_LOGKIND_ERROR,dbrc)
        goto close_and_release;
    }

    OSAPI_TRACE_NET("set socket buffer size:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_FALSE)
    OSAPI_TRACE_INT32("size",port_entry->_udp_intf->factory->property->max_receive_buffer_size,RTI_TRUE)

    if (netiosock_setsockopt(port_entry->_sock,SOL_SOCKET, SO_RCVBUF,
        (char *)&port_entry->_udp_intf->factory->property->max_receive_buffer_size,
        sizeof(port_entry->_udp_intf->factory->property->max_receive_buffer_size)) < 0)
    {
        UDP_LOG_SOCKET_RXBUF(OSAPI_LOGKIND_ERROR,
              port_entry->_udp_intf->factory->property->max_receive_buffer_size,
              errno)
        goto close_and_release;
    }

    OSAPI_TRACE_NET("created port entry:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

#if defined(RTI_FREERTOS)
    /* Set receive thread with high priority  */
    port_entry->_udp_intf->factory->property->recv_thread.priority =
        OSAPI_THREAD_PRIORITY_HIGH;
#endif /* RTI_FREERTOS */

#ifndef RTI_USE_NONBLOCKING_SOCKET
    port_entry->_rx_thread = OSAPI_Thread_create(tname,
                        &port_entry->_udp_intf->factory->property->recv_thread,
                        UDP_Interface_receive_thread,
                        (void*)port_entry,
                        UDP_Interface_wakeup_receive_thread);

    if (port_entry->_rx_thread == NULL)
    {
        UDP_LOG_PORT_THREAD(OSAPI_LOGKIND_ERROR)
#ifndef RTI_CERT
#ifndef RTI_USE_NONBLOCKING_SOCKET
        OSAPI_Heap_free_buffer(port_entry->_rx_buffer.buffer);
#endif /* !RTI_USE_NONBLOCKING_SOCKET */
#endif /* !RTI_CERT */
        port_entry->_rx_buffer.buffer = NULL;
        port_entry = NULL;
        if (DB_Table_remove_record(src_intf->rx_thread_table,
                                     (DB_Record_T*)&port_entry,
                                     (DB_Key_T)src_addr) != DB_RETCODE_OK)
        {
            /* If we fail to remove the record there is nothing to do,
             * prevent it from being deleted later. Note that the receive thread
             * is not started, so it was safe to delete the receive_buffer
             * above.
             */
            port_entry = NULL;
        }
        /* If the remove call succeeded port_entry points to the record
         * and it is deleted in the failure path
         */
        goto close_and_release;
    }
#endif /* #ifdef RTI_USE_NONBLOCKING_SOCKET */

    OSAPI_TRACE_NET("created port thread:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

#if !defined(RTI_AUTOSAR)
    if (!OSAPI_Thread_start(port_entry->_rx_thread))
    {
        goto close_and_release;
    }
#endif

    return port_entry;

close_and_release:

    /* This exit path is reached in case of failure and is a common path to
     * release resource allocated at the point of failure.
     */
    rc = netiosock_close(sock);
#if OSAPI_ENABLE_LOG
    if (rc < 0)
    {
        UDP_LOG_SOCKET_CLOSE(OSAPI_LOGKIND_ERROR,errno)
    }
#else
    IGNORE_RETVAL(rc);
#endif

    if (port_entry != NULL)
    {
#ifndef RTI_CERT
#ifndef RTI_USE_NONBLOCKING_SOCKET
        if (port_entry->_rx_buffer.buffer != NULL)
        {
            OSAPI_Heap_free_buffer(port_entry->_rx_buffer.buffer);
        }
#endif /* !RTI_USE_NONBLOCKING_SOCKET */
#endif /* !RTI_CERT */
        (void)DB_Table_delete_record(src_intf->rx_thread_table,
                                     (DB_Record_T)port_entry);
    }

    return NULL;
}

/*ci
 * \brief Bind a sequence of NETIO addresses
 *
 * \details
 *
 * Before UDP ports can be listened on they must be bound to the port.
 * This function takes a sequence of addresses, and the index to  start at,
 * and creates a new port entry for each NETIO_Address to listen to.
 *
 * \param[in]  src_intf    UDP interface to bind entries on
 * \param[in]  start_index The first index in the sequence to listen to
 * \param[in]  addresses   Sequence of addresses to bind to
 * \param[out] property    The properties of the bind
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa ref \UDP_Interface_create_bind_entry
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_bind_addresses(struct UDP_Interface *src_intf,
                             RTI_INT32 start_index,
                             struct NETIO_AddressSeq *addresses,
                             struct NETIOBindProperty *property)
{
    RTI_BOOL retval = RTI_TRUE;
    RTI_INT32 i,length;
    struct NETIO_Address *an_address = NULL;
    struct UDPPortEntry *port_entry = NULL;
    RTI_BOOL port_existed;
    struct NETIO_Address src_address;
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry = NULL;
    DB_ReturnCode_T dbrc;
#endif /* !RTI_CERT */

    length = NETIO_AddressSeq_get_length(addresses);
    for (i = start_index; i < length; ++i)
    {
        an_address = NETIO_AddressSeq_get_reference(addresses,i);
        if (an_address == NULL)
        {
            retval = RTI_FALSE;
            break;
        }
        src_address = *an_address;

#ifndef RTI_CERT
        if (src_intf->nat)
        {
            dbrc = DB_Table_select_match(src_intf->nat,
                    src_intf->nat_public_idx,
                    (DB_Record_T*)&nat_entry,an_address);

            if (dbrc == DB_RETCODE_OK)
            {
                src_address = nat_entry->local_address;
            }
        }
#endif /* !RTI_CERT */

        port_existed = RTI_FALSE;
        port_entry = UDP_Interface_create_bind_entry(
                                 src_intf,&src_address,property,
                                 &port_existed);
        if (port_entry == NULL)
        {
            retval = RTI_FALSE;
            break;
        }
    }

    return retval;
}

/*ci
 * \brief Get the next interface
 *
 * \details
 *
 * The UDP interface uses OS specific APIs to read the list of available
 * network interfaces. The functionality is implemented in the UDP interface
 * as an iterator, \ref UDP_Interface_get_next_interface, which returns the
 * next available interface until no more interfaces are found.
 *
 * \param[in]  src_intf      UDP interface to get interface from
 * \param[in]  context       Iterator context which must be
 *                           initialized before the first call to
 *                           \ref UDP_Interface_get_next_interface()
 * \param[in]  ifdata        Information about an interface
 * \param[out] if_error      RTI_TRUE if the function returned RTI_FALSE due
 *                           to an error, RTI_FALSE otherwise
 *
 * \return RTI_TRUE if the next interface was found, RTI_FALSE otherwise. When
 *         RTI_FALSE is returned the if_error value should be checked to
 *         determine if the return was due to an error (*if_error = RTI_TRUE)
 *         or a normal exit due to no more interfaces found
 *         (*if_error = RTI_FALSE)
 *
 * \sa \ref UDP_Interface_get_next_interface_done
 */
#if defined(RTI_UNIX) || defined(RTI_VXWORKS) || defined(RTI_FREERTOS) || defined(RTI_THREADX) || defined(RTI_AUTOSAR)
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_get_next_interface(struct UDP_Interface *src_intf,
                                 struct UDP_IfContext *context,
                                 struct UDP_NetworkIfInfo *ifdata,
                                 RTI_BOOL *if_error)
{
#if NETIO_CONFIG_HAVE_IFCONF
    struct ifreq ifrcopy, *ifr;
    RTI_SIZE_T ifrSize = sizeof(struct ifreq);
#if !UDP_ENABLE_IPALIASES
    char *cptr = NULL;
#endif
#endif
    struct UDP_InterfaceTableEntry *if_entry;
    RTI_BOOL next_if_found;

    if (!context->initialized)
    {
#if NETIO_CONFIG_HAVE_IFCONF
        *if_error = RTI_TRUE;
        if (!src_intf->factory->property->disable_auto_interface_config)
        {
            OSAPI_Memory_zero(context->interfaceBuffer,
                              NETIO_HOST_INTERFACE_BUFFER_INIT);

            context->s = netiosock_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
            if (context->s == -1)
            {
                UDP_LOG_SOCKET_CREATE(OSAPI_LOGKIND_ERROR,errno)
                return RTI_FALSE;
            }

            context->ifc.ifc_buf = context->interfaceBuffer;
#ifndef __QNXNTO__
            context->ifc.ifc_len = context->interfaceBufferSize;
#else
            context->ifc.ifc_len = (short)context->interfaceBufferSize;
#endif
            if (NETIO_ioctl(context->s, SIOCGIFCONF, &context->ifc) == -1)
            {
                UDP_LOG_SOCKET_IFLIST(OSAPI_LOGKIND_ERROR,errno)
                return RTI_FALSE;
            }
            context->bufferPointer = context->interfaceBuffer;
            ifdata->lastname[0] = 0;
            ifdata->fname[0] = 0;
        }
#endif
        context->if_table_index = 0;
        context->if_table_length = UDP_InterfaceTableEntrySeq_get_length(
                                        &src_intf->factory->property->if_table);

        context->initialized = RTI_TRUE;
    }

    *if_error = RTI_FALSE;
    next_if_found = RTI_FALSE;

#if NETIO_CONFIG_HAVE_IFCONF
    if (!src_intf->factory->property->disable_auto_interface_config)
    {
        while (!next_if_found &&
                (context->bufferPointer <
                (context->interfaceBuffer + context->ifc.ifc_len)))
        {
            OSAPI_Memory_zero(&ifrcopy, sizeof(struct ifreq));
            OSAPI_Memory_copy(&ifr,&context->bufferPointer,sizeof(struct ifreq *));

#if defined(RTI_VXWORKS)
            ifrSize = (RTI_SIZE_T)(ifr->ifr_addr.sa_len + sizeof(ifr->ifr_name));
            if (ifrSize < (RTI_SIZE_T)sizeof(struct ifreq))
            {
                ifrSize = (RTI_SIZE_T)sizeof(struct ifreq);
            }
#elif defined(RTI_DARWIN) && defined(_SIZEOF_ADDR_IFREQ)
            ifrSize = (RTI_SIZE_T)_SIZEOF_ADDR_IFREQ(*ifr);
#else
            ifrSize = (RTI_SIZE_T)sizeof(struct ifreq);
#endif /* RTI_VXWORKS */

            context->bufferPointer += ifrSize;       /* for the next interface */
            OSAPI_Memory_copy(&ifrcopy, ifr, sizeof(struct ifreq));

            if (ifrcopy.ifr_addr.sa_family != AF_INET)
            {
                /* only want IPv4 */
                continue;
            }

#if !UDP_ENABLE_IPALIASES
            /* filter out alias */
            cptr = UDP_Interface_strrchr(ifrcopy.ifr_name, ':');
            if (cptr != NULL)
            {
                /* replace possible colon alias designator with terminating NULL */
                *cptr = 0;
            }

            if (REDA_String_ncompare(ifdata->lastname,ifrcopy.ifr_name,
                    IF_NAMESIZE) == 0)
            {
                /* name already exists, alias! */
                continue;
            }
#endif

            OSAPI_Memory_copy(ifdata->lastname, ifrcopy.ifr_name, IF_NAMESIZE);

            /* now get flags */
            if (NETIO_ioctl(context->s, SIOCGIFFLAGS, &ifrcopy) == -1)
            {
                UDP_LOG_SOCKET_IFFLAGS(OSAPI_LOGKIND_ERROR,errno)
                netiosock_close(context->s);
                *if_error = RTI_TRUE;
                return RTI_FALSE;
            }

            if (!(ifrcopy.ifr_flags & IFF_UP))
            {
                continue;
            }


            ifdata->mc_enabled = (ifrcopy.ifr_flags & IFF_MULTICAST ? RTI_TRUE : RTI_FALSE);

            {
                struct sockaddr_in sinaddr;
                OSAPI_Memory_copy(&sinaddr,&ifrcopy.ifr_addr,sizeof(struct sockaddr_in));
                ifdata->intf_address = sinaddr.sin_addr.s_addr;
            }

            if (NETIO_ioctl(context->s, SIOCGIFNETMASK, &ifrcopy) == -1)
            {
                UDP_LOG_SOCKET_IFFLAGS(OSAPI_LOGKIND_ERROR,errno)
                *if_error = RTI_TRUE;
                return RTI_FALSE;
            }

            {
                struct sockaddr_in sinaddr;
                OSAPI_Memory_copy(&sinaddr,&ifrcopy.ifr_addr,sizeof(struct sockaddr_in));
                /* not that netmasks are stored in host order */
                ifdata->intf_netmask = NETIO_ntohl(sinaddr.sin_addr.s_addr);
            }

            next_if_found = RTI_TRUE;
        }
    }
#endif

    while (!next_if_found &&
           (context->if_table_index < context->if_table_length))
    {
        if_entry = UDP_InterfaceTableEntrySeq_get_reference(
                &src_intf->factory->property->if_table,context->if_table_index);

        ++context->if_table_index;

        if (if_entry == NULL)
        {
            continue;
        }

        if (!(if_entry->flags & UDP_INTERFACE_INTERFACE_UP_FLAG))
        {
            continue;
        }
        /* This interface is in host order, convert it */
        ifdata->intf_address = NETIO_htonl(if_entry->address);
        ifdata->intf_netmask = if_entry->netmask;
        ifdata->mc_enabled =
                (if_entry->flags & UDP_INTERFACE_INTERFACE_MULTICAST_FLAG ?
                                                    RTI_TRUE : RTI_FALSE);
        OSAPI_Memory_copy(ifdata->lastname, if_entry->ifname,
                          REDA_String_length(if_entry->ifname)+1);
        next_if_found = RTI_TRUE;
    }

    return next_if_found;
}

/*ci
 * \brief Reset an interface iterator
 *
 * \details
 * This function should be called when all the interfaces returned by
 * \ref UDP_Interface_get_next_interface is processed. If this function
 * is not called the interface iterator may leave resources open and may
 * result in undefined behavior if \ref UDP_Interface_get_next_interface
 * is called again.
 *
 * \param[in]  src_intf      UDP interface to reset iterator on
 * \param[in]  context       Iterator context to reset
 *
 * \sa \ref UDP_Interface_get_next_interface
 */
RTI_PRIVATE void
UDP_Interface_get_next_interface_done(struct UDP_Interface *src_intf,
                                      struct UDP_IfContext *context)
{
#if NETIO_CONFIG_HAVE_IFCONF
    int rc;

    if (!src_intf->factory->property->disable_auto_interface_config)
    {
        if (context->s != -1)
        {
            rc = netiosock_close(context->s);
#if OSAPI_ENABLE_LOG
            if (rc == -1)
            {
                UDP_LOG_SOCKET_CLOSE(OSAPI_LOGKIND_ERROR,errno)
            }
#else
            IGNORE_RETVAL(rc);
#endif
        }
    }
#endif
    UNUSED_ARG(src_intf);
    context->initialized = RTI_FALSE;
}

#elif defined (RTI_WIN32)
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_get_next_interface(struct UDP_Interface *src_intf,
                                 struct UDP_IfContext *context,
                                 struct UDP_NetworkIfInfo *ifdata,
                                 RTI_BOOL *if_error)
{
    const int interfaceInfoSize = sizeof(IP_ADAPTER_ADDRESSES);
    const int NUM_RETRIES = 5;
    ULONG retVal;
    RTI_INT32 i=0;
    char addressString[INET6_ADDRSTRLEN];
    struct UDP_InterfaceTableEntry *if_entry;
    RTI_BOOL next_if_found;
    IN_ADDR in_addr;

    if (!context->initialized)
    {
        *if_error = RTI_TRUE;
        if (!src_intf->factory->property->disable_auto_interface_config)
        {
            context->myGetAdaptersAddresses = NULL;
            context->interfaceBuffer = NULL;
            context->ifbufIter = NULL;
            context->hinstIphlpapiLib = NULL;
            ifdata->lastname[0] = 0;
            ifdata->fname[0] = 0;

            /* run-time dynamic linking to avoid failure on some Windows OS versions */
            context->hinstIphlpapiLib = LoadLibrary("iphlpapi");
            if (context->hinstIphlpapiLib != NULL)
            {
                context->myGetAdaptersAddresses = (GETADAPTERSADDRESSES_FUNC)
                     GetProcAddress(context->hinstIphlpapiLib,
                             "GetAdaptersAddresses");
            }
            else
            {
                UDP_LOG_LOADLIB(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }

            /* if we can't get the function, return an empty list */
            if (context->myGetAdaptersAddresses == NULL)
            {
                UDP_LOG_LOADLIB(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }

            /* Get needed size of buffer */
            context->interfaceBufferSize = 0;
            if (context->myGetAdaptersAddresses(AF_INET, 0, NULL,
                    context->interfaceBuffer,
                    &context->interfaceBufferSize) != ERROR_BUFFER_OVERFLOW)
            {
                UDP_LOG_GET_BUF_SIZE(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
            }

            retry_alloc:
            ++i;
#ifndef RTI_CERT
            if (context->interfaceBuffer)
            {
                OSAPI_Heap_free_buffer((char *)context->interfaceBuffer);
            }
#endif /* !RTI_CERT */
            OSAPI_Heap_allocate_buffer((char **)&context->interfaceBuffer,
                                       context->interfaceBufferSize, 1);

            if (context->interfaceBuffer == NULL)
            {
                UDP_LOG_GET_BUF_ALLOC(OSAPI_LOGKIND_ERROR,
                                      context->interfaceBufferSize)
                return RTI_FALSE;
            }

            retVal = context->myGetAdaptersAddresses(AF_INET, 0, NULL,
                    context->interfaceBuffer,&context->interfaceBufferSize);
            if (retVal == ERROR_BUFFER_OVERFLOW && i < NUM_RETRIES)
            {
                goto retry_alloc;
            }
            else if (retVal != 0)
            {
                UDP_LOG_GET_IFLIST(OSAPI_LOGKIND_ERROR,retVal)
                return RTI_FALSE;
            }
            context->ifbufIter = context->interfaceBuffer;
        }
        context->if_table_index = 0;
        context->if_table_length = UDP_InterfaceTableEntrySeq_get_length(
                                        &src_intf->factory->property->if_table);
        context->initialized = RTI_TRUE;
    }

    *if_error = RTI_FALSE;
    next_if_found = RTI_FALSE;

    while (!next_if_found && (context->ifbufIter != NULL))
    {
        if (context->ifbufIter->IfType == IF_TYPE_TUNNEL)
        {
            /* ignore tunnels (there are many when using IPv6) */
            context->ifbufIter = context->ifbufIter->Next;
            continue;
        }

        if ((context->ifbufIter->OperStatus != IfOperStatusUp) &&
            (context->ifbufIter->OperStatus != IfOperStatusDormant))
        {
            context->ifbufIter = context->ifbufIter->Next;
            continue;
        }

        if (0 == WideCharToMultiByte(CP_UTF8,0,context->ifbufIter->FriendlyName,
                -1,ifdata->fname,IF_NAMESIZE,NULL,NULL))
        {
            *if_error = RTI_TRUE;
            return RTI_FALSE;
        }

        OSAPI_Memory_copy(ifdata->lastname, context->ifbufIter->AdapterName,
                (1 + OSAPI_String_length(context->ifbufIter->AdapterName)));

        if (context->ifbufIter->FirstUnicastAddress == NULL)
        {
            /* ignore interfaces with no address */
            context->ifbufIter = context->ifbufIter->Next;
            continue;
        }

        if (getnameinfo(context->ifbufIter->FirstUnicastAddress->Address.lpSockaddr,
                        context->ifbufIter->FirstUnicastAddress->Address.iSockaddrLength,
                        addressString, INET6_ADDRSTRLEN, NULL, 0,
                        NI_NUMERICHOST) != 0)
        {
            UDP_LOG_GET_NAMEINFO(OSAPI_LOGKIND_ERROR,WSAGetLastError())
            *if_error = RTI_TRUE;
            return RTI_FALSE;
        }

        ifdata->mc_enabled = (context->ifbufIter->Flags & IP_ADAPTER_NO_MULTICAST ? RTI_FALSE : RTI_TRUE);

        /* Only one multicast interface is supported. It should be the same
         * as the interface configured by the OS to use for multicast
         * if none is specified the OS default is used.
         */
        if (InetPton(AF_INET, addressString, &in_addr) != 1)
        {
            in_addr.s_addr = INADDR_NONE;
        }
        ifdata->intf_address = in_addr.s_addr;
        ifdata->intf_netmask = 0xff000000;

        next_if_found = RTI_TRUE;
        context->ifbufIter = context->ifbufIter->Next;
    }

    while (!next_if_found &&
           (context->if_table_index < context->if_table_length))
    {
        if_entry = UDP_InterfaceTableEntrySeq_get_reference(
                &src_intf->factory->property->if_table,context->if_table_index);

        ++context->if_table_index;

        if (if_entry == NULL)
        {
            continue;
        }

        if (!(if_entry->flags & UDP_INTERFACE_INTERFACE_UP_FLAG))
        {
            continue;
        }
        ifdata->intf_address = NETIO_htonl(if_entry->address);
        ifdata->intf_netmask = if_entry->netmask;
        ifdata->mc_enabled =
                (if_entry->flags & UDP_INTERFACE_INTERFACE_MULTICAST_FLAG ?
                                                    RTI_TRUE : RTI_FALSE);
        OSAPI_Memory_copy(ifdata->lastname, if_entry->ifname,
                               REDA_String_length(if_entry->ifname)+1);

        next_if_found = RTI_TRUE;
    }

    return next_if_found;
}

RTI_PRIVATE void
UDP_Interface_get_next_interface_done(struct UDP_Interface *src_intf,
                                      struct UDP_IfContext *context)
{
    if (!src_intf->factory->property->disable_auto_interface_config)
    {
        if (context->hinstIphlpapiLib != NULL)
        {
            FreeLibrary(context->hinstIphlpapiLib);
        }
#ifndef RTI_CERT
        if (context->interfaceBuffer != NULL)
        {
            OSAPI_Heap_free_buffer((char *)context->interfaceBuffer);
        }
#endif /* !RTI_CERT */
    }
    context->initialized = RTI_FALSE;
}
#else
#error "UDP_Interface_get_next_interface() not ported"
#endif

/*ci
 * \brief Create a list of available addresses to listen to based on a list of
 *        candidates
 *
 * \details
 *
 * The UDP interface can reserve UDP port to receive on. This function takes
 * a list of candidate/requested address to be reserved and returns a list
 * of reserved addresses. If at least one of the requested addresses cannot
 * be reserved because it is already in use the call returns with failure.
 * It is the callers responsibility to release the already reserved addresses
 * (if any).
 *
 * \param[in]    src_intf    UDP interface to create a list of addresses for
 * \param[in]    req_addr    A sequence of candidate/requested addresses
 * \param[inout] rcvd_addr   A sequence of reserved addresses
 * \param[in]    property    Properties to bind entries with
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref UDP_Interface_get_next_interface
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_read_interface_list(struct UDP_Interface *src_intf,
                                  struct NETIO_AddressSeq *req_addr,
                                  struct NETIO_AddressSeq *rcvd_addr,
                                  struct NETIOBindProperty *property)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_INT32 max_size;
    RTI_INT32 i,k,j;
    RTI_INT32 cur_addr_len;
    RTI_INT32 start_index;
    struct NETIO_Address src_address;
    struct NETIO_Address *an_address;
    struct UDP_IfContext if_context = UDP_IFCONTEXT_INITIALIZER;
    struct UDP_NetworkIfInfo if_data = UDP_NetworkIfInfo_INITIALIZER;
    RTI_BOOL if_error;
#if OSAPI_ENABLE_LOG
    RTI_BOOL duplicated = RTI_FALSE;
#endif
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry;
    DB_ReturnCode_T dbrc;
#endif

    cur_addr_len = NETIO_AddressSeq_get_length(rcvd_addr);
    start_index = cur_addr_len;
    max_size = NETIO_AddressSeq_get_maximum(rcvd_addr) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }

    NETIO_Address_init(&src_address,0);

    i = 0;

    if_error = RTI_FALSE;

    while (UDP_Interface_get_next_interface(src_intf,&if_context,&if_data,&if_error) && (i < max_size))
    {
        if (!UDP_Interface_is_valid_address(&if_data))
        {
            continue;
        }

        if (!UDP_Interface_interface_allowed(src_intf,if_data.lastname))
        {
#ifdef RTI_WIN32
            if (!UDP_Interface_interface_allowed(src_intf,if_data.fname))
            {
                OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
                OSAPI_TRACE_STRING("name",if_data.fname,RTI_TRUE)
                continue;
            }
#else
            OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
            OSAPI_TRACE_STRING("name",if_data.lastname,RTI_TRUE)
            continue;
#endif
        }

#if NETIO_CONFIG_ENABLE_MULTICAST
        if (if_data.mc_enabled &&
            !UDP_Interface_add_multicast_interface(src_intf,&if_data))
        {
            UDP_Interface_get_next_interface_done(src_intf,&if_context);
            goto finally;
        }
#endif

        /* NOTE:
         * At this point we have an interface which are allowed per the
         * allow/deny lists. The next step is to check the requested application
         * list.
         *
         * - If the request list is empty, there is nothing to be done
         * - For each entry in the request list, check:
         *   - The entry _must_ have a port assigned
         *   - If the address is unassigned, add the interface's unicast
         *     address
         *   - If the address is assigned and is unicast, only add it if
         *     it matches the interface address
         *  -  If the address is assigned and is multicast, only add it if
         *     the interface supports multicast, multicast is enabled
         */
        for (k = 0; (k < NETIO_AddressSeq_get_length(req_addr)) && (i < max_size); ++k)
        {
            an_address = NETIO_AddressSeq_get_reference(req_addr,k);
            if (an_address == NULL)
            {
                OSAPI_TRACE_NET("address is unexpectedly NULL.",RTI_TRUE)
                continue;
            }
            if ((NETIO_Address_get_kind(an_address) != NETIO_ADDRESS_KIND_UDPv4)
#if UDP_TRANSFORMS_ENABLED
                && (NETIO_Address_get_kind(an_address) != src_intf->transform_locator_kind)
#endif
                )
            {
                OSAPI_TRACE_NET("ignoring address, not UDPv4 or transformed UDP:",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_TRUE)
                continue;
            }
            if ((an_address->port < 1024) || (an_address->port > 65535))
            {
                OSAPI_TRACE_NET("ignoring address, invalid port:",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_TRUE)
                continue;
            }

#if UDP_TRANSFORMS_ENABLED
            /* If this transport is configured with at least one source rule
             * all incoming packets are transformed and assumed to be of
             * type src_intf->transform_locator_kind. It is not possible to determine
             * if the incoming payload has to be transformed or not since regular
             * UDP sockets are used.
             */
            if (src_intf->src_transforms_enabled)
            {
                src_address.kind = src_intf->transform_locator_kind;
            }
            else if (src_intf->transform_udp_mode == UDP_TRANSFORM_UDP_MODE_ENABLED)
            {
                /* Fallback to regular UDP if no transformations are specified */
                src_address.kind = NETIO_ADDRESS_KIND_UDPv4;
            }
            else
            {
                /* mode is DISABLED, do not allow plain UDP to be announced */
                continue;
            }
#else
            src_address.kind = NETIO_ADDRESS_KIND_UDPv4;
#endif
            src_address.port = an_address->port;
            if (UDP_Interface_is_multicast(an_address))
            {
#if NETIO_CONFIG_ENABLE_MULTICAST
                /* multicast */
                if (!if_data.mc_enabled)
                {
                    OSAPI_TRACE_NET("ignoring address, multicast not enabled on interface:",RTI_FALSE)
                    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                    OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                    OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_FALSE)
                    OSAPI_TRACE_INT32("interface",if_data.intf_address,RTI_TRUE)
                    continue;
                }
                OSAPI_TRACE_NET("multicast is supported on interface:",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_FALSE)
                OSAPI_TRACE_INT32("interface",if_data.intf_address,RTI_TRUE)

                src_address.value.ipv4.address = an_address->value.ipv4.address;
                src_intf->multicast_enabled = RTI_TRUE;
                src_address.kind |= (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST;
#else
                UDP_LOG_MULTICAST_NOT_ENABLED(OSAPI_LOGKIND_WARNING)
                OSAPI_TRACE_NET("ignoring address, multicast not enabled",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_FALSE)
                OSAPI_TRACE_INT32("interface",if_data.intf_address,RTI_TRUE)
#ifdef RTI_CERT
                goto finally;
#else
                continue;
#endif
#endif
            }
            else
            {
                /* unicast */
                if (an_address->value.ipv4.address == 0)
                {
                    /* Use the interface address */
                    src_address.value.ipv4.address = if_data.intf_address;
                }
                else if (an_address->value.ipv4.address == if_data.intf_address)
                {
                    /* If the interface address is specified, it must be the
                     * same as the interface address. This allows us to filter
                     * out addresses that are listed but does not exist
                     */
                    src_address.value.ipv4.address = an_address->value.ipv4.address;
                }
                else
                {
                    OSAPI_TRACE_NET("ignoring address, not interface address:",RTI_FALSE)
                    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
                    OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
                    OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
                    continue;
                }
            }

#ifndef RTI_CERT
            /* Check if there is a NAT entry for this local address. If that
             * is case the case, return the public address
             */
            if (src_intf->nat)
            {
                dbrc = DB_Table_select_match(src_intf->nat,
                                             src_intf->nat_local_idx,
                                             (DB_Record_T*)&nat_entry,
                                             &src_address);
                if (dbrc == DB_RETCODE_OK)
                {
                    src_address = nat_entry->public_address;
                }
            }
#endif /* !RTI_CERT */

            /* Don't duplicate addresses */
            for (j = 0; j < NETIO_AddressSeq_get_length(rcvd_addr); ++j)
            {
                /* The get_reference return is not NULL because get_length
                 * succeeded.
                 */
                /* coverity[dereference] */
                /* coverity[cert_exp34_c_violation] */
                if ((NETIO_AddressSeq_get_reference(rcvd_addr,j)->value.ipv4.address == src_address.value.ipv4.address) &&
                        (NETIO_AddressSeq_get_reference(rcvd_addr,j)->port == src_address.port))
                {
#if OSAPI_ENABLE_LOG
                    duplicated = RTI_TRUE;
#endif
                    break;
                }
            }

            if (j < NETIO_AddressSeq_get_length(rcvd_addr))
            {
                OSAPI_TRACE_NET("ignoring duplicate address:",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(an_address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",an_address->port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",an_address->value.ipv4.address,RTI_TRUE)
                continue;
            }

            if (!NETIO_AddressSeq_set_length(rcvd_addr,cur_addr_len+i+1))
            {
                UDP_Interface_get_next_interface_done(src_intf,&if_context);
                goto finally;
            }

            OSAPI_TRACE_NET("reserve address:",RTI_FALSE)
            OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
            OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
            OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)

            /* The get_reference return is not NULL because set_length succeeded
             */
            /* coverity[dereference] */
            /* coverity[cert_exp34_c_violation] */
            *NETIO_AddressSeq_get_reference(rcvd_addr,cur_addr_len+i) =
                                            src_address;
            ++i;
        }
    }

#if OSAPI_ENABLE_LOG
    /* If there was no error, no new reserved address is added and duplicated
     * was not detected the transport doesn't have any interface or any of its
     * interfaces is allowed. A wrong UDP configuration might be used.
     */
    if ((NETIO_AddressSeq_get_length(rcvd_addr) == cur_addr_len) &&
        (!duplicated) && (!if_error))
    {
        UDP_LOG_TRANSPORT_NO_INTERFACES(OSAPI_LOGKIND_WARNING)
    }
#endif

    UDP_Interface_get_next_interface_done(src_intf,&if_context);

#ifndef RTI_CERT
    if (if_error)
    {
        goto finally;
    }
#else
    /* Ignore the return value in Cert because the function cannot fail */
    IGNORE_RETVAL(if_error);
#endif

    /* Bind will do the final filtering of reserved addresses */
    if (!UDP_Interface_bind_addresses(src_intf,start_index,rcvd_addr,property))
    {
        OSAPI_TRACE_NET("failed to bind one or more address:",RTI_TRUE)
        goto finally;
    }

#if NETIO_CONFIG_ENABLE_MULTICAST
    if ((NETIO_AddressSeq_get_length(&src_intf->multicast_interfaces) == 0)
        && (src_intf->mc_bind_address.value.ipv4.address != INADDR_ANY)
        && !src_intf->factory->property->disable_multicast_interface_select)
    {
        /* If no multicast addresses are added but
         * mc_bind_address.value.ipv4.address is set,
         * then set mc_bind_address.value.ipv4.address
         * as the outgoing interface unless multicast interface selection has
         * been completely disabled.
         */
        for (RTI_UINT32 m = 0 ; m < src_intf->factory->property->max_unicast_send_sockets ; m++)
        {
            if (netiosock_setsockopt(src_intf->unicast_send_sockets[m].sock,
                                    IPPROTO_IP,
                                    IP_MULTICAST_IF,
                                    (char *)&src_intf->mc_bind_address.value.ipv4.address,
                                    sizeof(src_intf->mc_bind_address.value.ipv4.address)) < 0)
            {
                UDP_LOG_SOCKET_SET_MCASTIF(OSAPI_LOGKIND_ERROR,errno)
                goto finally;
            }
        }
    }
#endif

    ok = RTI_TRUE;

finally:

    return ok;
}

/*ci
 * \brief Compare entries in the table of bind entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIOBindEntry already in the database
 * \param[in] op2   Either a NETIOBindEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_compare_bind(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct NETIOBindEntry *left_record = (struct NETIOBindEntry*)op1;
    struct NETIO_Address src;
    UNUSED_ARG(flags);

    src = ((struct NETIOBindEntry *)op2)->source;

    /* OSAPI_Memory_compare is used because struct NETIO_Address
     * is 24 bytes
     */
    return OSAPI_Memory_compare(&left_record->source, &src,
                                sizeof(struct NETIO_Address));
}

#ifdef IP_TOS


/*ci
 * \brief Build a priority-to-socket mapping table for QoS-based socket selection
 *
 * \details
 * This function constructs a lookup table that maps transport priority values
 * to socket indices, enabling the UDP interface to distribute traffic across
 * multiple unicast send sockets based on priority levels.
 *
 * The mapping algorithm performs the following steps:
 * 1. Determines the effective priority range by taking the:
 *    - The shifted mask range (mask_shifted + 1)
 * 2. For each distinct priority level in the effective range:
 *    - Scales the priority linearly from 0..mask_shifted to mapping_low..mapping_high
 *    - Computes the output priority byte (P = priority_out & 0xFF) to fit within the 8-bit TOS field
 *    - Maps the priority byte to a socket index using round-robin distribution
 *      (socket_index = cur_idx % max_socket)
 *
 *
 * \param[in] udp_intf Pointer to the UDP interface
 */

RTI_PRIVATE void
UDP_Interface_build_socket_map(struct UDP_Interface *udp_intf)
{
    RTI_UINT32 cur_idx = 0;
    RTI_UINT32 max_socket =
        udp_intf->factory->property->max_unicast_send_sockets;
    RTI_UINT32 mask_shifted =
        udp_intf->priority_map->mask_shifted;
    RTI_INT32 mapping_low =
        udp_intf->factory->property->transport_priority_mapping_low;
    RTI_UINT32 range = (RTI_UINT32)udp_intf->priority_map->range;
    RTI_UINT8 visited[UDP_PRIORITY_MAP_MAX] = {0};
    RTI_UINT32 effective_count = mask_shifted + 1;

    /* Iterate only distinct effective priorities */
    for (RTI_UINT32 m = 0; m < effective_count; ++m)
    {
        RTI_INT32 scaled_priority, priority_out;
        RTI_UINT8 priority_byte;

        /* note that this scaling is identical to the one in  UDP_Interface_compute_priority */
        scaled_priority = (RTI_INT32)((m * range) / mask_shifted);
        priority_out = scaled_priority + mapping_low;
        priority_byte = (RTI_UINT8)(priority_out & 0xFF);
        if (visited[priority_byte])
        {
            continue;
        }
        visited[priority_byte] = RTI_TRUE;

        udp_intf->priority_map->map[priority_byte] =
            (RTI_UINT8)(cur_idx % max_socket);
        cur_idx++;
    }

}
/*ci
 * \brief Compute the transport priority for a given UDP interface
 *
 * \details This function computes the transport priority based on the user
 *          defined transport priority and the UDP interface configuration.
 * \param[in]  udp_intf          UDP interface
 * \param[in]  priority_map      Precomputed priority map
 * \param[in]  transport_priority User defined transport priority
 * \param[out] priority_out      Computed priority
 */
RTI_PRIVATE void
UDP_Interface_compute_priority(const struct UDP_Interface *udp_intf,
                            struct UDP_TransportPriorityMap *priority_map,
                        RTI_INT32 transport_priority,
                        RTI_INT32 *priority_out)
{
    RTI_INT32 scaled_priority = 0;
    RTI_UINT32 range = udp_intf->priority_map->range;
    RTI_UINT32 mask = udp_intf->factory->property->transport_priority_mask;
    RTI_UINT32 masked_prio = 0;

    masked_prio = ((RTI_UINT32)((RTI_UINT32)transport_priority & mask)) >> priority_map->shift_nibbles;

    /* Scale:  (masked * range_num) / range_den + mapping_low */
    scaled_priority = (RTI_INT32)(masked_prio * range / priority_map->mask_shifted);
    *priority_out = scaled_priority + udp_intf->factory->property->transport_priority_mapping_low;
}


/*ci
 * \brief Precompute the priority mask for a given UDP interface
 * \details This function precomputes the priority mask based on the user
 *          defined the UDP interface configuration. It is saved in the UDP_TransportPriorityMap
 *          structure for later use.
 * \param[in] udp_intf UDP interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_precompute_priority_mask(struct UDP_Interface *udp_intf)

{
    RTI_UINT32 mask = udp_intf->factory->property->transport_priority_mask;
    RTI_UINT32 shift = 0;

    /* User does not have the mask set, we return true since nothing else needs to be done */
    if (mask == 0)
    {
        return RTI_TRUE;
    }

    OSAPI_Heap_allocate_struct(
            &udp_intf->priority_map,
            struct UDP_TransportPriorityMap);
    if (udp_intf->priority_map == NULL)
    {
        UDP_PRIORITY_MAP_ALLOC(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Normalize mask down so it fits in 16 bits, shifting by 4 bits */
    while (mask & 0xFFFF0000u)
    {
        mask >>= 4;
        shift += 4;
    }

    /* Save results */
    udp_intf->priority_map->mask_shifted = mask;
    udp_intf->priority_map->shift_nibbles = shift;
    /* cast is ok because we verified mapping high is greater than mapping low */
    udp_intf->priority_map->range =
       (RTI_UINT32) (udp_intf->factory->property->transport_priority_mapping_high -
        udp_intf->factory->property->transport_priority_mapping_low);

    UDP_Interface_build_socket_map(udp_intf);

    return RTI_TRUE;
}


/*ci
 * \brief Set the socket priority on a socket if it is not already set to the requested priority
 *
 * \param[in] sock      The socket to set the priority on
 * \param[in] priority  The priority to set on the socket
 *
 * \return Function always return RTI_TRUE when UDP_ENFORCE_TRANSPORT_PRIORITY is not set.
 * If UDP_ENFORCE_TRANSPORT_PRIORITY is set, it returns RTI_TRUE if the priority was successfully
 * set or was already set, and RTI_FALSE if there was an error setting the priority.
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_set_socket_priority(RTI_SEND_SOCKET *sock, RTI_INT32 priority)
{
    RTI_BOOL retval  = RTI_TRUE;
    if (sock->priority == priority)
    {
        goto done;
    }
    if (netiosock_setsockopt(sock->sock, IPPROTO_IP, IP_TOS,
                             (char *)&priority, sizeof(priority)) < 0)
    {
        UDP_LOG_SOCKET_PRIORITY(OSAPI_LOGKIND_ERROR, priority, errno)
#if UDP_ENFORCE_TRANSPORT_PRIORITY
        retval = RTI_FALSE;
#endif
        goto done;
    }
    sock->priority = priority;

    /* always return true, failure to set socket priority does not result in failure to send messages */
done:
    return retval;
}


/*ci
 * \brief Get a socket with the specified priority or set the priority on a potential socket if not found
 * \details This function searches for a socket with the same priority as the requested priority
 *          in the udp_intf->unicast_send_sockets and returns it if found.
 *          If not found, it uses the first socket with priority of -1 as a potential socket,
 *          sets the requested priority on it and returns it. If no socket with priority of -1 is found,
 *          it uses the socket at index 0, sets the requested priority on it and returns it.
 *
 * \param[in] udp_intf           UDP interface
 * \param[in] requested_priority User defined transport priority
 *
 * \return Pointer to the socket with the specified priority on success, NULL on failure
 */
RTI_PRIVATE RTI_SEND_SOCKET *
UDP_Interface_get_socket_with_priority(const struct UDP_Interface *udp_intf,
                                        RTI_INT32 requested_priority)
{
    RTI_INT32 priority_out = 0;
    RTI_SEND_SOCKET *sock = NULL;
    if (udp_intf->priority_map == NULL)
    {
        return &udp_intf->unicast_send_sockets[0];
    }

    UDP_Interface_compute_priority(udp_intf,udp_intf->priority_map,
                                    requested_priority,
                                    &priority_out);
    sock = &udp_intf->unicast_send_sockets[udp_intf->priority_map->map[priority_out & 0xFF]];
    if (!UDP_Interface_set_socket_priority(sock, priority_out))
    {
       return NULL;
    }
    return sock;

}
#endif /* IP_TOS */

/*ci
 * \brief Initialize a UDP interface instance
 *
 * \param[in] udp_intf Interface to delete
 * \param[in] factory  UDP factory that is creating the instance
 * \param[in] property The property of the new UDP interface
 * \param[in] listener The listener for the new UDP interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref UDP_Interface_finalize
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_initialize(struct UDP_Interface *udp_intf,
                         struct UDP_InterfaceFactory *factory,
                         const struct NETIO_InterfaceProperty *const property,
                         const struct NETIO_InterfaceListener *const listener)
{
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_record;
    struct UDP_NatEntrySeq *nat_seq;
    RTI_INT32 i, len;
    struct DB_IndexProperty idx_prop = DB_IndexProperty_INITIALIZER;
#endif /* !RTI_CERT */

#if UDP_TRANSFORMS_ENABLED
    struct UDP_TransformProperty transform_property = UDP_TransformProperty_INITIALIZER;
#endif

    if (property)
    {
        udp_intf->property = *property;
    }
    else
    {
        return RTI_FALSE;
    }

    if (!NETIO_Interface_initialize(&udp_intf->_parent,
                                    &UDP_Interface_fv_Intf,
                                    property,listener))
    {
        UDP_LOG_INITIALIZE_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* The UDP interface does not keep track of routes */
    udp_intf->_parent._rtable = NULL;

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
                                      factory->instance_counter);

    tbl_prop.max_records = property->max_binds;
    dbrc = DB_Database_create_table(&udp_intf->_parent._btable,
                                    property->_parent.db,&tbl_name[0],
                                    sizeof(struct UDPBindEntry),
                                    UDP_Interface_compare_bind,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_UDP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        goto done;
    }

    tbl_prop.max_records = property->max_binds;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'p',
                                       factory->instance_counter);

    udp_intf->rx_thread_table = NULL;
    dbrc = DB_Database_create_table(&udp_intf->rx_thread_table,
                                    property->_parent.db,&tbl_name[0],
                                    sizeof(struct UDPPortEntry),
                                    UDP_Interface_compare_port,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_UDP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        goto done;
    }

#ifndef RTI_CERT
    udp_intf->nat = NULL;
    udp_intf->nat_local_idx = NULL;
    udp_intf->nat_public_idx = NULL;

    len = UDP_NatEntrySeq_get_length(&factory->property->nat);
    if (len > 0)
    {
        nat_seq = &factory->property->nat;
        tbl_prop.max_indices = 2;
        tbl_prop.max_records = (RTI_SIZE_T)len;
        NETIO_Interface_Table_name_from_id(tbl_name,&id,'n',
                                           factory->instance_counter);

        dbrc = DB_Database_create_table(&udp_intf->nat,
                                property->_parent.db,&tbl_name[0],
                                sizeof(struct UDP_NatEntry),
                                UDP_Interface_compare_nat_entry,&tbl_prop);

        if (dbrc != DB_RETCODE_OK)
        {
            UDP_LOG_UDP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
            goto done;
        }

        dbrc = DB_Table_create_index(udp_intf->nat,
                                    &udp_intf->nat_local_idx,
                                    UDP_Interface_compare_nat_local,
                                    &idx_prop);

        if (dbrc != DB_RETCODE_OK)
        {
            UDP_LOG_UDP_CREATE_INDEX(OSAPI_LOGKIND_ERROR,"nat_local",dbrc)
            goto done;
        }

        dbrc = DB_Table_create_index(udp_intf->nat,
                                    &udp_intf->nat_public_idx,
                                    UDP_Interface_compare_nat_public,
                                    &idx_prop);

        if (dbrc != DB_RETCODE_OK)
        {
            UDP_LOG_UDP_CREATE_INDEX(OSAPI_LOGKIND_ERROR,"nat_public",dbrc)
            goto done;
        }

        for (i = 0; i < len; i++)
        {
            struct UDP_NatEntry *nat_record_tmp;
            nat_record = NULL;
            dbrc = DB_Table_create_record(udp_intf->nat,
                                          (DB_Record_T*)&nat_record);
            if (dbrc != DB_RETCODE_OK)
            {
                UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
                goto done;
            }

            nat_record_tmp = UDP_NatEntrySeq_get_reference(nat_seq,i);
            if (nat_record_tmp == NULL)
            {
                goto done;
            }

            *nat_record = *nat_record_tmp;
            nat_record->public_address.value.ipv4.address =
                NETIO_htonl(nat_record->public_address.value.ipv4.address);
            nat_record->local_address.value.ipv4.address =
                NETIO_htonl(nat_record->local_address.value.ipv4.address);

            dbrc = DB_Table_insert_record(udp_intf->nat,nat_record);
            if (dbrc != DB_RETCODE_OK)
            {
                UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
                (void)DB_Table_delete_record(udp_intf->nat,nat_record);
                goto done;
            }
        }
    }
#endif /* !RTI_CERT */

    udp_intf->factory = factory;
    OSAPI_Heap_allocate_array(&udp_intf->unicast_send_sockets,
                                udp_intf->factory->property->max_unicast_send_sockets,
                                struct RTI_SEND_SOCKET);

    if (udp_intf->unicast_send_sockets == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    for (RTI_UINT32 j = 0 ; j < udp_intf->factory->property->max_unicast_send_sockets; j++)
    {
        udp_intf->unicast_send_sockets[j].sock = UDP_INVALID_SOCKET;
        udp_intf->unicast_send_sockets[j].priority = UDP_INVALID_SOCKET;
    }

    for (RTI_UINT32 j = 0 ; j < udp_intf->factory->property->max_unicast_send_sockets; j++)
    {
        if (!UDP_Interface_create_send_socket(udp_intf, &udp_intf->unicast_send_sockets[j]))
        {
            goto done;
        }
    }


#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)
    if (!UDP_Interface_udp_thread_initialize(udp_intf,
                                             udp_intf->factory->property))
    {
        goto done;
    }
#endif

    /* Create an interface address. Use the name notation as for the table
     * name, but use - as table kind.
     */
    NETIO_Address_init(&udp_intf->_parent.local_address,
                       NETIO_ADDRESS_KIND_INTRA);

    NETIO_Interface_Table_name_from_id(
                    (char*)&udp_intf->_parent.local_address.value.guid,&id,'-',
                    factory->instance_counter);

#if UDP_TRANSFORMS_ENABLED
    transform_property.max_receive_message_size =
                        udp_intf->factory->property->max_message_size;

    if (udp_intf->factory->property->max_send_message_size == -1)
    {
        transform_property.max_send_message_size =
                        transform_property.max_receive_message_size;
    }
    else
    {
        transform_property.max_send_message_size =
                        udp_intf->factory->property->max_send_message_size;
    }

    if (!UDP_TransformTable_initialize(&udp_intf->transform_rules,
                               factory->property->_parent._parent.registry,
                               &factory->property->destination_rules,
                               &factory->property->source_rules,
                               &transform_property))
    {
        /* UDP_TransformTable_initialize logs the error */
        goto done;
    }

    if (UDP_TransformRuleSeq_get_length(
                &udp_intf->factory->property->source_rules) > 0)
    {
        udp_intf->src_transforms_enabled = RTI_TRUE;
    }
    else
    {
        udp_intf->src_transforms_enabled = RTI_FALSE;
    }

    if (UDP_TransformRuleSeq_get_length(
                &udp_intf->factory->property->destination_rules) > 0)
    {
        udp_intf->dst_transforms_enabled = RTI_TRUE;
    }
    else
    {
        udp_intf->dst_transforms_enabled = RTI_FALSE;
    }

    udp_intf->transforms_enabled =
            ((udp_intf->dst_transforms_enabled ||
              udp_intf->src_transforms_enabled) ? RTI_TRUE : RTI_FALSE);

    udp_intf->transform_udp_mode =
        udp_intf->factory->property->transform_udp_mode;
    udp_intf->transform_locator_kind =
        udp_intf->factory->property->transform_locator_kind;

#endif

    udp_intf->use_interface_bind = RTI_FALSE;

    if ((REDA_StringSeq_get_length(
            &udp_intf->factory->property->allow_interface) > 0) ||
        (REDA_StringSeq_get_length(
            &udp_intf->factory->property->deny_interface) > 0))
    {
#if UDP_TRANSFORMS_ENABLED
        if (udp_intf->src_transforms_enabled ||
            udp_intf->factory->property->enable_interface_bind)
        {
            udp_intf->use_interface_bind = RTI_TRUE;
        }
#else
        if (udp_intf->factory->property->enable_interface_bind)
        {
            udp_intf->use_interface_bind = RTI_TRUE;
        }
#endif
    }

#if NETIO_CONFIG_ENABLE_MULTICAST
    if (!NETIO_AddressSeq_initialize(&udp_intf->multicast_interfaces))
    {
        goto done;
    }

    if (!NETIO_AddressSeq_set_maximum(&udp_intf->multicast_interfaces,
                                      UDP_MAX_MULTICAST_INTERFACES))
    {
        goto done;
    }

    OSAPI_Heap_allocate_array(&udp_intf->multicast_sockets, UDP_MAX_MULTICAST_INTERFACES, RTI_SEND_SOCKET);
    if (udp_intf->multicast_sockets == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

#ifdef IP_TOS
    if (!UDP_Interface_precompute_priority_mask(udp_intf))
    {
        goto done;
    }
#endif /* IP_TOS */

    retval = RTI_TRUE;

done:

    if (!retval)
    {
#ifndef RTI_CERT
#if defined(RTI_USE_NONBLOCKING_SOCKET) && !defined(RTI_AUTOSAR)
        UDP_Interface_udp_thread_finalize(udp_intf);
#endif
        if (udp_intf->_parent._btable != NULL)
        {
            (void)DB_Database_delete_table(property->_parent.db,
                                           udp_intf->_parent._btable);
        }

        if (udp_intf->nat_local_idx != NULL)
        {
            (void)DB_Table_delete_index(udp_intf->nat,udp_intf->nat_local_idx);
        }

        if (udp_intf->nat_public_idx != NULL)
        {
            (void)DB_Table_delete_index(udp_intf->nat,udp_intf->nat_public_idx);
        }

        if (udp_intf->nat != NULL)
        {
            (void)DB_Database_delete_table(property->_parent.db,
                                           udp_intf->nat);
        }

        if (udp_intf->rx_thread_table != NULL)
        {
            (void)DB_Database_delete_table(property->_parent.db,
                                           udp_intf->rx_thread_table);
        }

#if NETIO_CONFIG_ENABLE_MULTICAST
        {
            /* we are finalizing and safe to ignore failures */
            RTI_BOOL bretval = UDP_Interface_multicast_interface_finalize(udp_intf);
            IGNORE_RETVAL(bretval);
        }
#endif
        if (udp_intf->unicast_send_sockets != NULL)
        {
            for (RTI_UINT32 k = 0; k < udp_intf->factory->property->max_unicast_send_sockets; k++)
            {
                if (udp_intf->unicast_send_sockets[k].sock != UDP_INVALID_SOCKET)
                {
                    netiosock_close(udp_intf->unicast_send_sockets[k].sock);
                    udp_intf->unicast_send_sockets[k].sock = UDP_INVALID_SOCKET;
                }
            }
            OSAPI_Heap_free_array(udp_intf->unicast_send_sockets);
        }

        /* this function is already returning error, so no need to
         * check for the returning value here
         */
#if UDP_TRANSFORMS_ENABLED
        (void)UDP_TransformTable_finalize(&udp_intf->transform_rules);
#endif

#endif /* !RTI_CERT */
        return RTI_FALSE;
    }

    NETIO_Address_init(&udp_intf->mc_bind_address,NETIO_ADDRESS_KIND_UDPv4);
    udp_intf->multicast_enabled = RTI_FALSE;
    udp_intf->mc_bind_address.value.ipv4.address = INADDR_ANY;
    udp_intf->upstream_intf = NULL;

    return RTI_TRUE;
}

/*******************************************************************************
 *                               PUBLIC API
 ******************************************************************************/
RTI_BOOL
UDP_InterfaceFactoryProperty_initialize(struct UDP_InterfaceFactoryProperty *self)
{
    struct UDP_InterfaceFactoryProperty init =
                            UDP_InterfaceFactoryProperty_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                              return RTI_FALSE,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = init;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
UDP_InterfaceFactoryProperty_finalize(struct UDP_InterfaceFactoryProperty *p)
{
    if (!REDA_StringSeq_finalize(&p->allow_interface))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!REDA_StringSeq_finalize(&p->deny_interface))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (p->multicast_interface != NULL)
    {
        REDA_String_free(p->multicast_interface);
        p->multicast_interface = NULL;
    }

    if (!UDP_NatEntrySeq_finalize(&p->nat))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!UDP_InterfaceTableEntrySeq_finalize(&p->if_table))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

#if UDP_TRANSFORMS_ENABLED
    if (!UDP_TransformRuleSeq_finalize(&p->source_rules))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!UDP_TransformRuleSeq_finalize(&p->destination_rules))
    {
        UDP_LOG_PROPERTY_FINALIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
#endif

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Create a new UDP interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new UDP interface
 * \param[in] listener  The listener for the new UDP interface
 *
 * \return Pointer to new UDP interface instance on success, NULL
 *         on failure
 *
 * \sa \ref UDP_Interface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct UDP_Interface*
UDP_Interface_create(struct UDP_InterfaceFactory *factory,
                     const struct NETIO_InterfaceProperty *const property,
                     const struct NETIO_InterfaceListener *const listener)
{
    struct UDP_Interface *udp_intf = NULL;

    OSAPI_PRECONDITION((factory == NULL) ||
                           (property == NULL),
                           return NULL,
                   OSAPI_Log_entry_add_pointer("factory",factory,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&udp_intf, struct UDP_Interface);
    if (udp_intf == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (!UDP_Interface_initialize(udp_intf,factory,property,listener))
    {
        UDP_LOG_INITIALIZE_FAILED(OSAPI_LOGKIND_ERROR)
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(udp_intf);
#endif
        return NULL;
    }

    return udp_intf;
}

/*ci
 * \brief Initialize a IO vector from a NETIO_Packet
 *
 * \param[in] packet Packet with payload
 * \param[out] msg  The message structure
 * \param[out] msgbufs  The message buffers to send
 *
 */
RTI_PRIVATE void
UDP_Interface_set_iovec(NETIO_Packet_T *packet,
                        IOVEC_MSG *msg,
                        IOVEC_MSGBUF *msgbufs)
{
    struct NETIO_PacketBuffer *pktbuf;

    msg->IOVEC_MSG_BUFS = msgbufs;
    msg->IOVEC_MSG_LEN = 0;

    pktbuf = packet->head_pbuf;
    while (pktbuf)
    {
        msgbufs[msg->IOVEC_MSG_LEN].IOVEC_MSGBUF_BUF = NETIO_PacketBuffer_get_head(pktbuf);
        msgbufs[msg->IOVEC_MSG_LEN].IOVEC_MSGBUF_LEN = NETIO_PacketBuffer_get_length(pktbuf);
        if (msgbufs[msg->IOVEC_MSG_LEN].IOVEC_MSGBUF_LEN > 0)
        {
            ++msg->IOVEC_MSG_LEN;
        }
        pktbuf = pktbuf->_next;
    }
}


/*ci
 * \brief Send a message or packet to the specified address on the specified
 *        socket.
 *
 * \param[in] self      THE UDP interface
 * \param[in] dst_addr  The destination address
 * \param[in] packet    The packet to send
 * \param[in] sock      The socket to send from
 * \param[in] sock_addr  The socket address to to send to
 * \param[in,out] message If not NULL, the message to send, otherwise send
 *                       the payload directly. Updated with sock_addr.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_sendmsg(const struct UDP_Interface *self,
                      const struct NETIO_Address *dst_addr,
                      const NETIO_Packet_T *packet,
                      RTI_SEND_SOCKET *sock,
                      struct sockaddr_in *sock_addr,
                      IOVEC_MSG *message)
{
#if HAVE_SSIZE_T
    ssize_t bytes_sent;
#else
    int bytes_sent;
#endif
    UNUSED_ARG(self);
    RTI_BOOL retval = RTI_TRUE;

    OSAPI_TRACE_ONLY_VARIABLE(dst_addr);
    OSAPI_TRACE_NET("sending to:",RTI_FALSE)
    OSAPI_TRACE_INT32("address",dst_addr->value.ipv4.address,RTI_TRUE)

#if OSAPI_ENABLE_PRECONDITION
    if (self->priority_map == NULL && packet->info.transport_priority != 0)
    {
        UDP_PRIORITY_MAP_PRIORITY_IGNORED(OSAPI_LOGKIND_INFO)
    }
#endif

    if (packet->head_pbuf == NULL)
    {
        bytes_sent = (RTI_SIZE_T)netiosock_sendto(sock->sock,
                          NETIO_Packet_get_head(packet),
                          NETIO_Packet_get_payload_length(packet), 0,
                          (struct sockaddr *)(sock_addr),
                          sizeof(struct sockaddr_in));
    }
    else
    {
#if defined RTI_WIN32
        if (WSASendTo(sock->sock,
                      message->lpBuffers,
                      message->dwBufferCount,
                      &bytes_sent,
                      0,
                     (struct sockaddr*)sock_addr,
                     sizeof(struct sockaddr_in),NULL,NULL) == SOCKET_ERROR)
        {
            bytes_sent = -1;
            errno = WSAGetLastError();
        }
#else
        message->msg_name = sock_addr;
        message->msg_namelen = sizeof(struct sockaddr_in);
        /* coverity[cert_pos39_c_violation : FALSE] */
        bytes_sent = netiosock_sendmsg(sock->sock, message, 0);
#endif
    }

    /* Casting the return value of NETIO_Packet_get_payload_length() to
     * 'int' or 'ssize_t' should be OK as the maximum packet length that can
     * be sent in a socket is 64Kbytes. For larger packets the call should
     * fail and the cast should be ok up to packets of size 2GBytes and we
     * should not have packets larger than 2Gbytes (2^31).
     */
    if ((bytes_sent < 0) ||
#if HAVE_SSIZE_T
        (bytes_sent < (ssize_t)NETIO_Packet_get_payload_length(packet)))
#else
        (bytes_sent < (int)NETIO_Packet_get_payload_length(packet)))
#endif
    {
#if defined(RTI_WIN32) || defined(RTI_AUTOSAR)
        UDP_LOG_SEND_ERROR(OSAPI_LOGKIND_ERROR,errno)
#else
        if (errno != ENOBUFS)
        {
            UDP_LOG_SEND_ERROR(OSAPI_LOGKIND_ERROR,errno)
        }
#endif
        retval = RTI_FALSE;
    }


    OSAPI_TRACE_NET("sent data to:",RTI_FALSE)
    OSAPI_TRACE_INT32("count",bytes_sent,RTI_FALSE)
    OSAPI_TRACE_INT32("port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",dst_addr->value.ipv4.address,RTI_TRUE)

    return retval;
}

/*ci
 * \brief Send a packet or message to the specified multicast address
 *
 * \param[in] self       THE UDP interface
 * \param[in] dst_addr   The destination address
 * \param[in] packet     The packet to send
 * \param[in] sock_addr  The socket address to to send to
 * \param[in,out] message If not NULL, the message to send, otherwise send
 *                        the payload directly. Updated with sock_addr.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_send_multicast(const struct UDP_Interface *self,
                             const struct NETIO_Address *dst_addr,
                             const NETIO_Packet_T *packet,
                             struct sockaddr_in *sock_addr,
                             IOVEC_MSG *message)
{
    RTI_INT32 length = 0;
    RTI_BOOL retval = RTI_TRUE;
    RTI_SEND_SOCKET *send_sock = NULL;

#if NETIO_CONFIG_ENABLE_MULTICAST
    length = NETIO_AddressSeq_get_length(&self->multicast_interfaces);
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

    if (length == 0)
    {
#ifdef IP_TOS
        send_sock = UDP_Interface_get_socket_with_priority(self,
                                                    packet->info.transport_priority);
        if (send_sock == NULL)
        {
            return RTI_FALSE;
        }
#else
        send_sock = &self->unicast_send_sockets[0];
#endif
        retval = UDP_Interface_sendmsg(self,dst_addr,packet,
                        send_sock,sock_addr,
                              message);
    }
#if NETIO_CONFIG_ENABLE_MULTICAST
    else
    {
        RTI_INT32 index;
        struct NETIO_Address *an_address;
        for (index = 0; index < length; index++)
        {
            an_address = NETIO_AddressSeq_get_reference(
                                &self->multicast_interfaces,index);
            if (an_address == NULL)
            {
                continue;
            }
#ifdef IP_TOS
            if (self->priority_map != NULL)
            {
                RTI_INT32 priority_out = 0;
                UDP_Interface_compute_priority(self,self->priority_map,
                    packet->info.transport_priority,
                    &priority_out);

                if (!UDP_Interface_set_socket_priority(&self->multicast_sockets[index], priority_out))
                {
                    retval = RTI_FALSE;
                    continue;
                }
            }
#endif /* IP_TOS */
            OSAPI_TRACE_NET("sent data to multicast:",RTI_FALSE)
            OSAPI_TRACE_INT32("address",dst_addr->value.ipv4.address,RTI_FALSE)
            OSAPI_TRACE_INT32("interface",an_address->value.ipv4.address,RTI_TRUE)


            retval = retval && UDP_Interface_sendmsg(self,dst_addr,packet,
                                  &self->multicast_sockets[index],sock_addr,
                                  message);
        }
    }
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */

    return retval;
}

/*ci
 * \brief Send a packet or message to the specified unicast address
 *
 * \param[in] self        THE UDP interface
 * \param[in] dst_addr    The destination address
 * \param[in] packet      The packet to send
 * \param[in] sock_addr   The socket address to to send to
 * \param[in,out] message If not NULL, the message to send, otherwise send
 *                        the payload directly. Updated with sock_addr.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
UDP_Interface_send_unicast(const struct UDP_Interface *self,
                           const struct NETIO_Address *dst_addr,
                           const NETIO_Packet_T *packet,
                           struct sockaddr_in *sock_addr,
                           IOVEC_MSG *message)
{
    RTI_SEND_SOCKET *send_sock = NULL;

    OSAPI_TRACE_NET("sent data to unicast:",RTI_FALSE)
    OSAPI_TRACE_INT32("address",dst_addr->value.ipv4.address,RTI_TRUE)
#ifdef IP_TOS
    send_sock = UDP_Interface_get_socket_with_priority(self,
                                                packet->info.transport_priority);
    if (send_sock == NULL)
    {
        return RTI_FALSE;
    }
#else
    send_sock = &self->unicast_send_sockets[0];
#endif
    return UDP_Interface_sendmsg(self,dst_addr,packet,
                                 send_sock,sock_addr,
                                 message);
}

/*ci
 * \brief Send a NETIO_Packet to one or more UDP destinations in the
 *        NETIO_Packet
 *
 * \details
 * Implementation of the NETIO_Interface_send function. This function
 * sends a NETIO_Packet to one or more UDP destinations as specified in
 * the NETIO_Packet. The address parameter is not used, but is part of the
 * NETIO_Interface and present for this reason.
 *
 * \param[in] netio_intf NETIO interface to send from
 * \param[in] source     The source of the packet
 * \param[in] address    The destination address
 * \param[in] packet     The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_send(NETIO_Interface_T *netio_intf,
                   struct NETIO_Interface *source,
                   struct NETIO_Address *address,
                   NETIO_Packet_T *packet)
{
/* Definitively enough buffers */
#define MAX_SCATTER_GATHER_BUFS (32)
    struct UDP_Interface *self = (struct UDP_Interface *)netio_intf;
    struct sockaddr_in sock_addr;
    RTI_INT32 i = 0;
    struct NETIO_Address *dest_addr = NULL;
    IOVEC_MSG message;
    IOVEC_MSGBUF msgbufs[MAX_SCATTER_GATHER_BUFS];
    RTI_BOOL retval = RTI_TRUE;

#if UDP_TRANSFORMS_ENABLED
    NETIO_Packet_T saved_packet;
#endif

    UNUSED_ARG(address);
    PRECOND_ARG(source)

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (source == NULL) || (packet == NULL),
                           return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("source",source,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    /* NOTE: There are some fields in sock_add that gives a warning in valgrind
     */
#if defined(RTI_WIN32)
   OSAPI_Memory_zero(&message,sizeof(WSAMSG));
#else
    OSAPI_Memory_zero(&message,sizeof(struct msghdr));
#endif
    OSAPI_Memory_zero(&sock_addr,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;

    /* no destinations to which to send */
    if (packet->dests == NULL)
    {
        return RTI_TRUE;
    }

    /* When a packet is not transformed the content of the iovec is guranteed
     * to not change so do it once.
     */
    if (packet->head_pbuf != NULL)
    {
        UDP_Interface_set_iovec(packet,&message,msgbufs);
    }

#if UDP_TRANSFORMS_ENABLED
    saved_packet = *packet;
#endif

    for (i = 0 ; i < NETIO_AddressSeq_get_length(packet->dests); ++i)
    {
        dest_addr = NETIO_AddressSeq_get_reference(packet->dests, i);
        if (dest_addr == NULL)
        {
            OSAPI_TRACE_NET("destination address is unexpectedly NULL.",RTI_TRUE)
            continue;
        }
#if UDP_TRANSFORMS_ENABLED
        if (NETIO_Address_get_kind(dest_addr) == self->transform_locator_kind)
        {
            if (!UDP_TransformTable_transform_outgoing(&self->transform_rules,
                                                       dest_addr,
                                                       &saved_packet,&packet))
            {
                OSAPI_TRACE_NET("dropped outgoing packet",RTI_TRUE);
                continue;
            }
            OSAPI_TRACE_NET("transformed outgoing payload ",RTI_TRUE);
        }
        else if ((NETIO_Address_get_kind(dest_addr) != NETIO_ADDRESS_KIND_UDPv4) ||
                 (self->transform_udp_mode == UDP_TRANSFORM_UDP_MODE_DISABLED))
        {
            continue;
        }

        /* When a packet has been transformed the content of the iovec may
         * change for each transform. Thus, recreate the iovec after each
         * transform if needed.
         */
        if (packet->head_pbuf != NULL)
        {
            UDP_Interface_set_iovec(packet,&message,msgbufs);
        }
#endif

        sock_addr.sin_port = NETIO_htons((RTI_UINT16)dest_addr->port);
        sock_addr.sin_addr.s_addr = dest_addr->value.ipv4.address;
        
        retval = retval && UDP_Interface_send_to_destination(self,
        /* coverity[cert_pos39_c_violation:FALSE] */
                                        dest_addr,packet,&sock_addr,&message);
    }

#if UDP_TRANSFORMS_ENABLED
    *packet = saved_packet;
#endif

    return retval;
}

#ifndef RTI_CERT
/*ci
 * \brief Cancel the transmission of a NETIO_Packet
 *
 * \details
 *
 * Implementation of the NETIO_Interface_xmit_remove function.
 * Although a UDP cannot cancel transmission of packet, an upstream
 * interface does not necessarily keep track of the capabilities of the
 * downstream interface and will call the xmite_remove function on the
 * downstream interface.
 *
 * \param[in] netio_intf  NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_xmit_remove(NETIO_Interface_T *netio_intf,
                          struct NETIO_Address *destination,
                          NETIO_PacketId_T *packet_id)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(destination)
    UNUSED_ARG(packet_id);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (destination == NULL),
               return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("destination",destination,RTI_TRUE);)

   return RTI_TRUE;
}
#endif /* !RTI_CERT */


/*ci
 * \brief Add a route to a destination
 *
 * \details
 *
 * Implementation of the NETIO add_route function.
 * The UDP interface does not keep track of any route since it is state-less.
 * However, it does send a ping message to prime the ARP tables for faster
 * address resolution when sending real data.
 *
 * \param[in] netio_intf NETIO interface to add the route too
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_add_route(NETIO_Interface_T *netio_intf,
                        struct NETIO_Address *dst_addr,
                        NETIO_Interface_T *via_intf,
                        struct NETIO_Address *via_addr,
                        struct NETIORouteProperty *property,
                        RTI_BOOL *existed)
{
#ifdef RTI_CERT
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);
#else
    NETIO_Packet_T ping_packet = NETIO_Packet_INITIALIZER;
    struct sockaddr_in sock_addr;
    PRECOND_ARG(via_intf)
    UNUSED_ARG(via_addr);
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                        (dst_addr == NULL),
                        return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("via_intf",via_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("via_addr",via_addr,RTI_TRUE);)

    /* Send NOOP packet (an INFO_TS submessage) to prime network route */
    if (!NETIO_Packet_set_payload(&ping_packet,
                                  NETIO_INFO_TS_PING_MSG,
                                  NETIO_INFO_TS_PING_MSG_SIZE))
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(&sock_addr, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = NETIO_htons((RTI_UINT16)dst_addr->port);
    sock_addr.sin_addr.s_addr = dst_addr->value.ipv4.address;

    if (!UDP_Interface_send_to_destination((struct UDP_Interface *)netio_intf,
    /* coverity[cert_pos39_c_violation:FALSE] */
                    dst_addr,&ping_packet,&sock_addr,NULL))
    {
        UDP_LOG_SEND_PING(OSAPI_LOGKIND_WARNING)
    }

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }
#endif /* !RTI_CERT */

    return RTI_TRUE;
}

/*ci
 * \brief Delete a route to a peer UDP interface
 *
 * \details
 *
 * Implementation of the NETIO delete_route function.
 * The UDP interface does not keep track of any routes, but implements
 * this function to be compliant with the NETIO interface and minimize the
 * burden on the controller to keep track of which UDP interface maintains
 * state and not.
 *
 * \param[in]  netio_intf NETIO interface to add the route too
 * \param[in]  dst_addr   The destination address for the route
 * \param[in]  via_intf   The downstream interface
 * \param[in]  via_addr   The address to pass to the downstream interface
 * \param[out] existed    Whether the route existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_delete_route(NETIO_Interface_T *netio_intf,
                           struct NETIO_Address *dst_addr,
                           NETIO_Interface_T *via_intf,
                           struct NETIO_Address *via_addr,
                           RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(dst_addr)
    PRECOND_ARG(via_intf)
    PRECOND_ARG(via_addr)

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                        (dst_addr == NULL),
                        return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("via_intf",via_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("via_addr",via_addr,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Reserve addresses to listen on
 *
 * \details
 * Implementation of the NETIO reserve_address function. Refer to
 * \ref UDP_Interface_read_interface_list for details.
 *
 * \param[in]    self       NETIO interface to reserve addresses on
 * \param[in]    req_addr   List of requested addresses
 * \param[inout] resvd_addr The downstream interface
 * \param[in]    property   Properties to use to listen on the reserved addresses
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref UDP_Interface_read_interface_list
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_reserve_address(NETIO_Interface_T *self,
                              struct NETIO_AddressSeq *req_addr,
                              struct NETIO_AddressSeq *resvd_addr,
                              struct NETIOBindProperty *property)
{
    struct UDP_Interface *src_intf = (struct UDP_Interface *)self;

    if (!UDP_Interface_read_interface_list(
                        src_intf,req_addr,resvd_addr,property))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO release_address function
 *
 * \details
 * This function releases a previously reserved address so it can be used
 * by another caller.
 *
 * \param[in] self     NETIO interface to release addresses on
 * \param[in] address  Address to release
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref UDP_Interface_reserve_address
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_release_address(NETIO_Interface_T *self,
                              struct NETIO_Address *address)
{
    struct UDP_Interface *src_intf = (struct UDP_Interface *)self;
    struct UDPPortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;
    struct NETIO_Address src_address;

#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry;
#endif

    src_address = *address;

    if (DB_Database_lock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

#ifndef RTI_CERT
    if (src_intf->nat)
    {
        dbrc = DB_Table_select_match(src_intf->nat,
                                     src_intf->nat_public_idx,
                                     (DB_Record_T*)&nat_entry,address);

        if (dbrc == DB_RETCODE_OK)
        {
            src_address = nat_entry->local_address;
        }
    }
#endif

    OSAPI_TRACE_NET("release address:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)


    dbrc = DB_Table_select_match(src_intf->rx_thread_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,
                                 (DB_Key_T)&src_address);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_NET("release address not found:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
        goto done;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("release error:",RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc",dbrc,RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
        goto done;
    }

    if (port_entry->_ref_count != 0)
    {
        OSAPI_TRACE_NET("trying to release address in use:",RTI_FALSE)
        OSAPI_TRACE_INT32("ref_count",port_entry->_ref_count,RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
        goto done;
    }

    --port_entry->_shared_ref_count;
    if (port_entry->_shared_ref_count > 0)
    {
        OSAPI_TRACE_NET("trying to release address in use:",RTI_FALSE)
        OSAPI_TRACE_INT32("ref_count",port_entry->_ref_count,RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
        retval = RTI_TRUE;
        goto done;
    }

    OSAPI_TRACE_NET("delete entry:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)

    port_entry = NULL;
    dbrc = DB_Table_remove_record(src_intf->rx_thread_table,
                                  (DB_Record_T*)&port_entry,
                                  (DB_Key_T)&src_address);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("release address not found:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&src_address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_TRUE)
        goto done;
    }

    if (DB_Database_unlock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

#ifndef RTI_USE_NONBLOCKING_SOCKET
    if (port_entry->_rx_thread != NULL)
    {
        if (!OSAPI_Thread_destroy(port_entry->_rx_thread))
        {
            return RTI_FALSE;
        }
        port_entry->_rx_thread = NULL;
    }
#else
    if (netiosock_recvnotify(port_entry->_sock, NULL, NULL) != 0)
    {
        return RTI_FALSE;
    }
#endif

    if (port_entry->_rx_buffer.buffer != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(port_entry->_rx_buffer.buffer);
#endif /* !RTI_CERT */
        port_entry->_rx_buffer.buffer = NULL;
    }

#if NETIO_CONFIG_ENABLE_MULTICAST
    if (UDP_Interface_is_multicast(&port_entry->source) &&
        !UDP_Interface_leave_multicast_group(src_intf,
                                     &port_entry->source,port_entry->_sock))
    {
        return RTI_FALSE;
    }
#endif

    netiosock_close(port_entry->_sock);

    port_entry->_sock = UDP_INVALID_SOCKET;

    if (DB_Database_lock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(src_intf->rx_thread_table,
                                  (DB_Record_T)port_entry);

#if OSAPI_ENABLE_TRACE
    if (dbrc == DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("deleted port entry:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&port_entry->source),RTI_FALSE)
        OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_TRUE)
    }
    else
    {
        OSAPI_TRACE_NET("failed to delete port entry:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&port_entry->source),RTI_FALSE)
        OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_TRUE)
    }
#endif

    retval = (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);

done:

    if (DB_Database_unlock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Listen to a UDP peer
 *
 * \details
 *
 * Implementation of the NETIO bind function.
 * UDP does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with distinguishing between different UDP implementations.
 *
 * \param[in]  self     NETIO interface to bind
 * \param[in]  src_addr The address to bind to
 * \param[in]  property The property to use for the bind
 * \param[out] existed  Whether a previous bind existed or not
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_bind(NETIO_Interface_T *netio_intf,
                   struct NETIO_Address *src_addr,
                   struct NETIOBindProperty *property,
                   RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(src_addr)
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (src_addr == NULL),
                           return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Stop listening to a UDP peer
 *
 * \details
 *
 * Implementation of the NETIO unbind function.
 * UDP does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with distinguishing between different UDP implementations.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   Interface
 * \param[out] existed    Whether a bind existed or not
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref UDP_Interface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_unbind(NETIO_Interface_T *netio_intf,
                     struct NETIO_Address *src_addr,
                     NETIO_Interface_T *dst_intf,
                     RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(src_addr)
    PRECOND_ARG(dst_intf)
    UNUSED_ARG(existed);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                        (src_addr == NULL) ||
                        (dst_intf == NULL),
                return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

#if defined(RTI_AUTOSAR)
/*ci
 * \brief Process a UDP packet
 *
 * \details
 *
 * \param[in] port_entry Port entry to use
 * \param[in] buffer Pointer to buffer with incoming packet
 * \param[in] rx_len Length of data in buffer
 * \param[in] ip_src Source address
 */
RTI_PRIVATE void
UDP_Interface_receive_packet(void *port_entry_param,
                             char *buffer,
                             RTI_INT32 rx_len,
                             const struct sockaddr_in *ip_src)
{
    struct UDPPortEntry *port_entry = (struct UDPPortEntry *)port_entry_param;
    struct NETIO_Packet packet;
    RTI_BOOL bretval;

    OSAPI_TRACE_NET("wait for data:",RTI_FALSE)
    OSAPI_TRACE_INT32("port",port_entry->source.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",port_entry->source.value.ipv4.address,RTI_FALSE)
    OSAPI_TRACE_INT32("length",rx_len,RTI_TRUE)

    if (rx_len <= 0)
    {
#if OSAPI_ENABLE_LOG
        if (rx_len < 0)
        {
            UDP_LOG_RECV_ERROR(OSAPI_LOGKIND_ERROR,errno)
        }
#endif
        return;
    }

    /* Ensure that properties pointer is still valid. If UDP factory is
     * unregistered before participant is deleted that pointer could be NULL.
     * To avoid a run time error we check for a NULL pointer, but there is
     * nothing we can do with the received message.
     */
    if (port_entry->_udp_intf->factory->property == NULL)
    {
        OSAPI_TRACE_NET("UDP factory property NULL, dropping msg",RTI_TRUE)
        return;
    }

    if (rx_len > port_entry->_udp_intf->factory->property->max_message_size)
    {
        OSAPI_TRACE_NET("received message larger then max message:",RTI_FALSE)
        OSAPI_TRACE_INT32("size",rx_len,RTI_TRUE)
        return;
    }

    if (!NETIO_Packet_initialize(&packet,buffer,
                                    (RTI_SIZE_T)rx_len, 0, NULL))
    {
        UDP_LOG_PACKET_INIT(OSAPI_LOGKIND_ERROR,
                            &packet,buffer,
                            rx_len)
        return;
    }

    if (!NETIO_Packet_set_head(&packet, 0 - rx_len))
    {
        UDP_LOG_PACKET_HEAD(OSAPI_LOGKIND_ERROR,&packet, 0 - rx_len)
        return;
    }

    OSAPI_TRACE_NET("process message",RTI_TRUE)

    if (NETIO_Packet_is_ndds_ping(&packet))
    {
        return;
    }

    NETIO_Address_set_ipv4(&packet.source,
                           (RTI_UINT32)NETIO_ntohs(ip_src->sin_port),
                           NETIO_ntohl(ip_src->sin_addr.s_addr));

    bretval = UDP_Interface_receive(&port_entry->_udp_intf->_parent,
                                    &port_entry->source,&packet);
#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        UDP_LOG_PACKET_FWD(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif
    OSAPI_TRACE_NET("message processed:",RTI_TRUE)
}
#endif

/*ci
 * \brief Add an upstream NETIO interface as a listener to data from the
 *        specified interface and address
 *
 * \details
 *
 * Implementation of the NETIO bind_external function.
 * When an upstream interface want to listen to a UDP port/address it
 * binds to the downstream interface using the external bind function.
 * When the UDP interface is bound to an upstream interface it starts the
 * receive thread for the port. If multiple upstream interfaces binds to the
 * same port the port is reference counted.
 *
 * \param[in]  self       NETIO interface to bind to upstream interface
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to pass to the upstream interface
 * \param[in]  property   The properties for the bind
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref UDP_Interface_unbind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_bind_external(NETIO_Interface_T *self,
                            struct NETIO_Address *src_addr,
                            NETIO_Interface_T *dst_intf,
                            struct NETIO_Address *dst_addr,
                            struct NETIOBindProperty *property,
                            RTI_BOOL *existed)
{
    struct UDP_Interface *src_intf = (struct UDP_Interface *)self;
    struct UDPBindEntry *bind_entry = NULL;
    struct UDPPortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    struct NETIO_Address src_address;
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry;
#endif
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((src_intf == NULL) ||
                            (src_addr == NULL) ||
                            (dst_intf == NULL) ||
                            (dst_addr == NULL),
                            return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    OSAPI_TRACE_NET("create route:",RTI_FALSE)
    OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address",&dst_addr->value.rtps_guid,RTI_TRUE)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

#ifndef RTI_CERT
    if (src_intf->nat)
    {
        dbrc = DB_Table_select_match(src_intf->nat,
                                     src_intf->nat_public_idx,
                                     (DB_Record_T*)&nat_entry,src_addr);

        if (dbrc == DB_RETCODE_OK)
        {
            src_address = nat_entry->local_address;
        }
    }
#endif /* !RTI_CERT */

    /* Find the receive entry */
    dbrc = DB_Table_select_match(src_intf->rx_thread_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,
                                 (DB_Key_T)&src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("address not found:",RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_address.port,RTI_FALSE)
        OSAPI_TRACE_GUID("address",&src_address.value.rtps_guid,RTI_TRUE)
        UDP_LOG_PORT_NOT_FOUND(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(src_intf->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&bind_entry,
                                (DB_Key_T)&bind_key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        ++bind_entry->ref_count;
        OSAPI_TRACE_NET("forward data:",RTI_FALSE)
        OSAPI_TRACE_INT32("ref_count",bind_entry->ref_count,RTI_FALSE)
        OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
        OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
        OSAPI_TRACE_GUID("dst.address",&dst_addr->value.rtps_guid,RTI_TRUE)

        return RTI_TRUE;
    }

    dbrc = DB_Table_create_record(src_intf->_parent._btable,
                                  (DB_Record_T*)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    bind_entry->_parent.source = src_address;
    bind_entry->_parent.destination = *dst_addr;
    bind_entry->_parent.intf = dst_intf;
    bind_entry->ref_count = 1;

    dbrc = DB_Table_insert_record(src_intf->_parent._btable,
                                  (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(src_intf->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    ++port_entry->_ref_count;

    OSAPI_TRACE_NET("forward data:",RTI_FALSE)
    OSAPI_TRACE_INT32("ref_count",bind_entry->ref_count,RTI_FALSE)
    OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address",&src_addr->value.rtps_guid,RTI_TRUE)

    if (port_entry->_ref_count > 1)
    {
        return RTI_TRUE;
    }

#ifndef RTI_USE_NONBLOCKING_SOCKET
    return OSAPI_Thread_start(port_entry->_rx_thread);
#else
#ifdef RTI_THREADX
    return (netiosock_recvnotify(port_entry->_sock,
                                 src_intf->udp_queue,
                                 port_entry) == 0) ? RTI_TRUE : RTI_FALSE;
#elif defined RTI_AUTOSAR
    return (netiosock_recvnotify(port_entry->_sock,
                                 UDP_Interface_receive_packet,
                                 port_entry) == 0) ? RTI_TRUE : RTI_FALSE;
#else
    #error "Unsupported OS for non-blocking sockets."
#endif
#endif
}

/*ci
 * \brief Stop forwarding data from to an upstream interface
 *
 * \details
 *
 * Implementation of the NETIO unbind_external function.
 * When an upstream interface wants to remove a listener to a UDP port/address
 * it unbinds from the downstream interface using the external unbind function.
 * When the UDP interface is unbound from an upstream interface it stops the
 * receive thread for the port if the reference count is 0.
 *
 * \param[in]  src_intf   NETIO interface to unbind from upstream interface
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to passed to the upstream interface
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref UDP_Interface_bind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_unbind_external(NETIO_Interface_T *src_intf,
                              struct NETIO_Address *src_addr,
                              NETIO_Interface_T *dst_intf,
                              struct NETIO_Address *dst_addr,
                              RTI_BOOL *existed)
{
    struct UDP_Interface *self = (struct UDP_Interface *)src_intf;
    struct UDPBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    struct UDPPortEntry *port_entry = NULL;
    struct NETIO_Address src_address;
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry;
#endif /* !RTI_CERT */
    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION((src_intf == NULL) ||
                            (src_addr == NULL) ||
                            (dst_intf == NULL) ||
                            (dst_addr == NULL),
                            return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    OSAPI_TRACE_NET("remove forwarding of data:",RTI_FALSE)
    OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address",&dst_addr->value.rtps_guid,RTI_TRUE)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

#ifndef RTI_CERT
    if (self->nat)
    {
        dbrc = DB_Table_select_match(self->nat,
                self->nat_public_idx,
                (DB_Record_T*)&nat_entry,src_addr);

        if (dbrc == DB_RETCODE_OK)
        {
            src_address = nat_entry->local_address;
        }
    }
#endif /* RTI_CERT */

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(self->_parent._btable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&bind_entry,
                                 (DB_Key_T)&bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("address does not exist",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
        OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

        return RTI_TRUE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    --bind_entry->ref_count;

    OSAPI_TRACE_NET("remove forwarding of data:",RTI_FALSE)
    OSAPI_TRACE_INT32("ref_count",bind_entry->ref_count,RTI_FALSE)
    OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address",&dst_addr->value.rtps_guid,RTI_TRUE)

    if (bind_entry->ref_count > 0)
    {
        return RTI_TRUE;
    }

    bind_entry = NULL;
    dbrc = DB_Table_remove_record(self->_parent._btable,
                                (DB_Record_T*)&bind_entry,
                                (DB_Key_T)&bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(self->_parent._btable,
                                 (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_WARNING,dbrc)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("deleted forwarding of data:",RTI_FALSE)
    OSAPI_TRACE_INT32("src.port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("src.address",&src_addr->value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port",dst_addr->port,RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address",&dst_addr->value.rtps_guid,RTI_TRUE)

    /* Find the receive entry */
    dbrc = DB_Table_select_match(self->rx_thread_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&port_entry,
                                (DB_Key_T)&src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    --port_entry->_ref_count;

    OSAPI_TRACE_NET("port unbound:",RTI_FALSE)
    OSAPI_TRACE_INT32("ref_count",port_entry->_ref_count,RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(src_addr),RTI_FALSE)
    OSAPI_TRACE_INT32("port",src_addr->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_addr->value.ipv4.address,RTI_TRUE)

    return RTI_TRUE;
}

/*ci
 * \brief UDP receive function
 *
 * \details
 *
 * This UDP interface does not have a downstream interface, it is terminated
 * at the socket layer. Thus, this function is not called from another
 * NETIO interface but from the internal UDP receive thread. Whenever the
 * UDP receive thread receives a packet it calls this function to pass the
 * packet to upstream NETIO interfaces bound to this interface.
 *
 * \param[in] netio_intf NETIO interface to receive on
 * \param[in] source     The source NETIO address of the packet
 * \param[in] packet     The packet to process
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_receive(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *source,
                      NETIO_Packet_T *packet)
{
    struct UDP_Interface *self = (struct UDP_Interface *)netio_intf;
    RTI_SIZE_T pkt_head, pkt_tail;
    RTI_BOOL retval = RTI_FALSE;
    struct NETIO_Address real_source;
    RTI_BOOL bretval;
    DB_Cursor_T cursor = NULL;
    struct UDPBindEntry *bind_entry;
    DB_ReturnCode_T dbrc;
#ifndef RTI_CERT
    struct UDP_NatEntry *nat_entry = NULL;
#endif /* !RTI_CERT */

#if UDP_TRANSFORMS_ENABLED
    NETIO_Packet_T saved_packet;
#endif

    if (DB_Database_lock(self->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Mutex_take(self->property.network_lock))
    {
        return RTI_FALSE;
    }

    NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);

#if UDP_TRANSFORMS_ENABLED
    saved_packet = *packet;
#endif

#if UDP_TRANSFORMS_ENABLED
    if (self->src_transforms_enabled)
    {
        if (!UDP_TransformTable_transform_incoming(&self->transform_rules,
                                                   source,
                                                   &saved_packet,&packet))
        {
            OSAPI_TRACE_NET("dropped incoming packet",RTI_TRUE);
            goto done;
        }
        OSAPI_TRACE_NET("transformed incoming packet",RTI_TRUE);
    }
#endif


    if (self->upstream_intf != NULL)
    {
        real_source = *source;

#ifndef RTI_CERT
        if (self->nat)
        {
            dbrc = DB_Table_select_match(self->nat,
                                        self->nat_local_idx,
                                        (DB_Record_T*)&nat_entry,source);

            if (dbrc == DB_RETCODE_OK)
            {
                real_source = nat_entry->public_address;
            }
        }
#endif
#if RTIME_UNITTEST_INCLUDE_SRCADDR
        packet->udp_origin_addr = source->value.ipv4.address;
#endif
        if (!NETIO_Interface_receive(self->upstream_intf,
                                     &real_source,NULL,packet))
        {
            retval = RTI_FALSE;
        }
        else
        {
            retval = RTI_TRUE;
        }
    }
    else
    {
        dbrc = DB_Table_select_all(self->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,&cursor);
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
            while (dbrc == DB_RETCODE_OK)
            {
                /* If we are receiving on unicast do not check the address
                 * because the same port is bound to _all_ interfaces. For
                 * multicast the port is bound to a specific address and thus
                 * the address is also checked.
                 */
                if ((UDP_Interface_is_multicast(&bind_entry->_parent.source) &&
                    ((bind_entry->_parent.source.value.ipv4.address == source->value.ipv4.address) &&
                    (bind_entry->_parent.source.port == source->port))) ||
                    (!UDP_Interface_is_multicast(&bind_entry->_parent.source) &&
                    (bind_entry->_parent.source.port == source->port)))
                {
                    OSAPI_TRACE_NET("received data:",RTI_FALSE)
                    OSAPI_TRACE_INT32("src.port",bind_entry->_parent.source.port,RTI_FALSE)
                    OSAPI_TRACE_GUID("src.address",&bind_entry->_parent.source.value.rtps_guid,RTI_FALSE)
                    OSAPI_TRACE_INT32("dst.port",bind_entry->_parent.destination.port,RTI_FALSE)
                    OSAPI_TRACE_GUID("dst.address",&bind_entry->_parent.destination.value.rtps_guid,RTI_TRUE)

                    /* Assign the packet which address the packet was received on */
                    packet->local_source = bind_entry->_parent.source;
                    packet->info.originating_transport = NETIO_ADDRESS_KIND_UDPv4;

                    bretval = NETIO_Interface_receive(bind_entry->_parent.intf,
                                                &self->_parent.local_address,
                                                &bind_entry->_parent.destination,
                                                packet);
    #if OSAPI_ENABLE_LOG
                    if (!bretval)
                    {
                        UDP_LOG_PACKET_FWD(OSAPI_LOGKIND_WARNING)
                    }
    #else
                    IGNORE_RETVAL(bretval);
    #endif

                    NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);
                    /* Force the while to terminate cleanly. All received UDP
                    * traffic is forwarded upstream to the same RTPS interface.
                    * This is by design,. future versions may forward the same
                    * packet to multiple upstream interfaces.
                    */
                    dbrc = DB_RETCODE_NO_DATA;
                }
                else
                {
                    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
                }
            }
            DB_Cursor_finish(self->_parent._btable,cursor);

            if (dbrc != DB_RETCODE_NO_DATA)
            {
                UDP_LOG_CURSOR_ERROR(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            retval = RTI_TRUE;
        }
    }

done:

#if UDP_TRANSFORMS_ENABLED
    *packet = saved_packet;
#endif

    /* Justification: OSAPI_Mutex_give is a thin wrapper function around
     * pthread_mutex_unlock. The rule's strict interpretation flags this
     * because the definition of OSAPI_Mutex_give resides in a different
     * compilation unit than this calling function. The mutex is
     * correctly acquired and released within the same function.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    if (!OSAPI_Mutex_give(self->property.network_lock))
    {
        return RTI_FALSE;
    }

    if (DB_Database_unlock(self->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Set the state of the UDP interface
 *
 * \details
 * Implementation of the NETIO set_state function. The UDP interface is
 * always enabled and simply sets the specified state.
 *
 * \param[in] src_intf NETIO interface to set state on
 * \param[in] state    New state
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_set_state(NETIO_Interface_T *src_intf,
                        NETIO_InterfaceState_T state)
{
    struct UDP_Interface *self = (struct UDP_Interface *)src_intf;

    OSAPI_PRECONDITION((src_intf == NULL),return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_TRUE);)
    self->_parent.state = state;

    return RTI_TRUE;
}

/*
 * String to address conversions.
 */

/*ci
 * \brief Try converting an IP address in dotted notation to an integer
 *
 * \param[in] src Address to convert
 * \param[in] dst Result on success
 *
 * \return NETIO_UDP_INTERFACE_ADDR_FORMAT_OK,
 *         NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID if the address is not in a
 *         valid format, NETIO_UDP_INTERFACE_ADDR_FORMAT_OUT_OF_RANGE if any
 *         of the 4 digits exceed 255
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
UDP_Interface_address_pton4(const char *src, unsigned char *dst)
{
#define INADDRSZ         4
    static const char digits[] = "0123456789";
    int saw_digit, octets, ch;
    unsigned char tmp[INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;

        if ((pch = UDP_Interface_strrchr(digits, ch)) != NULL)
        {
            unsigned int new = (unsigned int)(*tp * 10 + (pch - digits));

            if (new > 255)
            {
                return NETIO_UDP_INTERFACE_ADDR_FORMAT_OUT_OF_RANGE;
            }
            *tp = (unsigned char)new;
            if (!saw_digit)
            {
                if (++octets > 4)
                {
                    return NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID;
                }
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
            {
                return NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID;
            }
            *++tp = 0;
            saw_digit = 0;
        }
        else
        {
            return NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID;
        }
    }
    if (octets < 4)
    {
        return NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID;
    }

    OSAPI_Memory_copy(dst, tmp, INADDRSZ);

    return NETIO_UDP_INTERFACE_ADDR_FORMAT_OK;
#undef INADDRSZ
}

/*ci
 * \brief Try converting an IP address in char* format to a NETIO_Address
 *
 * \param[out]  address_out NETIO_Address on success
 * \param[in]   address_in  Address to convert
 * \param[out]  is_invalid  Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that the return
 *         value indicates the success of the function call itself, not if the
 *         address is valid or not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_address_ipv4_string_to_address(struct UDP_Interface *udp_intf,
                                             struct NETIO_Address *address_out,
                                             const char *address_in,
                                             RTI_BOOL *is_invalid)
{
#ifndef RTI_CERT
    struct sockaddr_in sockAddr;
#endif
    unsigned char addr[4];
    RTI_INT32 addr_kind = NETIO_ADDRESS_KIND_UDPv4;
    RTI_INT32 ret_code;

#if !UDP_TRANSFORMS_ENABLED
    UNUSED_ARG(udp_intf);
#endif

    if (address_in == NULL)
    {
        *is_invalid = RTI_TRUE;
        return RTI_FALSE;
    }

    /* We cannot be 100% sure if the address is valid, assume it is. */
    *is_invalid = RTI_FALSE;

    if (address_in[0] == 0)
    {
        return RTI_TRUE;
    }

    /* Determine if it is in IPv4 notation */
    ret_code = UDP_Interface_address_pton4(address_in, addr);
    if (ret_code == NETIO_UDP_INTERFACE_ADDR_FORMAT_OK)
    {
        OSAPI_Memory_copy(&address_out->value.ipv4.address,addr,4);
    }
    else if (ret_code == NETIO_UDP_INTERFACE_ADDR_FORMAT_OUT_OF_RANGE)
    {
        /* Address is invalid
         */
        *is_invalid = RTI_TRUE;

        return RTI_FALSE;
    }
    else
    {
        /* return value of UDP_Interface_address_pton4() is
         * NETIO_UDP_INTERFACE_ADDR_FORMAT_INVALID
         */
#ifdef RTI_CERT
        /* NETIO_Socket_get_hostbyname is not supported by Cert. Only valid
         * dot notation IP addresses are supported.
         */
        return RTI_FALSE;
#else
        if (NETIO_Socket_get_hostbyname(&sockAddr.sin_addr, address_in))
        {
            NETIO_LOG_GETHOST_BYNAME(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
        address_out->value.ipv4.address = sockAddr.sin_addr.s_addr;
#endif
    }

#if UDP_TRANSFORMS_ENABLED
    if (UDP_TransformTable_has_transform(&udp_intf->transform_rules,address_out) ||
        (udp_intf->transform_udp_mode == UDP_TRANSFORM_UDP_MODE_DISABLED))
    {
        addr_kind = udp_intf->transform_locator_kind;
    }
#endif

    if (UDP_Interface_is_multicast(address_out))
    {
        NETIO_Address_set_kind(address_out,(RTI_UINT32)addr_kind,
                               NETIO_ADDRESS_FLAG_MULTICAST);
    }
    else
    {
        NETIO_Address_set_kind(address_out,(RTI_UINT32)addr_kind,0);
    }

    return RTI_TRUE;
}

/*ci
 * \brief Resolve an IPv4 address string to NETIO_Address if possible
 *
 * \details
 * Implementation of the NETIO resolve_address function. This function
 * is called by resolvers to determine if this interface understands the
 * string address.
 *
 * \param[out] netio_intf     Interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_resolve_ipv4_address(NETIO_Interface_T *netio_intf,
                                   const char *address_string,
                                   struct NETIO_Address *address_value,
                                   RTI_BOOL *is_invalid)

{
    struct UDP_Interface *src_intf = (struct UDP_Interface*)netio_intf;

    NETIO_Address_init(address_value,NETIO_ADDRESS_KIND_UDPv4);

    return UDP_Interface_address_ipv4_string_to_address(src_intf,
                                                address_value,
                                                address_string,is_invalid);
}

/*ci
 * \brief Add an address and netmask to an NETIO_Address and NETIO_Netmask
 *        sequence
 *
 * \param[inout] address_seq  NETIO_AddressSeq to append to
 * \param[inout] netmask_seq  NETIO_NetmaskSeq to append to
 * \param[in]    address_kind Type of address
 * \param[in]    address      UDPv4 address to add
 * \param[in]    netmask      The netmask to add
 * \param[in]    netmask_bits The number of valid bits in the netmask
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_add_route_entry(struct NETIO_AddressSeq *address_seq,
                              struct NETIO_NetmaskSeq *netmask_seq,
                              RTI_INT32 address_kind,
                              RTI_UINT32 address,
                              RTI_UINT32 netmask,
                              RTI_UINT32 netmask_bits)
{
    struct NETIO_Address src_address = NETIO_Address_INITIALIZER;
    struct NETIO_Netmask src_netmask = NETIO_Netmask_INITIALIZER;
    RTI_INT32 len;

    len = NETIO_AddressSeq_get_length(address_seq);
    if (len >= NETIO_AddressSeq_get_maximum(address_seq))
    {
        UDP_LOG_GET_LENGTH(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_length(address_seq,len+1))
    {
        UDP_LOG_SET_LENGTH(OSAPI_LOGKIND_ERROR,len+1)
        return RTI_FALSE;
    }

    if (!NETIO_NetmaskSeq_set_length(netmask_seq,len+1))
    {
        (void)NETIO_AddressSeq_set_length(address_seq,len);
        UDP_LOG_SET_LENGTH(OSAPI_LOGKIND_ERROR,len+1)
        return RTI_FALSE;
    }

    NETIO_Address_init(&src_address,address_kind);
    src_address.value.ipv4.address = address;

    /* Since set_length succeeded for both sequences it is assumed that
     * get_reference returns a valid address.
     */
    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_AddressSeq_get_reference(address_seq,len) = src_address;
    src_netmask.bits = netmask_bits;
    src_netmask.mask[0] = netmask;

    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_NetmaskSeq_get_reference(netmask_seq,len) = src_netmask;

    OSAPI_TRACE_NET("added route entry address:",RTI_FALSE)
    OSAPI_TRACE_INT32("address",src_address.value.ipv4.address,RTI_FALSE)
    OSAPI_TRACE_GUID("netmask",&src_netmask.mask,RTI_FALSE)
    OSAPI_TRACE_INT32("netmask.bits",netmask_bits,RTI_TRUE)

    return RTI_TRUE;
}

/*ci
 * \brief Return the route table for the UDP interface
 *
 * \details
 * Implementation of the NETIO get_route_table function. This function
 * returns the addresses the UDP interface can send data to after applying
 * the properties to filter out interfaces.
 *
 * \param[in]    netio_intf The NETIO interface
 * \param[inout] address    Sequence of NETIO addresses this interface understands
 * \param[inout] netmask    Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_get_route_table(NETIO_Interface_T *netio_intf,
                             struct NETIO_AddressSeq *address,
                             struct NETIO_NetmaskSeq *netmask)
{
    RTI_BOOL ok = RTI_FALSE;
    struct UDP_Interface *src_intf = (struct UDP_Interface*)netio_intf;
    RTI_INT32 max_size;
    RTI_INT32 i;
    RTI_INT32 cur_addr_len;
    RTI_BOOL has_mc = RTI_FALSE;
    struct UDP_IfContext if_context = UDP_IFCONTEXT_INITIALIZER;
    struct UDP_NetworkIfInfo if_data = UDP_NetworkIfInfo_INITIALIZER;
    RTI_BOOL if_error = RTI_FALSE;
#if UDP_TRANSFORMS_ENABLED
    struct UDP_TransformRule *rule;
    RTI_INT32 l;
    RTI_BOOL allow_udp;
    RTI_UINT32 netmask_bits;
#endif

    cur_addr_len = NETIO_AddressSeq_get_length(address);
    max_size = NETIO_AddressSeq_get_maximum(address) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }

#if UDP_TRANSFORMS_ENABLED
    allow_udp = (src_intf->transform_udp_mode == UDP_TRANSFORM_UDP_MODE_ENABLED);

    if (allow_udp && src_intf->factory->property->is_default_interface)
#else
    if (src_intf->factory->property->is_default_interface)
#endif
    {
        if (!UDP_Interface_add_route_entry(address,netmask,
                                           NETIO_ADDRESS_KIND_UDPv4,0,0,0))
        {
            return RTI_FALSE;
        }
        --max_size;
    }

#if UDP_TRANSFORMS_ENABLED
    /* Add all the destination rules. Note that the destination addresses
     * are not necessarily related to routes indicated by the local transport.
     * This is because the destination rule addresses may be on different
     * networks, routed by intermediate routers.
     *
     * There is no implicit default route for destination rules since
     * there is no default transformation. However, a default route can
     * be added as rule if the implementation has a default transformation.
     * The same applies to multicast, it must be specifically added.
     */
    l = UDP_TransformRuleSeq_get_length(
                        &src_intf->factory->property->destination_rules);

    for (i = 0; (i < l) && (i < max_size); ++i)
    {
        rule = UDP_TransformRuleSeq_get_reference(
                        &src_intf->factory->property->destination_rules,
                        i);

        /* If there is no mask set the bits to 0 to allow the route to
         * match with all addresses.
         */
        if (rule->netmask.mask[0] == 0)
        {
            netmask_bits = 0;
        }
        else
        {
            netmask_bits = 32;
        }

        if (!UDP_Interface_add_route_entry(address,netmask,
                                           src_intf->transform_locator_kind,
                                           NETIO_htonl(rule->address.value.ipv4.address),
                                           rule->netmask.mask[0],netmask_bits))
        {
            return RTI_FALSE;
        }
    }

    cur_addr_len = NETIO_AddressSeq_get_length(address);
    max_size = NETIO_AddressSeq_get_maximum(address) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }
#endif


    i = 0;
#if UDP_TRANSFORMS_ENABLED
    while (allow_udp && UDP_Interface_get_next_interface(src_intf,
                     &if_context,&if_data,&if_error) && (i < max_size))
#else
    while (UDP_Interface_get_next_interface(src_intf,
                         &if_context,&if_data,&if_error) && (i < max_size))
#endif
    {
        if (!UDP_Interface_is_valid_address(&if_data))
        {
            continue;
        }

        if (!UDP_Interface_interface_allowed(src_intf,if_data.lastname))
        {
#ifdef RTI_WIN32
            if (!UDP_Interface_interface_allowed(src_intf,if_data.fname))
            {
                OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
                OSAPI_TRACE_STRING("id",if_data.fname,RTI_TRUE)
                continue;
            }
#else
            OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
            OSAPI_TRACE_STRING("id",if_data.lastname,RTI_TRUE)
            continue;
#endif
        }

        if (!has_mc && if_data.mc_enabled)
        {
            has_mc = RTI_TRUE;
        }

        if (!UDP_Interface_add_route_entry(address,netmask,
                           NETIO_ADDRESS_KIND_UDPv4,
                           if_data.intf_address,if_data.intf_netmask,32))
        {
            goto done;
        }

        ++i;
    }

#ifndef RTI_CERT
    if (if_error)
    {
        goto done;
    }
#else
    /* Ignore the return value in Cert because the function cannot fail */
    IGNORE_RETVAL(if_error);
#endif

#if UDP_TRANSFORMS_ENABLED
    if (allow_udp && has_mc)
#else
    if (has_mc)
#endif
    {
        if (!UDP_Interface_add_route_entry(address,netmask,
                                           NETIO_ADDRESS_KIND_UDPv4,
                                           NETIO_htonl(0xe0000000),0xf0000000,4))
        {
            goto done;
        }
    }

    ok = RTI_TRUE;

done:

#if UDP_TRANSFORMS_ENABLED
    if (allow_udp)
    {
        UDP_Interface_get_next_interface_done(src_intf,&if_context);
    }
#else
    UDP_Interface_get_next_interface_done(src_intf,&if_context);
#endif

    return ok;
}

RTI_PRIVATE RTI_BOOL
UDP_Interface_is_address_reachable(struct NETIO_Interface *netio_intf,
                                   const struct NETIO_Address *const address,
                                   RTI_BOOL *is_reachable)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(address);

    /* Thisd function would not be called if it had not already been
     * able to route to this address. It is only implemented to simiplify the
     * caller logic.
     */
    *is_reachable = RTI_TRUE;

    return RTI_TRUE;
}

RTI_PRIVATE void
UDP_Interface_get_transport_properties(struct NETIO_Interface *netio_intf,
                                       struct NETIO_TransportProperty *properties)
{
    struct UDP_Interface *self = (struct UDP_Interface*)netio_intf;

    /* The assumption these cannot be negative */
    properties->recv_size_max = (RTI_UINT32)self->factory->property->max_message_size;
    properties->send_size_max = (RTI_UINT32)self->factory->property->max_message_size;
}

/******************************************************************************
 *
 * UDP Component Interface
 */
/*ci
 * \brief The NETIO UDP interface implementation
 *
 * \details
 *
 * The UDP interface is always at the bottom of the stack, thus receive
 * is not implemented. It does also not receive any events
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI UDP_Interface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    UDP_Interface_send,                  /* send */
    NULL,                                /* acknack */
    NULL,                                /* request */
    NULL,                                /* return_loan */
#ifndef RTI_CERT
    UDP_Interface_xmit_remove,           /* xmit_remove */
#else
    NULL,
#endif
    UDP_Interface_add_route,             /* add_route */
    UDP_Interface_delete_route,          /* delete_route */
    UDP_Interface_reserve_address,       /* get_global_address */
    UDP_Interface_bind,                  /* bind */
    UDP_Interface_unbind,                /* unbind */
    NULL,                                /* UDP_Interface_receive */
    NULL,                                /* get_external_interface */
    UDP_Interface_bind_external,         /* bind_external */
    UDP_Interface_unbind_external,       /* unbind_external */
    UDP_Interface_set_state,
    UDP_Interface_release_address,
    UDP_Interface_resolve_ipv4_address,
    UDP_Interface_get_route_table,
    NULL,                                 /* post_event */
    NULL,
    UDP_Interface_is_address_reachable,
    UDP_Interface_get_transport_properties
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Create a new instance of the UDP interface
 *
 * \details
 * Implementation of the RT ComponentFactory create component method. This
 * method is not called directly, only via the factory interface.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
UDP_InterfaceFactory_create_component(struct RT_ComponentFactory *factory,
                                      struct RT_ComponentProperty *property,
                                      struct RT_ComponentListener *listener)
{
    struct UDP_Interface *retval = NULL;
    struct UDP_InterfaceFactory* udpf = (struct UDP_InterfaceFactory*)factory;

    retval = UDP_Interface_create(udpf,
                        (const struct NETIO_InterfaceProperty *const)property,
                        (const struct NETIO_InterfaceListener *const)listener);

    if (retval == NULL)
    {
        return NULL;
    }

    ++udpf->instance_counter;
    return &retval->_parent._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the UDP interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This method is not
 * called directly, only via the factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] property  The component to be deleted
 *
 * \sa \ref UDP_InterfaceFactory_create_component
 */
RTI_PRIVATE void
UDP_InterfaceFactory_delete_component(struct RT_ComponentFactory *factory,
                                      RT_Component_T *component)
{
    struct UDP_Interface *self = (struct UDP_Interface*)component;
    UNUSED_ARG(factory);

    UDP_Interface_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
UDP_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty* property,
                                struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
UDP_InterfaceFactory_finalize(struct RT_ComponentFactory *factory,
                              struct RT_ComponentFactoryProperty **property,
                              struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI UDP_InterfaceFactory_fv_Intf =
{
    UDP_INTERFACE_INTERFACE_ID,
    UDP_InterfaceFactory_initialize,
#ifndef RTI_CERT
    UDP_InterfaceFactory_finalize,
#else
    NULL,
#endif
    UDP_InterfaceFactory_create_component,
#ifndef RTI_CERT
    UDP_InterfaceFactory_delete_component,
#else
    NULL,
#endif
    NULL,
    NULL
};

#if UDP_TRANSFORMS_ENABLED
/*ci
 * \brief Basic sanity check of transformation rules
 *
 * \param[in] registry The registry to look for factories in
 * \param[in] rules    A sequence of rules to validate
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
UDP_InterfaceFactory_rules_are_valid(RT_Registry_T *registry,
                                     struct UDP_TransformRuleSeq *rules)
{
    RTI_INT32 index = 0;
    struct UDP_TransformRule *rule;
    RT_ComponentFactory_T *factory;

    for (index = 0; index < UDP_TransformRuleSeq_get_length(rules); ++index)
    {
        rule = UDP_TransformRuleSeq_get_reference(rules,index);

        factory = RT_Registry_lookup(
                    registry,
                    RT_ComponentFactoryId_get_name(&rule->transformation));

        if (factory == NULL)
        {
            UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                RT_ComponentFactoryId_get_name(&rule->transformation));

            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}
#endif

RTI_PRIVATE RTI_BOOL
UDP_InterfaceFactoryProperty_is_valid(struct UDP_InterfaceFactoryProperty *udp_property)
{
    if ((udp_property->max_message_size <= 0) ||
        (udp_property->max_message_size > udp_property->max_receive_buffer_size) ||
        (udp_property->max_message_size > udp_property->max_send_buffer_size) ||
        (udp_property->max_unicast_send_sockets == 0) ||
        (udp_property->max_unicast_send_sockets > UDP_UNICAST_SOCKETS_MAX) ||
        (udp_property->max_message_size > UDP_MAX_PACKET_SIZE))
    {
        UDP_LOG_INCONSISTENT_MAX_MESSAGE_SIZE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

#ifndef IP_TOS
    if (udp_property->transport_priority_mask != 0)
    {
        UDP_LOG_TOS_NOT_SUPPORTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
#endif /* IP_TOS */

    /* Validate transport priority mapping range when mask is set */
    if (udp_property->transport_priority_mask != 0)
    {
        /* Validate that mapping values are non-negative */
        if (udp_property->transport_priority_mapping_low < 0 ||
            udp_property->transport_priority_mapping_high < 0)
        {
            UDP_PRIORITY_MAP_NEGATIVE_MAPPING(OSAPI_LOGKIND_ERROR,
                udp_property->transport_priority_mapping_low,
                udp_property->transport_priority_mapping_high)
            return RTI_FALSE;
        }

        /* Validate that mapping_low <= mapping_high */
        if (udp_property->transport_priority_mapping_low >
            udp_property->transport_priority_mapping_high)
        {
            UDP_PRIORITY_MAP_INVALID_MAPPING(OSAPI_LOGKIND_ERROR,
                udp_property->transport_priority_mapping_low,
                udp_property->transport_priority_mapping_high)
            return RTI_FALSE;
        }
    }

#if UDP_TRANSFORMS_ENABLED
    if ((udp_property->transform_udp_mode != UDP_TRANSFORM_UDP_MODE_DISABLED) &&
        (udp_property->transform_udp_mode != UDP_TRANSFORM_UDP_MODE_ENABLED))
    {
        UDP_TRANSFORM_LOG_INVALID_UDP_MODE(OSAPI_LOGKIND_ERROR,
                                           udp_property->transform_udp_mode);
        return RTI_FALSE;
    }
    if ((udp_property->transform_locator_kind < NETIO_ADDRESS_KIND_USER) ||
        (udp_property->transform_locator_kind == NETIO_ADDRESS_KIND_SHMEM) ||
        (udp_property->transform_locator_kind == NETIO_ADDRESS_KIND_INTRA))
    {
        UDP_TRANSFORM_LOG_INVALID_TUDP_LOCATOR_KIND(OSAPI_LOGKIND_ERROR,
                                           udp_property->transform_locator_kind);
        return RTI_FALSE;
    }

    if (!UDP_InterfaceFactory_rules_are_valid(udp_property->_parent._parent.registry,
                                              &udp_property->source_rules))
    {
        return RTI_FALSE;
    }

    if (!UDP_InterfaceFactory_rules_are_valid(udp_property->_parent._parent.registry,
                                              &udp_property->destination_rules))
    {
        return RTI_FALSE;
    }

#endif
        return RTI_TRUE;
}

/*ci
 * \brief Initialize the UDP factory
 *
 * \details
 * UDP specific implementation of the RT ComponentFactory initialize
 * method. This method is called when the UDP interface factory is registered
 * with the RT.
 *
 * \param[in] property The properties registered with the UDP interface
 * \param[in] listener The listener registered with the UDP interface
 *
 * \return A fully initialized factory on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
UDP_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                struct RT_ComponentFactoryListener *listener)
{
    struct UDP_InterfaceFactory *factory = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property =
                                (struct UDP_InterfaceFactoryProperty*)property;

    /* Use the default property value */
    if (udp_property == NULL)
    {
        udp_property = &UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;
    }

    if (!UDP_InterfaceFactoryProperty_is_valid(udp_property))
    {
        return NULL;
    }

    OSAPI_Heap_allocate_struct(&factory,struct UDP_InterfaceFactory);
    if (factory == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    factory->_parent._factory = &factory->_parent;
    factory->_parent.intf = &UDP_InterfaceFactory_fv_Intf;
    factory->instance_counter = 0;
    factory->property = udp_property;

    if (listener)
    {
        factory->listener = *listener;
    }

#if defined(RTI_WIN32)
    NETIO_SocketModule_init();
#endif

#if defined(RTI_AUTOSAR)
    if (!NETIO_Autosar_initialize())
    {
        return NULL;
    }
#endif /* defined(RTI_AUTOSAR) */

    return (struct RT_ComponentFactory *)factory;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the UDP interface factory
 *
 * \details
 * Implementation of the RT ComponentFactory finalize method. This method
 * is called when the UDP interface factory is unregistered from the RT.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref UDP_InterfaceFactory_initialize
 */
RTI_PRIVATE void
UDP_InterfaceFactory_finalize(struct RT_ComponentFactory *factory,
                              struct RT_ComponentFactoryProperty **property,
                              struct RT_ComponentFactoryListener **listener)
{
    struct UDP_InterfaceFactory *udp_factory =
                                        (struct UDP_InterfaceFactory*)factory;
    UNUSED_ARG(listener);

    if ((property != NULL) &&
        (udp_factory->property != &UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT))
    {
        *property = (struct RT_ComponentFactoryProperty*)udp_factory->property;
    }

    OSAPI_Heap_free_struct(factory);
#if defined(RTI_AUTOSAR)
    NETIO_Autosar_finalize();
#endif /* defined(RTI_AUTOSAR) */
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
UDP_InterfaceFactory_get_interface(void)
{
    return &UDP_InterfaceFactory_fv_Intf;
}

#endif

#if UDP_TRANSFORMS_ENABLED

/*ci \brief Assert a rule to a transformation rule sequence
 *
 *  \param[in] seq The sequence to assert the rule to
 *  \param[in] ipv4_address The address for the rule
 *  \param[in] ipv4_netmask The netmask to apply before comparing
 *                          against the ipv4_address
 *  \param[in] name The name of the rule
 *  \param[in] user_data Opaque user-data associated with the rule
 *
 *  \return TRUE if the call succeeeded, FALSE otherwise.
 */
RTI_PRIVATE RTI_BOOL
UDP_InterfaceFactoryProperty_assert_rule(
                                 struct UDP_TransformRuleSeq *seq,
                                 RTI_UINT32 ipv4_address,
                                 RTI_UINT32 ipv4_netmask,
                                 const char *name,
                                 void *user_data)
{
    RTI_INT32 len,max;
    struct UDP_TransformRule *entry = NULL;

    OSAPI_PRECONDITION_ALWAYS((seq == NULL || name == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("seq",seq,RTI_TRUE);
                       OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);)

    len = UDP_TransformRuleSeq_get_length(seq);
    max = UDP_TransformRuleSeq_get_maximum(seq);

    if (len == max)
    {
        if (!UDP_TransformRuleSeq_set_maximum(seq,max+1))
        {
            return RTI_FALSE;
        }
    }

    if (!UDP_TransformRuleSeq_set_length(seq,len+1))
    {
        return RTI_FALSE;
    }

    entry = UDP_TransformRuleSeq_get_reference(seq,len);
    if (entry == NULL)
    {
        return RTI_FALSE;
    }

    NETIO_Address_set_ipv4(&entry->address,0,(RTI_UINT32)ipv4_address);

    entry->netmask.bits = 32;
    entry->netmask.mask[0] = ipv4_netmask;
    entry->netmask.mask[1] = 0;
    entry->netmask.mask[2] = 0;
    entry->netmask.mask[3] = 0;

    entry->user_data = user_data;

    if (!RT_ComponentFactoryId_set_name(&entry->transformation,name))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
UDP_TransformRules_assert_source_rule(
                                 struct UDP_TransformRuleSeq *src_rules,
                                 RTI_UINT32 ipv4_address,
                                 RTI_UINT32 ipv4_netmask,
                                 const char *name,
                                 void *user_data)
{
    return UDP_InterfaceFactoryProperty_assert_rule(
            src_rules,ipv4_address,ipv4_netmask,name,user_data);
}

RTI_BOOL
UDP_TransformRules_assert_destination_rule(
                                 struct UDP_TransformRuleSeq *dst_rules,
                                 RTI_UINT32 ipv4_address,
                                 RTI_UINT32 ipv4_netmask,
                                 const char *name,
                                 void *user_data)
{
    return UDP_InterfaceFactoryProperty_assert_rule(dst_rules,
                        ipv4_address,ipv4_netmask,name,user_data);
}

#endif /* UDP_TRANSFORMS_ENABLED */

/*-----------------------------------------------------------------------------
 * This code below is the integration between upstream and downstream
 *----------------------------------------------------------------------------*/
#include "netio_dgram/netio_dgram.h"

/* The use of this variable is thread-safe as long as instances are created
 * by a Participant.
 */
RTI_PRIVATE RTI_INT32 UDP_InterfaceFactory_fv_InstanceCounter = 0;

RTI_PRIVATE NETIO_Interface_T *
UDP_Interface_dgram_create_instance(NETIO_Interface_T *upstream,void *property)
{
#define req_addr_initialized_    0x1
#define resvd_addr_initialized_  0x2
#ifndef RTI_CERT
    RTI_UINT32 clean_flags  = 0;
#endif
    struct UDP_Interface *udp_intf = NULL;
    struct UDP_Interface *ret_intf = NULL;
    struct UDP_InterfaceFactory *udp_factory = NULL;
    struct NETIO_InterfaceProperty netio_property;

#ifdef RTI_WIN32
    if (!NETIO_SocketModule_init())
    {
        UDP_LOG_SOCKET_INIT(OSAPI_LOGKIND_ERROR,NETIO_Socket_get_error())
        return NULL;
    }
#endif

    OSAPI_Heap_allocate_struct(&udp_factory, struct UDP_InterfaceFactory);
    if (udp_factory == NULL)
    {
        UDP_LOG_ALLOC(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    NETIO_DGRAM_Interface_get_netio_property(upstream,&netio_property);

    udp_factory->_parent._id._value._high = 0xd6a3d6a3;
    udp_factory->_parent._id._value._low = 0xd6a30000;
    udp_factory->property = (struct UDP_InterfaceFactoryProperty*)property;
    udp_factory->instance_counter = UDP_InterfaceFactory_fv_InstanceCounter++;

    udp_intf = UDP_Interface_create(udp_factory,&netio_property,NULL);
    if (udp_intf == NULL)
    {
        goto done;
    }

    udp_intf->upstream_intf = upstream;

    /*ci \brief temporary sequences to call UDP_Interface_reserve_address with
     */
    if (!NETIO_AddressSeq_initialize(&udp_intf->req_addr))
    {
        goto done;
    }

#ifndef RTI_CERT
    clean_flags |= req_addr_initialized_;
#endif

    if (!NETIO_AddressSeq_initialize(&udp_intf->resvd_addr))
    {
        goto done;
    }

#ifndef RTI_CERT
    clean_flags |= resvd_addr_initialized_;
#endif

    if (!REDA_StringSeq_initialize(&udp_intf->if_name_seq))
    {
        goto done;
    }

    if (!NETIO_AddressSeq_set_maximum(&udp_intf->req_addr,1))
    {
        goto done;
    }

    if (!NETIO_AddressSeq_set_maximum(&udp_intf->resvd_addr,1))
    {
        goto done;
    }

    if (!REDA_StringSeq_set_maximum(&udp_intf->if_name_seq,
                            NETIO_DGRAM_INTERFACE_MAX_INTERFACES))
    {
        goto done;
    }

    ret_intf = udp_intf;

done:
#ifndef RTI_CERT
    if (ret_intf == NULL)
    {
        if (clean_flags & req_addr_initialized_)
        {
            NETIO_AddressSeq_finalize(&udp_intf->req_addr);
        }
        if (clean_flags & resvd_addr_initialized_)
        {
            NETIO_AddressSeq_finalize(&udp_intf->resvd_addr);
        }
        OSAPI_Heap_free_struct(udp_factory);
    }
#endif

    return (NETIO_Interface_T*)ret_intf;

#undef req_addr_initialized
#undef resvd_addr_initialized
}

#ifndef RTI_CERT
RTI_PRIVATE void
UDP_Interface_dgram_delete_instance(NETIO_Interface_T *netio_intf)
{
    struct UDP_Interface *udp_intf = (struct UDP_Interface *)netio_intf;
    struct UDP_InterfaceFactory *udp_factory;
    RTI_BOOL rtn;
    udp_factory = udp_intf->factory;

    NETIO_AddressSeq_finalize(&udp_intf->req_addr);
    NETIO_AddressSeq_finalize(&udp_intf->resvd_addr);
    rtn = REDA_StringSeq_finalize(&udp_intf->if_name_seq);
    UNUSED_ARG(rtn);
    UDP_Interface_delete(udp_intf);
    OSAPI_Heap_free_struct(udp_factory);
}
#endif

RTI_PRIVATE RTI_BOOL
UDP_Interface_dgram_get_interface_list(
                NETIO_Interface_T *dgram_intf,
                struct NETIO_DGRAM_InterfaceTableEntrySeq *if_table)
{
    struct UDP_Interface *self = (struct UDP_Interface*)dgram_intf;
    RTI_INT32 if_len, if_max,if_index;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;
    struct UDP_IfContext if_context = UDP_IFCONTEXT_INITIALIZER;
    struct UDP_NetworkIfInfo if_data = UDP_NetworkIfInfo_INITIALIZER;
    RTI_BOOL if_error = RTI_FALSE;

    if_max = NETIO_DGRAM_InterfaceTableEntrySeq_get_maximum(if_table);
    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(if_table);

    if_index = if_len;

    while (UDP_Interface_get_next_interface(self,
                         &if_context,&if_data,&if_error) && (if_index < if_max))
    {
        if (!UDP_Interface_is_valid_address(&if_data))
        {
            continue;
        }

        if (!UDP_Interface_interface_allowed(self,if_data.lastname))
        {
#ifdef RTI_WIN32
            if (!UDP_Interface_interface_allowed(self,if_data.fname))
            {
                OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
                OSAPI_TRACE_STRING("id",if_data.fname,RTI_TRUE)
                continue;
            }
#else
            OSAPI_TRACE_NET("ignoring interface:",RTI_FALSE)
            OSAPI_TRACE_STRING("id",if_data.lastname,RTI_TRUE)
            continue;
#endif
        }

        if (!NETIO_DGRAM_InterfaceTableEntrySeq_set_length(if_table,if_index+1))
        {
            UDP_Interface_get_next_interface_done(self,&if_context);
            return RTI_FALSE;
        }

        if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                            if_table,if_index);

        if (if_entry == NULL)
        {
            UDP_Interface_get_next_interface_done(self,&if_context);
            return RTI_FALSE;
        }

        NETIO_Address_init(&if_entry->address,NETIO_ADDRESS_KIND_UDPv4);

        if_entry->address.value.ipv4.address = if_data.intf_address;

#if !UDP_INTERFACE_BIND_ENABLED
        if_entry->flags |= NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG;
#else
        if (!self->factory->property->enable_interface_bind)
        {
            if_entry->flags |= NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG;
        }
#endif

        if_entry->ifname = REDA_String_dup(if_data.fname);

        /* Store the allocated name so the memory can be freed later */
        {
            RTI_INT32 if_name_len = REDA_StringSeq_get_length(&self->if_name_seq);
            REDA_StringSeq_set_length(&self->if_name_seq,if_name_len + 1);
            *REDA_StringSeq_get_reference(&self->if_name_seq,if_name_len) = (char*)if_entry->ifname;
        }

        if_entry->mtu = (RTI_UINT32)self->factory->property->max_message_size;
        if_entry->locator_kind = NETIO_ADDRESS_KIND_UDPv4;

        if (if_data.mc_enabled)
        {
            if_entry->multicast_group.netmask.bits = 4;
            if_entry->multicast_group.netmask.mask[0] = 0;
            if_entry->multicast_group.address_low.kind = NETIO_ADDRESS_KIND_UDPv4;
            if_entry->multicast_group.address_low.value.as_uint32.value[0] = NETIO_htonl(0xe0000000);
            if_entry->multicast_group.address_high.kind = NETIO_ADDRESS_KIND_UDPv4;
            if_entry->multicast_group.address_high.value.as_uint32.value[0] = NETIO_htonl(0xefffffff);
        }
        ++if_index;
    }

    UDP_Interface_get_next_interface_done(self,&if_context);

    if (if_error)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
UDP_Interface_dgram_bind_address(NETIO_Interface_T *netio_intf,
                                 struct NETIO_Address *addr)
{
    struct UDP_Interface *udp_intf = (struct UDP_Interface*)netio_intf;

    if (!NETIO_AddressSeq_set_length(&udp_intf->req_addr,1))
    {
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_length(&udp_intf->resvd_addr,0))
    {
        return RTI_FALSE;
    }

    /* The get_reference return is not NULL because set_length succeeded
     */
    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_AddressSeq_get_reference(&udp_intf->req_addr,0) = *addr;

    return UDP_Interface_reserve_address(netio_intf,
                                         &udp_intf->req_addr,
                                         &udp_intf->resvd_addr,
                                         NULL);
}

/*ci
 * \brief Resolve an IPv4 address string to NETIO_Address if possible
 *
 * \details
 * Implementation of the NETIO resolve_address function. This function
 * is called by resolvers to determine if this interface understands the
 * string address.
 *
 * \param[out] netio_intf     Interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
UDP_Interface_dgram_resolve_ipv4_address(NETIO_Interface_T *netio_intf,
                const struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                const char *address_string,
                struct NETIO_Address *address_value,
                RTI_BOOL *is_invalid)

{
    NETIO_Interface_T *src_intf = (NETIO_Interface_T*)netio_intf;

    if (if_entry->locator_kind != NETIO_ADDRESS_KIND_UDPv4)
    {
        *is_invalid = RTI_FALSE;
        return RTI_FALSE;
    }

    return UDP_Interface_resolve_ipv4_address(src_intf,
                                                address_string,
                                                address_value,
                                                is_invalid);
}

RTI_PRIVATE struct NETIO_DGRAM_InterfaceI UDP_Interface_fv_DgramIntf =
{
    UDP_Interface_dgram_create_instance,
#ifndef RTI_CERT
    UDP_Interface_dgram_delete_instance,
#endif
    UDP_Interface_dgram_get_interface_list,
    UDP_Interface_release_address,
    UDP_Interface_dgram_resolve_ipv4_address,
    UDP_Interface_send,
    UDP_Interface_get_route_table,
    UDP_Interface_dgram_bind_address
};

RTI_BOOL
UDP_Interface_register(RT_Registry_T *registry,
                       const char *name,
                       struct UDP_InterfaceFactoryProperty *property)
{
    /* Use the default property value */
    if (property == NULL)
    {
        property = &UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;
    }

    if (!UDP_InterfaceFactoryProperty_is_valid(property))
    {
        return RTI_FALSE;
    }

    if (!NETIO_DGRAM_InterfaceFactory_register(registry,name,
                                               &UDP_Interface_fv_DgramIntf,
                                               property))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_BOOL
UDP_Interface_leak_NETIO_DGRAM_InterfaceI(struct NETIO_DGRAM_InterfaceI **user_intfI)
{
    *user_intfI = &UDP_Interface_fv_DgramIntf;

    return RTI_TRUE;
}

#endif /* !UDP_EXCLUDE_BUILTIN */

/*ci @} */

