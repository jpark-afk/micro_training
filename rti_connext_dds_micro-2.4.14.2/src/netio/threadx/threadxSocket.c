/*
 * FILE: threadxSocket.c - NetX socket functionality
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.      Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 13dec2016,francisco      File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI thread routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"
#include "nx_api.h"

#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_log.h"

#include "netio/netio_config.h"
#include "netio/netio_common.h"

#include "../common/Thread.h"

#include "threadxSocket.h"

/* It is needed to bind the socket to a port before sending a packet.
 * But Micro assumes that this is not needed to sendto() functionality
 * binds the socket if needed to a free UDP port. To avoid a using
 * the same UDP port as a receive socket, set this constant to a number
 * greater than 7400 + (250 * Domain) + (2 * Participant) + 11
 */
#define SEND_BIND_PORT_START 50000

/* Defines the maximum number of UDP datagrams 
 * that can be queued for the socket. 
 */
#define SOCKET_QUEUE 50

#ifdef RTI_USE_NONBLOCKING_SOCKET
struct OSAPI_ThreadXSocket_rcvnotify_data
{
    TX_QUEUE *udp_queue;
    void     *user_data;
};
#endif /* #ifdef RTI_USE_NONBLOCKING_SOCKET */

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

INT
OSAPI_ThreadXSocket_socket(int protocol_family, int type, int protocol)
{
    UINT rc;
    NX_UDP_SOCKET *socket;
#ifdef RTI_USE_NONBLOCKING_SOCKET
    struct OSAPI_ThreadXSocket_rcvnotify_data *socket_user_data;
#endif /* #ifdef RTI_USE_NONBLOCKING_SOCKET */

    OSAPI_PRECONDITION((protocol_family != AF_INET) || (type != SOCK_DGRAM) ||
                       (protocol != IPPROTO_IP),
                       return -1,
                       OSAPI_Log_entry_add_int("protocol_family",protocol_family,RTI_FALSE);
                       OSAPI_Log_entry_add_int("type",type,RTI_FALSE);
                       OSAPI_Log_entry_add_int("protocol",protocol,RTI_TRUE);)

    if ((protocol_family != AF_INET) || (type != SOCK_DGRAM) ||
        (protocol != IPPROTO_IP))
    {
        return SOCKET_ERROR;
    }

#ifdef RTI_USE_NONBLOCKING_SOCKET
    OSAPI_Heap_allocate_struct(&socket_user_data, 
                               struct OSAPI_ThreadXSocket_rcvnotify_data);
    if (socket_user_data == NULL)
    {
        return SOCKET_ERROR;
    }
#endif /* #ifdef RTI_USE_NONBLOCKING_SOCKET */

    OSAPI_Heap_allocate_struct(&socket, NX_UDP_SOCKET);
    if (socket == NULL)
    {
#ifdef RTI_CERT
#ifdef RTI_USE_NONBLOCKING_SOCKET
        OSAPI_Heap_free_struct(socket_user_data);
#endif
#endif
        return SOCKET_ERROR;
    }

    rc = nx_udp_socket_create(&bsp_ip_bus,
                              socket,
                              "micro-socket",
                              NX_IP_NORMAL,
                              NX_DONT_FRAGMENT,
                              NX_IP_TIME_TO_LIVE,
                              SOCKET_QUEUE);

    if (rc != NX_SUCCESS)
    {
#ifndef RTI_CERT
#ifdef RTI_USE_NONBLOCKING_SOCKET
        OSAPI_Heap_free_struct(socket_user_data);
#endif
        OSAPI_Heap_free_struct(socket);
#endif
        return SOCKET_ERROR;
    }

#ifdef RTI_USE_NONBLOCKING_SOCKET
    socket->nx_udp_socket_reserved_ptr = socket_user_data;
#endif

    return (INT)socket;
}

INT
OSAPI_ThreadXSocket_close(int sock_id)
{
    INT ret_value = SOCKET_ERROR;

    OSAPI_PRECONDITION(sock_id <= 0,
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_TRUE);)

    if (sock_id > 0)
    {
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;
        UINT port;

        if (nx_udp_socket_port_get(socket, &port) == NX_SUCCESS)
        {
            (void)nx_udp_socket_unbind(socket);
        }

#ifndef RTI_CERT
#ifdef RTI_USE_NONBLOCKING_SOCKET
        OSAPI_Heap_free_struct(socket->nx_udp_socket_reserved_ptr);
        socket->nx_udp_socket_reserved_ptr = NULL;
#endif

        ret_value = (nx_udp_socket_delete(socket) == NX_SUCCESS) ? 0 : SOCKET_ERROR;

        OSAPI_Heap_free_struct(socket);
#else
        ret_value = 0;
#endif
    }

    return ret_value;
}

