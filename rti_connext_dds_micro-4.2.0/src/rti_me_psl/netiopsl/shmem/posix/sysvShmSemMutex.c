/*
 * FILE: sysvShmSemMutex.c - Shared memory Semaphore/Mutex API using SysV API
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

#if RTI_HAVE_POSIX_ROBUST_MUTEX
#include "posixRobustShmMutex.h"
#include "posixShmMutex.h"
#elif RTI_HAVE_POSIX_MUTEX
#include "posixShmMutex.h"
#endif

/******************************************************************************
 *                Using SystemV interprocess communication (IPC)
 ******************************************************************************/

#if !RTI_HAVE_POSIX_ROBUST_MUTEX && !RTI_HAVE_POSIX_MUTEX
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#if defined(RTI_UNIX) && !defined(RTI_LYNX) && !defined(RTI_DARWIN)
union semun
{
    RTI_INT32 val;
    struct semid_ds *mbuf;
    unsigned short int *array;
};
#endif /*  defined(RTI_UNIX) && !defined(RTI_LYNX) && !defined(RTI_DARWIN) */

#endif /* !RTI_HAVE_POSIX_ROBUST_MUTEX && !RTI_HAVE_POSIX_MUTEX */

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemoryMutex_is_robust(void)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    return RTI_TRUE;
#else
    return RTI_FALSE;
#endif
}

/*
 * The permission required for a semaphore operation  is  given
 * as  {token},  where  token is the type of permission needed.
 * The types of permission are interpreted as follows:
 *
 * 00400       READ by user
 * 00200       ALTER by user
 * 00040       READ by group
 * 00020       ALTER by group
 * 00004       READ by others
 * 00002       ALTER by others
 *
 * We use 00666.
 */
RTIBool
NETIO_SharedMemorySemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_create_impl(h_impl,status_out,key);
    }   
    return NETIO_SharedMemoryPosixSemMutex_create_impl(h_impl,status_out,key,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_create_impl(h_impl,status_out,key,sem_type);
#else  /* !(RTI_HAVE_POSIX_ROBUST_MUTEX || RTI_HAVE_POSIX_MUTEX) */

    /* Initialize semaphore with an initial count of zero for semaphores
     * (including binary semaphores) and to one for mutex
     */
    union semun semaphore_init;
    RTI_INT32 errnum;
    int sem_handle = -1;
    
    semaphore_init.val = (sem_type == SEMMUTEX_TYPE_MUTEX) ? 1 : 0;

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    /* Force creation (IPC_EXCL) error if already exists
     * because I want to be the owner of this semaphore
     */
    sem_handle = semget(
            key, /* Semaphore/Mutex key */
            1,   /* Number of semaphores to create */
            (IPC_CREAT | IPC_EXCL | 00666) /* flags */);

    if (sem_handle == -1)
    {
        /* creation failed */
        errnum = errno;
        switch (errnum)
        {
            case EEXIST:
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
                break;

            default:
                OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED(
                    OSAPI_LOGKIND_ERROR,
                    errnum,
                    sem_type)
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        }
        return RTI_FALSE;
    }

    /* semget success, initialize the semaphore */
    if (-1 == semctl(
            sem_handle,
            0, /* semnum-th semaphore of the set */
            SETVAL, /* cmd */
            semaphore_init))
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_SEM_MUTEX_CREATED(OSAPI_LOGKIND_ERROR,errnum,sem_type);
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        semctl(sem_handle, 0, IPC_RMID, 0);
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
#endif
}

RTIBool
NETIO_SharedMemorySemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_attach_impl(h_impl,status_out,key);
    }
    return NETIO_SharedMemoryPosixSemMutex_attach_impl(h_impl,status_out,key,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_attach_impl(h_impl,status_out,key,sem_type);
#else
    UNUSED_ARG(sem_type);
    RTI_INT32 errnum;
    int sem_handle = -1;

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    /* Do not ask for creation (IPC_EXCL) not set */
    sem_handle = semget(
            key, /* Semaphore/Mutex key */
            1, /* Number of semaphores to create */
            0 /* flags */);

    if (sem_handle == -1)
    {
        /* creation failed */
        errnum = errno;
        if (errnum == ENOENT)
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            OSAPI_LOG_SHMEM_ATTACH_FAILED(OSAPI_LOGKIND_ERROR, errnum)
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        }
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&sem_handle,sizeof(sem_handle));

    return RTI_TRUE;
#endif
}

