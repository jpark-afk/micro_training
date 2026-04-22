/*********************************************************************************************
Copyright (c) 2025-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RTI_WIN32
#include <winsock2.h>
#include <process.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif

#include "HelloWorld_dgram_udpv4.h"

/* Arbitrary maximum number of ports to listen to */
#define MAX_PORTS (256)

/* Arbitrary maximum send and receive buffer size */
#define MAX_BUFFER_SIZE (65535)

/* A port is either free to use or active */
#define PORT_STATE_FREE       (0)
#define PORT_STATE_ACTIVE     (1)

static RTI_BOOL is_running;

struct HelloWorld_dgram_udpv4_PortEntry
{
    RTI_INT32 state;

    struct NETIO_Address source;
#ifdef RTI_WIN32
    SOCKET sock;
#else
    int sock;
#endif
};

/* State information for the UDPv4 transport instance */
typedef struct HelloWorld_dgram_udpv4
{
    /* Must inherit from the NETIO_Interface base-class */
    struct NETIO_Interface _parent;

    /* Descriptor for active sockets being listened to */
    fd_set r_fdset;

    /* Maximum receive buffer */
    char buffer[MAX_BUFFER_SIZE];

    /* The packet being passed upstream */
    NETIO_Packet_T _rx_buffer;

    /* The receive thread */
#ifdef RTI_WIN32
    HANDLE rx_thread;
#else
    pthread_t rx_thread;
#endif

    /* List of ports for the receive thread to listen on */
    struct HelloWorld_dgram_udpv4_PortEntry ports[MAX_PORTS];

    /* Socket to send data from, shared for all destination */
    int send_socket;

    /* The upstream interface that created the instance */
    NETIO_Interface_T *ull;

    /* Maximum send buffer */
    char send_buffer[MAX_BUFFER_SIZE];

} HelloWorld_dgram_udpv4;

#ifdef RTI_WIN32
/**
 * Initialize the socket module for windows.
 *
 * \return RTI_TRUE on success, RTI_FALSE in failure.
 */
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
        !(rtcode = WSAStartup(0x0002, &wd))) /* 2.0 success */
    {
    }
    else
    {
        return RTI_FALSE;
    }

    /* Check version returned must not be older than 2.0 */
    if (HIBYTE(wd.wVersion) != 2)
    {
        return RTI_FALSE;
    }

    already_called = RTI_TRUE;

    return RTI_TRUE;
}
#endif


/**
 * Initialize a port entry to listen for data.
 * The RCC will only request a port to be listened to once. This example
 * only uses the loopback address.
 *
 * port_entry  The port entry to initialize.
 * src_addr    The address to listen to.
 *
 * RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_init_port_entry(
            struct HelloWorld_dgram_udpv4_PortEntry *port_entry,
            struct NETIO_Address *src_addr)
{
    struct sockaddr_in sock_addr;
    int rc;

    /* A basic DGRAM socket*/
    port_entry->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (port_entry->sock < 0)
    {
        return RTI_FALSE;
    }

    /* Check that the specified socket can be reserved */
    sock_addr.sin_family = AF_INET;

    /* Important: Ports are passed in network order */
    sock_addr.sin_port = htons((RTI_UINT16)src_addr->port);

    /* Important: Addresses are passed in network order */
    sock_addr.sin_addr.s_addr = htonl(0x7f000001);

    rc = bind(port_entry->sock,
            (struct sockaddr *)&sock_addr,
            sizeof(struct sockaddr_in));

    if (rc < 0)
    {
        goto close_and_release;
    }

    port_entry->source = *src_addr;

    return RTI_TRUE;

close_and_release:

    /* This exit path is reached in case of failure and is a common path to
     * release resources allocated at the point of failure.
     */
#ifdef RTI_WIN32
    rc = closesocket(port_entry->sock);
#else
    rc = close(port_entry->sock);
#endif
    if (rc < 0)
    {
    }

    return RTI_FALSE;
}

/**
 * A task used to receive data.
 *
 * param   The instance of HelloWorld_dgram_udpv4 used to create the task.
 */
