/*
 * FILE: ShmSegment.c
 *
 * Copyright 2022-2025 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"

#include "osapi/osapi_heap.h"
#include "osapi/osapi_log.h"
#include "netio_shmem/netio_shmem.h"

/*** SOURCE_BEGIN ***/

RTI_BOOL
NETIO_SharedMemorySegment_create_or_attach(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 segment_key,
        RTI_UINT32 size,
        OSAPI_ProcessId pid_in)
{
    RTI_BOOL success = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;

    OSAPI_PRECONDITION(
            handle == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE); )

    if (size > NETIO_SharedMemorySegment_get_max_size())
    {
        OSAPI_LOG_SHMEM_SIZE_EXCEEDED(OSAPI_LOGKIND_ERROR,size);
        local_status = OSAPI_SHARED_MEMORY_REQUESTED_SEGMENT_TOO_BIG;
        goto done;
    }

    /* Jump to the platform-specific implementation */
    success = NETIO_SharedMemorySegment_create_or_attach_impl(
            (struct NETIO_SharedMemorySegmentHandleImpl*)handle,
            &local_status,
            segment_key,
            (RTI_INT32)size,
            pid_in);

done:

    if (success)
    {
        if (local_status == OSAPI_SHARED_MEMORY_CREATED)
        {
            OSAPI_TRACE_SHMEM("segment create", RTI_FALSE)
            OSAPI_TRACE_INT32("key", segment_key, RTI_TRUE);
        }

        if (local_status == OSAPI_SHARED_MEMORY_ATTACHED)
        {
            OSAPI_TRACE_SHMEM("attached to segment", RTI_FALSE)
            OSAPI_TRACE_INT32("key", segment_key, RTI_TRUE);
        }
    }

    if (status_out != NULL)
    {
        *status_out = local_status;
    }

    return success;
}

RTI_BOOL
NETIO_SharedMemorySegment_create(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_UINT32 size,
        OSAPI_ProcessId pid_in)
{
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;
    RTI_BOOL return_code = RTI_FALSE;

    OSAPI_PRECONDITION(
            handle == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE); )

    if (size > NETIO_SharedMemorySegment_get_max_size())
    {
        local_status = OSAPI_SHARED_MEMORY_REQUESTED_SEGMENT_TOO_BIG;
        OSAPI_LOG_SHMEM_SIZE_EXCEEDED(OSAPI_LOGKIND_ERROR, size);
        goto done;
    }

    /* Jump to the platform-specific implementation */
    return_code = NETIO_SharedMemorySegment_create_impl(
            (struct NETIO_SharedMemorySegmentHandleImpl*)handle,
            &local_status,
            key,
            (RTI_INT32)size,
            pid_in);

done:

    if (status_out != NULL)
    {
        *status_out = local_status;
    }

    if (return_code == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("segment create", RTI_FALSE)
        OSAPI_TRACE_INT32("key", key, RTI_TRUE);
    }
    return return_code;
}

RTI_BOOL
NETIO_SharedMemorySegment_attach(
        struct NETIO_SharedMemorySegmentHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;
    RTI_BOOL return_code = RTI_FALSE;

    OSAPI_PRECONDITION(
            handle == NULL,
            goto done,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE); )

    /* Jump to the platform-specific implementation */
    return_code = NETIO_SharedMemorySegment_attach_impl(
            (struct NETIO_SharedMemorySegmentHandleImpl*)handle,
            &local_status,
            key);

#if OSAPI_ENABLE_PRECONDITION
done:
#endif
    if (status_out != NULL)
    {
        *status_out = local_status;
    }

    if (return_code == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("attached to segment", RTI_FALSE)
        OSAPI_TRACE_INT32("key", key, RTI_TRUE);
    }
    return return_code;
}

RTI_BOOL
NETIO_SharedMemorySegment_detach(struct NETIO_SharedMemorySegmentHandle *handle)
{
    RTI_BOOL return_code;
    struct NETIO_SharedMemorySegmentHandleImpl *h_impl;

    OSAPI_PRECONDITION(
            handle == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE); )
    h_impl = (struct NETIO_SharedMemorySegmentHandleImpl*)handle;

    /* Jump to the platform-specific implementation */
    return_code = NETIO_SharedMemorySegment_detach_impl(h_impl, RTI_FALSE);
    if (return_code == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("detached from segment", RTI_TRUE);
    }
    return return_code;
}

RTI_BOOL
NETIO_SharedMemorySegment_delete(struct NETIO_SharedMemorySegmentHandle *handle)
{
    RTI_BOOL return_code;
    struct NETIO_SharedMemorySegmentHandleImpl *h_impl;

    OSAPI_PRECONDITION(
            handle == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE); )
    h_impl = (struct NETIO_SharedMemorySegmentHandleImpl*)handle;

    /* Jump to the platform-specific implementation */
    return_code = NETIO_SharedMemorySegment_delete_impl(h_impl);
    if (return_code == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("deleted segment", RTI_TRUE);
    }
    return return_code;
}

char*
NETIO_SharedMemorySegment_get_address(
        struct NETIO_SharedMemorySegmentHandle *handle)
{
    return ((struct
           NETIO_SharedMemorySegmentHandleImpl*)handle)->ptr_user_data;
}