RTIBool
NETIO_SharedMemorySemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_give_impl(h_impl,status_out);
    }
    return NETIO_SharedMemoryPosixSemMutex_give_impl(h_impl,status_out,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_give_impl(h_impl,status_out,sem_type);
#else
    RTI_INT32 retval = -1;
    RTI_INT32 errnum;
    int sem_handle;

    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    switch (sem_type)
    {
        case SEMMUTEX_TYPE_SEMAPHORE: 
        {
            struct sembuf sem_give[1] =
            {
                {
                    0,         /* sem_num */
                    1,         /* increase the sem value by 1 */
                    0         /* sem flag */
                }
            };
            retval = semop(sem_handle, sem_give, 1);
            break;
        }

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
            else
            {
                struct sembuf sem_give[1] =
                {
                    {
                        0, /* sem_num */
                        1, /* increase the sem value by 1 */
                        SEM_UNDO /* sem flag */
                    }
                };
                /* else, lockCount must be 1 */
                h_impl->_lock_count = 0;
                OSAPI_Memory_zero(&h_impl->_lock_pid,sizeof(h_impl->_lock_pid));
                retval = semop(sem_handle, sem_give, 1);
            }
            break;
        }

        case SEMMUTEX_TYPE_BINARYSEM: 
        {
            union semun mutex_give = {1};
            retval = semctl(sem_handle, 0, SETVAL, mutex_give);
            break;
        }

    }
    if (retval == -1)
    {
        errnum = errno;
        if (errnum == ERANGE)
        {
            /* semop failed, reached semaphore maxcount */
            *status_out = OSAPI_SHARED_MEMORY_MAXCOUNT_REACHED;
            return RTI_TRUE;
        }
        if ((errnum == EINVAL) || (errnum == EIDRM))
        {
            /* EIDRM: the semaphore was removed while waiting in the semop
             * EINVAL: the semaphore doesn't exist anymore.
             */
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_GIVE_FAILED(OSAPI_LOGKIND_ERROR, errnum)

        }
        return RTI_FALSE;
    }

    return RTI_TRUE;
 #endif
}

RTIBool
NETIO_SharedMemorySemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_take_impl(h_impl,status_out);
    }
    return NETIO_SharedMemoryPosixSemMutex_take_impl(h_impl,status_out,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_take_impl(h_impl,status_out,sem_type);
#else
    struct sembuf sem_take[1] =
    {
        {
            0,          /* sem_num */
            -1,         /* sleep until semval >= 1. Also if semval > 1
                         * (multi-gives) take() succeeds immediately
                         */
            0           /* Will be set to SEM_UNDO for mutex. Unfortunately
                         * Sun's cc doesn't allow the '?' operator in initializers
                         * so we can't do:
                         *   (sem_type == SEMMUTEX_TYPE_MUTEX) ? SEM_UNDO : 0
                         */
        }
    };

    RTI_INT32 retval;
    RTI_INT32 errnum;
    int sem_handle;

    /* We cannot use getpid() because different threads inside the
     * same process will create a conflict.
     * Using pthread_self guarantee that the unique value for the
     * calling thread.
     * NOTE: There is *NO* problem with other threads running on other
     *       process and having the same ThreadID because those other
     *       threads will use a different mutex handle
     */
    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    /* Set SEM_UNDO for semaphore of type MUTEX. By doing this we tell
     * the system to automatically release any lock if the process that locked
     * the mutex dies without releasing it.
     */

    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        sem_take[0].sem_flg = SEM_UNDO;
    }

    /* _lockCount is used for mutex only, sem_type is always MUTEX */
    if (OSAPI_Thread_is_self(&h_impl->_lock_pid))
    {
        ++h_impl->_lock_count;
        return RTI_TRUE;
    }

    /* Be robust to signals as they will wake up the thread but
     * do not indicate that the semaphore has been given
     */
    do
    {
        retval = semop(sem_handle, sem_take, 1 /* # of sem ops */);
    } while ((retval == -1) && ( errno == EINTR));

    if (retval == -1)
    {
        errnum = errno;

        if ((errnum == EINVAL) || (errnum == EIDRM))
        {
            /* EIDRM: the semaphore was removed while executing semop
             * EINVAL: the semaphore doesn't exist anymore.
             * Probably checking for EIDRM doesn't make sense too much here
             * because it's returned only if the semaphore is deleted
             * while blocking on a semop (in the middle of a _get)
             */
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
        }
        else
        {
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_SHMEM_TAKE_FAILED(OSAPI_LOGKIND_ERROR, errnum)
        }

        return RTI_FALSE;
    }

    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        h_impl->_lock_pid = OSAPI_Thread_self();
        ++h_impl->_lock_count;
    }

    return RTI_TRUE;
#endif
}


RTIBool
NETIO_SharedMemorySemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_detach_impl(h_impl);
    }
    return NETIO_SharedMemoryPosixSemMutex_detach_impl(h_impl,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_detach_impl(h_impl,sem_type);
#else
    UNUSED_ARG(h_impl);
    UNUSED_ARG(sem_type);
    return RTI_TRUE;
#endif
}

RTIBool
NETIO_SharedMemorySemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
#if RTI_HAVE_POSIX_ROBUST_MUTEX
    if (sem_type == SEMMUTEX_TYPE_MUTEX)
    {
        return NETIO_SharedMemoryPthreadMutex_delete_os(h_impl);
    }
    return NETIO_SharedMemoryPosixSemMutex_delete_os(h_impl,sem_type);
#elif RTI_HAVE_POSIX_MUTEX
    return NETIO_SharedMemoryPosixSemMutex_delete_os(h_impl,sem_type);
#else
#if OSAPI_ENABLE_LOG
    RTI_INT32 errnum;
#endif
    int sem_handle;

    UNUSED_ARG(sem_type);
    OSAPI_Memory_copy(&sem_handle,&h_impl->_native_hndl,sizeof(sem_handle));

    if (semctl(sem_handle, 0, IPC_RMID, 0) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_SEM_DELETE_FAILED(OSAPI_LOGKIND_ERROR, errnum)
#endif
        return RTI_FALSE;
    }
    return RTI_TRUE;
#endif
}
