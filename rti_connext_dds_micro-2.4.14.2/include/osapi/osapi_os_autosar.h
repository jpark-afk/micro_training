/*
 * FILE: osapi_os_autosar.h - OS configuration file for AutoSAR like OSs
 *
 * (c) Copyright, Real-Time Innovations, 2019-2021
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
 * 21jul2021,tk MICRO-3045 Fixed filenames and dates in file header comments
 * 15apr2021,tk MICRO-2979/PR.27540
 * - Exclude thread support for OSEK
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Made all public documentation external
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 16oct2020,fmt MICRO-2609/PR.28209
 *     - Rename field number_of_sockets to max_receive_sockets
 * 9sep2020,fmt MICRO-2518/PR.28066 [EB-Tricore] autosarSocket.c - Empty functions
 * 13mar2019,fmt Refactored from osapi_config.h
 *
 */
/*ce \file
 *   \brief AutoSAR OS API Configuration
 */
#ifndef osapi_os_autosar_h
#define osapi_os_autosar_h

#ifndef osapi_cc_h
#include "osapi_cc.h"
#endif

#include <errno.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdio.h>

/* C++ compilation fails when including Os.h for Elektrobit build.
 * Mentor doesn't have files Os_user.h or Os_api.h.
 * This is not happening in case GNU extensions are enabled in the compiler. Option
 * --g++.
 */
#ifndef OS_KERNEL_TYPE
#include <Os.h>
#else
#include <Os.h>
#include <Os_user.h>
#include <Os_api.h>
#include <Os_tool.h>
#endif

#include <Platform_Types.h>
#include <Compiler.h>
#include <TcpIp.h>

/*e \dref_OSAPIAUTOSARGroupDocs
 */

#ifndef RTI_AUTOSAR
#define RTI_AUTOSAR
#endif /* RTI_AUTOSAR */

/*ci \brief Do not include the standard thread support
 *    since it is not supported.
 */
#define OSAPI_NO_THREADS (1)
/*ci
 * \brief Include Autosar port.
 */
#define OSAPI_INCLUDE_AUTOSAR 1

/*ci
 * \brief Do not rely on std. libs realloc
 */
#define OSAPI_DONT_HAVE_REALLOC 1

/*ci
 * \brief Include OSEK specific system properties
 */
#define OSAPI_HAVE_PORT_PROPERTY 1

/* Module ID used to report errors to Det. This is identical to the Micro
 * OMG vendor Id
 */
#define RTIME_DDS_MODULE_ID ((0x01U << 8) + 0x0aU)

/*ci
 * \brief Instance ID used to report errors to Det
 */
#define RTIME_DDS_INSTANCE_ID 0

/*ci
 * \brief AutoSAR uses only non-blocking sockets.
 */
#define RTI_USE_NONBLOCKING_SOCKET 1

#ifdef RTI_CERT
#define RTIME_AUTOSAR_ENABLE_SPINLOCK 0
#else
#ifndef RTIME_AUTOSAR_ENABLE_SPINLOCK
#if defined(GetSpinlock) && defined(ReleaseSpinlock)
#define RTIME_AUTOSAR_ENABLE_SPINLOCK (1)
#else
#define RTIME_AUTOSAR_ENABLE_SPINLOCK (0)
#endif
#endif /* !RTIME_AUTOSAR_ENABLE_SPINLOCK */
#endif /* RTI_CERT */

#if RTIME_AUTOSAR_ENABLE_SPINLOCK
#define RTIME_AUTOSAR_SPINLOCK_ENABLED (1)
#endif

#if RTIME_AUTOSAR_SPINLOCK_ENABLED
#define RTI_OS_DSYNC() OS_DSYNC()
#else
#define RTI_OS_DSYNC()
#endif

/*e \dref_OSAPI_Autosar_SyncKind_T
 */
typedef enum
{
    /*e \dref_OSAPI_Autosar_SyncKind_T_OSAPI_AUTOSAR_SYNCKIND_RESOURCES
     */
    OSAPI_AUTOSAR_SYNCKIND_RESOURCES
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
    /*e \dref_OSAPI_Autosar_SyncKind_T_OSAPI_AUTOSAR_SYNCKIND_SPINLOCK
     */
    , OSAPI_AUTOSAR_SYNCKIND_SPINLOCK
#endif /* RTIME_AUTOSAR_SPINLOCK_ENABLED */
} OSAPI_Autosar_SyncKind_T;

/*e \dref_NETIO_Autosar_TcpIp_get_socket
 */
typedef P2FUNC(Std_ReturnType, SOAD_CODE, NETIO_Autosar_TcpIp_get_socket)
    (TcpIp_DomainType domain,
     TcpIp_ProtocolType protocol,
     P2VAR(TcpIp_SocketIdType, AUTOMATIC, SOAD_APPL_DATA) socket);

