/*
 * FILE: posixShmMutex.c = Shared Memory using POSIX APIs
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
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

#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "posixShmMutex.h"

#if defined(RTI_DARWIN)

/* Mac OS X doesn't handle names that long... */
#define OSAPI_SHARED_MEMORY_NAME_LENGTH     (32)
#define NETIO_SharedMemory_MACRO_KEY2STRING(name, sem_type, key) \
    snprintf(                                                    \
        name,                                                    \
        OSAPI_SHARED_MEMORY_NAME_LENGTH,                         \
        "/RTISemx%d-%x",                                         \
        sem_type,                                                \
        key)

#else

#define OSAPI_SHARED_MEMORY_NAME_LENGTH     (64)
#define NETIO_SharedMemory_MACRO_KEY2STRING(name, sem_type, key) \
    snprintf(                                                    \
        name,                                                    \
        OSAPI_SHARED_MEMORY_NAME_LENGTH,                         \
        "/RTIOsapiSharedMemorySemMutex%d-%x",                    \
        sem_type,                                                \
        key)

#endif

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemoryPosixSemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    char sem_name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    const unsigned semaphore_init = (sem_type == SEMMUTEX_TYPE_MUTEX ? 1 : 0);
    int errnum;
    sem_t *sem_handle = NULL;

    NETIO_SharedMemory_MACRO_KEY2STRING(sem_name, sem_type, key);

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    /* Creates and initialize the semaphore */
   sem_handle = sem_open(
            sem_name,                         /* Semaphore name */
            O_CREAT | O_EXCL, /* Flags */
            00777, /* Permissions */
            semaphore_init); /* Initial count */

    if (sem_handle == SEM_FAILED)
    {
        /* creation failed */
        errnum = errno;
        switch (errnum)
        {
            case EEXIST:
                *status_out =
                        OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
                break;

            case ENOSPC:
                OSAPI_LOG_EXHAUSTED_POSIX_SEM_LIMIT(OSAPI_LOGKIND_ERROR)
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                break;

            default:
                OSAPI_LOG_SEM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        }
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemoryPosixSemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    char sem_name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
    int errnum;
    NETIO_SharedMemory_MACRO_KEY2STRING(sem_name, sem_type, key);
    sem_t *sem_handle = NULL;

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    /* Don't create the semaphore, just get it */
    sem_handle = sem_open(sem_name, 0);
    if (sem_handle == SEM_FAILED)
    {
        /* creation failed */
        errnum = errno;
        if (errnum == ENOENT)
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            OSAPI_LOG_SEM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        }
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemoryPosixSemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    int retval = -1;
#if OSAPI_ENABLE_LOG
    const char *fnName = NULL; /* Used to report failure */
#endif
    int errnum;
    sem_t *sem_handle = NULL;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    switch (sem_type)
    {
        case SEMMUTEX_TYPE_MUTEX:
        {
            if (!OSAPI_Thread_is_self(&h_impl->_lock_pid))
            {
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NOT_OWNER;
                return RTI_FALSE;
            }
            if (h_impl->_lock_count > 1)
            {
                h_impl->_lock_count--;
                return RTI_TRUE;
            }
            /* else, lockCount must be 1 */
#if OSAPI_ENABLE_LOG
            fnName = "sem_post";
#endif
            h_impl->_lock_count = 0;
            OSAPI_Memory_zero(&h_impl->_lock_pid, sizeof(h_impl->_lock_pid));
            retval = sem_post(sem_handle);
            break;
        }

        case SEMMUTEX_TYPE_BINARYSEM:
        {
            int value;
#if OSAPI_ENABLE_LOG
            fnName = "sem_getvalue";
#endif
            retval = sem_getvalue(sem_handle, &value);
            if (retval == -1 || value >= 1)
            {
                /* Do not do a sem_post if the semaphore is already set or
                 * in case of an error
                 */
                break;
            }
            /* Note: if this process gets interrupted here and another
             * process call _give on the same semaphore, there is the
             * risk that sem_post gets called twice.
             */
#if OSAPI_ENABLE_LOG
            fnName = "sem_post";
#endif
            retval = sem_post(sem_handle);
            break;
        }
        case SEMMUTEX_TYPE_SEMAPHORE:
        {
#if OSAPI_ENABLE_LOG
            fnName = "sem_post";
#endif
            retval = sem_post(sem_handle);
            break;
        }
    }
    if (retval == -1)
    {
        errnum = errno;
        if (errnum == ERANGE)
        {
            /* sem_post failed, reached semaphore maxcount */
            *status_out = OSAPI_SHARED_MEMORY_MAXCOUNT_REACHED;
            return RTI_TRUE;
        }
        if (errnum == EINVAL || errnum == EIDRM)
        {
            /* EIDRM: the semaphore was removed while waiting in the semop
             * EINVAL: the semaphore doesn't exist anymore.
             */
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            /* any other error */
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
#if OSAPI_ENABLE_LOG
            OSAPI_LOG_SHMEM_POSIX_GIVE(OSAPI_LOGKIND_ERROR, fnName, errnum)
#endif
        }
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemoryPosixSemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    int retval;
#if OSAPI_ENABLE_LOG
    int errnum;
#endif
    sem_t *sem_handle = NULL;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    /* We cannot use getpid() because different threads inside the
     * same process will create a conflict.
     * Using pthread_self guarantee that the unique value for the
     * calling thread.
     * NOTE: There is *NO* problem with other threads running on other
     *       process and having the same ThreadID because those other
     *       threads will use a different mutex handle
     */
    /* _lockCount is used for mutex only, sem_type is always MUTEX */
    if (OSAPI_Thread_is_self(&h_impl->_lock_pid))
    {
        ++h_impl->_lock_count;
        return RTI_TRUE;
    }

    /* SignalingSemaphores may have the semaphore value set to
     * 2. Is there a way we can detect this situation and do a sem_wait
     * twice?
     */
    retval = sem_wait(sem_handle);

    /* Posix API says that sem_wait always return 0... still we check for
     * errors.
     */
    if (retval == -1)
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_SEM_WAIT(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        h_impl->_lock_pid = OSAPI_Thread_self();
        ++h_impl->_lock_count;
    }
    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemoryPosixSemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
#if OSAPI_ENABLE_LOG
    int errnum;
#endif
    sem_t *sem_handle = NULL;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    UNUSED_ARG(sem_type);

    if (sem_close(sem_handle) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SEM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

RTIBool
NETIO_SharedMemoryPosixSemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
    char sem_name[OSAPI_SHARED_MEMORY_NAME_LENGTH];
#if OSAPI_ENABLE_LOG
    int errnum;
#endif

    NETIO_SharedMemory_MACRO_KEY2STRING(sem_name, sem_type, h_impl->_key);

    /* Always detach first (and ignore any possible error) */
    NETIO_SharedMemoryPosixSemMutex_detach_impl(h_impl, sem_type);

    /* Then unlink */
    if (sem_unlink(sem_name) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_SEM_UNLINK(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    return RTI_TRUE;
}
