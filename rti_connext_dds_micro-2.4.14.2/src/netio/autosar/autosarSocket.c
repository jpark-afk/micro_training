/*
 * FILE: autosarSocket.c - AutoSAR socket functionality
 *
 * Copyright 2019-2021 Real-Time Innovations, Inc.
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
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Conditionally add thread semaphores only when
 *   OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 14sep2021,tk MICRO-3152/PR.29564
 * - Add one thread semaphore in NETIO_Autosar_udp_receive_callback if a
 *   UDP task is used to process packets.
 * 15jun2021,tk MICRO-3053/PR.29151
 * - Fail in NETIO_AutosarSocket_sendto() if the message packet length
 *   exceed UDP_MAX_PACKET_LENGTH bytes since TcpIp_UdpTransmit() does not
 *   perform this check and sends truncated messages.
 * 1dec2020, fmt MICRO-2709/PR#28377
 *    - Remove check of TCPIP_E_PHYS_ADDR_MISS and TCPIP_E_ARP_CACHE_MISS
 *      after call to TcpIp_UdpTransmit().
 * 26nov2020,fmt MICRO-2699/PR#28346 Check NULL pointer after memory allocation
 * 26nov2020,fmt MICRO-2670/PR.28263
 *    - Set value of field 'sin_family' in function
 *      NETIO_Autosar_TcpIp_get_sockaddr_in()
 * 26nov2020,fmt MICRO-2667/PR#28272
 *    - Fix description of returned value in
 *      function NETIO_Autosar_TcpIp_udp_rx_indication_impl_direct()
 *    - Check return value of NETIO_Autosar_give_mutex() and return error if it
 *      fails.
 *    - Update type of variables used to store the result of functions
 *      NETIO_Autosar_take_mutex() and NETIO_Autosar_give_mutex().
 * 26oct2020,fmt MICRO-2630 Reset variable content in NETIO_Autosar_finalize().
 * 20oct2020,fmt MICRO-2669/PR#28261
 *    - Fix parameter documentation of function
 *      NETIO_Autosar_TcpIp_get_sockaddr_in(). Update parameters name.
 * 20oct2020,fmt  MICRO-2668/PR#28264
 *    - Fix documentation description of function
 *      NETIO_Autosar_TcpIp_udp_rx_indication_impl_add_to_buffer()
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 16oct2020,fmt MICRO-2609/PR.28209
 *    - Rename field number_of_sockets to max_receive_sockets
 * 6oct2020,fmt MICRO-2585/PR.28155
 *    - Move consistency check of properties from NETIO_Autosar_initialize() to
 *      OSAPI_SystemAutosar_initialize()
 * 2oct2020,fmt MICRO-2589 Handle return value of ActivateTask() correctly.
 * 9sept2020,fmt MICRO-2518/PR.28066 [EB-Tricore] autosarSocket.c - Empty functions
 * 9sept2020,fmt MICRO-2519/PR.28069 Remove "TODO" in comments
 * 25aug2020,fmt MICRO-2500/PR.28028
 *    - The AUTOSAR port does not check the return value for AUTOSAR API calls
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 13mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of socket interface
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_AUTOSAR

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif

#include "autosarSocket.h"

#endif

/*** SOURCE_BEGIN ***/

#if !UDP_EXCLUDE_BUILTIN

#if OSAPI_INCLUDE_AUTOSAR

/*i
 * \brief Mutex used to protect this module internal data.
 */
RTI_PRIVATE P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA)
OSAPI_AutosarSocket_fv_mutex = NULL_PTR;

/*i
 * \brief This is the SocketID that will be used when SocketOwner is not configured.
 * When SocketOwner is not configured this module uses only one "socket", although
 * several sockets are created all of them are the same socket.
 */
#define SOCKET_ID_NO_SOCKET_OWNER 0

/*i
 * \brief Last received index. Index used to add a packet to the received data
 * circular buffer.
 */
RTI_PRIVATE sint32 AutosarSocket_fv_last_rcv_index = -1;

/*i
 * \brief Last read index. Index used to read packet from the received data
 * circular buffer.
 */
RTI_PRIVATE sint32 AutosarSocket_fv_last_read_index = -1;

/*i
 * \brief Structure to store a received UDP packet
 */
struct AutosarSocket_ReceivedData
{
   /*i
    * \brief Buffer where content of UDP packet is copied. The size of this
    * buffer is configured in the Autosar port properties.
    */
    uint8  *rcv_socket_buffer;

    /*i
     * \brief Length of the UDP packet copied to buffer rcv_socket_buffer.
     */
    uint16 rcv_length;

    /*i
     * \brief Socket ID where the UDP packet was received.
     */
    uint16 rcv_socket;

    /*i
     * \brief Remote internet address of the UDP packet.
     */
    struct sockaddr_in remote_addr;

    /*i
     * \brief Indicates whether this struct holds a valid UDP packet.
     */
    boolean valid_data;
};

/*i
 * \brief Circular buffer with all the UDP received packets. The size of this
 * circular buffer is configured in the Autosar port properties.
 */
RTI_PRIVATE
struct AutosarSocket_ReceivedData *AutosarSocket_fv_received_data = NULL_PTR;

/*i
 * \brief Struct with the information needed by this module to call a callback
 * when a UDP packet is received.
 */
struct AutosarSocket_RcvNotifyData
{
    /*i
     * \brief Socket id where the UDP packet was received.
     */
    TcpIp_SocketIdType socket;

    /*i
     * \brief Pointer to user data.
     */
    P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) notify_data;

    /*i
     * \brief UDP receive callback function pointer.
     */
    NETIO_AutosarSocket_interface_receive udp_receive_cb;
};

