/*
 * FILE: posixRobustShmMutex.h - Shared memory mutex using Robust POSIX Mutex
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "posixRobustShmMutex.h"

/*i
 * \brief Magic value to indicate that a shared memory segment containing
 *        a mutex has been initialized.
 *
 * \details This value is stored in the shared memory segment to indicate
 *          that the segment has been initialized and that the mutex is
 *          ready to be used. This value is necessary because it is
 *          undefined behavior to use a mutex that has not been initialized.
 *          The specific value was choosen by Connext Pro.
 */
#define OSAPI_SHARED_MEMORY_SEGMENT_INITIALIZED_MAGIC_COOKIE (0xDECADE)

#define OSAPI_SHARED_MEMORY_SHM_OPEN_MAX_RETRIES (20)
#define OSAPI_SHARED_MEMORY_WAIT_FOR_INIT_MAX_RETRIES (20)

/*i
 * \brief Maximum length of a shared memory mutex
 *        name, including the null terminator.
 */
#define OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH (64)

/*i
 * \brief Prefix for shared memory mutex names, common with Connext Pro
 */
#define OSAPI_SHARED_MEMORY_MUTEX_NAME_PREFIX "/RTIOsapiSharedMemoryMutex-"

/*i
 * \brief Pthread mutex structure stored in shared memory
 */
struct NETIO_SharedMemoryPthreadMutex
{
    /*i The pthread mutex
     */
    pthread_mutex_t mutex;

    /*i \brief Flag to indicate that the mutex has been initialized
     *
     *  \details This member is atomic because it is used to check if the
     *           pthread mutex has been initialized without being protected
     *           by a critical section.
     */
    RTI_ATOMIC(RTI_INT32) initialized;
};

/*i
 * \brief Convert a shared memory key to a string representation
 *
 * \long This function converts a 32-bit key into a hexadecimal string
 *       representation that can be used as a shared memory name. A prefix
 *       is prepended to the key to create a unique name.
 *
 * \param[out] buf the buffer to store the string
 * \param[in] buf_len the length of the buffer
 * \param[in] key the key to convert
 * \param[in] prefix the prefix to prepend to the key
 */
RTI_PRIVATE RTI_BOOL
NETIO_SharedMemoryPthreadMutex_key_to_string(char *buf, RTI_SIZE_T buf_len,
                                             RTI_INT32 key, const char *prefix)
{
    RTI_SIZE_T prefix_length;
    RTI_INT32 shift;
    RTI_BOOL leading_zero = RTI_TRUE;

    /* Check if the buffer is large enough to hold the prefix and the
     * key. The key is 8 characters (4 bits per character) plus the null
     * terminator is 9 bytes in addition to the prefix.
     */
    prefix_length = OSAPI_String_length(prefix);
    if (buf_len < prefix_length + 9)
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(buf, prefix, prefix_length);
    buf += prefix_length;

    /* Special case for key 0 to keep one zero */
    if (key == 0)
    {
        *buf = '0';
        buf++;
        *buf = '\0';
        return RTI_TRUE;
    }

    /* Convert four bits at a time of key into a hex digit */
    for (shift = 28; shift >= 0; shift -= 4)
    {
        RTI_UINT8 digit = (key >> shift) & 0xF;

        /* Skip leading zeros */
        if ((digit == 0) && leading_zero)
        {
            continue;
        }
        leading_zero = RTI_FALSE;

        if (digit < 10)
        {
            *buf = (char)('0' + digit);
        }
        else
        {
            *buf = (char)('a' + (digit - 10));
        }
        buf++;
    }

    /* Null-terminate the string */
    *buf = '\0';

    return RTI_TRUE;
}

/*i
 * \brief Sleep for a number of microseconds
 *
 * \param us[in] The number of microseconds to sleep
 * \return RTI_TRUE if the sleep was successful, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
NETIO_SharedMemoryPthreadMutex_sleep_us(RTI_UINT32 us)
{
    struct timespec sleep_time, rem_time;
    sleep_time.tv_sec = us / 1000000;
    sleep_time.tv_nsec = (us % 1000000) * 1000;

    while (RTI_TRUE)
    {
        if (nanosleep(&sleep_time, &rem_time) == 0)
        {
            return RTI_TRUE;
        }
        else if (errno == EINTR)
        {
            sleep_time = rem_time;
        }
        else
        {
            return RTI_FALSE;
        }
    }
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    char mutex_name[OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH];
    pthread_mutexattr_t mutexattr;
    int max_priority = 0;
    int shm_open_retries = 0;
    int shm_fd = 0;
    RTI_BOOL shm_is_open = RTI_FALSE;
    int rc, errnum;
    RTI_BOOL mutexattr_outstanding = RTI_FALSE;
    RTI_BOOL ok = RTI_FALSE;
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;

    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;

    if (!NETIO_SharedMemoryPthreadMutex_key_to_string(
            mutex_name,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH,
            key,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_PREFIX))
    {
        goto done;
    }

    while(RTI_TRUE)
    {
        shm_fd = shm_open(
                mutex_name,         /* Mutex name */
                O_RDWR |            /* read+write permissions on the segment */
