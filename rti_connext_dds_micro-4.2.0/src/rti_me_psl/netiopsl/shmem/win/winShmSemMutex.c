/*
 * FILE: winShmSemMutex.c
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
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

#define OSAPI_SHARED_MEMORY_NAME_LENGTH     (64)
#define NETIO_SharedMemory_MACRO_KEY2STRING(name, sem_type, key) \
    sprintf(name, "/RTIOsapiSharedMemorySemMutex%d-%x", sem_type, key)

/* NETIO_SharedMemory_setSecurity (Win32 only) {{{ */
/* ----------------------------------------------------------------- */
/*i
 * Windows 32 (NOT CE) security setting for win32 service app
 * shared memory access is from article on
 * "Default security is not always good enough" by Anne Gunn
 * http://www.codeguru.com/win32/NotQuiteNullDacl.html
 */
extern RTIBool
NETIO_SharedMemory_setSecurity(
        SECURITY_ATTRIBUTES *securityAttribute,
        SECURITY_DESCRIPTOR *securityDescriptor);

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemoryMutex_is_robust(void)
{
#if OSAPI_HAVE_POSIX_ROBUST_MUTEX
    return RTI_TRUE;
#else
    return RTI_FALSE;
#endif
}

RTIBool
NETIO_SharedMemorySemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    SECURITY_ATTRIBUTES *securityAttributePtr;
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    const char *fnName = "CreateSemaphore";
    DWORD lastError;
    HANDLE sem_handle = NULL;

    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    if (!NETIO_SharedMemory_setSecurity(
            &securityAttribute,
            &securityDescriptor))
    {
        return RTI_FALSE;
    }
    securityAttributePtr = &securityAttribute;

    NETIO_SharedMemory_MACRO_KEY2STRING(name, sem_type, key);
    /* This will do either an open or an init */
    switch (sem_type)
    {
        case SEMMUTEX_TYPE_SEMAPHORE:
            sem_handle = CreateSemaphore(
                securityAttributePtr,
                0,       /* initial count=0 as documented in API */
                MAXLONG, /* counting semaphore */
                name);

            break;

        case SEMMUTEX_TYPE_BINARYSEM:
            sem_handle = CreateSemaphore(
                securityAttributePtr,
                0, /* initial count=0 as documented in API */
                1, /* binary semaphore */
                name);
            break;

        case SEMMUTEX_TYPE_MUTEX:
            fnName = "CreateMutex";
            sem_handle = CreateMutex(
                securityAttributePtr,
                FALSE,
                name);

            break;
    }

    /* If the sem/mutex already exist, CreateSemaphore/CreateMutex will
     * NOT fail, but will return the handle to the existing semaphore
     * and GetLastError() will return ERROR_ALREADY_EXIST
     */
    lastError = GetLastError();
    if (lastError == ERROR_ALREADY_EXISTS)
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
        CloseHandle(sem_handle);
        return RTI_FALSE;
    }

    if (sem_handle == NULL)
    {
        OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED(OSAPI_LOGKIND_ERROR,lastError,sem_type)
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{

    SECURITY_ATTRIBUTES *securityAttributePtr;
    char name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    const char *fnName = "OpenSemaphore";
    HANDLE sem_handle = NULL;

    SECURITY_ATTRIBUTES securityAttribute;
    SECURITY_DESCRIPTOR securityDescriptor;

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    if (!NETIO_SharedMemory_setSecurity(
            &securityAttribute,
            &securityDescriptor))
    {
        return RTI_FALSE;
    }
    securityAttributePtr = &securityAttribute;

    NETIO_SharedMemory_MACRO_KEY2STRING(name, sem_type, key);

    /* This will do either an open or an init */
    switch (sem_type)
    {
        case SEMMUTEX_TYPE_SEMAPHORE:
            sem_handle = OpenSemaphore(
                SEMAPHORE_ALL_ACCESS,
                FALSE,
                name);
            break;

        case SEMMUTEX_TYPE_BINARYSEM:
            sem_handle = OpenSemaphore(
                SEMAPHORE_ALL_ACCESS,
                FALSE, /* inherit */
                name);
            break;

        case SEMMUTEX_TYPE_MUTEX:
            sem_handle = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);
            fnName = "OpenMutex";
            break;
    }

    if (sem_handle == NULL)
    {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_FILE_NOT_FOUND)
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            OSAPI_LOG_SHMEM_ATTACH_FAILED(OSAPI_LOGKIND_ERROR, lastError)
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        }
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    BOOL retval = FALSE;
    const char *fnName = "ReleaseSemaphore"; /* Used to report failure */
    HANDLE sem_handle;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    switch (sem_type)
    {
        case SEMMUTEX_TYPE_MUTEX:
            fnName = "ReleaseMutex";
            retval = ReleaseMutex(sem_handle);
            break;

        case SEMMUTEX_TYPE_SEMAPHORE:
        case SEMMUTEX_TYPE_BINARYSEM:
            retval = ReleaseSemaphore(sem_handle, 1, NULL);

            break;

    }
    if (!retval)
    {
        /* Check if we reached the maxlimit of the semaphore */
        DWORD lastError = GetLastError();

        if (lastError == ERROR_TOO_MANY_POSTS)
        {
            *status_out = OSAPI_SHARED_MEMORY_MAXCOUNT_REACHED;
            return RTI_TRUE;
        }

        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    int errnum;
    HANDLE sem_handle;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    if (WaitForSingleObject(sem_handle, INFINITE) == WAIT_FAILED)
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        errnum = GetLastError();
        OSAPI_LOG_SHMEM_TAKE_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
    int errnum;
    HANDLE sem_handle;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    if (!CloseHandle(sem_handle))
    {
        errnum = GetLastError();
        OSAPI_LOG_SEGMENT_DETACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemorySemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
    return NETIO_SharedMemorySemMutex_detach_impl(h_impl, sem_type);
}
