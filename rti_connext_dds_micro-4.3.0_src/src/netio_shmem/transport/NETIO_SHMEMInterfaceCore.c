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
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"
#include "netio/netio_log.h"

#include "NETIO_SHMEMInterfaceCore.h"

/* cookie is negative because in NDDS 4.0g and earlier, first member
 * of structure was sharedMemorySize, which cannot be negative
 */
const unsigned int VALID_COOKIE = 0xCE444453; /* "NDDS" | 0x80000000 */

void
CoreShmTransport_flush_queue_read_ea(
        struct SHMEM_RecvResource *handle)
{
    struct NETIO_SHMEM_ConcurrentQueueHandle *q_hndl = &handle->concurrent_q;
    struct NETIO_SHMEM_ConcurrentQueueStateInfo current_info;

    for (;;)
    {
        NETIO_SHMEM_ConcurrentQueue_flush_read_ea(
                q_hndl,
                NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID);
        NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(
                q_hndl,
                &current_info);
        if (!current_info._next_to_be_read_msg_is_being_written)
        {
            /* I'm done, queue flushed and no writer stuck */
            break;
        }

        /* Writer is potentially stuck
         * Make determination, for example if cookie is a PID
         * check that the process with that PID is still alive
         */
        if (OSAPI_Process_is_alive(
                (OSAPI_ProcessId)current_info._next_to_be_read_msg_writer_cookie))
        {
            /* Writer is still alive. Interrupt this operation and let's
             * defer it to the first time the 'receive' method is called.
             * (the receive method have support to handle this situations)
             */
            break;
        }
        /* else */
        /* Writer is not alive: we are stuck. Un-stuck it and skip
         * the partially-written message.
         */
        NETIO_SHMEM_ConcurrentQueue_finish_write(
                q_hndl,
                current_info._next_to_be_read_msg_write_finish_hndl,
                current_info._next_to_read_msg_size);
    }
}

void
CoreShmTransport_destroy_send_resource(
        struct SHMEM_SendResource *send_res)
{
    if (send_res->is_valid)
    {
        NETIO_SharedMemorySegment_detach(&send_res->_shm_segment_hndl);
        NETIO_SharedMemorySignalingSemaphore_detach(&send_res->_shm_sem_hndl);
        NETIO_SharedMemoryMutex_detach(&send_res->_shmem_mutex_hndl);
    }
}

void
CoreShmTransport_destroy_recv_resource(
        struct SHMEM_RecvResource *recvresource_in)
{
    struct SHMEM_RecvResource *recv_resource = recvresource_in;
    RTI_INT32 return_status = 0;

    OSAPI_PRECONDITION(
            recvresource_in == NULL,
            return ,
            OSAPI_Log_entry_add_pointer("handle", recvresource_in, RTI_TRUE); )

    /*
     * CORE-8295: Protect as much as possible with the mutex so that the send
     * resources won't get into an inconsistent state.
     */
    if (!NETIO_SharedMemoryMutex_lock(
            &recv_resource->_shm_mutex,
            &return_status))
    {
        NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(
                OSAPI_LOGKIND_ERROR,
                return_status);
    }

    /* _shmHeader may be NULL if connected to a 40g app */
    if (recv_resource->_shm_transport_hdr)
    {
        /* increment epoch to notify writers that shared memory is being
         * destroyed */
        ++recv_resource->_shm_transport_hdr->epoch;
    }

    recv_resource->_shm_transport_hdr = NULL;
    NETIO_SharedMemorySegment_delete(&recv_resource->_shm_segment);
    NETIO_SharedMemorySignalingSemaphore_delete(
            &recv_resource->_shm_semaphore);
    if (!NETIO_SharedMemoryMutex_unlock(
            &recv_resource->_shm_mutex,
            &return_status))
    {
        NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                OSAPI_LOGKIND_ERROR,
                return_status);
    }
    NETIO_SharedMemoryMutex_delete(&recv_resource->_shm_mutex);
    OSAPI_Heap_free(recv_resource);
}