INT
OSAPI_ThreadXSocket_bind(int sock_id, const struct sockaddr *addr,
                         socklen_t addr_len)
{
    INT ret_value = SOCKET_ERROR;

    OSAPI_PRECONDITION((sock_id <= 0) || (addr == NULL) ||
                       (addr_len != sizeof(struct sockaddr_in)),
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("addr",addr,RTI_FALSE);
                       OSAPI_Log_entry_add_int("addr_len",addr_len,RTI_TRUE);)

    if ((sock_id > 0) && (addr != NULL) &&
        (addr_len == sizeof(struct sockaddr_in)))
    {
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;
        struct sockaddr_in *address = (struct sockaddr_in*)addr;

        ret_value = (nx_udp_socket_bind(socket, NETIO_ntohs(address->sin_port),
                                 NX_NO_WAIT) == NX_SUCCESS) ? 0 : SOCKET_ERROR;
    }

    return ret_value;
}

#ifdef RTI_USE_NONBLOCKING_SOCKET

RTI_PRIVATE VOID
OSAPI_ThreadXSocket_udp_receive_notify(NX_UDP_SOCKET *socket_ptr)
{
    struct OSAPI_ThreadXSocket_rcvnotify_data *socket_user_data;
    UINT rc;

    socket_user_data = (struct OSAPI_ThreadXSocket_rcvnotify_data *)
                                        socket_ptr->nx_udp_socket_reserved_ptr;

    if (socket_user_data != NULL)
    {
        if (socket_user_data->udp_queue != NULL)
        {
            rc = tx_queue_send(socket_user_data->udp_queue,
                               &socket_user_data->user_data,
                               TX_NO_WAIT);
#if OSAPI_ENABLE_LOG
            if (rc != NX_SUCCESS)
            {
                OSAPI_LOG_RECEIVE_NOTIFY(OSAPI_LOGKIND_ERROR, rc)
            }
#else
            IGNORE_RETVAL(rc);
#endif
        }
    }
}

INT
OSAPI_ThreadXSocket_recvnotify(int sock_id,
                               TX_QUEUE *udp_queue,
                               void *user_data)
{
    INT ret_value = SOCKET_ERROR;
    UINT rc;
    UINT rc2;

    OSAPI_PRECONDITION(sock_id <= 0,
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_TRUE);)

    if (sock_id > 0)
    {
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;
        struct OSAPI_ThreadXSocket_rcvnotify_data *socket_user_data =
                                   (struct OSAPI_ThreadXSocket_rcvnotify_data*)
                                            socket->nx_udp_socket_reserved_ptr;

        if (udp_queue == NULL)
        {
            rc = nx_udp_socket_receive_notify(socket, NULL);
            if (socket_user_data->udp_queue != NULL)
            {
                rc2 = tx_queue_flush(socket_user_data->udp_queue);
                IGNORE_RETVAL(rc2);
            }

            /* do not clear queue and user data until callback is canceled  
             * and queue is empty
             */
            socket_user_data->udp_queue = NULL;
            socket_user_data->user_data = NULL;
        }
        else
        {
            /* set queue and user data before callback is set */
            socket_user_data->udp_queue = udp_queue;
            socket_user_data->user_data = user_data;

            rc = nx_udp_socket_receive_notify(socket,
                                       OSAPI_ThreadXSocket_udp_receive_notify);

            if (rc == NX_SUCCESS)
            {
                /* because bind() might be done before callback notification
                 * is set we need to check if there is any packet already in
                 * the socket, and in that case post a notification in the
                 * queue
                 */
                if (OSAPI_ThreadXSocket_packets_queued(sock_id) > 0)
                {
                    rc = tx_queue_send(socket_user_data->udp_queue,
                                       &socket_user_data->user_data,
                                       TX_NO_WAIT);
                    IGNORE_RETVAL(rc);
                }
            }
        }

        if (rc == NX_SUCCESS)
        {
            ret_value = 0;
        }
    }

    return ret_value;
}

