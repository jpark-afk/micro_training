/*
 * FILE: ospsl_os_autosar.h - OS configuration file for AutoSAR like OSs
 *
 * (c) Copyright, Real-Time Innovations, 2019-2026
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
/* AutoSAR OS API Configuration
 */
#ifndef ospsl_os_autosar_h
#define ospsl_os_autosar_h

#include "osapi/osapi_config.h" /* TODO: Is this correct here?*/
#include "osapi/osapi_system.h"

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

#ifndef RTI_AUTOSAR
#define RTI_AUTOSAR
#endif /* RTI_AUTOSAR */

/* Do not include the standard thread support
 *    since it is not supported.
 */
#define OSAPI_NO_THREADS (1)
/*
 * Include Autosar port.
 */
#define OSAPI_INCLUDE_AUTOSAR 1

/*
 * Do not rely on std. libs realloc
 */
#define OSAPI_DONT_HAVE_REALLOC 1

/*
 * Include OSEK specific system properties
 */
#define OSAPI_HAVE_PORT_PROPERTY 1

/* Module ID used to report errors to Det. This is identical to the Micro
 * OMG vendor Id
 */
#define RTIME_DDS_MODULE_ID ((0x01U << 8) + 0x0aU)

/*
 * Instance ID used to report errors to Det
 */
#define RTIME_DDS_INSTANCE_ID 0

/*
 * AutoSAR uses only non-blocking sockets.
 */
#define RTI_USE_NONBLOCKING_SOCKET 1

/* Maximum number of timers that the Autosar system can create.
 */
#define OSAPISYSTEM_MAX_TIMERS          8

#if MENTOR_OLD_COMPAT
    /* Defect DR/ER #: VOL-VSTARMOD-16814. See SR #: 3386249991 */
    #define DOMAIN_FIELD TcpIp_Domain
#else
    #define DOMAIN_FIELD domain
#endif

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

/* Synchronization method enum.
 */
typedef enum
{
    /* Use Resources as synchronization method.
     */
    OSAPI_AUTOSAR_SYNCKIND_RESOURCES
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
    , /* Use spinlock as synchronization method.
       */
    OSAPI_AUTOSAR_SYNCKIND_SPINLOCK
#endif /* RTIME_AUTOSAR_SPINLOCK_ENABLED */
} OSAPI_Autosar_SyncKind_T;

/* Function used to create a socket. Users must provide this function in case
 * SocketOwner is used.
 */
typedef P2FUNC(Std_ReturnType, SOAD_CODE, NETIO_Autosar_TcpIp_get_socket)
    (TcpIp_DomainType domain,
     TcpIp_ProtocolType protocol,
     P2VAR(TcpIp_SocketIdType, AUTOMATIC, SOAD_APPL_DATA) socket);

/* Function used to send data. Users must provide this function only in case
 * SocketOwner is not used.
 */
typedef P2FUNC(Std_ReturnType, SOAD_CODE, NETIO_Autosar_TcpIp_send_dyn_data)
    (P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) data_ptr,
     uint16 length,
     P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC) remote_addr_ptr);

/* Callback function to be called on UDP packet reception.
 */
typedef P2FUNC(void, SOAD_CODE, NETIO_Autosar_TcpIp_dds_rxindication)
    (void);

/* System timer handler structure.
 */
struct OSAPI_SystemTimerHandler
{
    /*
     * Timer handler.
     */
    OSAPI_TimerTickHandlerFunction handler;

    /*
     * Param for timer handler.
     */
    P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) param;
};

/* AutoSAR System properties.
 */
struct OSAPI_PortProperty
{
    /****************************************
     ***** System related configuration *****
     ****************************************/

    uint32 timer_resolution_ms;

    /**************************************
     ***** Heap related configuration *****
     **************************************/

    /*  See AutoSAR Platform Notes for more information on
        how to configure the heap areas.
     */
    uint32 number_of_heap_areas;

    P2CONST(uint32, AUTOMATIC, SOAD_APPL_DATA) heap_area_size;

    P2CONST(P2VAR(char, AUTOMATIC, SOAD_APPL_DATA), AUTOMATIC, SOAD_APPL_DATA)
    heap_area;

    /*  Enable the heap mutex, defaults to TRUE.
     */
    boolean enable_thread_safe_heap;
     
    /***************************************
     ***** Mutex related configuration *****
     ***************************************/

    OSAPI_Autosar_SyncKind_T sync_type;

    /* Resource id used to synchronize concurrent access.
     */
    ResourceType mutex_resource_id;

    
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
    /* Spinlock id used in case sync_type is OSAPI_AUTOSAR_SYNCKIND_SPINLOCK.
     * Type SpinlockIdType could be used for this field but some AutoSAR
     * implementations don't include this type in case spinlock module is not
     * added. To avoid ifdef we use uint16.
     */
    uint16 spinlock_id;
#endif

    /*******************************************
     ***** Semaphore related configuration *****
     *******************************************/

    uint32 semaphore_max_count;
    
    EventMaskType first_give_event;

    EventMaskType first_timeout_event;

    AlarmType first_alarm;

    /****************************************
     ***** Socket related configuration *****
     ****************************************/

    /* TRUE if SocketOwner is configured in TcpIp module configuration.
     */
    boolean use_socket_owner;

    /* Maximum number of sockets for receiving UDP packets.
     */
    uint8 max_receive_sockets;

    /* Number of receive buffers.
     */
    uint8 number_of_rcv_buffers;

    /* Receive buffer size.
     */
    uint32 rcv_buffer_size;

    /* Pointer to function that creates a socket.
     */
    NETIO_Autosar_TcpIp_get_socket get_socket;