RTIBool
CoreShmTransport_is_segment_compatible(
        char *shmem_addr,
        RTI_INT32 requested_shmem_size,
        RTI_INT32 major_version)
{
    struct NETIO_SHMEMTransportHdrShared *hdr =
                            OSAPI_Compiler_reinterpret_cast(
                                    struct NETIO_SHMEMTransportHdrShared*,
                                    shmem_addr);

    if (hdr->cookie != VALID_COOKIE)
    {
        NETIO_SHMEM_LOG_INCOMPATIBLE_COOKIE(OSAPI_LOGKIND_ERROR, (RTI_INT32)hdr->cookie);
        return RTI_FALSE;
    }
    if (hdr->major_version != major_version)
    {
        NETIO_SHMEM_LOG_INCOMPATIBLE_VERSION(
                OSAPI_LOGKIND_ERROR,
                hdr->major_version);
        return RTI_FALSE;
    }
    if (requested_shmem_size > hdr->alloc_shm_size)
    {
        NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER_SIZE(
                OSAPI_LOGKIND_ERROR,
                hdr->alloc_shm_size);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_INT32
CoreShmTransport_create_recv_resource(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_RecvResource **recvresource_out,
        RTI_INT32 *dest_port_inout)
{
    struct NETIO_SHMEMTransport *me = self;
    struct NETIO_SHMEMTransportHdrShared *transport_hdr = NULL;
    struct NETIO_SHMEMTransportProperty *transport_prop = &me->_property;
    struct SHMEM_RecvResource *new_recv_resource = NULL;

    char *shm_addr = NULL;
    char *cq_mem_addr = NULL;
    RTI_INT32 concurrent_q_size;
    RTI_INT32 key;
    RTI_INT32 return_status;
    RTI_INT32 req_shm_size;
    RTIBool mutex_taken = RTI_FALSE;
    RTIBool segment_already_claimed = RTI_FALSE;
    /* In case of error, we need to remember
     * if we created or attached to the mutex */
    RTIBool mutex_created;
    struct NETIO_SHMEM_ConcurrentQueueProperty queue_property =
            NETIO_SHMEM_ConcurrentQueueProperty_INITIALIZER;
    OSAPI_ProcessId pid = OSAPI_Process_getpid();
    RTI_UINT32 loop_count = 0; /* To avoid to loop forever */

    /* Number of attempts to create... lock mutex */
    RTI_INT32 create_retry_count = 0;

    OSAPI_Heap_allocate_struct(
            &new_recv_resource,
            struct SHMEM_RecvResource);

    if (new_recv_resource == NULL)
    {
        NETIO_SHMEM_LOG_FAILED_TO_ALLOCATE_STRUCT(OSAPI_LOGKIND_ERROR);
        goto failed0;
    }

    new_recv_resource->_is_force_unblocked = RTI_FALSE;
    new_recv_resource->port = *dest_port_inout;

    /* shared memory mutex */
    key = NETIO_SHMEMTransport_compute_mutex_key(
            transport_prop,
            new_recv_resource->port);

try_attach_again:
    if (!NETIO_SharedMemoryMutex_create_or_attach(
            &new_recv_resource->_shm_mutex,
            &return_status,
            key))
    {
        NETIO_SHMEM_LOG_FAILED_TO_INIT_MUTEX(OSAPI_LOGKIND_ERROR, key);
        goto failed1;
    }
    mutex_created = (return_status == OSAPI_SHARED_MEMORY_CREATED);
    if (!NETIO_SharedMemoryMutex_lock(
            &new_recv_resource->_shm_mutex,
            &return_status))
    {
        /* If fails because of no entry...
         * The mutex could get destroyed between createOrAttach and lock
         * It's worth trying again a couple of times before failing
         */
        if (return_status == OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY &&
                create_retry_count < 20)
        {
            ++create_retry_count;
            NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(
                    OSAPI_LOGKIND_WARNING,
                    return_status);
            goto try_attach_again;
        }

        NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(
                OSAPI_LOGKIND_ERROR,
                return_status);
        goto failed1;
    }
    mutex_taken = RTI_TRUE;

    /* begin multi-process critical region */

    /* shared memory segment */
    key = NETIO_SHMEMTransport_compute_segment_key(
            transport_prop,
            new_recv_resource->port);

    switch(self->_major_version)
    {
        case NETIO_SHMEM_MAJOR_AFTER_BUG_14240_FIX:
            /* Major 2 */
            queue_property.version = NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_BUG_14240_FIX;
            break;
        case NETIO_SHMEM_MAJOR_AFTER_WRITER_COOKIE_SUPPORT:
        case NETIO_SHMEM_MAJOR_AFTER_ROBUST_PTHREAD_MUTEX:
            /* Major 4 or 5 (contains PID used in Network Capture ) */
            /*
             * NETIO_SHMEM_MAJOR_AFTER_ROBUST_PTHREAD_MUTEX does not add
             * any changes to concurrent queue. This version was introduced to
             * indicate that shared memory mutexes are implemented using robust
             * pthread mutexes. Therefore,
             * NETIO_SHMEM_MAJOR_AFTER_ROBUST_PTHREAD_MUTEX should select
             * the same version of the concurrent queue as
             * NETIO_SHMEM_MAJOR_AFTER_WRITER_COOKIE_SUPPORT.
             */
            queue_property.version = NETIO_SHMEM_CONCURRENT_QUEUE_VERSION_AFTER_WRITER_COOKIE_SUPPORT;
            break;
        default:
            goto failed2;
    }

    /* We will try to honor the size requested of the transport.
     *
     * We will try to honor the size requested by the user of the
     *  plugin via xportProperty->receive_buffer_size, i.e., allocate
     *  enough shared memory so that the plugin actually gets to use
     *  receive_buffer_size bytes.  This is "malloc" semantics.
     *
     *  So the shared memory segment size that we will ask for from OSAPI is
     *
     *  xportProperty->receive_buffer_size + NETIO_SHMEM_ConcurrentQueue
     *  overhead + shared memory transport plugin header
     */

    concurrent_q_size = NETIO_SHMEM_ConcurrentQueue_get_size_required(
            transport_prop->message_size_max,
            transport_prop->received_message_count_max,
            transport_prop->receive_buffer_size,
            &queue_property.version);

    req_shm_size = concurrent_q_size
            + (RTI_INT32)OSAPI_Heap_align_size_up(
            sizeof(struct NETIO_SHMEMTransportHdrShared),
            sizeof(void*));

    while (1)
    {
        /* See bug #12405 */
        if (++loop_count > 3)
        {
            NETIO_SHMEM_LOG_CREATE_ATTACH_INFINITE(OSAPI_LOGKIND_ERROR);
            goto failed3;
        }
        if (NETIO_SharedMemorySegment_create_or_attach(
                &new_recv_resource->_shm_segment,
                &return_status,
                key,
                (RTI_UINT32)req_shm_size,
                pid) == RTI_FALSE)
        {
            if (return_status ==
                    OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED)
            {
                /* let the higher layer know; this might be due to participant
                 * index conflict
                 */
                NETIO_SHMEM_LOG_ADDRESS_IN_USE(
                        OSAPI_LOGKIND_INFO,
                        new_recv_resource->port);
                segment_already_claimed = RTI_TRUE;
                goto failed2;
            }

#ifdef RTI_DARWIN
            NETIO_SHMEM_LOG_CHECK_SYSTEM_SETTINGS(OSAPI_LOGKIND_ERROR);
#endif
            goto failed2;
        }

        /* Two cases here
         *
         *  A. Reusing an existing segment.  In that case we
         *
         *  1) check for compatibility, this means that the segment
         *        a) created by a NDDS Shared Memory transport (cookie)
         *        b) created to use a compatible version of the Shared
         *           Memory protocol (version)
         *        c) is the same size or larger than what we wanted,
         *           check the shmHeader for size of the segment
         *        d) the existing concurrent queue is compatible in
         *           size as well
         *
         *        To do step d, we must attach to the concurrent queue
         *        that is in shared memory.
         *
         *  2) if any of a, b, c is false, we detach and try to close
         *     the shared memory segment, this may fail if the uid of
         *     this process was not the one that actually created it in
         *     the first place.
         *
         *     if closing is successful, then go back and try to create
         *     the segment again.
         *
         *  3) if reusing and is compatible, then we
         *
         *        can immediately return having successfully
         *        reused an existing shared memory segment and have
         *        already attached a concurrent queue to the shmem
         *        back in step 1.
         *
         *        this is fine because any writers that are writing
         *        to the queue are ones that we want to be talking to
         *        in the first place.  The NDDS layer will have to
         *        drop packets that weren't meant specifically for
         *        this instance of the application.
         *
         *
         *  B. Got a newly created segment.  In that case, stamp the
         *     Plugin shared memory header with cookie, version and other
         *     info, epoch, shared memory size and create a current
         *     queue from the shmem.
         */

        shm_addr = NETIO_SharedMemorySegment_get_address(
                                        &new_recv_resource->_shm_segment);

        if (shm_addr == NULL)
        {
            NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR(OSAPI_LOGKIND_ERROR);
            goto failed3;
        }

        if (return_status == OSAPI_SHARED_MEMORY_ATTACHED)
        {
            transport_hdr = OSAPI_Compiler_reinterpret_cast(
                                        struct NETIO_SHMEMTransportHdrShared*,
                                        shm_addr);

            /* Shared memory plugin compatibility */
            if (CoreShmTransport_is_segment_compatible(
                    shm_addr,
                    req_shm_size,
                    me->_major_version))
            {
                cq_mem_addr = shm_addr
                        + transport_hdr->header_size;

                /* Concurrent queue compatibility */
                if (NETIO_SHMEM_ConcurrentQueue_attach(
                        &new_recv_resource->concurrent_q,
                        cq_mem_addr))
                {
                    struct NETIO_SHMEM_ConcurrentQueueDesc qDesc;
                    NETIO_SHMEM_ConcurrentQueue_get_queue_desc(
                            &new_recv_resource->concurrent_q,
                            &qDesc);

                    if ((transport_prop->message_size_max
                            <= qDesc._message_size_max) ||
                            (transport_prop->received_message_count_max
                            <= qDesc._message_count_max) ||
                            (concurrent_q_size <=
                            qDesc._max_data_bytes))
                    {

                        /* Success! Shared memory segment can and will
                         * be reused! */
                        CoreShmTransport_flush_queue_read_ea(
                                new_recv_resource);
                        goto success;
                    }
                }
            }

            /*
             * Not compatible, we will try to close it, if we succeed
             * we will try to create it again.  If not, that's an error.
             */
            ++transport_hdr->epoch;

#if defined(RTI_WIN32) || defined(RTI_WINCE) || defined(RTI_VXWORKS)
            /* Fabrizio:
             * Windows doesn't have a 'delete' of the shared segment: instead it
             * simply closes it. The OS will automatically delete when all the
             * process that are attached to it, release it.
             * If we get here, it means somebody is attached.
             * We have two options:
             *   1. We fail (there's nothing we can do
             *   2. We attempt to loop again (and perhaps wait some time).
             * Solution #2 involves introducing a delay, that's not always
             * a nice thing to do. For now we simply fail and exit.
             *
             * Same for VxWorks. We choose not to use SD_LINGER where
             * we can delete, but delete fails when there is still someone
             * attached to it.
             */
            NETIO_SharedMemorySegment_detach(
                    &new_recv_resource->_shm_segment);
            goto failed2;
#else
            if (!NETIO_SharedMemorySegment_delete(
                    &new_recv_resource->_shm_segment))
            {
                /* failed to close, probably not created by same user
                 * as this process */
                goto failed3;
            }
#endif
            /* close was successful, try to create again, goto top of
             * do/while
             */
            continue;
        }
        else
        {
            /*
             * a new segment was created, break out of the loop
             */
            break;
        }
    } /* createOrAttach() loop */

    /*
     * This is a paranoid precondition test only, shouldn't get to
     * here without having a new shared memory segment created
     */

    OSAPI_PRECONDITION(
            return_status != OSAPI_SHARED_MEMORY_CREATED,
            goto failed3,
            OSAPI_Log_entry_add_int("rc", return_status, RTI_TRUE); )

    /*
     * The shared memory returned has been newly created: no previous
     * data is present, we need to initialize & reset the concurrent queue
     */

    /* Copy the transport properties as the fist part into the
     * shared-memory area */
    transport_hdr = OSAPI_Compiler_reinterpret_cast(
                                    struct NETIO_SHMEMTransportHdrShared*,
                                    shm_addr);

    transport_hdr->cookie = VALID_COOKIE;
    transport_hdr->major_version = me->_major_version;
    transport_hdr->minor_version = NETIO_SHMEM_VERSION_MINOR_DEFAULT;
    transport_hdr->header_size = OSAPI_Heap_align_size_up(
            sizeof(struct NETIO_SHMEMTransportHdrShared),
            sizeof(void*));
    transport_hdr->alloc_shm_size = req_shm_size;
    transport_hdr->epoch = 0;

    cq_mem_addr = shm_addr + transport_hdr->header_size;

    /* Create the concurrent queue */
    if (!NETIO_SHMEM_ConcurrentQueue_create(
            &new_recv_resource->concurrent_q,
            transport_prop->received_message_count_max,
            transport_prop->message_size_max,
            cq_mem_addr,
            concurrent_q_size,
            &queue_property))
    {

        NETIO_SHMEM_LOG_FAILED_TO_INIT_CONCURRENT_Q(
                OSAPI_LOGKIND_ERROR,
                new_recv_resource->port);
        goto failed3;
    }

    /* If we get here, then we have successfully created or attached
     * to a shared memory segment and have created or attached to a
     * concurrent queue stored within.
     */
success:
    new_recv_resource->_shm_transport_hdr = transport_hdr;

    /* Create a shared memory semaphore to allow others to signal us
     * letting us know that there is data waiting in the concurrent queue
     */
    key = NETIO_SHMEMTransport_compute_semaphore_key(
            transport_prop,
            new_recv_resource->port);

    if (!NETIO_SharedMemorySignalingSemaphore_create_or_attach(
            &new_recv_resource->_shm_semaphore,
            &return_status,
            key))
    {
        NETIO_SHMEM_LOG_FAILED_TO_INIT_SIG_SEM(OSAPI_LOGKIND_ERROR, key);
        goto failed3;
    }

    /* Put the pointer to the structure in the RecvResource passed
     * in by the caller.
     */

    *recvresource_out = (void*)new_recv_resource;
    if (mutex_taken)
    {
        mutex_taken = RTI_FALSE;
        if (!NETIO_SharedMemoryMutex_unlock(
                &new_recv_resource->_shm_mutex,
                &return_status))
        {

            NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                    OSAPI_LOGKIND_ERROR,
                    return_status);
            goto failed4;
        }
    }

    return RTI_TRUE;

failed4:
    NETIO_SharedMemorySignalingSemaphore_delete(
            &new_recv_resource->_shm_semaphore);

failed3:
    NETIO_SharedMemorySegment_delete(
            &new_recv_resource->_shm_segment);

failed2:

    if (mutex_taken)
    {
        if (!NETIO_SharedMemoryMutex_unlock(
                &new_recv_resource->_shm_mutex,
                &return_status))
        {
            NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                    OSAPI_LOGKIND_ERROR,
                    return_status);
        }
    }

    if (mutex_created)
    {
        if (!segment_already_claimed)
        {
            NETIO_SharedMemoryMutex_delete(&new_recv_resource->_shm_mutex);
        }
    }
    else
    {
        /* Mutex was attached */
        NETIO_SharedMemoryMutex_detach(&new_recv_resource->_shm_mutex);
    }

failed1:
    OSAPI_Heap_free(new_recv_resource);

failed0:
    return RTI_FALSE;
}

