/*
 * FILE: NETIO_SHMEMInterfaceCore.h
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
#ifndef NETIO_SHMEMInterfaceCore_h
#define NETIO_SHMEMInterfaceCore_h

#include "reda/reda_bufferpool.h"
#include "netio_shmem/netio_shmem.h"
#include "dds_c/dds_c_infrastructure.h"
#include "cdr/cdr_md5.h"

#include "NETIO_SHMEM_ConcurrentQueue.h"

#define NETIO_SHMEMTransport_compute_segment_key(property, port) \
    (property->segment_key_offset + property->segment_key_factor * port )

#define NETIO_SHMEMTransport_compute_semaphore_key(property, port) \
    (property->semaphore_key_offset + property->semaphore_key_factor * port)

#define NETIO_SHMEMTransport_compute_mutex_key(property, port) \
    (property->mutex_key_offset + property->mutex_key_factor * port)

struct SHMEM_SendResource
{
    /*i The port number that identifies this send resource */
    RTI_UINT32 port;

    /*i The address that identifies this send resource */
    struct NETIO_Guid address;

    /*i A mutex to protect the concurrent queue */
    struct NETIO_SharedMemoryMutexHandle _shmem_mutex_hndl;

    /*i A semaphore to signal/wait for new data */
    struct NETIO_SharedMemorySignalingSemaphoreHandle _shm_sem_hndl;

    /*i Handle to shared memory area where the concurrent queue is placed */
    struct NETIO_SharedMemorySegmentHandle _shm_segment_hndl;

    /*i Shared memory header */
    struct NETIO_SHMEMTransportHdrShared *_shm_header;

    /*i The concurrent queue */
    struct NETIO_SHMEM_ConcurrentQueueHandle _concurrent_queue;

    /*i Identifies the reader that we originally connected to */
    int epoch;

    /*i
     *  Whether or not the resource has been totally initialized
     *  Assertion: if is_valid == TRUE -> _shm_header will point
     *  to a valid shared memory header.
     *  NOTE: is not vice-versa!
     */
    int is_valid;

    /* Connext Pro also had a member send_count which kept track of how many
     * threads are currently using this send resource within a send operation.
     * In Connext Micro, we do not have a need for this send_count because
     * a DDS_DataWriter write call takes the global database lock.
     */
};

struct NETIO_SHMEMTransportProperty
{
    /*i message_size_max
     */
    RTI_INT32 message_size_max;

    /*i received_message_count_max
     */
    RTI_INT32 received_message_count_max;

    /*i receive_buffer_size
     */
    RTI_INT32 receive_buffer_size;

    /*i offset used for port -> segment key mapping */
    RTI_INT32 segment_key_offset;

    /*i factor used for port -> segment key mapping */
    RTI_INT32 segment_key_factor;

    /*i offset used for port -> semaphore key mapping */
    RTI_INT32 semaphore_key_offset;

    /*i factor used for port -> semaphore key mapping */
    RTI_INT32 semaphore_key_factor;

    /*i offset used for port -> mutex key mapping */
    RTI_INT32 mutex_key_offset;

    /*i factor used for port -> mutex key mapping */
    RTI_INT32 mutex_key_factor;

    /*i
     * The maximum duration that a writer can take to write this message.
     * If a writer takes longer, it can be potentially be aborted writing
     * by a reader believing the writer is stuck (i.e. in an infinite loop).
     * Default value is 1000 milliseconds
     */
    struct DDS_Duration_t max_allowed_writer_send_duration;
};

/*i
 * \ingroup NDDS_Transport_Shmem
 *
 * This is the header info that is placed ahead of the NETIO_SHMEM_ConcurrentQueue
 * in shared memory
 */
struct NETIO_SHMEMTransportHdrShared
{
    /*i Cookie to be check to make sure that it's NDDS that created the
     * segment */
    unsigned int cookie;

    /*i Major version change indicates incompatible protocol */
    RTI_INT16 major_version;

    /*i Minor version change indicates compatible protocol */
    RTI_INT16 minor_version;

    /*i Size of header, helps make it forward compatible */
    int header_size;

    /*i Used by writers to identify if the reader has changed */
    int epoch;

    /*i The size of the block. Includes the header plus the
     *  ConcurrentQueue. The size of the memory given to the
     *  ConcurrentQueue is therefore
     *  sharedMemorySize - sizeof(struct NDDS_Transport_Shmem_Header)
     */
    int alloc_shm_size;

    /*i
     * Use for determining whether a given shared memory locator is coming
     * from a domain participant on the same node
     */
    RTI_UINT32 domain_participant_guid[4];
};

struct SHMEM_RecvResource
{
    /*i The port number that identifies this RecvResource */
    RTI_INT32 port;

    /*i A mutex to provide the WriteEA required by the concurrent
     * queue. Its stored here because it is created when the RecvResource is
     * created but it is not used by the receiver, just the sender.
     */
    struct NETIO_SharedMemoryMutexHandle _shm_mutex;

    /*i A semaphore to signal/wait for new data */
    struct NETIO_SharedMemorySignalingSemaphoreHandle _shm_semaphore;

    /*i Shared memory area where the concurrent queue is placed */
    struct NETIO_SharedMemorySegmentHandle _shm_segment;

    /*i Shared memory header */
    struct NETIO_SHMEMTransportHdrShared *_shm_transport_hdr;

    /*i The concurrent queue */
    struct NETIO_SHMEM_ConcurrentQueueHandle concurrent_q;

    /*i
     * Indicate to the reader that we forcibly unblocked in order to
     * shut down
     */
    int _is_force_unblocked;
};

struct NETIO_SHMEMTransport
{
    struct NETIO_SHMEMTransportProperty _property;
    RTI_INT16 _major_version;
};

void
CoreShmTransport_flush_queue_read_ea(
        struct SHMEM_RecvResource *handle);

void
CoreShmTransport_destroy_send_resource(
        struct SHMEM_SendResource *sendResourceHandle);

void
CoreShmTransport_destroy_recv_resource(
        struct SHMEM_RecvResource *recvresource_in);

RTIBool
CoreShmTransport_is_segment_compatible(
        char *shmem_addr,
        RTI_INT32 requested_shmem_size,
        RTI_INT32 major_version);

RTI_INT32
CoreShmTransport_create_recv_resource(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_RecvResource **recvresource_out,
        RTI_INT32 *dest_port_inout);

RTI_BOOL
CoreShmTransport_attach_writer(
        struct NETIO_SHMEMTransport *me,
        struct SHMEM_SendResource *sendresource_inout,
        RTI_INT32 port_in);

RTI_BOOL
CoreShmTransport_create_send_resource(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_SendResource *sendresource_out,
        const RTI_INT32 dest_port_in);

void
CoreShmTransport_destuck_writer_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *qHandle,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *qInfo);

RTI_BOOL
CoreShmTransport_send(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_SendResource **sendresource_in,
        const RTI_INT32 dest_port_in,
        NETIO_Packet_T *packet);

RTI_BOOL
CoreShmTransport_recv_from(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_RecvResource *recvResourceHandle,
        char **buffer,
        RTI_INT32 *cqHandle,
        RTI_INT32 *length);

#endif /* NETIO_SHMEMInterfaceCore */
