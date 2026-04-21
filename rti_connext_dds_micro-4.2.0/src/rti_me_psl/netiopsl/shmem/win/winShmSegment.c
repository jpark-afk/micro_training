/*
 * FILE: winShmSegment.c
 *
 * Copyright 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"

#include <windows.h>
#include <stdio.h>

#include "osapi/osapi_log.h"
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"
#include "rti_me_psl/netio/netio_shmem_segment.h"

#define OSAPI_SHARED_MEMORY_NAME_LENGTH     (64)
#define NETIO_SharedMemory_MACRO_KEY2STRING(name, key) \
    sprintf(name, "/RTIOsapiSharedMemorySegment/%x", key)

#define NETIO_SharedMemory_MACRO_KEY2ID(name, key) \
    sprintf(name, "/RTIOsapiSharedMemorySegmentID/%x", key)

#define NETIO_SharedMemorySegment_is_task_myself(handle, pid_in) \
    (handle)->header_ptr->owner_pid == (pid_in)

/*** SOURCE_BEGIN ***/

/* NETIO_SharedMemory_setSecurity (Win32 only) {{{ */
/* ----------------------------------------------------------------- */
/*i
 * Windows 32 (NOT CE) security setting for win32 service app
 * shared memory access is from article on
 * "Default security is not always good enough" by Anne Gunn
 * http://www.codeguru.com/win32/NotQuiteNullDacl.html
 */
RTIBool
NETIO_SharedMemory_setSecurity(
        SECURITY_ATTRIBUTES *securityAttribute,
        SECURITY_DESCRIPTOR *securityDescriptor)
{
    RTIBool ok = RTI_FALSE;
    const char *METHOD_NAME = "NETIO_SharedMemory_setSecurity";
    int errnum;

    if (!InitializeSecurityDescriptor(
            securityDescriptor,
            SECURITY_DESCRIPTOR_REVISION))
    {
        errnum = GetLastError();
        OSAPI_LOG_SHMEM_INIT_DESCRIPTOR(OSAPI_LOGKIND_ERROR, errnum)
        goto done;
    }

    if (!SetSecurityDescriptorDacl(
            securityDescriptor,
            TRUE,
            (PACL)NULL,
            FALSE))
    {
        errnum = GetLastError();
        OSAPI_LOG_SHMEM_SET_DESCRIPTOR(OSAPI_LOGKIND_ERROR, errnum)
        goto done;
    }

    /* new set up the security attributes */
    securityAttribute->nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttribute->bInheritHandle = FALSE; /* TRUE? */
    securityAttribute->lpSecurityDescriptor = securityDescriptor;
    ok = RTI_TRUE;
done:
    return ok;
}


/* NETIO_SharedMemorySegment_isTaskAlive_os {{{
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
RTI_PRIVATE RTIBool
NETIO_SharedMemorySegment_is_task_alive(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        int key)
{
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    HANDLE hMutex;
    NETIO_SharedMemory_MACRO_KEY2ID(name, key);
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);

    if (hMutex == NULL)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return RTI_FALSE;
        }
        /* Assume is another generic error and the mutex exist */
        return RTI_TRUE;
    }
    /* Success opening the mutex, close it and return */
    CloseHandle(hMutex);
    return RTI_TRUE;
}