/*i
 * \brief List of notification callbacks information for sockets created.
 */
RTI_PRIVATE
struct AutosarSocket_RcvNotifyData *AutosarSocket_fv_rcv_notify_data = NULL_PTR;

/*i
 * \brief Indicates whether this module is initialized or not.
 */
RTI_PRIVATE RTI_BOOL AutosarSocket_fv_initialized = RTI_FALSE;

/*i
 * \brief Indicates whether the UDP recive task is running or not.
 */
RTI_PRIVATE RTI_BOOL AutosarSocket_fv_udp_task_running = RTI_FALSE;

/*i
 * \brief Indicates whether the UDP recive task should stop.
 */
RTI_PRIVATE RTI_BOOL AutosarSocket_fv_udp_task_stop = RTI_FALSE;

/******************************************************************************
 *************************** Private functions ********************************
 ******************************************************************************/

/*i
 * \brief Takes the mutext used to synchronize this module internal data.
 *
 * \return TRUE if no error, otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
NETIO_Autosar_take_mutex(void)
{
    if (OSAPI_AutosarSocket_fv_mutex == NULL_PTR)
    {
        return FALSE;
    }
    
    return (OSAPI_Mutex_take(OSAPI_AutosarSocket_fv_mutex) == RTI_TRUE) ? TRUE : FALSE;
}

/*i
 * \brief Gives the mutext used to synchronize this module internal data.
 *
 * \return TRUE if no error, otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
NETIO_Autosar_give_mutex(void)
{
    if (OSAPI_AutosarSocket_fv_mutex == NULL_PTR)
    {
        return FALSE;
    }
    
    return (OSAPI_Mutex_give(OSAPI_AutosarSocket_fv_mutex) == RTI_TRUE) ? TRUE : FALSE;
}

/*i
 * \brief Transforms an Internet address (IP address and port) in Autosar format
 * to Posix-like format.
 *
 * \param[out] dst Internet address in Posix format
 * \param[in]  src Internet address in Autosar format
 */
RTI_PRIVATE FUNC(void, SOAD_CODE)
NETIO_Autosar_TcpIp_get_sockaddr_in(
    P2VAR(struct sockaddr_in, AUTOMATIC, SOAD_APPL_DATA) dst,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) src)
{
    if ((src != NULL_PTR) && 
#if MENTOR_OLD_COMPAT
        /* Defect DR/ER #: VOL-VSTARMOD-16814. See SR #: 3386249991 */
        (src->TcpIp_Domain == TCPIP_AF_INET))
#else
        (src->domain == TCPIP_AF_INET))
#endif
    {
        P2CONST(TcpIp_SockAddrInetType, AUTOMATIC, SOAD_APPL_DATA)
            remote_addr_ptr_aux;

        remote_addr_ptr_aux = 
            (P2CONST(TcpIp_SockAddrInetType, AUTOMATIC, SOAD_APPL_DATA))
                src;
        dst->sin_port = remote_addr_ptr_aux->port;
        dst->sin_addr.s_addr = remote_addr_ptr_aux->addr[0];
        dst->sin_family = AF_INET;
    }
    else
    {
        dst->sin_port = 0;
        dst->sin_addr.s_addr = 0;
        dst->sin_family = 0;
    }
}

/*i
 * \brief Returns the next UDP packet in the circular buffer of received packets
 * and the data associated to the socket where this packet was received.
 *
 * \param[out] notify_data Pointer to the socket notification data
 * \param[out] received_data Pointer to the UDP packet data struct
 *
 * \return SOCKET_ERROR if circular buffer is empty.
 * 0 if valid UDP packet is returned.
 */
RTI_PRIVATE FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_get_received_data(
    P2VAR(struct AutosarSocket_RcvNotifyData*, AUTOMATIC, SOAD_APPL_DATA) notify_data,
    P2VAR(struct AutosarSocket_ReceivedData*, AUTOMATIC, SOAD_APPL_DATA) received_data)
{
    sint32 next_read_index;

    *notify_data = NULL_PTR;
    *received_data = NULL_PTR;

    /* Check if there is data in the next slot.
     * But don't move to next slot unless there is data, this function
     * is called in a loop just to check if there is more data.
     */
    next_read_index =
        (AutosarSocket_fv_last_read_index + 1) %
            OSAPI_System_gv_PortProperty->number_of_rcv_buffers;

    /* Verify that there is valid data (it might happen that the task was awaken
     * to be finalized)
     */
    if (AutosarSocket_fv_received_data[next_read_index].valid_data
        == FALSE)
    {
        return SOCKET_ERROR;
    }

    /* Move to the next slot */
    AutosarSocket_fv_last_read_index = next_read_index;

    /* Find the socket receive notify data for the last received UDP packet */
    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        sint32 i;

        for (i = 0; i < OSAPI_System_gv_PortProperty->max_receive_sockets; i++)
        {
            if (AutosarSocket_fv_rcv_notify_data[i].socket ==
                AutosarSocket_fv_received_data[AutosarSocket_fv_last_read_index].rcv_socket)
            {
                *notify_data = &AutosarSocket_fv_rcv_notify_data[i];
                break;
            }
        }
    }
    else
    {
        /* When SocketOwner is not configured, only one socket is used */
        *notify_data = &AutosarSocket_fv_rcv_notify_data[0];
    }

    if ((*notify_data) != NULL_PTR)
    {
        *received_data =
            &AutosarSocket_fv_received_data[AutosarSocket_fv_last_read_index];
        return 0;
    }

    /* remove valid data as it doesn't have a socket */
    AutosarSocket_fv_received_data[AutosarSocket_fv_last_read_index].valid_data = FALSE;

    return SOCKET_ERROR;
}