RTI_BOOL
CoreShmTransport_attach_writer(
        struct NETIO_SHMEMTransport *me,
        struct SHMEM_SendResource *sendresource_inout,
        RTI_INT32 port_in)
{
    char *shm_addr;
    char *cq_mem_addr;
    RTI_INT32 key = 0;
    RTI_INT32 old_epoch = 0;
    RTI_INT32 return_status = 0;
    struct NETIO_SHMEM_ConcurrentQueueDesc queueDesc;

    struct NETIO_SHMEMTransportHdrShared *shm_hdr;
    struct NETIO_SHMEMTransportProperty *transport_prop = &me->_property;

    /* Save the old epoch so that we can restore it if we fail. */
    old_epoch = sendresource_inout->epoch;

    /* try to open the shared mutex */
    key = NETIO_SHMEMTransport_compute_mutex_key(transport_prop, port_in);

    if (!NETIO_SharedMemoryMutex_attach(
            &sendresource_inout->_shmem_mutex_hndl,
            &return_status,
            key))
    {
        if (return_status != OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
        {
            NETIO_SHMEM_LOG_FAILED_TO_INIT_MUTEX(OSAPI_LOGKIND_ERROR, key);
        }
        goto failed1;
    }

    /* take the mutex to protect access to the shared memory segment */
    if (!NETIO_SharedMemoryMutex_lock(
            &sendresource_inout->_shmem_mutex_hndl,
            &return_status))
    {
        NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(
                OSAPI_LOGKIND_ERROR,
                return_status);
        goto failed2;
    }

    /*  try to attach to the shared memory */
    key = NETIO_SHMEMTransport_compute_segment_key(transport_prop, port_in);
    if (NETIO_SharedMemorySegment_attach(
            &sendresource_inout->_shm_segment_hndl,
            &return_status,
            key) == RTI_FALSE)
    {
        /* may not be able to open because it doesn't exist yet */
        if (return_status != OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
        {
            /* otherwise log exception */
            NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR(OSAPI_LOGKIND_ERROR);
        }
        goto failed3;
    }

    /* get address */
    shm_addr = NETIO_SharedMemorySegment_get_address(
            &sendresource_inout->_shm_segment_hndl);

    if (shm_addr == NULL)
    {
        NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR(OSAPI_LOGKIND_ERROR);
        goto failed4;
    }

    /* We don't care as much about the total size of the shared memory
     * segment as we do that it's big enough to hold the concurrent
     * queue that we're going to be using.  So don't need to add the
     * size of what we think the shared memory header should be...thus
     * we can be forwards compatible.
     */

    if (!CoreShmTransport_is_segment_compatible(
            shm_addr,
            0,
            me->_major_version))
    {
        NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER(OSAPI_LOGKIND_ERROR);
        goto failed4;
    }

    /* Check concurrent queue compatibility */
    shm_hdr = OSAPI_Compiler_reinterpret_cast(
                                    struct NETIO_SHMEMTransportHdrShared*,
                                    shm_addr);

    cq_mem_addr = shm_addr + shm_hdr->header_size;

    /* Concurrent queue compatibility */
    if (!NETIO_SHMEM_ConcurrentQueue_attach(
            &sendresource_inout->_concurrent_queue,
            cq_mem_addr))
    {
        NETIO_SHMEM_LOG_FAILED_TO_ATTACH_CONCURRENT_Q(OSAPI_LOGKIND_ERROR);
        goto failed4;
    }

    /* One more check: make sure we can fit our maximum message size in
     * the concurrent queue
     */
    NETIO_SHMEM_ConcurrentQueue_get_queue_desc(
            &sendresource_inout->_concurrent_queue,
            &queueDesc);
    if (queueDesc._message_size_max < transport_prop->message_size_max)
    {

        NETIO_SHMEM_LOG_INCOMPATIBLE_CONCURRENT_Q(
                OSAPI_LOGKIND_ERROR,
                queueDesc._message_size_max,
                transport_prop->message_size_max);
        goto failed4;
    }

    /* Success! Shared memory segment can and will be reused!
     * Store the epoch.
     */
    sendresource_inout->epoch = shm_hdr->epoch;
    sendresource_inout->_shm_header = shm_hdr;

    /* Try to open the shared semaphore for signaling the reader */
    key = NETIO_SHMEMTransport_compute_semaphore_key(transport_prop, port_in);

    if (!NETIO_SharedMemorySignalingSemaphore_attach(
            &sendresource_inout->_shm_sem_hndl,
            &return_status,
            key))
    {
        NETIO_SHMEM_LOG_FAILED_TO_ATTACH_SIG_SEM(OSAPI_LOGKIND_ERROR, key);
        goto failed4;
    }

    sendresource_inout->port = (RTI_UINT32)port_in;

    /* Release mutex */
    if (!NETIO_SharedMemoryMutex_unlock(
            &sendresource_inout->_shmem_mutex_hndl,
            &return_status))
    {
        NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                OSAPI_LOGKIND_ERROR,
                return_status);
        goto failed5;
    }

    return RTI_TRUE;

failed5:
    /* Semaphore attached */
    NETIO_SharedMemorySignalingSemaphore_detach(
            &sendresource_inout->_shm_sem_hndl);

failed4:
    /* Segment attached */
    NETIO_SharedMemorySegment_detach(&sendresource_inout->_shm_segment_hndl);

failed3:
    /* Mutex taken */
    if (!NETIO_SharedMemoryMutex_unlock(
            &sendresource_inout->_shmem_mutex_hndl,
            &return_status))
    {
        /* It is possible that mutex fails to unlock because it has been
         * deleted. Do not print an error here.
         */
        NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                OSAPI_LOGKIND_INFO,
                return_status);
    }

failed2:
    /* Mutex attached */
    NETIO_SharedMemoryMutex_detach(&sendresource_inout->_shmem_mutex_hndl);

failed1:
    /* Restore epoch */
    sendresource_inout->epoch = old_epoch;
    return RTI_FALSE;
}