RTI_PRIVATE void*
HelloWorld_dgram_udpv4_Interface_receive_task(void *param)
{
    struct HelloWorld_dgram_udpv4 *instance = (struct HelloWorld_dgram_udpv4*)param;
    int i;
    struct sockaddr_in ip_src;
    socklen_t ip_len = sizeof(struct sockaddr);
    RTI_INT32 rx_len;
    struct timeval sel_timeout;
#ifdef RTI_WIN32
    WSABUF mbuf;
    int flags = 0;
#endif
    sel_timeout.tv_sec = 0;

    /* Wait at most 50ms before trying again in case a new port was made active
     * This can be better handled by waking up a blocked receive call.
     */
    sel_timeout.tv_usec = 50000;

    while (is_running)
    {
        FD_ZERO(&instance->r_fdset);

        /* There are many ways to manage ports, this is simple */
        for (i = 0; i < MAX_PORTS; ++i)
        {
            if (instance->ports[i].state == PORT_STATE_ACTIVE)
            {
                /* Only check active ports */
                FD_SET(instance->ports[i].sock,&instance->r_fdset);
            }
        }

        if (select(MAX_PORTS+1, &instance->r_fdset,NULL,NULL,&sel_timeout) <= 0)
        {
            /* No active ports */
            continue;
        }

        for (i = 0; i < MAX_PORTS; ++i)
        {
            if ((instance->ports[i].state == PORT_STATE_ACTIVE) &&
                FD_ISSET(instance->ports[i].sock,&instance->r_fdset))
            {
#ifdef RTI_WIN32
                mbuf.buf = instance->buffer;
                mbuf.len = MAX_BUFFER_SIZE;
                if (WSARecvFrom((SOCKET)instance->ports[i].sock,
                        (WSABUF*)&mbuf, 1, &rx_len, &flags,
                        (struct sockaddr *)&ip_src, &ip_len, NULL, NULL) != 0)
                {
                }
#else
                rx_len = (RTI_INT32)recvfrom(instance->ports[i].sock,
                                             instance->buffer,
                                             MAX_BUFFER_SIZE,
                                             0,(struct sockaddr*)&ip_src,
                                             &ip_len);
#endif
                 if (rx_len <= 0)
                 {
                    continue;
                 }

                /* Assign the received data to the packet */
                if (!NETIO_Packet_set_payload(&instance->_rx_buffer,
                                     instance->buffer, rx_len))
                {
                    continue;
                }

                /* We must pass the received data to the upstream interface.
                 * It is important to pass the address the data was received on
                 * (the source) upstream. The upstream interface expects
                 * only one buffer.
                 */
                if (!NETIO_DGRAM_Interface_upstream_receive(
                            instance->ull,
                            &instance->ports[i].source,
                            &instance->_rx_buffer))
                {
                }
            }
        }
    }

    return NULL;
}

/**
 * Creates an instance of the user defined datagram Interface transport.
 *
 * upstream   The upstream NETIO interface to pass the packet to.
 * property   The property is not used and is transparent to \RCC.
 *
 * The function must return a pointer to a new instance.
 *
 * New instance on success and NULL on failure.
 */
RTI_PRIVATE NETIO_Interface_T*
HelloWorld_dgram_udpv4_Interface_initialize_instance(
            NETIO_Interface_T *upstream,
            void *property)
{
    int i;
#ifndef RTI_WIN32
    pthread_attr_t attr;
    int rc;
#endif
    HelloWorld_dgram_udpv4 *instance;

    UNUSED_ARG(property);

#ifdef RTI_WIN32
    if (!NETIO_SocketModule_init())
    {
        return NULL;
    }
#endif

    OSAPI_Heap_allocate_struct(&instance, struct HelloWorld_dgram_udpv4);
	if (instance == NULL)
	{
		return NULL;
    }

    /* The upstream interface. This is the interface to send received data
     * to on the receive thread.
     */
    instance->ull = upstream;

    instance->send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (instance->send_socket < 0)
    {
        return NULL;
    }

    for (i = 0; i < MAX_PORTS; ++i)
    {
        instance->ports[i].state = PORT_STATE_FREE;
    }

    FD_ZERO(&instance->r_fdset);

    is_running = RTI_TRUE;

#ifdef RTI_WIN32
    instance->rx_thread = (HANDLE) _beginthread(
        (void (__cdecl *) (void *))HelloWorld_dgram_udpv4_Interface_receive_task,
        0, (void *)instance);

    if (instance->rx_thread == NULL)
    {
        return NULL;
    }
#else
    rc = pthread_attr_init(&attr);
    if (rc != 0)
    {
        return NULL;
    }

    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (rc != 0)
    {
        return NULL;
    }

    rc = pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    if (rc != 0)
    {
        return NULL;
    }

    /* Create the receive thread that serves all receive sockets
     */
    rc = pthread_create(&instance->rx_thread,&attr,
                        HelloWorld_dgram_udpv4_Interface_receive_task,
                        (void *)instance);

    if (rc != 0)
    {
        return NULL;
    }

    rc = pthread_attr_destroy(&attr);
    if (rc != 0)
    {
    }
#endif
    return (struct NETIO_Interface*)instance;
}