/*i
 * \brief Returns the localAddrId configured with the provided address
 *
 * \param[in] addr Pointer to the address to be found.
 * \param[out] local_addr_id Pointer to the localAddrId to be retrieved
 *
 * \return E_OK if localAddrId is found and returned.
 * E_NOT_OK otherwise.
 */
RTI_PRIVATE FUNC(Std_ReturnType, SOAD_CODE)
    NETIO_AutosarSocket_find_local_addr_id(
        P2CONST(struct sockaddr, AUTOMATIC, SOAD_APPL_DATA) addr,
        TcpIp_LocalAddrIdType *local_addr_id)
{
    TcpIp_LocalAddrIdType idx = 0;
    Std_ReturnType ret;
    /* Allocate enough space for any TcpIp_SockAddrType variant */
    uint32 ipVxAddrBuffer[2][8] = {{0}};
    uint8 netmask;

    if (local_addr_id == NULL_PTR || addr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    for (idx = 0; idx <= OSAPI_System_gv_PortProperty->max_local_addr_id; idx++)
    {
        switch (addr->sa_family)
        {
        case AF_INET:
            /* Compliance with SWS_TCPIP_00205 and SWS_TCPIP_00206 requirements */
#if MENTOR_OLD_COMPAT
            ((TcpIp_SockAddrType *)&ipVxAddrBuffer[0])->TcpIp_Domain = TCPIP_AF_INET;
            ((TcpIp_SockAddrType *)&ipVxAddrBuffer[1])->TcpIp_Domain = TCPIP_AF_INET;
#else
            ((TcpIp_SockAddrType *)&ipVxAddrBuffer[0])->domain = TCPIP_AF_INET;
            ((TcpIp_SockAddrType *)&ipVxAddrBuffer[1])->domain = TCPIP_AF_INET;
#endif

            ret = TcpIp_GetIpAddr(
                idx,
                (TcpIp_SockAddrType *) &ipVxAddrBuffer[0],
                (uint8 *) &netmask,
                (TcpIp_SockAddrType *) &ipVxAddrBuffer[1]);
    
            if ((ret == E_OK)
#if MENTOR_OLD_COMPAT
                && (((TcpIp_SockAddrType *)&ipVxAddrBuffer[0])->TcpIp_Domain == TCPIP_AF_INET)
#else
                && (((TcpIp_SockAddrType *)&ipVxAddrBuffer[0])->domain == TCPIP_AF_INET)
#endif
                && ((((TcpIp_SockAddrInetType *)&ipVxAddrBuffer[0])->addr[0] == ((struct sockaddr_in *)addr)->sin_addr.s_addr) 
                    || ((INADDR_ANY == ((struct sockaddr_in *)addr)->sin_addr.s_addr) 
                        && ((((TcpIp_SockAddrInetType *)&ipVxAddrBuffer[0])->addr[0] & 0xF) != 0xE))))
            {
                *local_addr_id = idx;
                return E_OK;
            }
            break;
        default:
            /* IPv6 currently not supported */
            return E_NOT_OK;
        }
    }

    return E_NOT_OK;
}


/******************************************************************************
 *************************** Public functions *********************************
 ******************************************************************************/

FUNC(void, SOAD_CODE)
NETIO_Autosar_udp_receive_callback(void)
{
    boolean stat;
    P2VAR(struct AutosarSocket_RcvNotifyData, AUTOMATIC, SOAD_APPL_DATA) notify_data;
    P2VAR(struct AutosarSocket_ReceivedData, AUTOMATIC, SOAD_APPL_DATA) received_data;
    StatusType status;

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    {
        RTI_BOOL bretval;
        /* If the thread semaphore allocation fails it is not possible to suspend
         * a thread. This does not impact functional correctness.
         */
        bretval = OSAPI_System_add_thread_semaphore();
        IGNORE_RETVAL(bretval);
    }
#endif

    AutosarSocket_fv_udp_task_running = RTI_TRUE;

    while(!AutosarSocket_fv_udp_task_stop)
    {
        /* There is nothing we can do in case ClearEvent() or WaitEvent() fail.
         * User is responsible for correctly configuring the events that the task
         * can wait for
         */
        status = ClearEvent(OSAPI_System_gv_PortProperty->udp_packet_received_event);
#if OSAPI_ENABLE_LOG
        if (status != E_OK)
        {
            OSAPI_LOG_AUTOSAR_CLEAREVENT_CALL(OSAPI_LOGKIND_ERROR, status);
        }
#else
        IGNORE_RETVAL(status);
#endif

        status = WaitEvent(OSAPI_System_gv_PortProperty->udp_packet_received_event);
#if OSAPI_ENABLE_LOG
        if (status != E_OK)
        {
            OSAPI_LOG_AUTOSAR_WAITEVENT_CALL(OSAPI_LOGKIND_ERROR, status);
        }
#else
        IGNORE_RETVAL(status);
#endif

        stat = NETIO_Autosar_take_mutex();
        if (stat)
        {
            while (NETIO_AutosarSocket_get_received_data(&notify_data,
                                                         &received_data) == 0)
            {
                /* Note that a NULL pointer might be added to the queue in case
                 * this thread needs to be awaken 
                 */
                if ((notify_data != NULL_PTR) && (received_data != NULL_PTR) &&
                    (!AutosarSocket_fv_udp_task_stop))
                {
                    stat = NETIO_Autosar_give_mutex();
                    if (!stat)
                    {
                        break;
                    }
                           
                    notify_data->udp_receive_cb(
                        notify_data->notify_data, 
                        (P2VAR(char, AUTOMATIC, SOAD_APPL_DATA))
                            received_data->rcv_socket_buffer,
                        (RTI_INT32)received_data->rcv_length,
                        &received_data->remote_addr);

                    stat = NETIO_Autosar_take_mutex();
                    if (!stat)
                    {
                        break;
                    }
                    
                    /* Packet can be overwritten now, only after it has been
                     * processed by upper layer
                     */
                    received_data->valid_data = FALSE;
                }
            }
        
            if (stat)
            {
                /* not much that we can do at this point if release resource fails */
                stat = NETIO_Autosar_give_mutex();
                IGNORE_RETVAL(stat);
            }
        }
    }

    AutosarSocket_fv_udp_task_running = RTI_FALSE;
    AutosarSocket_fv_udp_task_stop = RTI_FALSE;

#if OSAPI_THREAD_SEMAPHORE_ENABLED
#ifndef RTI_CERT
    OSAPI_System_delete_thread_semaphore();
#endif
#endif

    OSAPI_TRACE_NET("stopped received thread:",RTI_FALSE)
}

DeclareTask(NETIO_Autosar_udp_receive_task);

/*ci
 * \brief UDP receive task implementation. This task can be used to decouple the
 * UDP packet receive callback from the packet processing. In case this task
 * is used, every received packet is copied to a circular buffer and an event
 * is set. This task waits in this event, and when set, it reads the packets
 * from the circular buffer and process it.
 */
TASK(NETIO_Autosar_udp_receive_task)
{

    NETIO_Autosar_udp_receive_callback();

    (void) TerminateTask();
}

FUNC(RTI_BOOL, SOAD_CODE)
NETIO_Autosar_initialize(void)
{
    uint32 i;
    StatusType status;

    if (AutosarSocket_fv_initialized)
    {
#ifndef RTI_CERT
        /* If already initialized we only need to ensure that task is running.
         * This might happen becuase the DomainParticipantFactory registers a
         * default UDP transport. Normally the user unregisters that UDP
         * transport and registers a new one.
         * Note that because the Autosar port is not able to release memory,
         * once this module is initialized, it can not be finalized. 
         * NETIO_Autosar_finalize() only tries to stop the UDP task, but it
         * can not release all buffers allocated here, so once 
         * AutosarSocket_fv_initialized is set to RTI_TRUE, it is never
         * set to RTI_FALSE.
         */
        if (OSAPI_System_gv_PortProperty->use_udp_thread)
        {
            AutosarSocket_fv_udp_task_stop = RTI_FALSE;
            if (!AutosarSocket_fv_udp_task_running)
            {
                status = ActivateTask(
                             OSAPI_System_gv_PortProperty->udp_receive_task_id);
                /* ActivateTask returns the following status codes that are
                 * treated as a successful activation:
                 *
                 * E_OK - If the task was not previously activated and was
                 *        activated successfully. 
                 * E_OS_LIMIT - This status code is returned if a task was
                 *              already activated. E_OS_LIMIT is a warning
                 *              according to OSEK spec and means the task
                 *              was activated more than once. This is benign.
                 *
                 * Any other combination of status codes are treated as an
                 * error here as it means the task was not successfully
                 * activated.
                 */
                if ((status != E_OK) && (status != E_OS_LIMIT))
                {
                    OSAPI_LOG_AUTOSAR_ACTIVATETASK_CALL(OSAPI_LOGKIND_ERROR,
                                                        status)
                    return FALSE;
                }

                /* Some Osek configurations do not allow Schedule() so we do
                 * not check the return value. This function is called so
                 * the receive thread is scheduled for first time faster
                 */
                status = Schedule();
#if OSAPI_ENABLE_LOG
                if (status != E_OK)
                {
                    OSAPI_LOG_AUTOSAR_SCHEDULE_CALL(OSAPI_LOGKIND_ERROR,
                                                    status)
                }
#else
                IGNORE_RETVAL(status);
#endif
            }
        }
#endif /* RTI_CERT */

        return RTI_TRUE;
    }

    OSAPI_Heap_allocate_buffer(
        (char **)&AutosarSocket_fv_rcv_notify_data,
        OSAPI_System_gv_PortProperty->max_receive_sockets *
            sizeof(struct AutosarSocket_RcvNotifyData),
        OSAPI_ALIGNMENT_DEFAULT);
    if (AutosarSocket_fv_rcv_notify_data == NULL)
    {
        goto fail;
    }
    for (i = 0; i < OSAPI_System_gv_PortProperty->max_receive_sockets; i++)
    {
        AutosarSocket_fv_rcv_notify_data[i].socket =
            (TcpIp_SocketIdType)SOCKET_ERROR;
        AutosarSocket_fv_rcv_notify_data[i].notify_data = NULL_PTR;
        AutosarSocket_fv_rcv_notify_data[i].udp_receive_cb = NULL_PTR;
    }

    OSAPI_AutosarSocket_fv_mutex = OSAPI_Mutex_new();
    if (OSAPI_AutosarSocket_fv_mutex == NULL_PTR)
    {
        goto fail;
    }

    if (OSAPI_System_gv_PortProperty->use_udp_thread)
    {
        OSAPI_Heap_allocate_buffer(
            (char **)&AutosarSocket_fv_received_data,
            OSAPI_System_gv_PortProperty->number_of_rcv_buffers *
                sizeof(struct AutosarSocket_ReceivedData),
            OSAPI_ALIGNMENT_DEFAULT);
        if (AutosarSocket_fv_received_data == NULL_PTR)
        {
            goto fail;
        }

        for (i = 0; i < OSAPI_System_gv_PortProperty->number_of_rcv_buffers; i++)
        {
            OSAPI_Heap_allocate_buffer(
                (char **)&AutosarSocket_fv_received_data[i].rcv_socket_buffer,
                OSAPI_System_gv_PortProperty->rcv_buffer_size,
                OSAPI_ALIGNMENT_DEFAULT);
            if (AutosarSocket_fv_received_data[i].rcv_socket_buffer == NULL_PTR)
            {
                goto fail;
            }
            AutosarSocket_fv_received_data[i].rcv_length = 0;
            AutosarSocket_fv_received_data[i].rcv_socket = 0;
            AutosarSocket_fv_received_data[i].valid_data = FALSE;
        }

        /* ensure that UDP receive task is started */
        AutosarSocket_fv_udp_task_stop = RTI_FALSE;
        if (!AutosarSocket_fv_udp_task_running)
        {
            status = ActivateTask(
                         OSAPI_System_gv_PortProperty->udp_receive_task_id);
            /* ActivateTask returns the following status codes that are
             * treated as a successful activation:
             *
             * E_OK - If the task was not previously activated and was
             *        activated successfully. 
             * E_OS_LIMIT - This status code is returned if a task was
             *              already activated. E_OS_LIMIT is a warning
             *              according to OSEK spec and means the task
             *              was activated more than once. This is benign.
             *
             * Any other combination of status codes are treated as an
             * error here as it means the task was not successfully
             * activated.
             *
             * OS might be configured to auto activate the task or
             * the user might manually activate it. This is why we check
             * for return code E_OS_LIMIT.
             */
            if ((status != E_OK) && (status != E_OS_LIMIT))
            {
                OSAPI_LOG_AUTOSAR_ACTIVATETASK_CALL(OSAPI_LOGKIND_ERROR,
                                                    status)
                return FALSE;
            }

            /* Some Osek configuration do not allow Schedule() so we do
             * not check the return value. This function is called so
             * the receive thread is scheduled for first time faster
             */
            status = Schedule();
#if OSAPI_ENABLE_LOG
            if (status != E_OK)
            {
                OSAPI_LOG_AUTOSAR_SCHEDULE_CALL(OSAPI_LOGKIND_ERROR, status)
            }
#else
            IGNORE_RETVAL(status);
#endif
        }
    }

    AutosarSocket_fv_initialized = RTI_TRUE;

    return RTI_TRUE;

fail:
    return RTI_FALSE;
}

#ifndef RTI_CERT
FUNC(void, SOAD_CODE)
NETIO_Autosar_finalize(void)
{
    uint32 i;

    /* Stop task if needed */
    if (OSAPI_System_gv_PortProperty->use_udp_thread)
    {
        StatusType status;

        /* ensure that UDP receive task is finalized */
        AutosarSocket_fv_udp_task_stop = RTI_TRUE;

        status = SetEvent(
                     OSAPI_System_gv_PortProperty->udp_receive_task_id,
                     OSAPI_System_gv_PortProperty->udp_packet_received_event);
#if OSAPI_ENABLE_LOG
        if (status != E_OK)
        {
            OSAPI_LOG_AUTOSAR_SETEVENT_CALL(OSAPI_LOGKIND_ERROR, status);
        }
#else
        IGNORE_RETVAL(status);
#endif

        status = Schedule();
#if OSAPI_ENABLE_LOG
        if (status != E_OK)
        {
            OSAPI_LOG_AUTOSAR_SCHEDULE_CALL(OSAPI_LOGKIND_ERROR, status)
        }
#else
        IGNORE_RETVAL(status);
#endif
    }

    /* Important: 
     * Once this module has been initialized it cannot be stopped and buffers
     * cannot be deleted because the OSAPI heap implementation for OSEK 
     * does not support freeing memory. Thus, the socket structure is only
     * reset.
     * 
     * For example, a UDP transport is automatically registered with a default 
     * configuration by the DomainParticipantFactory. When the UDP transport is 
     * unregisted this module is finalized and the socket structures are reset so 
     * they can be used if the transport is registered again. 
     */
    for (i = 0; i < OSAPI_System_gv_PortProperty->max_receive_sockets; i++)
    {
        AutosarSocket_fv_rcv_notify_data[i].socket =
            (TcpIp_SocketIdType)SOCKET_ERROR;
        AutosarSocket_fv_rcv_notify_data[i].notify_data = NULL_PTR;
        AutosarSocket_fv_rcv_notify_data[i].udp_receive_cb = NULL_PTR;
    }
}
#endif /* !RTI_CERT */

/*******************************************************************************
 ********************* Tcp/IP SocketOwner functions ****************************
 ******************************************************************************/

/*i
 * \brief Adds a new UDP packet to the circular list of UDP packets and signals
 * the UDP receive task. If the circular buffer is full the new packet is
 * dropped.
 *
 * This function is called when a UDP receive task is configured.
 *
 * \param[in] socket Socket id where the UDP packet is received.
 * \param[in] remote_addr_ptr Pointer to remote Internet address.
 * \param[in] buf_ptr Pointer to buffer with packet content.
 * \param[in] length Size of UDP packet.
 *
 * \return TRUE if packet was correctly added to the circular buffer. FALSE in
 * case of error.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
NETIO_Autosar_TcpIp_udp_rx_indication_impl_add_to_buffer(
    TcpIp_SocketIdType socket,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) remote_addr_ptr,
    P2VAR(uint8, AUTOMATIC, SOAD_APPL_DATA) buf_ptr,
    uint16 length)
{
    boolean stat;
    sint32 next_last_rcv_index;
    StatusType status;

    if (length > OSAPI_System_gv_PortProperty->rcv_buffer_size)
    {
        OSAPI_TRACE_NET("received message larger then max message:",RTI_FALSE)
        OSAPI_TRACE_INT32("size",length,RTI_TRUE)
        return FALSE;
    }

    stat = NETIO_Autosar_take_mutex();
    if (!stat)
    {
        return FALSE;
    }

    next_last_rcv_index = (AutosarSocket_fv_last_rcv_index + 1) % 
                            OSAPI_System_gv_PortProperty->number_of_rcv_buffers;
                            
    /* ensure an already received packet is not dropped */
    if (AutosarSocket_fv_received_data[next_last_rcv_index].valid_data == FALSE)
    {
        AutosarSocket_fv_last_rcv_index = next_last_rcv_index;
        OSAPI_Memory_copy(
            (P2VAR(void, AUTOMATIC, SOAD_APPL_DATA))
            AutosarSocket_fv_received_data[next_last_rcv_index].rcv_socket_buffer,
            (P2CONST(void, AUTOMATIC, SOAD_APPL_DATA))buf_ptr,
            (RTI_SIZE_T)length);
        AutosarSocket_fv_received_data[next_last_rcv_index].rcv_socket =
            socket;
        AutosarSocket_fv_received_data[next_last_rcv_index].rcv_length =
            length;
        AutosarSocket_fv_received_data[next_last_rcv_index].valid_data = TRUE;
        NETIO_Autosar_TcpIp_get_sockaddr_in(
            &AutosarSocket_fv_received_data[next_last_rcv_index].remote_addr,
            remote_addr_ptr);
    }

    stat = NETIO_Autosar_give_mutex();
    if (!stat)
    {
        return FALSE;
    }
        
    status = SetEvent(OSAPI_System_gv_PortProperty->udp_receive_task_id,
                      OSAPI_System_gv_PortProperty->udp_packet_received_event);
    if (status != E_OK)
    {
        OSAPI_LOG_AUTOSAR_SETEVENT_CALL(OSAPI_LOGKIND_ERROR, status);
        return FALSE;
    }

    return TRUE;
}

