/*
 * FILE: ShmSemMutex.c
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "ShmSemMutex.h"

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemorySemMutex_create(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    RTIBool ret_val = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;

    OSAPI_PRECONDITION(
            (handle == NULL ||
            (sem_type != SEMMUTEX_TYPE_SEMAPHORE &&
            sem_type != SEMMUTEX_TYPE_BINARYSEM &&
            sem_type != SEMMUTEX_TYPE_MUTEX)),
            goto done,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type", key, RTI_TRUE); )

    OSAPI_Memory_zero(
            handle,
            sizeof(struct NETIO_SharedMemorySemMutexHandle));
    {
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl =
                &handle->impl.handle;

        ret_val = NETIO_SharedMemorySemMutex_create_impl(
                h_impl,
                &local_status,
                key,
                sem_type);
        if (ret_val == RTI_TRUE)
        {
            /* Created successfully */
            OSAPI_TRACE_SHMEM("created sem/mutex", RTI_FALSE)
            OSAPI_TRACE_INT32("type", sem_type, RTI_FALSE)
            OSAPI_TRACE_INT32("key", key, RTI_TRUE)

            h_impl->_key = key;
            h_impl->_sem_type = sem_type; /* Used only in precondition tests */
            h_impl->_lock_count = 0;
            h_impl->_lock_pid.handle.handle64 = 0;
            local_status = OSAPI_SHARED_MEMORY_CREATED;
        }
    }
#if OSAPI_ENABLE_PRECONDITION
done:
#endif
    if (status_out != NULL)
    {
        *status_out = local_status;
    }
    return ret_val;
}

RTIBool
NETIO_SharedMemorySemMutex_attach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    RTIBool retval = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;

    OSAPI_PRECONDITION(
            (handle == NULL ||
            (sem_type != SEMMUTEX_TYPE_SEMAPHORE &&
            sem_type != SEMMUTEX_TYPE_BINARYSEM &&
            sem_type != SEMMUTEX_TYPE_MUTEX)),
            goto done,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type", key, RTI_TRUE); )

    OSAPI_Memory_zero(
            handle,
            sizeof(struct NETIO_SharedMemorySemMutexHandle));
    {
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl = &handle->impl
                .handle;

        retval = NETIO_SharedMemorySemMutex_attach_impl(
                h_impl,
                &local_status,
                key,
                sem_type);
        if (retval == RTI_TRUE)
        {
            /* Created successfully */
            OSAPI_TRACE_SHMEM("attached to sem/mutex", RTI_FALSE)
            OSAPI_TRACE_INT32("type", sem_type, RTI_FALSE)
            OSAPI_TRACE_INT32("key", key, RTI_TRUE)

            h_impl->_key = key;
            h_impl->_sem_type = sem_type;
            h_impl->_lock_count = 0;
            h_impl->_lock_pid.handle.handle64 = 0;
            local_status = OSAPI_SHARED_MEMORY_ATTACHED;
        }
    }
#if OSAPI_ENABLE_PRECONDITION
done:
#endif
    if (status_out != NULL)
    {
        *status_out = local_status;
    }
    return retval;
}

RTIBool
NETIO_SharedMemorySemMutex_create_or_attach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    RTIBool success = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;

    OSAPI_PRECONDITION(
            (handle == NULL ||
            (sem_type != SEMMUTEX_TYPE_SEMAPHORE &&
            sem_type != SEMMUTEX_TYPE_BINARYSEM &&
            sem_type != SEMMUTEX_TYPE_MUTEX)),
            goto done,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type", key, RTI_TRUE); )


    success = NETIO_SharedMemorySemMutex_create(
            handle,
            &local_status,
            key,
            sem_type);
    if ((success == RTI_FALSE) &&
            (local_status ==
            OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED))
    {
        /* SemMutex exist, attach to it */
        success = NETIO_SharedMemorySemMutex_attach(
                handle,
                &local_status,
                key,
                sem_type);
    }
#if OSAPI_ENABLE_PRECONDITION
done:
#endif
    if (status_out != NULL)
    {
        *status_out = local_status;
    }
    return success;
}