#ifndef RTI_CERT
/**
 * Deletes the instance of the user defined datagram interface.
 *
 * upstream   The upstream interface
 *
 * Return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE void
HelloWorld_dgram_udpv4_Interface_delete_instance(NETIO_Interface_T *self)
{
    HelloWorld_dgram_udpv4 *instance = (HelloWorld_dgram_udpv4 *)self;

#ifdef RTI_WIN32
    closesocket(instance->send_socket);
#else
    close(instance->send_socket);
#endif

    OSAPI_Heap_free_struct(self);
}
#endif

/**
 * Sends a message with the DGRAM interface.
 *
 * A user defined datagram function that is called by the RCC to forward
 * a packet from a source interface to a destination.
 *
 * self         DGRAM interface to send from.
 * source       The source interface for the packet.
 * destination  The destination address for the packet.
 * packet       The packet to send.
 *
 * return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_send(
                NETIO_Interface_T *netio_intf,
                struct NETIO_Interface *source,
                struct NETIO_Address *address,
                NETIO_Packet_T *packet)
{
    HelloWorld_dgram_udpv4 *instance = (HelloWorld_dgram_udpv4 *)netio_intf;
    struct sockaddr_in sock_addr;
#ifdef RTI_WIN32
    int bytes_sent;
#else
    ssize_t bytes_sent;
#endif
    struct NETIO_PacketBuffer *pktbuf;
    RTI_SIZE_T pbuf_length = 0;
    RTI_SIZE_T offset = 0;

    /* This example sends from the loopback interface. User implementations
     * may want to maintain a list of sources and send the data based on user
     * defined criteria.
     */
    UNUSED_ARG(source);

    /* The packet contains a list of buffers. It is assumed that one buffer is
     * sent from the DGRAM for UDP. This integration later may choose to send
     * more than one buffer but the upstream interface expects a single buffer
     * on the receiving end. The DGRAM interface must maintain a buffer
     * for sending packet data.
     */
    OSAPI_Memory_zero(&instance->send_buffer,sizeof(instance->send_buffer));
    pktbuf = packet->head_pbuf;

    while (pktbuf)
    {
        pbuf_length = NETIO_PacketBuffer_get_length(pktbuf);

        if ((offset + pbuf_length) > MAX_BUFFER_SIZE)
        {
            return RTI_FALSE;
        }

        OSAPI_Memory_copy(&instance->send_buffer[offset],
                          NETIO_PacketBuffer_get_head(pktbuf),
                          pbuf_length);

        offset += pbuf_length;
        pktbuf = pktbuf->_next;
    }

    memset(&sock_addr,0,sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons((RTI_UINT16)address->port);
    sock_addr.sin_addr.s_addr = address->value.ipv4.address;

    bytes_sent = sendto(instance->send_socket,
                        instance->send_buffer,
                        offset,
                        0,
                        (struct sockaddr*)(&sock_addr),
                        sizeof(struct sockaddr_in));

    if (bytes_sent < 1)
    {
    }

    /* Always return TRUE in this example. */
    return RTI_TRUE;
}