/*i
 * \brief Processes a received UDP packet.
 *
 * This function is used when a UDP received task is not configured.
 *
 * \param[in] socket Socket id where the UDP packet is received.
 * \param[in] remote_addr_ptr Pointer to remote Internet address.
 * \param[in] buf_ptr Pointer to buffer with packet content.
 * \param[in] length Size of UDP packet.
 *
 * \return TRUE if received UDP packet is correctly processed. FALSE in case
 * of error.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
NETIO_Autosar_TcpIp_udp_rx_indication_impl_direct(
    TcpIp_SocketIdType socket,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) remote_addr_ptr,
    P2VAR(uint8, AUTOMATIC, SOAD_APPL_DATA) buf_ptr,
    uint16 length)
{
    boolean stat;
    P2VAR(struct AutosarSocket_RcvNotifyData, AUTOMATIC, SOAD_APPL_DATA)
        notify_data = NULL_PTR;
    struct sockaddr_in ip_src;
    uint32 i;

    stat = NETIO_Autosar_take_mutex();
    if (!stat)
    {
        return FALSE;
    }

    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        /* find the socket receive notify data */
        for (i = 0; i < OSAPI_System_gv_PortProperty->max_receive_sockets; i++)
        {
            if (AutosarSocket_fv_rcv_notify_data[i].socket == socket)
            {
                notify_data = &AutosarSocket_fv_rcv_notify_data[i];           
                break;
            }
        }
    }
    else
    {
        notify_data = &AutosarSocket_fv_rcv_notify_data[0];
    }

    stat = NETIO_Autosar_give_mutex();
    if (!stat)
    {
        return FALSE;
    }

    if (notify_data != NULL_PTR)
    {
        NETIO_Autosar_TcpIp_get_sockaddr_in(&ip_src, remote_addr_ptr);
        
        notify_data->udp_receive_cb(notify_data->notify_data, 
                                    (P2VAR(char, AUTOMATIC, SOAD_APPL_DATA))buf_ptr,
                                    (RTI_INT32)length,
                                    &ip_src);
    }

    return TRUE;
}