RTIBool
NETIO_SharedMemorySemMutex_take(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    RTIBool retval = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;

    OSAPI_PRECONDITION(
            handle == NULL ||
            handle->impl.handle._sem_type != sem_type,
            goto done,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type",sem_type,RTI_TRUE); )

    {
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl =
                &handle->impl.handle;

        /* Jump to the platform-specific implementation */
        retval = NETIO_SharedMemorySemMutex_take_impl(
                h_impl,
                &local_status,
                sem_type);
        if (retval == RTI_TRUE)
        {
            local_status = OSAPI_SHARED_MEMORY_SUCCESS;
        }
    }

#if OSAPI_ENABLE_PRECONDITION
done:
#endif
    if (status_out != NULL)
    {
        *status_out = local_status;
    }
    return retval;
}

RTIBool
NETIO_SharedMemorySemMutex_give(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    RTIBool retval = RTI_FALSE;
    RTI_INT32 local_status = OSAPI_SHARED_MEMORY_FAIL_REASON_PRECONDITION;
    OSAPI_PRECONDITION(
            handle == NULL ||
            handle->impl.handle._sem_type != sem_type,
            goto done,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type",sem_type,RTI_TRUE); )

    {
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl =
                &handle->impl.handle;

        /* Jump to the platform-specific implementation */
        retval = NETIO_SharedMemorySemMutex_give_impl(
                h_impl,
                &local_status,
                sem_type);
        if (retval == RTI_TRUE)
        {
            local_status = OSAPI_SHARED_MEMORY_SUCCESS;
        }
    }
#if OSAPI_ENABLE_PRECONDITION
done:
#endif

    if (status_out != NULL)
    {
        *status_out = local_status;
    }
    return retval;
}

RTIBool
NETIO_SharedMemorySemMutex_detach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 sem_type)
{
    RTIBool retval = RTI_FALSE;
    struct NETIO_SharedMemorySemMutexHandleImpl *h_impl = NULL;
    RTI_INT32 key = -1;
    OSAPI_TRACE_ONLY_VARIABLE(key);

    OSAPI_PRECONDITION(
            handle == NULL ||
            handle->impl.handle._sem_type != sem_type,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type",sem_type,RTI_TRUE); )

    h_impl = &handle->impl.handle;
    key = h_impl->_key;

    /* Jump to the platform-specific implementation */
    retval = NETIO_SharedMemorySemMutex_detach_impl(h_impl, sem_type);
    if (retval == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("detached from sem/mutex", RTI_FALSE)
        OSAPI_TRACE_INT32("type", sem_type, RTI_FALSE)
        OSAPI_TRACE_INT32("key", key, RTI_TRUE)
    }

    return retval;
}

RTIBool
NETIO_SharedMemorySemMutex_delete(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 sem_type)
{
    RTIBool retval = RTI_FALSE;
    struct NETIO_SharedMemorySemMutexHandleImpl *h_impl = NULL;
    RTI_INT32 key = -1;
    OSAPI_TRACE_ONLY_VARIABLE(key);

    OSAPI_PRECONDITION(
            handle == NULL ||
            handle->impl.handle._sem_type != sem_type,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
            OSAPI_Log_entry_add_int("type",sem_type,RTI_TRUE); )

    h_impl = &handle->impl.handle;
    key = h_impl->_key;

    /* Jump to the platform-specific implementation */
    retval = NETIO_SharedMemorySemMutex_delete_os(h_impl, sem_type);
    if (retval == RTI_TRUE)
    {
        OSAPI_TRACE_SHMEM("deleted sem/mutex", RTI_FALSE)
        OSAPI_TRACE_INT32("type", sem_type, RTI_FALSE)
        OSAPI_TRACE_INT32("key", key, RTI_TRUE)
    }

    return retval;
}
