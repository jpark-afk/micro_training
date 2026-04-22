/*
 * FILE: stubShmSegment.c - shared memory segment stubbed implementation
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rti_me_psl.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"


/*** SOURCE_BEGIN ***/


RTI_BOOL
OSAPI_SharedMemory_is_robust_mutex_supported(void)
{
    return RTI_FALSE;
}


RTI_UINT64
OSAPI_SharedMemorySegment_get_max_size()
{
    return 0;
}

RTI_BOOL
OSAPI_SharedMemorySegment_is_owner_alive(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        RTI_BOOL *alive)
{
    UNUSED_ARG(handle);
    UNUSED_ARG(alive);
    return RTI_FALSE;
}

RTI_SIZE_T
OSAPI_SharedMemorySegmentHandle_get_size(OSAPI_SHMEM_MODE mode)
{
    UNUSED_ARG(mode);
    return 0;
}

RTI_SIZE_T
OSAPI_SharedMemorySegmentHeader_get_size(OSAPI_SHMEM_MODE mode)
{
    UNUSED_ARG(mode);
    return 0;
}

RTI_BOOL
OSAPI_SharedMemorySegment_exists(const char *name)
{
    UNUSED_ARG(name);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_create_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        RTI_UINT64 size,
        OSAPI_SHMEM_MODE mode,
        RTI_BOOL *exists)
{
    UNUSED_ARG(handle);
    UNUSED_ARG(name);
    UNUSED_ARG(size);
    UNUSED_ARG(mode);
    UNUSED_ARG(exists);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_attach_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        OSAPI_SHMEM_MODE mode,
        OSAPI_SHMEM_ATTACH_STATUS *status_out)
{
    UNUSED_ARG(handle);
    UNUSED_ARG(name);
    UNUSED_ARG(mode);
    UNUSED_ARG(status_out);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_detach_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    UNUSED_ARG(handle);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_delete_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    UNUSED_ARG(handle);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_lock_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        OSAPI_SHMEM_STATUS *status_out)
{
    UNUSED_ARG(handle);
    UNUSED_ARG(status_out);
    return RTI_FALSE;
   
}

RTI_BOOL
OSAPI_SharedMemorySegment_unlock_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    UNUSED_ARG(handle);
    return RTI_FALSE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_mark_consistent_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle)
{
    UNUSED_ARG(handle);
    return RTI_FALSE;
}