RTI_BOOL
CoreShmTransport_create_send_resource(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_SendResource *sendresource_out,
        const RTI_INT32 dest_port_in)
{
    struct NETIO_SHMEMTransport *me = (struct NETIO_SHMEMTransport*)self;

    OSAPI_PRECONDITION(
            self == NULL || sendresource_out == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("res", sendresource_out, RTI_TRUE); )

    sendresource_out->is_valid = RTI_FALSE;

    /* This may succeed or fail because the reader may or may not exist
     * yet. If it fails, that's OK since we can do lazy initialization in send()
     */
    if (CoreShmTransport_attach_writer(
            me,
            sendresource_out,
            dest_port_in))
    {
        sendresource_out->is_valid = RTI_TRUE;
    }

    return RTI_TRUE;
}

void
CoreShmTransport_destuck_writer_read_ea(
        struct NETIO_SHMEM_ConcurrentQueueHandle *q_hndl,
        struct NETIO_SHMEM_ConcurrentQueueStateInfo *q_info)
{
    RTI_INT32 read_finished_hndl = 0; /* Used to de-stuck the writer */
    char *where_from; /* Used to de-stuck the writer */

    NETIO_SHMEM_ConcurrentQueue_finish_write(
            q_hndl,
            q_info->_next_to_be_read_msg_write_finish_hndl,
            q_info->_next_to_read_msg_size);
    NETIO_SHMEM_ConcurrentQueue_start_read_ea(
            q_hndl,
            &read_finished_hndl,
            &where_from,
            NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID);
    NETIO_SHMEM_ConcurrentQueue_finish_read_ea(q_hndl, read_finished_hndl);
}