FUNC(void, SOAD_CODE)
NETIO_Autosar_TcpIp_udp_rx_indication(
    TcpIp_SocketIdType socket,
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, SOAD_APPL_DATA) remote_addr_ptr,
    P2VAR(uint8, AUTOMATIC, SOAD_APPL_DATA) buf_ptr,
    uint16 length)
{
    boolean ret_val;

    if (!OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        /* This function can only be configured as indication callback
         * when SocketOwner is configured
         */
        return;
    }

    if (OSAPI_System_gv_PortProperty->use_udp_thread)
    {
        ret_val = NETIO_Autosar_TcpIp_udp_rx_indication_impl_add_to_buffer(
                      socket, remote_addr_ptr, buf_ptr, length);
    }
    else
    {
        ret_val = NETIO_Autosar_TcpIp_udp_rx_indication_impl_direct(
                      socket, remote_addr_ptr, buf_ptr, length);
    }
    IGNORE_RETVAL(ret_val);
}

FUNC(boolean, SOAD_CODE)
NETIO_Autosar_TcpIp_pdu_callout(
        VAR(PduIdType, AUTOMATIC) rx_pdu_id,
        P2CONST(PduInfoType, AUTOMATIC, SOAD_APPL_DATA) pdu_info_ptr)
{
    boolean ret_val;

    UNUSED_ARG(rx_pdu_id);

    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        /* This function can only be configured as indication callback
         * when SocketOwner is NOT configured
         */
        return FALSE;
    }

    if (OSAPI_System_gv_PortProperty->use_udp_thread)
    {
        ret_val = NETIO_Autosar_TcpIp_udp_rx_indication_impl_add_to_buffer(
                      SOCKET_ID_NO_SOCKET_OWNER,
                      NULL,
                      pdu_info_ptr->SduDataPtr,
                      pdu_info_ptr->SduLength);
    }
    else
    {
        ret_val = NETIO_Autosar_TcpIp_udp_rx_indication_impl_direct(
                      SOCKET_ID_NO_SOCKET_OWNER,
                      NULL,
                      pdu_info_ptr->SduDataPtr,
                      pdu_info_ptr->SduLength);
    }

    return ret_val;
}