ULONG
OSAPI_ThreadXSocket_packets_queued(int sock_id)
{
    ULONG udp_packets_queued = 0;

    OSAPI_PRECONDITION(sock_id <= 0,
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_TRUE);)

    if (sock_id > 0)
    {
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;

        ULONG udp_packets_sent;
        ULONG udp_bytes_sent;
        ULONG udp_packets_received;
        ULONG udp_bytes_received;
        ULONG udp_receive_packets_dropped;
        ULONG udp_checksum_errors;

        if (nx_udp_socket_info_get(socket,
                                   &udp_packets_sent, 
                                   &udp_bytes_sent,
                                   &udp_packets_received,
                                   &udp_bytes_received,
                                   &udp_packets_queued,
                                   &udp_receive_packets_dropped,
                                   &udp_checksum_errors) != NX_SUCCESS)
        {
            /* in case of error return there are 0 packets in the queue */
            udp_packets_queued = 0;
        }
    }

    return udp_packets_queued;
}
#endif /* RTI_USE_NONBLOCKING_SOCKET */

INT
OSAPI_ThreadXSocket_recvfrom(int sock_id, void *rcv_buffer, int buffer_len,
                             int flags, struct sockaddr *local_addr,
                             socklen_t *addr_len)
{
    INT ret_value = SOCKET_ERROR;

    OSAPI_PRECONDITION((sock_id <= 0) || (rcv_buffer == NULL) || (local_addr == NULL),
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("rcv_buffer",rcv_buffer,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("localAddress",local_addr,RTI_TRUE);)

    UNUSED_ARG(addr_len);
    UNUSED_ARG(flags);

    if ((sock_id > 0) && (rcv_buffer != NULL) && (local_addr != NULL))
    {
        UINT rc;
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;
        NX_PACKET *packet_ptr = NULL;

#ifdef RTI_USE_NONBLOCKING_SOCKET
        /* in case there is a thread for many receiving socket this 
         * function is supposed to be called only when there is 
         * data available in the socket; so no need to wait */
        rc = nx_udp_socket_receive(socket, &packet_ptr, NX_NO_WAIT);
#else
        /* in case there is one thread for each receiving socket this 
         * function is supposed to block until data is received */
        rc = nx_udp_socket_receive(socket, &packet_ptr, NX_WAIT_FOREVER);
#endif /* RTI_USE_NONBLOCKING_SOCKET */
        if (rc == NX_SUCCESS)
        {
            ULONG length;

            rc = nx_packet_length_get(packet_ptr, &length);

            if ((rc == NX_SUCCESS) && (length <= ((UINT)buffer_len)))
            {
                rc = nx_packet_data_retrieve(packet_ptr, rcv_buffer, &length);

                if (rc == NX_SUCCESS)
                {
                    UINT port;
                    ULONG address;

                    rc = nx_udp_source_extract(packet_ptr,
                                               &address,
                                               &port);
                    if (rc == NX_SUCCESS)
                    {
                        ((struct sockaddr_in *)local_addr)->sin_port = port;
                        ((struct sockaddr_in *)local_addr)->sin_addr.s_addr = address;
                        ret_value = (INT)length;
                    }
                }
            }

            (void)nx_packet_release(packet_ptr);
        }
    }

    return ret_value;
}

/*    \brief Ensure that the socket is bound to a port and in case it is not
 *           find an unused port and binds the socket to this port. NextX
 *           needs that
 *
 *    \param [in] socket Pointer to socket.
 *
 *    \return 0 on success or -1 number on error.
 */
RTI_PRIVATE int
OSAPI_ThreadXSocket_ensure_bind(NX_UDP_SOCKET *socket)
{
    UINT port;
    INT ret_value = SOCKET_ERROR;
    UINT rc;

    rc = nx_udp_socket_port_get(socket, &port);

    if (rc == NX_NOT_BOUND)
    {
        if (nx_udp_free_port_find(&bsp_ip_bus,
                                  SEND_BIND_PORT_START, &port) == NX_SUCCESS)
        {
            if (nx_udp_socket_bind(socket, port, NX_NO_WAIT) == NX_SUCCESS)
            {
                ret_value = 0;
            }
        }
    }
    else if (rc == NX_SUCCESS)
    {
        ret_value = 0;
    }
    else
    {
        /* error, just return ret_value which is already initialized to -1 */
    }

    return ret_value;
}

INT
OSAPI_ThreadXSocket_sendto(int sock_id, void *msg, int msg_len, int flags,
                           const struct sockaddr *src_addr, socklen_t addr_len)
{
    INT ret_value = SOCKET_ERROR;

    OSAPI_PRECONDITION((sock_id <= 0) || (msg == NULL) || (src_addr == NULL) ||
                       (addr_len != (sizeof(struct sockaddr_in))),
                       return -1,
                       OSAPI_Log_entry_add_int("sock_id",sock_id,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("msg",msg,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                       OSAPI_Log_entry_add_int("addr_len",addr_len,RTI_TRUE);)

    UNUSED_ARG(flags);

    if ((sock_id > 0) && (msg != NULL) &&
        (src_addr != NULL) && (addr_len == sizeof(struct sockaddr_in)))
    {
        UINT                      rc;
        NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)sock_id;
        const struct sockaddr_in *address = (const struct sockaddr_in*)src_addr;
        NX_PACKET                *packet_ptr = NULL;

        /* bind socket to a port as it is necessary before sending */
        if (OSAPI_ThreadXSocket_ensure_bind(socket) != 0)
        {
            return SOCKET_ERROR;
        }

        rc = nx_packet_allocate(&bsp_pool_bus,
                                &packet_ptr,
                                NX_UDP_PACKET,
                                NX_NO_WAIT);
        if ((rc == NX_SUCCESS) && (packet_ptr != NULL))
        {
            rc = nx_packet_data_append(packet_ptr, msg,
                                       msg_len,
                                       &bsp_pool_bus,
                                       NX_NO_WAIT);

            if (rc == NX_SUCCESS)
            {
                rc = nx_udp_socket_send(socket, packet_ptr,
                                        NETIO_ntohl(address->sin_addr.s_addr),
                                        NETIO_ntohs(address->sin_port));

                if (rc == NX_SUCCESS)
                {
                    ret_value = msg_len;
                }
            }

            /* release the packet only if not successfully sent */
            if (rc != NX_SUCCESS)
            {
                (void)nx_packet_release(packet_ptr);
            }
        }
    }

    return ret_value;
}

INT
OSAPI_ThreadXSocket_setsockopt(int sock_fd, int level, int opt_name,
                               const void *opt_val, socklen_t opt_len)
{
    INT ret_value = SOCKET_ERROR;

    OSAPI_PRECONDITION((sock_fd <= 0) || (opt_val == NULL),
                       return -1,
                       OSAPI_Log_entry_add_int("sock_fd",sock_fd,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("opt_val",opt_val,RTI_TRUE);)

    if ((sock_fd <= 0) || (opt_val == NULL))
    {
        return -1;
    }

    switch (level)
    {
        case IPPROTO_IP:
            switch (opt_name)
            {
                case IP_MULTICAST_LOOP:
                    if (opt_len == sizeof(RTI_UINT8))
                    {
                        RTI_UINT8 *opt = (RTI_UINT8*)opt_val;
                        UINT rc;

                        if ((*opt) == 1)
                        {
                            rc = nx_igmp_loopback_enable(&bsp_ip_bus);
                        }
                        else
                        {
                            rc = nx_igmp_loopback_disable(&bsp_ip_bus);
                        }

                        ret_value = (rc == NX_SUCCESS) ? 0 : -1;
                    }
                    break;

#if NETIO_CONFIG_ENABLE_MULTICAST
                case IP_MULTICAST_IF:
                    ret_value = 0;
                    break;

                case IP_MULTICAST_TTL:
                    //should be passed when socket is created.
                    ret_value = 0;
                    break;

                case IP_ADD_MEMBERSHIP:
                    if (opt_len == sizeof(struct ip_mreq))
                    {
                        struct ip_mreq *imr = (struct ip_mreq*)opt_val;

                        ret_value =
                             (nx_igmp_multicast_join(&bsp_ip_bus,
                                     NETIO_ntohl(imr->imr_multiaddr.s_addr))
                                                       == NX_SUCCESS) ? 0 : -1;
                    }
                    break;

                case IP_DROP_MEMBERSHIP:
                    if (opt_len == sizeof(struct ip_mreq))
                    {
                        struct ip_mreq *imr = (struct ip_mreq*)opt_val;

                        ret_value =
                             (nx_igmp_multicast_leave(&bsp_ip_bus,
                                        NETIO_ntohl(imr->imr_multiaddr.s_addr))
                                                        == NX_SUCCESS) ? 0 : -1;
                    }
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

#endif