#if !OSAPI_ENABLE_STRICT_POSIX
                        O_CLOEXEC | /* if process exec()'s, release the handle */
#endif
                        O_CREAT   | /* create if it doesn't exist */
                        O_EXCL,     /* fail the open if segment already exists */
                /* permissions for our handle (not file itself): */
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

        if (shm_fd == -1)
        {
            /* creation failed */
            errnum = errno;
            switch (errnum)
            {
                case EACCES:
                    /* Consider segment already claimed if we do not have
                     * permission to create it
                     */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
                    break;
                case EEXIST:
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_ALREADY_CLAIMED;
                    goto done;
                case EINTR:
                    if (shm_open_retries < OSAPI_SHARED_MEMORY_SHM_OPEN_MAX_RETRIES)
                    {
                        shm_open_retries++;
                        continue;
                    }
                    /* shm_open failed with EINTR repeatedly */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    goto done;
                case ENOSPC:
                    /* shm_open fails with error ENOSPC: increment POSIX shmem limit */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    break;
                default:
                    /* shm_open fails with unknown error */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    break;
            }
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
            goto done;
        }
        break;
    }
    shm_is_open = RTI_TRUE;

    /* ensure shmem segment will fit our mutex + initialization flag */
    if (ftruncate(shm_fd, RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex)))
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_FTRUNCATE(OSAPI_LOGKIND_ERROR, errnum)
        goto done;
    }
    /*
     * we now have a file descriptor pointing to large enough shmem segment,
     * let's map it to local memory and we will initialize the mutex there
     */
    shm_mutex = mmap(0,
                     RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     shm_fd,
                     0);
    if (shm_mutex == NULL)
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
        goto done;
    }
    OSAPI_Memory_zero(shm_mutex, RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex));

    /* initialize mutex attribute */
    rc = pthread_mutexattr_init(&mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    mutexattr_outstanding = RTI_TRUE;

    rc = pthread_mutexattr_setrobust(&mutexattr, PTHREAD_MUTEX_ROBUST);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    rc = pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    /* priority inheritance */
    max_priority = sched_get_priority_max(SCHED_FIFO);
    if (max_priority == -1)
    {
        errnum = errno;
        OSAPI_LOG_THREAD_GET_MAX_PRIORITY(OSAPI_LOGKIND_ERROR, errnum)
        goto done;
    }
    rc = pthread_mutexattr_setprotocol(&mutexattr, PTHREAD_PRIO_INHERIT);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETPROTOCOL(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_mutexattr_setprioceiling(&mutexattr, max_priority);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETPRIOCEILING(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    /* initialize mutex */
    rc = pthread_mutex_init(&shm_mutex->mutex, &mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    /* Mark as initialized and allow other processes to attach.
     *
     * Store release to guarantee the code before this instruction to be run
     * before this point (it cannot be delayed).
     */
    OSAPI_Atomic_store(&shm_mutex->initialized,
                       OSAPI_SHARED_MEMORY_SEGMENT_INITIALIZED_MAGIC_COOKIE,
                       OSAPI_ATOMIC_MEMORY_ORDER_RELEASE);

    OSAPI_Memory_copy(&h_impl->_native_hndl,&shm_mutex,sizeof(shm_mutex));

    ok = RTI_TRUE;

done:
    if (mutexattr_outstanding)
    {
        rc = pthread_mutexattr_destroy(&mutexattr);
        if (rc != 0)
        {
            OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY(OSAPI_LOGKIND_ERROR, rc)
            ok = RTI_FALSE;
        }
    }

    if (shm_is_open)
    {
        if (close(shm_fd) != 0)
        {
            errnum = errno;
            OSAPI_LOG_CLOSE(OSAPI_LOGKIND_ERROR, errnum)
            ok = RTI_FALSE;
        }
    }

    if (!ok)
    {
        if (shm_mutex != NULL)
        {
            if (munmap(shm_mutex, RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex)) != 0)
            {
                errnum = errno;
                OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum)
            }
        }
        if (shm_is_open)
        {
            if (shm_unlink(mutex_name) == -1)
            {
                errnum = errno;
                OSAPI_LOG_SHMEM_UNLINK(OSAPI_LOGKIND_ERROR, errnum)
            }
        }
    }

    return ok;
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    char mutex_name[OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH];
    int shm_open_retries = 0;
    int ftruncate_check_retries = 0;
    int shm_fd = 0;
    int errnum;
    RTI_BOOL shm_is_open = RTI_FALSE;
    RTI_BOOL ok = RTI_FALSE;
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;

    if (!NETIO_SharedMemoryPthreadMutex_key_to_string(
            mutex_name,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH,
            key,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_PREFIX))
    {
        goto done;
    }

    while(RTI_TRUE)
    {
        shm_fd = shm_open(mutex_name, O_RDWR, 0);
        if (shm_fd == -1)
        {
            /* open failed */
            errnum = errno;
            switch (errnum)
            {
                case EACCES:
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    break;
                case EINTR:
                    if (shm_open_retries < OSAPI_SHARED_MEMORY_SHM_OPEN_MAX_RETRIES)
                    {
                        shm_open_retries++;
                        continue;
                    }
                    /* shm_open failed with EINTR repeatedly */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    break;
                case ENOENT:
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NO_ENTRY;
                    goto done;
                default:
                    /* shm_open failed with unknown error */
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    break;
            }
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum)
            goto done;
        }
        break;
    }
    shm_is_open = RTI_TRUE;

    /* Don't progress until the segment has been truncated */
    while (RTI_TRUE)
    {
        struct stat statbuf;

        OSAPI_Memory_zero(&statbuf, RTI_SIZEOF(struct stat));

        if (fstat(shm_fd, &statbuf) == -1)
        {
            errnum = errno;
            OSAPI_LOG_SHMEM_FSTAT(OSAPI_LOGKIND_ERROR, errnum)
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            goto done;
        }

        if (statbuf.st_size > 0)
        {
            /* Segment has been truncated, the length wil be verified by mmap */
            break;
        }
        else if (ftruncate_check_retries < OSAPI_SHARED_MEMORY_WAIT_FOR_INIT_MAX_RETRIES)
        {
            if (!NETIO_SharedMemoryPthreadMutex_sleep_us(200000))
            {
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                goto done;
            }
        }
        else
        {
            /* Segment has not been truncated, but we've waited too long */
            OSAPI_LOG_SHMEM_MUTEX_INIT_TIMEOUT(OSAPI_LOGKIND_ERROR, key)
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            goto done;
        }
    }

    /* map the shared memory segment */
    shm_mutex = mmap(0,
                     RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     shm_fd,
                     0);
    if (shm_mutex == NULL)
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
        goto done;
    }

    /* Don't progress unless mutex has been initialized. */
    {
        int retry_number = 0;
        int max_retries = OSAPI_SHARED_MEMORY_WAIT_FOR_INIT_MAX_RETRIES;
        while (RTI_TRUE)
        {
            /*
             * Load acquire to prevent any code after this point to be executed
             * before this point.
             */
            if (OSAPI_Atomic_load(&shm_mutex->initialized,
                                  OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE)
                    == OSAPI_SHARED_MEMORY_SEGMENT_INITIALIZED_MAGIC_COOKIE)
            {
                /* ok, mutex has been initialized, carry on */
                break;
            }
            else
            {
                if (retry_number < max_retries)
                {
                    /*
                     * another thread is likely creating the mutex, yield the
                     * cpu to allow it to finish initializing
                     */
                    if (!NETIO_SharedMemoryPthreadMutex_sleep_us(200000))
                    {
                        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                        goto done;
                    }
                    continue;
                }
                else
                {
                    /* we've waited long enough, there's an issue */
                    OSAPI_LOG_SHMEM_MUTEX_INIT_TIMEOUT(OSAPI_LOGKIND_ERROR, key)
                    *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                    goto done;
                }
            }
        }
    }

    OSAPI_Memory_copy(&h_impl->_native_hndl,&shm_mutex,sizeof(shm_mutex));

    ok = RTI_TRUE;