/**
 * The address resolver converts a string into a NETIO_Address.
 *
 * Only the address field is set.
 *
 * netio_intf      The NETIO_Interface_T.
 * if_entry        The if_entry asked to resolve the address.
 * address_string  The address string to resolve.
 * address_value   The numerical value.
 * is_invalid      Set to TRUE on output if the address is invalid,
 *                  RTI_FALSE if unknown.
 *
 * Return RTI_TRUE on success, RTI_FALSE on unknown or failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_resolve_address(
                NETIO_Interface_T *netio_intf,
                const struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                const char *address_string,
                struct NETIO_Address *address_value,
                RTI_BOOL *is_invalid)
{
    unsigned char addr[4];
    int rc;
    UNUSED_ARG(netio_intf);

    *is_invalid = RTI_FALSE;

    if (address_string[0] == 0)
    {
        return RTI_TRUE;
    }

    if (if_entry->locator_kind != NETIO_ADDRESS_KIND_UDPv4)
    {
        return RTI_FALSE;
    }

    rc = inet_pton(AF_INET, address_string, &addr);

    if (rc == 1)
    {
        memset(&address_value->value.address,0,16);

        /* NOTE: Although RTPS uses the 12-15 for the IPV4 address, it is
         * necessary to return an IPv4 address in the first 4 bytes.
         */
        address_value->value.address.octet[0] = addr[0];
        address_value->value.address.octet[1] = addr[1];
        address_value->value.address.octet[2] = addr[2];
        address_value->value.address.octet[3] = addr[3];
    }
    else if (rc == 0)
    {
        /* not parsable */
        *is_invalid = RTI_TRUE;
        return RTI_FALSE;
    }
    else
    {
        /* unkown error */
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


/**
 * Release a previously reserved address.
 *
 * When the RCC no longer listens to an address it is released.
 *
 * netio_intf  The transport that resevered the address.
 * address     The address to release.
 *
 * Return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_release_address(
            NETIO_Interface_T *netio_intf,
            struct NETIO_Address *address)
{
    HelloWorld_dgram_udpv4 *self = (HelloWorld_dgram_udpv4 *)netio_intf;
    RTI_INT32 i;
    RTI_INT32 active_sockets = 0;

    for (i = 0; i < MAX_PORTS; ++i)
    {
        if ((self->ports[i].state == PORT_STATE_ACTIVE) &&
            memcmp(&self->ports[i].source,
                address,sizeof(struct NETIO_Address)) == 0)
        {
#ifdef RTI_WIN32
            closesocket(self->ports[i].sock);
#else
            close(self->ports[i].sock);
#endif
            self->ports[i].state = PORT_STATE_FREE;
            break;
        }

        if ((self->ports[i].state == PORT_STATE_ACTIVE))
        {
            active_sockets++;
        }
    }

    if (i == MAX_PORTS)
    {
        return RTI_FALSE;
    }
    
    /* When we close the last socket we can end the thread. */
    for (; i < MAX_PORTS; ++i)
    {
        if ((self->ports[i].state == PORT_STATE_ACTIVE))
        {
            active_sockets++;
        }
    }

    if (active_sockets == 0)
    {
        is_running = RTI_FALSE;
        pthread_join(self->rx_thread, NULL); 
    }

    return RTI_TRUE;
}

/**
 * Return the type of locators this transport can route.
 *
 * This example can route all UDPv4 unicast and multicast traffic.
 *
 * netio_intf   The transport.
 * addr_seq     The addresses the transport can route to.
 * netmask_seq  The netmask to apply to the addresses.
 *
 * Return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_get_route_table(
            NETIO_Interface_T *netio_intf,
            struct NETIO_AddressSeq *addr_seq,
            struct NETIO_NetmaskSeq *netmask_seq)
{
    UNUSED_ARG(netio_intf);

    /* NETIO_ADDRESS_UDPv4_ANY_UNICAST_ROUTE allows for any unicast route */
    if (!NETIO_DGRAM_Interface_add_route_entry_to_seq(
                        addr_seq,netmask_seq,
                        &NETIO_ADDRESS_UDPv4_ANY_UNICAST_ROUTE))
    {
        return RTI_FALSE;
    }

    /* NETIO_ADDRESS_UDPv4_ANY_MULTICAST_ROUTE allows for any multicast route */
    if (!NETIO_DGRAM_Interface_add_route_entry_to_seq(
                        addr_seq,netmask_seq,
                        &NETIO_ADDRESS_UDPv4_ANY_MULTICAST_ROUTE))
    {
        return RTI_FALSE;
    }

   return RTI_TRUE;
}