/*******************************************************************************
 *********************** Socket Wrapper Functions ******************************
 ******************************************************************************/

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_socket(int protocol_family, int type, int protocol)
{

    OSAPI_PRECONDITION((protocol_family != AF_INET) || (type != SOCK_DGRAM) ||
                       (protocol != IPPROTO_IP),
                       return -1,
                       OSAPI_Log_entry_add_int("protocol_family",protocol_family,RTI_FALSE);
                       OSAPI_Log_entry_add_int("type",type,RTI_FALSE);
                       OSAPI_Log_entry_add_int("protocol",protocol,RTI_TRUE);)

    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        TcpIp_SocketIdType socket;
        Std_ReturnType ret_val = SOCKET_ERROR;

        if (OSAPI_System_gv_PortProperty->get_socket != NULL_PTR)
        {
            ret_val = OSAPI_System_gv_PortProperty->get_socket(TCPIP_AF_INET,
                                                               TCPIP_IPPROTO_UDP,
                                                               &socket);
        }

        return (ret_val == E_OK) ? socket : SOCKET_ERROR;
    }
    else
    {
        /* If socket owner is not used, sockets can't be created, but this needs
         * to be transparent to the upper UDP layer. So, socket = 0 is always
         * returned and this socket wrapper will manage internally how to send
         * packets using the function pointer in the port configuration.
         * to be transparent to the upper UDP layer.
         */
        return SOCKET_ID_NO_SOCKET_OWNER;
    }
}

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_close(int socket)
{
    OSAPI_PRECONDITION(socket < 0,
                       return -1,
                       OSAPI_Log_entry_add_int("socket",socket,RTI_TRUE);)

    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        Std_ReturnType ret_val = E_NOT_OK;

        ret_val = TcpIp_Close((TcpIp_SocketIdType)socket, TRUE);

        return (ret_val == E_OK) ? E_OK : SOCKET_ERROR;
    }
    else
    {
        return E_OK;
    }
}

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_bind(
    int socket,
    P2CONST(struct sockaddr, AUTOMATIC, SOAD_APPL_DATA)addr, 
    socklen_t addr_len)
{
    OSAPI_PRECONDITION((socket < 0) || (addr == NULL_PTR) ||
                       (addr_len != sizeof(struct sockaddr_in)),
                       return -1,
                       OSAPI_Log_entry_add_int("socket",socket,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("addr",addr,RTI_FALSE);
                       OSAPI_Log_entry_add_int("addr_len",addr_len,RTI_TRUE);)

    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        Std_ReturnType ret_val = E_NOT_OK;
        P2CONST(struct sockaddr_in, AUTOMATIC, SOAD_APPL_DATA) address;
        TcpIp_LocalAddrIdType local_addr_id;
        uint16 port_ptr;

        address = (P2CONST(struct sockaddr_in, AUTOMATIC, SOAD_APPL_DATA))addr;

        port_ptr = NETIO_ntohs(address->sin_port);

        ret_val = NETIO_AutosarSocket_find_local_addr_id(addr, &local_addr_id);

        if (ret_val == E_NOT_OK) {
            return SOCKET_ERROR;
        }

        ret_val = TcpIp_Bind((TcpIp_SocketIdType)socket,
                              local_addr_id,
                              &port_ptr);

        return (ret_val == E_OK) ? E_OK : SOCKET_ERROR;
    }
    else
    {
        return E_OK;
    }
}

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_recvnotify(
    int socket,
    NETIO_AutosarSocket_interface_receive udp_queue,
    P2VAR(void, AUTOMATIC, SOAD_APPL_DATA)user_data)
{
    boolean stat;
    int ret_value = SOCKET_ERROR;
    sint32 i;

    OSAPI_PRECONDITION(socket < 0,
                       return -1,
                       OSAPI_Log_entry_add_int("socket",socket,RTI_TRUE);)

    /* find a free socket receive notify data */
    stat = NETIO_Autosar_take_mutex();
    if (!stat)
    {
        return SOCKET_ERROR;
    }

    for (i = 0; i < OSAPI_System_gv_PortProperty->max_receive_sockets; i++)
    {
        /* find the socket passed as parameter or an empty index */
        if ((AutosarSocket_fv_rcv_notify_data[i].socket ==
                (TcpIp_SocketIdType)SOCKET_ERROR) ||
            (AutosarSocket_fv_rcv_notify_data[i].socket ==
                (TcpIp_SocketIdType)socket))
        {
            /* delete socket id if there is no user data */
            if (user_data == NULL_PTR)
            {
                AutosarSocket_fv_rcv_notify_data[i].socket =
                                               (TcpIp_SocketIdType)SOCKET_ERROR;
                AutosarSocket_fv_rcv_notify_data[i].notify_data = NULL_PTR;                   
                AutosarSocket_fv_rcv_notify_data[i].udp_receive_cb = NULL_PTR;                   
            }
            else
            {
                AutosarSocket_fv_rcv_notify_data[i].socket =
                                                    (TcpIp_SocketIdType)socket;
                AutosarSocket_fv_rcv_notify_data[i].notify_data = user_data;
                AutosarSocket_fv_rcv_notify_data[i].udp_receive_cb = udp_queue;                   
            }

            ret_value = 0;
            break;
        }
    }

    stat = NETIO_Autosar_give_mutex();
    if (!stat)
    {
        return SOCKET_ERROR;
    }

    return ret_value;
}

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_sendto(
    int socket,
    P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) msg,
    int msg_len, int flags,
    P2CONST(struct sockaddr, AUTOMATIC, SOAD_APPL_DATA) dest_addr,
    socklen_t addr_len)
{
    Std_ReturnType ret_val = SOCKET_ERROR;
    P2CONST(struct sockaddr_in, AUTOMATIC, SOAD_APPL_DATA) address;
    TcpIp_SockAddrInetType remote_addr;
    P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC) remote_addr_ptr;

    OSAPI_PRECONDITION((socket < 0) || (msg == NULL_PTR) ||
                       (dest_addr == NULL_PTR) ||
                       (addr_len != (sizeof(struct sockaddr_in))),
                       return -1,
                       OSAPI_Log_entry_add_int("socket",socket,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("msg",msg,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dest_addr",dest_addr,RTI_FALSE);
                       OSAPI_Log_entry_add_int("addr_len",addr_len,RTI_TRUE);)

    UNUSED_ARG(flags);

    /* TcpIp_UdpTransmit does check that if msg_len exceeds the maximum
     * allowed UDP payload and will send truncated messages.
     */
    if (msg_len > UDP_MAX_PACKET_LENGTH)
    {
        return SOCKET_ERROR;
    }

    address = (P2CONST(struct sockaddr_in, AUTOMATIC, SOAD_APPL_DATA))dest_addr;
    remote_addr_ptr =
            (P2CONST(TcpIp_SockAddrType, AUTOMATIC, AUTOMATIC))
            (P2CONST(void, AUTOMATIC, AUTOMATIC))
            &remote_addr;
        
    remote_addr.addr[0] = address->sin_addr.s_addr;
#if OS_VENDOR_ID == 30
    /* Vector vendor id = 0x001E.
     * There seems to be a problem in the Vector stack and it expect the
     * destination port to be in network byte order; note that sendto()
     * specs the port in network byte order, this is why for other
     * AUTOSAR implementations we need to change it back to host byte order.
     */
    remote_addr.port = address->sin_port;
#else
    remote_addr.port = NETIO_ntohs(address->sin_port);
#endif
#if MENTOR_OLD_COMPAT
    /* Defect DR/ER #: VOL-VSTARMOD-16814. See SR #: 3386249991 */
    remote_addr.TcpIp_Domain = TCPIP_AF_INET;
#else
    remote_addr.domain = TCPIP_AF_INET;
#endif


    if (OSAPI_System_gv_PortProperty->use_socket_owner)
    {
        ret_val = TcpIp_UdpTransmit((TcpIp_SocketIdType)socket, 
                                    (P2CONST(uint8, AUTOMATIC, SOAD_APPL_DATA))msg, 
                                    remote_addr_ptr, 
                                    (uint16)msg_len);
    }
    else
    {
        if (OSAPI_System_gv_PortProperty->send_data != NULL)
        {
            ret_val = OSAPI_System_gv_PortProperty->send_data(
                          msg, (uint16)msg_len, remote_addr_ptr);
        }
    }

    return (ret_val == E_OK) ? msg_len : SOCKET_ERROR;
}

FUNC(int, SOAD_CODE)
NETIO_AutosarSocket_setsockopt(
    int socket, int level, int opt_name,
    P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) opt_val,
    socklen_t opt_len)
{
    int ret_value = -1;

    UNUSED_ARG(socket);
    UNUSED_ARG(opt_val);
    UNUSED_ARG(opt_len);

    /* Return 0 only for known options, -1 for unknown options */
    switch (level)
    {
        case IPPROTO_IP:
            switch (opt_name)
            {
#if NETIO_CONFIG_ENABLE_MULTICAST
                case IP_MULTICAST_LOOP:
                    ret_value = 0;
                    break;

                case IP_MULTICAST_IF:
                    ret_value = 0;
                    break;

                case IP_MULTICAST_TTL:
                    ret_value = 0;
                    break;

                case IP_ADD_MEMBERSHIP:
                    ret_value = 0;
                    break;

                case IP_DROP_MEMBERSHIP:
                    ret_value = 0;
                    break;
#endif

                default:
                    break;
            }
            break;

        case SOL_SOCKET:
            switch (opt_name)
            {
                case SO_SNDBUF:
                    ret_value = 0;
                    break;

                case SO_RCVBUF:
                    ret_value = 0;
                    break;

                case SO_REUSEPORT:
                    ret_value = 0;
                    break;

                case SO_REUSEADDR:
                    ret_value = 0;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return ret_value;
}

#endif /* OSAPI_INCLUDE_AUTOSAR */

#endif /* !UDP_EXCLUDE_BUILTIN */