    /* Pointer to function that sends data.
     */
    NETIO_Autosar_TcpIp_send_dyn_data send_data;

    /*
     * Maximum configured IP address identifier representing the range top for
     * the configured IP addresses.
     */
    TcpIp_LocalAddrIdType max_local_addr_id;

    /*
     * Index of the send IP address identifier. 
     */
    TcpIp_LocalAddrIdType send_local_addr_id;

    /*
     * Set to TRUE to use a thread to receive data. If this field is set to
     * FALSE, the incoming UDP packets will be processed in the callback
     * notification function. If this field is set to TRUE, the callback
     * notification function will copy the UDP packet in an internal buffer and 
     * set an event for the UDP thread to process the packet.
     */
    boolean use_udp_thread;

    /* Callback function to be called on UDP packet reception.
     */
    NETIO_Autosar_TcpIp_dds_rxindication dds_rxindication;

#if OSAPI_ENABLE_LOG
    /*
     * Destination IP address used to send logs to. The format of the address
     * must be compatible with the format needed for function TcpIp_UdpTransmit(),
     * that is in host format instead of network format. I.e. 0xCA00A8C0 would be
     * IP address 192.168.0.202.
     * If 0, logs will not be send to any address. This is the default value.
     */
    uint32 log_dst_address;
    
    /*
     * Destination IP port used to send logs to. The format of the port
     * must be compatible with the format needed for function TcpIp_UdpTransmit().
     */
    uint16 log_dst_port;
#endif
};

/* Autosar system structure.
 */
struct OSAPI_SystemAutosar
{

    struct OSAPI_System _parent;

#ifndef RTI_CERT
    RTI_BOOL is_deleted;
#endif

    /* Array of active timer handlers.
     */
    struct OSAPI_SystemTimerHandler timer_handler[OSAPISYSTEM_MAX_TIMERS];

    /* Count of active timers.
     */
    RTI_INT32 timer_count;

    /* Mutex to lock access to Autosar system mutex.
     */
    P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex;

    /* Seconds component of the current time.
     */
    RTI_INT32 tick_sec;

    /* Nanosecond component of the current time.
     */
    RTI_UINT32 tick_nanosec;

    /* Mutex to lock access to the tick variables.
     */
    P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) tick_mutex;

    /* PSL port properties set by the application.
     */
    struct OSAPI_PortProperty psl_property;
};

/*
 * Global variables for port properties.
 *
 * This global variable is an exception to the coding stds about global
 * variables. It is allowed because the properties for a port may be
 * accessed across different modules.
 */
extern OSAPIDllVariable
struct OSAPI_PortProperty *OSAPI_System_gv_PortProperty;

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
    FALSE, /* enable_thread_safe_heap */ \
    OSAPI_AUTOSAR_SYNCKIND_RESOURCES, /* sync_type */ \
    0, /* mutex_resource_id */ \
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
    0, /* send_local_addr_id */ \
    FALSE, /* use_udp_thread */ \
    NULL_PTR, /* dds_rxindication */ \
    OSAPI_PortProperty_LOG_INITIALIZER \
}

#if OSAPI_ENABLE_MUTEX_TRACE
#define OSAPI_MUTEX_TRACE_INITIALIZER   \
    .stack = {0},                       \
    .stack_depth = 0
#else
#define OSAPI_MUTEX_TRACE_INITIALIZER
#endif

#define OSAPI_MUTEX_INITIALIZER         \
{                                       \
    ._base =                            \
    {                                   \
        .owner = {{0}},                 \
        OSAPI_MUTEX_TRACE_INITIALIZER   \
    },                                  \
    .hmutex = 0,                        \
    .depth = 0,                         \
    .owned = RTI_FALSE                  \
}

#define OSAPI_PLATFORM_AUTOSAR_HOSTNAME "Autosar-host"

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

FUNC(boolean, SOAD_CODE)
NETIO_Autosar_TcpIp_pdu_callout(
        VAR(PduIdType, AUTOMATIC) rx_pdu_id,
        P2CONST(PduInfoType, AUTOMATIC, SOAD_APPL_DATA) pdu_info_ptr);

FUNC(void, SOAD_CODE)
NETIO_Autosar_TcpIp_udp_rx_indication(
    TcpIp_SocketIdType socket,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) remote_addr_ptr,
    P2VAR(uint8, AUTOMATIC, SOAD_APPL_DATA) buf_ptr,
    uint16 length);

struct OSAPI_LogEntry;

/*
 * Repots a log message to Det
 *
 * This function reports a log message to Det
 *
 * param[in] param     - Optional trace parameter (not used)
 * param[in] log_entry - The log_entry to report
 *
 */
FUNC(void, SOAD_CODE)
OSAPI_AutosarLog_default_display(void *param, struct OSAPI_LogEntry *log_entry);

/*
 * Writes a log message to a socket. This destination IP and port are
 * configurable in the port configuration.
 *
 * Use this function as log write function to send logs over UDP.
 *
 * param[in] buffer  - Buffer with the log to write
 * param[in] length  - Length of buffer to write
 *
 */
FUNC(void, SOAD_CODE) 
OSAPI_AutosarLog_socket_write(const char *buffer, RTI_SIZE_T length);

/*
 * OSAPI timer thread implementation
 *
 * Implementation of Autosar OSAPI timer thread implementation.
 */
FUNC(void, SOAD_CODE)
OSAPI_SystemAutosar_timer_callback(void);

/*
 * NETIO udp receive thread implementation
 *
 * Implementation of Autosar NETIO UDP receive thread implementation.
 */
FUNC(void, SOAD_CODE)
NETIO_Autosar_udp_receive_callback(void);

#endif /* ospsl_os_autosar_h */