/**
 * The network interfaces known to this transport.
 *
 * The list of interface addresses available for DDS endpoints to
 * listen for data. This example only uses the loopback.
 *
 * netio_intf  The transport.
 * if_table    The list of interfaces on return.
 *
 * RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_get_interface_list(
                    NETIO_Interface_T *netio_intf,
                    struct NETIO_DGRAM_InterfaceTableEntrySeq *if_table)
{
    RTI_INT32 if_len;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;
    struct NETIO_DGRAM_InterfaceTableEntry if_entry_tmp =
                                NETIO_DGRAM_InterfaceTableEntry_INITIALIZER;
    UNUSED_ARG(netio_intf);

    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(if_table);
    if (!NETIO_DGRAM_InterfaceTableEntrySeq_set_length(if_table,if_len+1))
    {
        return RTI_FALSE;
    }

    if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(if_table,if_len);
    *if_entry = if_entry_tmp;

    /* The type of address which this interface can listen to */
    if_entry->locator_kind = NETIO_ADDRESS_KIND_UDPv4;

    /* The interface address. Note that addresses must be returned in
     * network order and for IPv4 in the first 4 bytes.
     */
    if_entry->address.value.address.octet[0] = 0x7f;
    if_entry->address.value.address.octet[1] = 0x00;
    if_entry->address.value.address.octet[2] = 0x00;
    if_entry->address.value.address.octet[3] = 0x01;

    /* If the interface uses shared ports, the address which the
     * message was received on is ignored.
     */
    if_entry->flags = NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG;

    /* The interface name, not used for anything by the RCC */
    if_entry->ifname = REDA_String_dup("loopback");

    /* The MTU, not used for anything by the RCC */
    if_entry->mtu = 65507;

    return RTI_TRUE;
}

/**
 * Listen on the specified address.
 *
 * netio_intf  The transport.
 * addr        The address to listen to.
 *
 * RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
HelloWorld_dgram_udpv4_Interface_bind_address(NETIO_Interface_T *netio_intf,
                                   struct NETIO_Address *addr)
{
    HelloWorld_dgram_udpv4 *instance = (HelloWorld_dgram_udpv4 *)netio_intf;
    RTI_INT32 index;
    RTI_BOOL retval = RTI_FALSE;

    /* ports that are out of range */
    if ((addr->port < 1024) || (addr->port > 65535))
    {
        goto done;
    }

    /* Find a free port */
    for (index = 0; index < MAX_PORTS; ++index)
    {
        if (instance->ports[index].state == PORT_STATE_FREE)
        {
            break;
        }
    }

    if (index == MAX_PORTS)
    {
        goto done;
    }

    /* Initialize the port */
    if (!HelloWorld_dgram_udpv4_init_port_entry(&instance->ports[index],addr))
    {
        goto done;
    }

    instance->ports[index].state = PORT_STATE_ACTIVE;

    retval = RTI_TRUE;

done:

    return retval;
}

/* Implementation of the NETIO_DGRAM_InterfaceI */
static struct NETIO_DGRAM_InterfaceI HelloWorld_dgram_udpv4_Interface =
{
    HelloWorld_dgram_udpv4_Interface_initialize_instance,
#ifndef RTI_CERT
    HelloWorld_dgram_udpv4_Interface_delete_instance,
#endif
    HelloWorld_dgram_udpv4_Interface_get_interface_list,
    HelloWorld_dgram_udpv4_Interface_release_address,
    HelloWorld_dgram_udpv4_Interface_resolve_address,
    HelloWorld_dgram_udpv4_Interface_send,
    HelloWorld_dgram_udpv4_Interface_get_route_table,
    HelloWorld_dgram_udpv4_Interface_bind_address
};

RTI_BOOL
HelloWorld_dgram_udpv4_Interface_register(RT_Registry_T *registry,const char *name)
{
    if (!NETIO_DGRAM_InterfaceFactory_register(registry,name,
                                              &HelloWorld_dgram_udpv4_Interface,NULL))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