RTI_PRIVATE void
NETIO_SharedMemorySegment_take_ownership_os(
        struct NETIO_SharedMemorySegmentHandleImpl *handle,
        OSAPI_ProcessId pidIn,
        int key)
{
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    HANDLE hMutex = NULL;
    SECURITY_ATTRIBUTES *securityAttributePtr;
    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;
    if (!NETIO_SharedMemory_setSecurity(
            &securityAttribute,
            &securityDescriptor))
    {
        securityAttributePtr = NULL;
    }
    else
    {
        securityAttributePtr = &securityAttribute;
    }

    NETIO_SharedMemory_MACRO_KEY2ID(name, key);
    hMutex = CreateMutex(
            securityAttributePtr,
            FALSE,              /* It shouldn't matter */
            name);

    /* Mutex now is created. Store the handle on the shared memory
     * segment so it can be retrieved by the _delete_os method.
     * This mutex will be closed only when the _delete_os is called or
     * by the automatic cleanup system when the process exit.
     */
    handle->ptr_header->owner_pid = *(RTI_UINT32*)((void*)&hMutex);
}

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
    if (NETIO_SharedMemorySegment_attach_impl(
            h_impl,
            status_out,
            segment_key) == RTI_FALSE)
    {
        /* attach attempt failed: error code and log already set */
        goto done;
    }

    /* Not myself: is the owner still alive? */
    if (NETIO_SharedMemorySegment_is_task_alive(h_impl, segment_key) ==
            RTI_TRUE)
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
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 size,
        OSAPI_ProcessId pidIn)
{
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    SECURITY_ATTRIBUTES *securityAttributePtr;
    char *sharedMemory;
    int errnum;
    int total_size = NETIO_SharedMemorySegment_MACRO_GETSIZE(size);
    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;
    HANDLE shm_handle = NULL;

    OSAPI_Memory_copy(&himpl->native_handle,&shm_handle,sizeof(shm_handle));

    if (!NETIO_SharedMemory_setSecurity(
            &securityAttribute,
            &securityDescriptor))
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        return RTI_FALSE;
    }

    securityAttributePtr = &securityAttribute;
    /* Convert key into a string. This mapping has been documented in the
     * inteface we must keep it consistent 
     */
    NETIO_SharedMemory_MACRO_KEY2STRING(name, key);

    /* Create the shared memory segment 
     */
    shm_handle = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            securityAttributePtr,
            PAGE_READWRITE,
            0,
            total_size,
            name);

    if (shm_handle == NULL)
    {
        errnum = GetLastError();
        OSAPI_LOG_SEGMENT_CREATED(OSAPI_LOGKIND_ERROR, errnum)
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        return RTI_FALSE;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        /* CreateFileMapping will still succeed if the segment already exist
         * so we need to close it here.
         */
        CloseHandle(shm_handle);
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
        OSAPI_LOG_SEGMENT_ALREADY_EXISTS(OSAPI_LOGKIND_INFO, key)
        return RTI_FALSE;
    }

    sharedMemory = (char*)MapViewOfFile(shm_handle,FILE_MAP_ALL_ACCESS,0,0,0);
    if (sharedMemory == NULL)
    {
        errnum = GetLastError();
        OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        return RTI_FALSE;
    }

    himpl->ptr_header = (struct NETIO_SharedMemorySegmentHeader*)sharedMemory;
    himpl->ptr_user_data = sharedMemory + 
                            sizeof(struct NETIO_SharedMemorySegmentHeader);
    himpl->ptr_header->size = size;
    himpl->ptr_header->owner_pid = (RTI_UINT32)pidIn;
    himpl->ptr_header->key = key;
    himpl->ptr_header->allocated_size = total_size;

    *status_out = OSAPI_SHARED_MEMORY_CREATED;
    OSAPI_Memory_copy(&himpl->native_handle,&shm_handle,sizeof(shm_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySegment_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    char *sharedMemory = NULL;
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    int errnum;
    NETIO_SharedMemory_MACRO_KEY2STRING(name, key);
    HANDLE shm_handle = NULL;

    OSAPI_Memory_copy(&himpl->native_handle,&shm_handle,sizeof(shm_handle));

    shm_handle = OpenFileMapping(FILE_MAP_WRITE,
                                 FALSE, /* disallow inheritance */
                                 name);
    if (shm_handle == NULL)
    {
        errnum = GetLastError();
        if (errnum == ERROR_FILE_NOT_FOUND)
        {
            if (status_out)
            {
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
                OSAPI_LOG_SEGMENT_DOESNT_EXIST(OSAPI_LOGKIND_INFO, key)
            }
        }
        else
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED(OSAPI_LOGKIND_ERROR,errnum)
        }
        return RTI_FALSE;
    }

    sharedMemory = (char*)MapViewOfFile(shm_handle,FILE_MAP_ALL_ACCESS,0,0,0);
    if (sharedMemory == NULL)
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        errnum = GetLastError();
        OSAPI_LOG_SHMEM_SEGMENT_FILE_MAP_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }

    himpl->ptr_header =
            (struct NETIO_SharedMemorySegmentHeader*)sharedMemory;
    himpl->ptr_user_data = sharedMemory
            + sizeof(struct NETIO_SharedMemorySegmentHeader);
           
    *status_out = OSAPI_SHARED_MEMORY_ATTACHED;
    OSAPI_Memory_copy(&himpl->native_handle,&shm_handle,sizeof(shm_handle));

    return RTI_TRUE;
}

/*
 * Just like the POSIX implementation we clear the ptr_header but not the
 * native handle. In this case we don't need to use the native_hndl to carry
 * the key.
 */
RTIBool
NETIO_SharedMemorySegment_detach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTIBool clearPid)
{
    int errnum;
    HANDLE shm_handle;

    OSAPI_Memory_copy(&shm_handle,&himpl->native_handle,sizeof(shm_handle));

    if (himpl->ptr_header == NULL)
    {
        return RTI_FALSE;
    }
    if (clearPid)
    {
        himpl->ptr_header->owner_pid = 0;
    }
    UnmapViewOfFile(himpl->ptr_header);
    if (!CloseHandle(shm_handle))
    {
        errnum = GetLastError();
        OSAPI_LOG_SEGMENT_DETACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

/*
 * Returns RTI_FALSE if delete fails. Do not log an error in this case.
 * A failure in the detach is ignored.
 */
RTIBool
NETIO_SharedMemorySegment_delete_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl)
{

    HANDLE hMutex = NULL;
    HANDLE shm_handle;

    OSAPI_Memory_copy(&shm_handle,&himpl->native_handle,sizeof(shm_handle));

    if (shm_handle == NULL)
    {
        /* Already deleted */
        return RTI_FALSE;
    }

    if (himpl->ptr_header != NULL)
    {
        /* Not detached yet, do it now */
        *(RTI_UINT32*)((void*)&hMutex) = himpl->ptr_header->owner_pid;

        /* Delete the mutex that hold the ownership */
        NETIO_SharedMemorySegment_detach_impl(himpl, RTI_TRUE);
        CloseHandle(hMutex);
    }

    /* Invalidate the native handle */
    shm_handle = NULL;

    OSAPI_Memory_copy(&himpl->native_handle,&shm_handle,sizeof(shm_handle));

    return RTI_TRUE;
}
