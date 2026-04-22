/*
 * FILE: sysvShmSegment.c
 *
 * Copyright 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"
#include "rti_me_psl/netio/netio_shmem_segment.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#define NETIO_SharedMemorySegment_is_task_alive(handle, key) \
    OSAPI_Process_is_alive((OSAPI_ProcessId)((handle)->ptr_header->owner_pid))

#define NETIO_SharedMemorySegment_is_task_myself(handle, pid_in) \
    (handle)->ptr_header->owner_pid == (RTI_UINT32)(pid_in)


#define NETIO_SharedMemorySegment_take_ownership_os(handle, pid_in, key) \
    (handle)->ptr_header->owner_pid = (RTI_UINT32)(pid_in)

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemorySegment_create_or_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 seg_key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in)
{
    RTIBool return_code = RTI_FALSE;

    return_code = NETIO_SharedMemorySegment_create_impl(
            h_impl,
            status_out,
            seg_key,
            size,
            pid_in);
    if (return_code == RTI_TRUE)
    {
        /* Created, no need to do further research */
        NETIO_SharedMemorySegment_take_ownership_os(h_impl, pid_in, seg_key);
        goto done;
    }

    if (*status_out == OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN)
    {
        /* Creation failed, there's nothing more we can do */
        goto done;
    }

    /* Failed because the segment exists. Attempt to attach */
    if (!NETIO_SharedMemorySegment_attach_impl(h_impl, status_out, seg_key))
    {
        /* attach attempt failed: error code and log already set */
        goto done;
    }

    /* Not myself: is the owner still alive? */
    if (NETIO_SharedMemorySegment_is_task_alive(h_impl, seg_key))
    {
        /* Yes, the owner is still alive, cannot recycle it. */
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
        NETIO_SharedMemorySegment_detach_impl(h_impl, RTI_FALSE);
        goto done;
    }

    /* The memory was already allocated, and the creator process is dead.
     * we recycle it and I mark it like I become the owner.
     */
    *status_out = OSAPI_SHARED_MEMORY_ATTACHED;
    NETIO_SharedMemorySegment_take_ownership_os(h_impl, pid_in, seg_key);
    return_code = RTI_TRUE;

done:

    return return_code;
}

RTIBool
NETIO_SharedMemorySegment_create_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in)
{
    /* In SystemV, the size requested doesn't have to be page aligned.
     * shmget will do the alignment for us. As result, the physical
     * buffer allocated may be bigger than the one requested.
     */
    RTI_INT32 alloc_size = NETIO_SharedMemorySegment_MACRO_GETSIZE(size);
#if OSAPI_ENABLE_LOG
    RTI_INT32 errnum;
#endif
    int shm_handle = -1;

    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

    shm_handle = shmget(
            key,
            (size_t)alloc_size,
            (IPC_CREAT | IPC_EXCL | 00666));

    if (shm_handle != -1)
    {
        OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

        /* Success: shared memory created */
        char *shm_seg = (char*)shmat(shm_handle, 0, 0);
        if (shm_seg == (char*)-1)
        {
#if OSAPI_ENABLE_LOG
            errnum = errno;
            OSAPI_LOG_SEGMENT_CREATED(OSAPI_LOGKIND_ERROR, errnum)
#endif
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            return RTI_FALSE;
        }

        handle->ptr_header = OSAPI_Compiler_reinterpret_cast(
                                    struct NETIO_SharedMemorySegmentHeader*,
                                    shm_seg);

        handle->ptr_user_data = shm_seg + sizeof(struct NETIO_SharedMemorySegmentHeader);
        handle->ptr_header->size = size;
        NETIO_SharedMemorySegment_take_ownership_os(handle,pid_in,key);
        handle->ptr_header->key = key;
        handle->ptr_header->allocated_size = alloc_size;

        *status_out = OSAPI_SHARED_MEMORY_CREATED;
        return RTI_TRUE;
    }

    if (errno == ENOSPC)
    {
        /* Out of space/memory/reached sysV shm limit */
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        OSAPI_LOG_SEGMENT_CREATED(OSAPI_LOGKIND_ERROR, errno)
    }
    else
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
        OSAPI_LOG_SEGMENT_ALREADY_EXISTS(OSAPI_LOGKIND_INFO, key)
    }

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySegment_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    char *shared_memory;
    RTI_INT32 errnum;
    int shm_handle = -1;

    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

    shm_handle = shmget(key, 0, 0);
    if (shm_handle == -1)
    {
        /* Error opening the native handle */
        errnum = errno;
        if (errnum == ENOENT)
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
            OSAPI_LOG_SEGMENT_DOESNT_EXIST(OSAPI_LOGKIND_INFO, key)

        }
        else
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_ATTACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        }
        
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));
    /* Success getting the shared memory id, need to attach now */
    shared_memory = (char*)shmat(shm_handle, 0, 0);
    if (shared_memory == (char*)-1)
    {
        errnum = errno;
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        OSAPI_LOG_SHMEM_ATTACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)

        /* No need to do any close. shmget doesn't allocate anything */
        return RTI_FALSE;
    }

    handle->ptr_header = OSAPI_Compiler_reinterpret_cast(
                                    struct NETIO_SharedMemorySegmentHeader*,
                                    shared_memory);

    handle->ptr_user_data = shared_memory
                            + sizeof(struct NETIO_SharedMemorySegmentHeader);
    *status_out = OSAPI_SHARED_MEMORY_ATTACHED;
    return RTI_TRUE;
}

/*
 * Differently from POSIX, SytemV requires the native handle to destroy the
 * segment, so, just like the Windows implementation, after detaching we
 * clear the header_ptr, but we don't touch the native handle.
 */
RTIBool
NETIO_SharedMemorySegment_detach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        RTIBool clear_pid)
{
#if OSAPI_ENABLE_LOG
    RTI_INT32 errnum;
#endif

    if (handle->ptr_header == NULL)
    {
        return RTI_FALSE;
    }
    if (clear_pid)
    {
        handle->ptr_header->owner_pid = 0;
    }
    if (shmdt((char*)handle->ptr_header) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SEGMENT_DETACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    handle->ptr_header = NULL;
    return RTI_TRUE;
}

/*
 * Returns RTI_FALSE if delete fails. Do not log an error in this case.
 * A failure in the detach is ignored.
 */
RTIBool
NETIO_SharedMemorySegment_delete_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle)
{
    int shm_handle;

    OSAPI_Memory_copy(&shm_handle,&handle->native_handle,sizeof(shm_handle));

    if (shm_handle == -1)
    {
        return RTI_FALSE;
    }

    if (handle->ptr_header != NULL)
    {
        /* Not detached yet, do it now */
        NETIO_SharedMemorySegment_detach_impl(handle, RTI_TRUE);
    }

    /* Mark the segment as destroyed with shmctl(), which is different than
     * detaching with shmdt(). That's why we check. The segment is only
     * destroyed by the OS when the last handle detaches from it.
     */
    if (shmctl(shm_handle, IPC_RMID, NULL) == -1)
    {
        /* Do not log any error now, but let the upper layers handle it */
        return RTI_FALSE;
    }

    shm_handle = -1;

    OSAPI_Memory_copy(&shm_handle,&handle->native_handle,sizeof(shm_handle));

    return RTI_TRUE;
}