done:
    if (!ok)
    {
        /* shmem mapping */
        if (shm_mutex != NULL)
        {
            OSAPI_Memory_copy(&h_impl->_native_hndl,&shm_mutex,
                              sizeof(shm_mutex));
            if (!NETIO_SharedMemoryPthreadMutex_detach_impl(h_impl))
            {
                ok = RTI_FALSE;
            }
        }
    }

    /*
     * finally, close our shmem file descriptor - shmem will remain linked
     * unless there was an error
     */
    if (shm_is_open)
    {
        if (close(shm_fd) != 0)
        {
            errnum = errno;
            OSAPI_LOG_CLOSE(OSAPI_LOGKIND_ERROR, errnum)
            ok = RTI_FALSE;
        }
    }

    return ok;
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out)
{
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;
    RTI_BOOL ok = RTI_FALSE;
    int rc = 0;
    
    OSAPI_Memory_copy(&shm_mutex,&h_impl->_native_hndl,sizeof(shm_mutex));

    if (!OSAPI_Thread_is_self(&h_impl->_lock_pid))
    {
        *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NOT_OWNER;
        goto done;
    }
    if (h_impl->_lock_count > 1)
    {
        h_impl->_lock_count--;
        *status_out = OSAPI_SHARED_MEMORY_SUCCESS;
        ok = RTI_TRUE;
        goto done;
    }

    h_impl->_lock_count = 0;
    OSAPI_Memory_zero(&h_impl->_lock_pid, sizeof(h_impl->_lock_pid));

    rc = pthread_mutex_unlock(&shm_mutex->mutex);

    switch (rc)
    {
        case 0:
            *status_out = OSAPI_SHARED_MEMORY_SUCCESS;
            ok = RTI_TRUE;
            break;
        case EPERM:
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_NOT_OWNER;
            OSAPI_LOG_PTHREAD_MUTEX_UNLOCK(OSAPI_LOGKIND_ERROR, rc)
            break;
        default:
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_PTHREAD_MUTEX_UNLOCK(OSAPI_LOGKIND_ERROR, rc)
            break;
    }

done:
    return ok;
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out)
{
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;
    RTI_BOOL ok = RTI_FALSE;
    int rc = 0;
    OSAPI_ThreadId self_id;

    OSAPI_Memory_copy(&shm_mutex,&h_impl->_native_hndl,sizeof(shm_mutex));

    self_id = OSAPI_Thread_self();

    if (h_impl->_lock_pid.handle.handle64 == self_id.handle.handle64)
    {
        h_impl->_lock_count++;
        *status_out = OSAPI_SHARED_MEMORY_SUCCESS;
        ok = RTI_TRUE;
        goto done;
    }

    rc = pthread_mutex_lock(&shm_mutex->mutex);

    switch (rc)
    {
        case 0:
            h_impl->_lock_pid = OSAPI_Thread_self();
            h_impl->_lock_count = 1;
            *status_out = OSAPI_SHARED_MEMORY_SUCCESS;
            ok = RTI_TRUE;
            break;
        case EOWNERDEAD:
            /*
            * all good; let's recover. Our concurrent queue (protected by
            * this mutex) already has recovery mechanisms. So all we need
            * to do is mark the mutex as consistent. We hold the lock
            * already.
            */
            rc = pthread_mutex_consistent(&shm_mutex->mutex);
            if (rc != 0)
            {
                *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
                OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT(OSAPI_LOGKIND_ERROR, rc)
            }
            else
            {
                *status_out = OSAPI_SHARED_MEMORY_SUCCESS;
                ok = RTI_TRUE;
            }
            break;
        default:
            *status_out = OSAPI_SHARED_MEMORY_FAIL_REASON_UNKNOWN;
            OSAPI_LOG_PTHREAD_MUTEX_LOCK(OSAPI_LOGKIND_ERROR, rc)
            break;
    }

done:

    return ok;
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl)
{
    int errnum;
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;

    OSAPI_Memory_copy(&shm_mutex,&h_impl->_native_hndl,sizeof(shm_mutex));

    if (munmap(&shm_mutex->mutex,
               RTI_SIZEOF(struct NETIO_SharedMemoryPthreadMutex)) != 0)
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
NETIO_SharedMemoryPthreadMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl)
{
    char mutex_name[OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH];
    struct NETIO_SharedMemoryPthreadMutex *shm_mutex = NULL;
    int rc, errnum;

    OSAPI_Memory_copy(&shm_mutex,&h_impl->_native_hndl,sizeof(shm_mutex));

    if (!NETIO_SharedMemoryPthreadMutex_key_to_string(
            mutex_name,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_MAX_LENGTH,
            h_impl->_key,
            OSAPI_SHARED_MEMORY_MUTEX_NAME_PREFIX))
    {
        return RTI_FALSE;
    }

    rc = pthread_mutex_destroy(&shm_mutex->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    /* Always detach first */
    if (!NETIO_SharedMemoryPthreadMutex_detach_impl(h_impl))
    {
        return RTI_FALSE;
    }

    /* Then delete the segment */
    if (shm_unlink(mutex_name) == -1)
    {
        errnum = errno;
        OSAPI_LOG_SHMEM_UNLINK(OSAPI_LOGKIND_ERROR, errnum)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
