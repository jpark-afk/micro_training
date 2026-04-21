/*
 * FILE: threadxSocket.c - NetX socket functionality
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission. Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 16jan2017,francisco    File created
 *
 */
/*ce
 * \file
 */

#ifndef threadxSocket_h
#define threadxSocket_h

#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"
#include "nx_api.h"

#if defined(NETIO_UDP_ENABLE_DNSQUERY) && NETIO_UDP_ENABLE_DNSQUERY
#include "nx_application_layer/nx_dns.h"
#endif

#define SOCKET_ERROR ((INT)-1)

/*  \brief Wrapper to call nx_udp_socket_create() using a posix interface.
 *         Creates a socket for communication.
 *
 *  \param [in] protocol_family The only supported value is AF_INET
 *  \param [in] type The only supported value is SOCK_DGRAM
 *  \param [in] protocol The only supported value is IPPROTO_IP
 *
 *  \return SOCKET_ERROR if error, otherwise a valid socket id
 */
extern INT
OSAPI_ThreadXSocket_socket(int protocol_family, int type, int protocol);

/*  \brief Wrapper to call nx_udp_socket_delete() using a posix interface.
 *         Closes a previously created socket. Unbind the socket in case
 *         it is bound.
 *
 *  \param [in] socket_id Socket to close.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_close(int sock_id);

/*  \brief Wrapper to call function nx_udp_socket_bind() using a posix 
 *         interface.
 *         Binds a socket to a port.
 *
 *  \param [in] socket_id Socket to bind.
 *  \param [in] addr Port to bind socket to.
 *  \param [in] addr_len Size of parameter addr.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_bind(int sock_id, const struct sockaddr *addr, 
                         socklen_t addr_len);

/*  \brief Wrapper to call function nx_udp_socket_receive() using a posix 
 *         interface.
 *         Receives data from a socket. Blocks until data is received.
 *
 *  \param [in] socket_id Socket to receive data from.
 *  \param [in] rcv_buffer Address of the buffer where received data will 
 *                         be stored. 
 *  \param [in] buffer_len Size of the buffer to received data.
 *  \param [in] flags Not used.
 *  \param [in] local_addr Not used.
 *  \param [in] addr_len Not used.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_recvfrom(int sock_id, void *rcv_buffer, int buffer_len,
                             int flags, struct sockaddr *local_addr,
                             socklen_t *addr_len);

#ifdef RTI_USE_NONBLOCKING_SOCKET
/*  \brief Sets a callback function that will be called when new data is
 *         received in the socket. Wrapper to call function nx_udp_socket_receive() 
 *         using a posix interface.
 *
 *  \param [in] socket_id Socket to receive data from.
 *  \param [in] udp_queue Queue where to post a message, the pointer /ref user_data,
 *                        when a new packet is available to read in the socket.
 *                        IF this parameter is NULL the callback is removed.
 *  \param [in] user_data User data to post in the queue when a new packet is available
 *                        in the socket.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_recvnotify(int sock_id, 
                               TX_QUEUE *udp_queue,
                               void *user_data);

/*  \brief Returns the number of packets queued in the socket.
 *
 *  \param [in] socket_id Socket to know how many packets are available 
 *                        to be read.
 *
 *  \return Number of packets in the socket; 0 if no packets or error.
 */
extern ULONG
OSAPI_ThreadXSocket_packets_queued(int sock_id);
#endif /* RTI_USE_NONBLOCKING_SOCKET */

/*  \brief Wrapper to call function nx_udp_socket_send() using a posix 
 *         interface.
 *         Sends data using a socket.
 *
 *  \param [in] socket_id Socket to receive data from.
 *  \param [in] msg Pointer to data to send. 
 *  \param [in] msg_len Size of data to send.
 *  \param [in] flags Not used.
 *  \param [in] src_addr Destination address and port.
 *  \param [in] addr_len Size of parameter addr_len.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_sendto(int sock_id, void *msg, int msg_len, int flags,
                           const struct sockaddr *src_addr, socklen_t addr_len);

/*  \brief Wrapper to call functions nx_igmp_multicast_join(),
 *         nx_igmp_multicast_leave(), nx_igmp_loopback_enable() and
 *         nx_igmp_loopback_disable() using a posix interface.
 *
 *  \param [in] socket_fd Socket id.
 *  \param [in] level Only IPPROTO_IP and SOL_SOCKET are supported. 
 *  \param [in] opt_name Option name.
 *  \param [in] opt_val Option value.
 *  \param [in] opt_len Option value length.
 *
 *  \return SOCKET_ERROR if error, otherwise 0
 */
extern INT
OSAPI_ThreadXSocket_setsockopt(int sock_fd, int level, int opt_name,
                               const void *opt_val, socklen_t opt_len);

#endif /* OSAPI_INCLUDE_THREADX */

#endif  /* threadxSocket_h */