/*e \dref_NETIO_Autosar_TcpIp_send_dyn_data
 */
typedef P2FUNC(Std_ReturnType, SOAD_CODE, NETIO_Autosar_TcpIp_send_dyn_data)
    (P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) data_ptr,
     uint16 length,
     P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC) remote_addr_ptr);

/*e \dref_OSAPI_PortProperty
 */
struct OSAPI_PortProperty
{
    /****************************************
     ***** System related configuration *****
     ****************************************/

    /*e \dref_OSAPI_PortProperty_timer_resolution_ms
     */
    uint32 timer_resolution_ms;

    /**************************************
     ***** Heap related configuration *****
     **************************************/

    /*e \dref_OSAPI_PortProperty_number_of_heap_areas
     */
    uint32 number_of_heap_areas;

    /*e \dref_OSAPI_PortProperty_heap_area_size
     */
    P2CONST(uint32, AUTOMATIC, SOAD_APPL_DATA) heap_area_size;

    /*e \dref_OSAPI_PortProperty_heap_area
     */
    P2CONST(P2VAR(char, AUTOMATIC, SOAD_APPL_DATA), AUTOMATIC, SOAD_APPL_DATA)
    heap_area;
     
    /***************************************
     ***** Mutex related configuration *****
     ***************************************/

    /*e \dref_OSAPI_PortProperty_sync_type
     */
    OSAPI_Autosar_SyncKind_T sync_type;

    /*e \dref_OSAPI_PortProperty_first_resource_id
     */
    ResourceType first_resource_id;

    /*e \dref_OSAPI_PortProperty_last_resource_id
     */
    ResourceType last_resource_id;
    
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
    /*e \dref_OSAPI_PortProperty_spinlock_id
     */
    uint16 spinlock_id;
#endif

    /*******************************************
     ***** Semaphore related configuration *****
     *******************************************/

    /*e \dref_OSAPI_PortProperty_semaphore_max_count
     */
    uint32 semaphore_max_count;
    
    /*e \dref_OSAPI_PortProperty_first_give_event
     */
    EventMaskType first_give_event;

    /*e \dref_OSAPI_PortProperty_first_timeout_event
     */
    EventMaskType first_timeout_event;

    /*e \dref_OSAPI_PortProperty_first_alarm
     */
    AlarmType first_alarm;

    /****************************************
     ***** Socket related configuration *****
     ****************************************/

    /*e \dref_OSAPI_PortProperty_use_socket_owner
     */
    boolean use_socket_owner;

    /*e \dref_OSAPI_PortProperty_max_receive_sockets
     */
    uint8 max_receive_sockets;

    /*e \dref_OSAPI_PortProperty_number_of_rcv_buffers
     */
    uint8 number_of_rcv_buffers;

    /*e \dref_OSAPI_PortProperty_rcv_buffer_size
     */
    uint32 rcv_buffer_size;

    /*e \dref_OSAPI_PortProperty_get_socket
     */
    NETIO_Autosar_TcpIp_get_socket get_socket;

    /*e \dref_OSAPI_PortProperty_send_data
     */
    NETIO_Autosar_TcpIp_send_dyn_data send_data;

    /*e \dref_OSAPI_PortProperty_max_local_addr_id
     *
     * Maximum configured IP address identifier representing the range top for
     * the configured IP addresses [0..max_local_addr_id]
     */
    TcpIp_LocalAddrIdType max_local_addr_id;

    /*e \dref_OSAPI_PortProperty_use_udp_thread
     *
     * Set to TRUE to use a thread to receive data. If this field is set to
     * FALSE, the incoming UDP packets will be processed in the callback
     * notification function. If this field is set to TRUE, the callback
     * notification function will copy the UDP packet in an internal buffer and 
     * set an event for the UDP thread to process the packet.
     */
    boolean use_udp_thread;

    /*e \dref_OSAPI_PortProperty_udp_receive_task_id
     */
    TaskType udp_receive_task_id;

    /*e \dref_OSAPI_PortProperty_udp_packet_received_event
     */
    EventMaskType udp_packet_received_event;

#if OSAPI_ENABLE_LOG
    /*e \ingroup OSAPI_AutosarClass
     *
     * Destination IP address used to send logs to. The format of the address
     * must be compatible with the format needed for function TcpIp_UdpTransmit(),
     * that is in host format instead of network format. I.e. 0xCA00A8C0 would be
     * IP address 192.168.0.202.
     * If 0, logs will not be send to any address. This is the default value.
     */
    uint32 log_dst_address;
    
