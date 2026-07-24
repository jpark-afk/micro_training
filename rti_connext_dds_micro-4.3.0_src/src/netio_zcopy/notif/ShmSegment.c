/*
 * FILE: ShmSegment.c - Shared memory segment implementation
 *
 * Copyright 2022-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"


/*** SOURCE_BEGIN ***/

struct OSAPI_SharedMemorySegmentHandle *
OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE mode)
{
    struct OSAPI_SharedMemorySegmentHandle *handle = NULL;

    if ((mode != OSAPI_SHMEM_MODE_READ) && (mode != OSAPI_SHMEM_MODE_WRITE) &&
        (mode != OSAPI_SHMEM_MODE_LOCKABLE) && (mode != OSAPI_SHMEM_MODE_ROBUST))
    {
        return NULL;
    }

    /* Allocate memory */
    handle = OSAPI_Heap_allocate(1, OSAPI_SharedMemorySegmentHandle_get_size(mode));
    if (handle == NULL)
    {
        return NULL;
    }
    handle->ptr_header = NULL;
    handle->ptr_user_data = NULL;
    handle->mode = mode;

    return handle;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_SharedMemorySegmentHandle_delete(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    if (handle == NULL)
    {
        return RTI_FALSE;
    }

    /* Check if attached and detach */
    if (OSAPI_SharedMemorySegmentHandle_is_attached(handle))
    {
        if (!OSAPI_SharedMemorySegment_detach(handle))
        {
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free(handle);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
OSAPI_SharedMemorySegment_create(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        RTI_UINT64 size)
{
    RTI_BOOL success = RTI_FALSE;
    RTI_BOOL exists = RTI_FALSE;
    RTI_BOOL alive = RTI_FALSE;
    RTI_BOOL retval;
    RTI_SIZE_T name_length;
    OSAPI_SHMEM_ATTACH_STATUS shmem_status;

    /* Check preconditions */
    OSAPI_PRECONDITION((handle == NULL) || (name == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_string("name", name, RTI_TRUE);)

    if (OSAPI_SharedMemorySegmentHandle_is_attached(handle))
    {
        OSAPI_LOG_SHMEM_ALREADY_ATTACHED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    if (size > OSAPI_SharedMemorySegment_get_max_size())
    {
        OSAPI_LOG_SHMEM_SIZE_EXCEEDED(OSAPI_LOGKIND_ERROR, size);
        goto done;
    }

    name_length = OSAPI_String_length(name);
    if (name_length > OSAPI_SHMEM_MAX_NAME_LENGTH)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, name)
        goto done;
    }

    /* Created segment must have write permissions to write header */
    if (handle->mode < OSAPI_SHMEM_MODE_WRITE)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE(OSAPI_LOGKIND_ERROR, name, handle->mode)
        goto done;
    }

    /* Jump to the platform-specific implementation */
    success = OSAPI_SharedMemorySegment_create_impl(
            handle,
            name,
            size,
            handle->mode,
            &exists);
    if (success)
    {
        goto done;
    }
    if (!exists)
    {
        /* Creation failed for unknown reason, there's nothing more we can do */
        goto done;
    }

    /* Failed because the segment exists. Attempt to recycle it.
     *   First attach to it, to see what is going on */
    success = OSAPI_SharedMemorySegment_attach_impl(
        handle, name, handle->mode, &shmem_status);
    if (!success || (shmem_status != OSAPI_SHMEM_ATTACH_STATUS_OK))
    {
        /* attach attempt failed: error code and log already set */
        goto done;
    }

    /* We were able to attach to the segment, so now check to see if the
     *   previous owner is still alive */
    success = OSAPI_SharedMemorySegment_is_owner_alive(handle, &alive);
    if(!success)
    {
        goto done;
    }
    if (alive)
    {
        /* Yes, the owner is still alive, cannot recycle it.
         *
         * The return value from OSAPI_SharedMemorySegment_detach() is ignored
         * below because our presence in this block already indicates a fatal
         * error.
         */
        retval = OSAPI_SharedMemorySegment_detach(handle);
        IGNORE_RETVAL(retval);
        success = RTI_FALSE;
        goto done;
    }

    /* No, the owner is no longer alive, so we will destroy and recreate
     *   the segment. */
    success = OSAPI_SharedMemorySegment_delete(handle);
    if (!success)
    {
        goto done;
    }
    success = OSAPI_SharedMemorySegment_create_impl(
            handle,
            name,
            size,
            handle->mode,
            &exists);

done:
    return success;
}
RTI_BOOL
OSAPI_SharedMemorySegment_attach(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        OSAPI_SHMEM_ATTACH_STATUS *status_out)
{
    RTI_BOOL success = RTI_FALSE;
    OSAPI_SHMEM_ATTACH_STATUS shmem_status;
    RTI_BOOL retval;
    RTI_SIZE_T name_length;

    /* Check preconditions */
    OSAPI_PRECONDITION((handle == NULL) || (name == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_string("name", name, RTI_TRUE);)

    if (OSAPI_SharedMemorySegmentHandle_is_attached(handle))
    {
        OSAPI_LOG_SHMEM_ALREADY_ATTACHED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    name_length = OSAPI_String_length(name);
    if (name_length > OSAPI_SHMEM_MAX_NAME_LENGTH)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, name)
        goto done;
    }

    if (handle->mode < OSAPI_SHMEM_MODE_READ)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE(OSAPI_LOGKIND_ERROR, name, handle->mode)
        goto done;
    }

    /* Jump to the platform-specific implementation */
    if (!OSAPI_SharedMemorySegment_attach_impl(
        handle, name, handle->mode, &shmem_status))
    {
        goto done;
    }

    if (shmem_status == OSAPI_SHMEM_ATTACH_STATUS_OK)
    {
        if ((handle->mode >= OSAPI_SHMEM_MODE_LOCKABLE) &&
            (handle->ptr_header->mode != handle->mode))
        {
            OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE(OSAPI_LOGKIND_ERROR)
            /* The return value from OSAPI_SharedMemorySegment_detach() is ignored
            * below because our presence in this block already indicates a fatal
            * error.
            */
            retval = OSAPI_SharedMemorySegment_detach(handle);
            IGNORE_RETVAL(retval);
            goto done;
        }
    }
    if (status_out != NULL)
    {
        *status_out = shmem_status;
    }
    success = RTI_TRUE;
done:
    return success;
}

RTI_BOOL
OSAPI_SharedMemorySegment_detach(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    RTI_BOOL return_code;

    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    /* Check if already detached or deleted */
    if (!OSAPI_SharedMemorySegmentHandle_is_attached(handle))
    {
        OSAPI_LOG_SHMEM_NOT_ATTACHED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    /* Jump to the platform-specific implementation */
    return_code = OSAPI_SharedMemorySegment_detach_impl(handle);

    /* Invalidate handle */
    handle->ptr_header = NULL;
    handle->ptr_user_data = NULL;

    return return_code;
}

RTI_BOOL
OSAPI_SharedMemorySegment_delete(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    RTI_BOOL return_code;

    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    /* Check if already detached or deleted */
    if (!OSAPI_SharedMemorySegmentHandle_is_attached(handle))
    {
        OSAPI_LOG_SHMEM_NOT_ATTACHED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    /* Jump to the platform-specific implementation */
    return_code = OSAPI_SharedMemorySegment_delete_impl(handle);

    /* Invalidate handle */
    handle->ptr_header = NULL;
    handle->ptr_user_data = NULL;

    return return_code;
}

RTI_BOOL
OSAPI_SharedMemorySegment_lock(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        OSAPI_SHMEM_STATUS *status_out)
{
    OSAPI_PRECONDITION((handle == NULL) || (status_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status_out", status_out, RTI_TRUE);)

    if (handle->mode < OSAPI_SHMEM_MODE_LOCKABLE)
    {
        OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return OSAPI_SharedMemorySegment_lock_impl(handle, status_out);
}

RTI_BOOL
OSAPI_SharedMemorySegment_unlock(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (handle->mode < OSAPI_SHMEM_MODE_LOCKABLE)
    {
        OSAPI_LOG_SHMEM_SEGMENT_NOT_LOCKABLE(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return OSAPI_SharedMemorySegment_unlock_impl(handle);
}

RTI_BOOL
OSAPI_SharedMemorySegment_mark_consistent(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (handle->mode < OSAPI_SHMEM_MODE_ROBUST)
    {
        OSAPI_LOG_SHMEM_SEGMENT_NOT_ROBUST(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return OSAPI_SharedMemorySegment_mark_consistent_impl(handle);
}

void *
OSAPI_SharedMemorySegment_get_address(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    return handle->ptr_user_data;
}

RTI_UINT64
OSAPI_SharedMemorySegment_get_size(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    return handle->ptr_header->size;
}