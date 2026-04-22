/*
 * FILE: posixShmSegment.c
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

#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#if defined(RTI_VXWORKS) && defined(RTI_64BIT) && !defined(RTI_RTP)
#define POSIX_PID_TYPE RTI_UINT64
#else
#define POSIX_PID_TYPE RTI_UINT32
#endif

#define NETIO_SharedMemorySegment_is_task_alive(handle, key) \
    OSAPI_Process_is_alive((OSAPI_ProcessId)((handle)->ptr_header->owner_pid))

#define NETIO_SharedMemorySegment_is_task_myself(handle, pid_in) \
    (handle)->ptr_header->owner_pid == (POSIX_PID_TYPE)(pid_in)

#define NETIO_SharedMemorySegment_take_ownership_os(handle, pid_in, key) \
    (handle)->ptr_header->owner_pid = (POSIX_PID_TYPE)(pid_in)


#define OSAPI_SHARED_MEMORY_NAME_LENGTH     (64)
#define NETIO_SharedMemory_MACRO_KEY2STRING(name, key) \
    snprintf(                                          \
        name,                                          \
        OSAPI_SHARED_MEMORY_NAME_LENGTH,               \
        "/RTIOsapiSharedMemorySegment-%x",             \
        key)

#if defined(RTI_LYNXOS_SE)
#define MMAP_PROT_ARG (PROT_READ | PROT_WRITE)
#else
#define MMAP_PROT_ARG (PROT_READ | PROT_WRITE | PROT_EXEC)
#endif

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemorySegment_create_or_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 segment_key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in)
{
    RTIBool return_code = RTI_FALSE;

    return_code = NETIO_SharedMemorySegment_create_impl(
            h_impl,
            status_out,
            segment_key,
            size,
            pid_in);
    if (return_code == RTI_TRUE)
    {
        /* Created, no need to do further research */
        NETIO_SharedMemorySegment_take_ownership_os(
                h_impl,
                pid_in,
                segment_key);
        goto done;
    }

    if (*status_out == OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN)
    {
        /* Creation failed, there's nothing more we can do */
        goto done;
    }

    /* Failed because the segment exists. Attempt to attach */
    if (!NETIO_SharedMemorySegment_attach_impl(h_impl, status_out, segment_key))
    {
        /* attach attempt failed: error code and log already set */
        goto done;
    }

    /* Not myself: is the owner still alive? */
    if (NETIO_SharedMemorySegment_is_task_alive(h_impl, segment_key))
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
    NETIO_SharedMemorySegment_take_ownership_os(h_impl, pid_in, segment_key);
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
        OSAPI_ProcessId pidIn)
{
    char memName[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    int errnum;
    int shm_handle = -1;

    NETIO_SharedMemory_MACRO_KEY2STRING(memName, key);

    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

    shm_handle = shm_open(memName, O_CREAT | O_RDWR | O_EXCL, 00666);
    if (shm_handle != -1)
    {
        /* Calculate the effective size of memory to allocate. The size must
         * be a multiple of the processor's page size.
         */
        char *sharedMemory;
        int total_size = NETIO_SharedMemorySegment_MACRO_GETSIZE(size);
        long sys_page_size = sysconf(_SC_PAGESIZE);
        int page_size;
        int num_pages;

        if ((sys_page_size < 0) || (sys_page_size > INT_MAX))
        {
            return RTI_FALSE;
        }

        page_size = (int)sys_page_size;
        num_pages = (total_size / page_size) + 1;
        total_size = num_pages * page_size;


        /* Defines the size of the created shmem */
        if (ftruncate(shm_handle, total_size))
        {
            errnum = errno;
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_FTRUNCATE(OSAPI_LOGKIND_ERROR, errnum)
            close(shm_handle);
            return RTI_FALSE;
        }

        sharedMemory = (char*)mmap(
                0,          /* Addr: not supported */
                (size_t)total_size, /* Size of memory to map: all */
                MMAP_PROT_ARG,
                MAP_SHARED,
                shm_handle,
                0);

        if (sharedMemory == MAP_FAILED)
        {
            errnum = errno;
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
            close(shm_handle);
            return RTI_FALSE;
        }
        handle->ptr_header =
                (struct NETIO_SharedMemorySegmentHeader*)sharedMemory;
        handle->ptr_user_data = sharedMemory +
                sizeof(struct NETIO_SharedMemorySegmentHeader);
        handle->ptr_header->size = size;
        handle->ptr_header->owner_pid = (RTI_UINT32)pidIn;
        handle->ptr_header->key = key;
        handle->ptr_header->allocated_size = total_size;

        *status_out = OSAPI_SHARED_MEMORY_CREATED;

        OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

        return RTI_TRUE;
    }

    /* shm_open failed, check why... */
    errnum = errno;
    if (errnum != EEXIST)
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
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
    char *sharedMemory;
    long sys_page_size;
    int page_size; /* Memory page size */
    int total_size; /* Total size of memory allocated */
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    int errnum;
    int shm_handle = -1;

    NETIO_SharedMemory_MACRO_KEY2STRING(name, key);
    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

    shm_handle = shm_open(name, O_RDWR, 0);

    if (shm_handle == -1)
    {
        errnum = errno;
        if (errnum == ENOENT)
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
            OSAPI_LOG_SEGMENT_DOESNT_EXIST(OSAPI_LOGKIND_INFO, key)
        }
        else
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
        }
        return RTI_FALSE;
    }

    /* First map only the first page, so total_size can be retrieved */
    sys_page_size = sysconf(_SC_PAGESIZE);
    if ((sys_page_size < 0) || (sys_page_size > INT_MAX))
    {
        return RTI_FALSE;
    }

    page_size = (int)sys_page_size;

    sharedMemory = (char*)mmap(
            0,                      /* Addr */
            (size_t)page_size, /* Size of memory to map */
            MMAP_PROT_ARG,
            MAP_SHARED,
            shm_handle,
            0);

    if (sharedMemory == MAP_FAILED)
    {
        errnum = errno;
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
        close(shm_handle);
        return RTI_FALSE;
    }

    /* Excellent, memory mapped. At the top of the segment I can find the
     * header, containing the total size of the created segment.
     */
    total_size = ((struct NETIO_SharedMemorySegmentHeader*)sharedMemory)
            ->allocated_size;
    if (total_size > page_size)
    {
        /* Un-map the mapped page if the real size is bigger than page_size */
        if (munmap(sharedMemory, (size_t)page_size) == -1)
        {
            errnum = errno;
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum)
            close(shm_handle);
            return RTI_FALSE;
        }

        /* Re-Map the area, this time the correct size */
        sharedMemory = (char*)mmap(
                0,                     /* Addr: not supported */
                (size_t)total_size, /* Size of memory to map: all */
                MMAP_PROT_ARG,
                MAP_SHARED,
                shm_handle,
                0);

        if (sharedMemory == MAP_FAILED)
        {
            errnum = errno;
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
            close(shm_handle);
            return RTI_FALSE;
        }
    }
    handle->ptr_header =
            (struct NETIO_SharedMemorySegmentHeader*)sharedMemory;
    handle->ptr_user_data = sharedMemory
            + sizeof(struct NETIO_SharedMemorySegmentHeader);
    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));
    *status_out = OSAPI_SHARED_MEMORY_ATTACHED;
    return RTI_TRUE;
}