RTI_BOOL
CoreShmTransport_send(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_SendResource **sendresource_in,
        const RTI_INT32 dest_port_in,
        struct NETIO_Packet *packet)
{
    struct NETIO_SHMEMTransport *me = (struct NETIO_SHMEMTransport*)self;
    struct SHMEM_SendResource *dst =
            (struct SHMEM_SendResource*)*sendresource_in;
    int cq_finish_hndl;
    char *cq_where_to = NULL;
    RTI_SIZE_T total_size = 0;
    int ok;
    int fail_reason;
    struct NETIO_PacketBuffer *pbuf;
    RTI_SIZE_T pbuf_length;

    /* The pid is truncated to RTI_UINT32 to be used to generate the cookie in
     * NETIO_SHMEM_ConcurrentQueue_startWriteEA
     */
    RTI_UINT32 pid = (RTI_UINT32)OSAPI_Process_getpid();
    RTI_INT32 return_value = 0;

    OSAPI_PRECONDITION(
            *sendresource_in == NULL || me == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("handle", *sendresource_in, RTI_TRUE); )

    /* This test is done such that we can skip to the fast-path in the
     * common case where we are attached and the epoch has not changed
     */
    if ((!dst->is_valid) || (dst->_shm_header->epoch != dst->epoch))
    {
        /* Check to see if shared memory segment is still valid. The
         * reading participant may have deleted it. If so,
         * they should have marked it invalid
         */
        if (dst->is_valid && dst->_shm_header->epoch != dst->epoch)
        {
            /* after closing shared memory segment can't use the shared memory
             * address anymore
             */
            dst->is_valid = RTI_FALSE;
            dst->_shm_header = NULL;
            NETIO_SharedMemorySegment_detach(&dst->_shm_segment_hndl);
            NETIO_SharedMemoryMutex_detach(&dst->_shmem_mutex_hndl);
            NETIO_SharedMemorySignalingSemaphore_detach(&dst->_shm_sem_hndl);
        }

        if (!dst->is_valid)
        {
            /* have to do lazy initialization */
            if (!CoreShmTransport_attach_writer(me, dst, dest_port_in))
            {
                return RTI_FALSE;
            }
            else
            {
                dst->is_valid = RTI_TRUE;
            }
        }
    }