    /*e \ingroup OSAPI_AutosarClass
     *
     * Destination IP port used to send logs to. The format of the port
     * must be compatible with the format needed for function TcpIp_UdpTransmit().
     */
    uint16 log_dst_port;
#endif
};

#if RTIME_AUTOSAR_SPINLOCK_ENABLED
#define OSAPI_PortProperty_SPINLOCK_INITIALIZER \
    0,  /* spinlock_id */
#else
#define OSAPI_PortProperty_SPINLOCK_INITIALIZER
#endif

#if OSAPI_ENABLE_LOG
#define OSAPI_PortProperty_LOG_INITIALIZER \
    , 0  /* log_dst_address */ \
    , 0  /* log_dst_port */
#else
#define OSAPI_PortProperty_LOG_INITIALIZER
#endif /* OSAPI_ENABLE_LOG */

#define OSAPI_PortProperty_INITIALIZER \
{\
    0, /* timer_resolution_ms */ \
    0, /* number_of_heap_areas */ \
    NULL, /* heap_area_size */ \
    NULL, /* heap_area */ \
    OSAPI_AUTOSAR_SYNCKIND_RESOURCES, /* sync_type */ \
    0, /* first_resource_id */ \
    0, /* last_resource_id */ \
    OSAPI_PortProperty_SPINLOCK_INITIALIZER \
    0, /* semaphore_max_count */ \
    0, /* first_give_event */ \
    0, /* first_timeout_event */ \
    0, /* first_alarm */ \
    FALSE, /* use_socket_owner */ \
    0, /* max_receive_sockets */ \
    0, /* number_of_rcv_buffers */ \
    0, /* rcv_buffer_size */ \
    NULL_PTR, /* get_socket */ \
    NULL_PTR, /* send_data */ \
    0, /* max_local_addr_id */ \
    FALSE, /* use_udp_thread */ \
    0, /* udp_receive_task_id */ \
    0 /* udp_packet_received_event */ \
    OSAPI_PortProperty_LOG_INITIALIZER \
}

#define OSAPI_PLATFORM_AUTOSAR_HOSTNAME "Autosar-host"

/* Define the native thread-handle that represents the thread it self */
#define OSAPI_ThreadHandle RTI_UINT32

/* Define the native ThreadId */
#define OSAPI_ThreadId RTI_UINT32

/* Define the native ProcessId */
#define OSAPI_ProcessId RTI_UINT32

#define HAVE_SOCKET_API

#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) \
    do \
    { \
        UNUSED_ARG(len_); \
        /* printf("%s", buf_);*/ \
    }\
    while (0);
#endif

/* Autosar TcpIp callback function declaration */

/*e \dref_NETIO_Autosar_TcpIp_pdu_callout
 */
FUNC(boolean, SOAD_CODE)
NETIO_Autosar_TcpIp_pdu_callout(
        VAR(PduIdType, AUTOMATIC) rx_pdu_id,
        P2CONST(PduInfoType, AUTOMATIC, SOAD_APPL_DATA) pdu_info_ptr);

/*e \dref_NETIO_Autosar_TcpIp_udp_rx_indication
 */
FUNC(void, SOAD_CODE)
NETIO_Autosar_TcpIp_udp_rx_indication(
    TcpIp_SocketIdType socket,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) remote_addr_ptr,
    P2VAR(uint8, AUTOMATIC, SOAD_APPL_DATA) buf_ptr,
    uint16 length);

struct OSAPI_LogEntry;

/*ci
 * \brief Repots a log message to Det
 *
 * \details
 * This function reports a log message to Det
 *
 * \param[in] param     - Optional trace parameter (not used)
 * \param[in] log_entry - The log_entry to report
 *
 */
FUNC(void, SOAD_CODE)
OSAPI_AutosarLog_default_display(void *param, struct OSAPI_LogEntry *log_entry);

/*ci
 * \brief Writes a log message to a socket. This destination IP and port are
 * configurable in the port configuration.
 *
 * \details
 * Use this function as log write function to send logs over UDP.
 *
 * \param[in] buffer  - Buffer with the log to write
 * \param[in] length  - Length of buffer to write
 *
 */
FUNC(void, SOAD_CODE) 
OSAPI_AutosarLog_socket_write(const char *buffer, RTI_SIZE_T length);

/*ci
 * \brief OSAPI timer thread implementation
 *
 * \details
 * Implementation of Autosar OSAPI timer thread implementation.
 */
FUNC(void, SOAD_CODE)
OSAPI_SystemAutosar_timer_callback(void);

/*ci
 * \brief NETIO udp receive thread implementation
 *
 * \details
 * Implementation of Autosar NETIO UDP receive thread implementation.
 */
FUNC(void, SOAD_CODE)
NETIO_Autosar_udp_receive_callback(void);

#endif /* osapi_os_autosar_h */