/* The POSIX implementation uses the following properties of the handle
 * to determine the state of a shared memory segment:
 *  - ptr_header != NULL & native_handle != invalid ?
 *              -> The memory is valid and attached
 *  - ptr_header == NULL & native_handle != invalid ?
 *              -> The memory is detached but not deleted. In this case the
 *                 value of the key is copied in the native_handle
 *  - ptr_header == NULL & native_handle == invalid ?
 *              -> Memory is detached *AND* deleted.
 */
RTIBool
NETIO_SharedMemorySegment_detach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        RTIBool clearPid)
{
    int key;
#if OSAPI_ENABLE_LOG
    int errnum;
#endif
    int shm_handle = -1;

    OSAPI_Memory_copy(&shm_handle,&handle->native_handle,sizeof(shm_handle));

    if (handle->ptr_header == NULL)
    {
        return RTI_FALSE;
    }
    if (clearPid)
    {
        handle->ptr_header->owner_pid = 0;
    }
    key = handle->ptr_header->key;
    if (munmap(handle->ptr_header, (size_t)handle->ptr_header->allocated_size) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    if (close(shm_handle))
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_CLOSE(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    handle->ptr_header = NULL;

    /* The native handle change its meaning now, it is used to store the key.
     */
    OSAPI_Memory_copy(&handle->native_handle,&key,sizeof(shm_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySegment_delete_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *handle)
{
    char memName[OSAPI_SHARED_MEMORY_NAME_LENGTH];
#if OSAPI_ENABLE_LOG
    int errnum;
#endif
    int shm_handle = -1;

    OSAPI_Memory_copy(&shm_handle,&handle->native_handle,sizeof(shm_handle));

    if (shm_handle == -1)
    {
        /* Already deleted */
        return RTI_FALSE;
    }

    if (handle->ptr_header != NULL)
    {
        /* Not detached yet, do it now */
        NETIO_SharedMemorySegment_detach_impl(handle, RTI_TRUE);
    }

    /* Detached, I get the key from the native_handle
     * copy the native handle back to shm_handle since it has
     * changed
     */
    OSAPI_Memory_copy(&shm_handle,&handle->native_handle,sizeof(shm_handle));

    NETIO_SharedMemory_MACRO_KEY2STRING(memName, shm_handle);

    /* Mark the segment as destroyed with shm_unlink().
     * The segment is only destroyed by the OS when the last handle
     * detaches from it.
     */
    if (shm_unlink(memName))
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_UNLINK(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    /* Invalidate the native handle */
    shm_handle = -1;
    OSAPI_Memory_copy(&handle->native_handle,&shm_handle,sizeof(shm_handle));

    return RTI_TRUE;
}