    if (!dst->is_valid)
    {
        /* The segment is not valid and we have to return */
        return RTI_FALSE;
    }

    /* Enter critical section for shared memory queue
     */
    if (!NETIO_SharedMemoryMutex_lock(&dst->_shmem_mutex_hndl, &fail_reason))
    {
        if (fail_reason != OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
        {
            NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(
                    OSAPI_LOGKIND_ERROR,
                    fail_reason);
        }
        return RTI_FALSE;
    }

    if (packet->head_pbuf != NULL)
    {
        pbuf = packet->head_pbuf;
        total_size = 0;
        while (pbuf)
        {
            total_size += NETIO_PacketBuffer_get_length(pbuf);
            pbuf = pbuf->_next;
        }
    }
    else
    {
        total_size = NETIO_Packet_get_payload_length(packet);
    }

    ok = NETIO_SHMEM_ConcurrentQueue_start_write_ea(
            &dst->_concurrent_queue,
            &cq_finish_hndl,
            &cq_where_to,
            (RTI_INT32)total_size,
            pid);

    if (!NETIO_SharedMemoryMutex_unlock(
            &dst->_shmem_mutex_hndl,
            &fail_reason))
    {
        if (fail_reason != OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
        {
            NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(
                    OSAPI_LOGKIND_ERROR,
                    fail_reason);
        }
        return RTI_FALSE;
    }

    /* Reserved space to write in the concurrent queue
     */
    if (!ok)
    {
        /* Even if write failed i.e. because the queue was full
         * the signal should be sent anyway. This is to avoid that a
         * stuck writer blocks the reader and fill up the queue.
         * Errors from this signal are ignored, since we're already under
         * an error condition.
         */

        NETIO_SHMEM_LOG_QUEUE_FULL(
                OSAPI_LOGKIND_WARNING,
                dest_port_in,
                me->_property.received_message_count_max,
                me->_property.receive_buffer_size);
        goto done;
    }

    if (packet->head_pbuf != NULL)
    {
        pbuf = packet->head_pbuf;
        while (pbuf)
        {
            pbuf_length = NETIO_PacketBuffer_get_length(pbuf);

            OSAPI_Memory_copy(cq_where_to,
                              NETIO_PacketBuffer_get_head(pbuf),
                              pbuf_length);

            cq_where_to += pbuf_length;
            pbuf = pbuf->_next;
        }
    }
    else
    {
        OSAPI_Memory_copy(cq_where_to,
                          NETIO_Packet_get_head(packet),
                          total_size);
    }

    /* There is no need to protect this section because queue operations are
     * already thread-safe.
     */
    NETIO_SHMEM_ConcurrentQueue_finish_write(
            &dst->_concurrent_queue,
            cq_finish_hndl,
            (RTI_INT32)total_size);

    return_value = RTI_TRUE; /* Success */

done:

    /* Signals to the receivers is sent when this function exit. The
     * signal is sent every time, even in case of failure, to allow the
     * writer to unblock as often as possible (to check for example for
     * stuck writers).
     */
    if (!NETIO_SharedMemorySignalingSemaphore_signal(
            &dst->_shm_sem_hndl,
            &fail_reason))
    {
        if (fail_reason != OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
        {
            NETIO_SHMEM_LOG_SIG_SEM_SIGNAL_FAILED(OSAPI_LOGKIND_ERROR);
        }
        return_value = RTI_FALSE;
    }

    return return_value;
}

RTI_BOOL
CoreShmTransport_recv_from(
        struct NETIO_SHMEMTransport *self,
        struct SHMEM_RecvResource *recv_resource,
        char **buffer,
        RTI_INT32 *cq_handle,
        RTI_INT32 *length)

{
    RTI_INT32 fail_reason;

    /* Number of  times we waited for a message */
    RTI_INT32 num_wait;

    /* These queue state infos are used to detect stuck writers */
    struct NETIO_SHMEM_ConcurrentQueueStateInfo curr_queue_info =
                                    NETIO_SHMEM_ConcurrentQueueStateInfo_INITIALIZER;
    struct NETIO_SHMEM_ConcurrentQueueStateInfo prev_queue_info =
                                    NETIO_SHMEM_ConcurrentQueueStateInfo_INITIALIZER;

    /* The timestamp taken the first time we wake up and find an empty queue */
    struct DDS_Duration_t start_timestamp;

    for (num_wait = 0;; ++num_wait)
    {
        /*
         * We get the pointer containing the message to be read
         */
        *length = NETIO_SHMEM_ConcurrentQueue_start_read_ea(
                &(recv_resource->concurrent_q),
                cq_handle,
                buffer,
                NETIO_SHMEM_CONCURRENT_QUEUE_COOKIE_INVALID);

        if (*length > 0)
        {
            OSAPI_TRACE_NET("bytes received:", RTI_FALSE);
            OSAPI_TRACE_INT32("count", *length, RTI_TRUE);
            goto success;
        }

        if (num_wait == 2)
        {
            /* First time I woke up from a wait and I found that
             * I cannot read.
             */
            NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(
                    &recv_resource->concurrent_q,
                    &curr_queue_info);

            if (!OSAPI_System_get_ticktime(
                    &start_timestamp.sec,
                    &start_timestamp.nanosec))
            {
                NETIO_SHMEM_LOG_FAILED_TO_GET_TIMESTAMP(OSAPI_LOGKIND_ERROR);
            }
        }
        else if (num_wait > 2)
        {
            /* It's more than one time that I woke up from a wait
             * and I found that I still cannot read.
             */
            prev_queue_info = curr_queue_info;
            NETIO_SHMEM_ConcurrentQueue_get_queue_state_info_read_ea(
                    &recv_resource->concurrent_q,
                    &curr_queue_info);
            if (NETIO_SHMEM_ConcurrentQueue_is_writer_potentially_stuck(
                    &recv_resource->concurrent_q,
                    &prev_queue_info,
                    &curr_queue_info) == RTI_TRUE)
            {
                /* There is a writer potentially stuck */
                /* Check if the writer is still alive */
                if (OSAPI_Process_is_alive(
                        (OSAPI_ProcessId)curr_queue_info._next_to_be_read_msg_writer_cookie))
                {
                    struct DDS_Duration_t now_timestamp;

                    if (!OSAPI_System_get_ticktime(
                            &now_timestamp.sec,
                            &now_timestamp.nanosec))
                    {
                        NETIO_SHMEM_LOG_FAILED_TO_GET_TIMESTAMP(
                                OSAPI_LOGKIND_ERROR);
                    }

                    if (DDS_Duration_delta_gt(
                            &self->_property.max_allowed_writer_send_duration,
                            &now_timestamp,
                            &start_timestamp))
                    {
                        CoreShmTransport_destuck_writer_read_ea(
                                &recv_resource->concurrent_q,
                                &curr_queue_info);

                        /* I've de-stuck the writer,
                         * I want to retry to read immediately */
                        continue;
                    }
                    else
                    {
                        /* else we don't have a clock, keep sleeping */
                    }
                }
                else
                {
                    /* Writer process is dead */
                    CoreShmTransport_destuck_writer_read_ea(
                            &recv_resource->concurrent_q,
                            &curr_queue_info);
                    /* I've de-stuck the writer, I want to retry immediately */
                    continue;
                }
            }
            else
            {
                /* No data and no writers stuck ==> this should never happen
                 * setting numWait==0 and going to the beginning of the loop
                 * starts the process again.
                 */
                num_wait = 0;
                continue;
            }
        } /* else numWait > 1 */

        if (recv_resource->_is_force_unblocked)
        {
            /* We either cannot block or woke up (due to a forced unblock)
             * No need to set buffer.length as is already 0
             * success with zero data
             */
            goto success;
        }

        if (!NETIO_SharedMemorySignalingSemaphore_wait(
                &recv_resource->_shm_semaphore,
                &fail_reason))
        {
            OSAPI_TRACE_NET("bytes received:", RTI_FALSE);
            OSAPI_TRACE_INT32("count", 0, RTI_TRUE);

            if (fail_reason == OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY)
            {
                /* Perhaps not a fatal failure. Treat as an unblock */
                NETIO_SHMEM_LOG_NONFATAL_SIG_SEM_RC(OSAPI_LOGKIND_WARNING);
                goto success;
            }
            else
            {
                NETIO_SHMEM_LOG_FAILED_TAKE_SIGN_SEM(OSAPI_LOGKIND_ERROR);
                goto fail;
            }
        }

    } /* for (;;) */

success:
    return RTI_TRUE;

fail:
    return RTI_FALSE;

}
