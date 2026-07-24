/*
 * FILE: NETIO_SHMEMInterface.h - NETIO_SHMEMInterface interface implementation
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef NETIO_SHMEMInterface_h
#define NETIO_SHMEMInterface_h

#ifndef netio_shmem_h
#include "netio_shmem/netio_shmem.h"
#endif
#include "reda/reda_indexer.h"
#include "osapi/osapi_string.h"

#include "NETIO_SHMEMInterfaceCore.h"

struct NETIO_SHMEMInterfaceFactory;

struct NETIO_SHMEMInterface
{
    /*ci
     * \brief Inherited NETIO interface base-class
     */
    struct NETIO_Interface _parent;

    struct NETIO_InterfaceProperty property;
    /*ci
     * \brief The factory
     */
    struct NETIO_SHMEMInterfaceFactory *factory;

    struct NETIO_SHMEMTransport *shm_transport;

    /*ci
     * \brief Table with SHMEM port entries, once for each unique shared memory
     * key
     */
    DB_Table_T shmem_receive_thread_table;

};

struct SHMEMRouteEntry
{
    struct SHMEM_SendResource send_resource;
    RTI_INT32 ref_count;
};

struct SHMEMPortEntry
{
    /*ci
     * \brief The address to bind to and receive data from
     */
    struct NETIO_Address source;

    /*ci
     * \brief Receive buffer passed to the socket receive call
     */
    NETIO_Packet_T _rx_buffer;

    /*ci
     * \brief Thread blocked on receive call
     */
    struct OSAPI_Thread *_rx_thread;

    /*ci
     * \brief The number of listeners to this port
     */
    RTI_UINT32 _ref_count;

    /*ci
     * \brief Back reference to the SHMEM interface that created this port
     */
    struct NETIO_SHMEMInterface *_shm_intf;

    struct NETIO_SHMEMTransport *_shm_transport;

    struct SHMEM_RecvResource *_recv_resource;
};

struct SHMEMBindEntry
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
 * \brief SHMEMIntefaceFactor factory class
 */
struct NETIO_SHMEMInterfaceFactory
{
    /*ci
     * \brief Inherited factory
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief counter used when instantiating interfaces
     */
    RTI_INT32 instance_counter;

    /*ci
     * \brief The factory properties
     */
    struct NETIO_SHMEMInterfaceFactoryProperty property;

    /*ci
     * \brief The major version of the shared memory transport for all
     *        instances created by this factory
     */
    RTI_INT16 major_version;
};

#endif /* NETIO_SHMEMInterface_h */
