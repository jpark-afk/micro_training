/*
 * FILE: posixShmSegment.c - POSIX shared memory segment implementation
 *
 * Copyright 2022-2022 Real-Time Innovations, Inc.
 * All rights reserved.
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rti_me_psl.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "osapi/osapi_log.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"
#include "posixShmSegment.h"


/*** SOURCE_BEGIN ***/


RTI_PRIVATE const RTI_UINT64 OSAPI_SharedMemorySegment_fv_DefaultMaxSize = (1U << 31) -
                                                                           1U;


RTI_BOOL
OSAPI_SharedMemory_is_robust_mutex_supported(void)
{
#if OSAPI_ROBUST_LOCKING_ENABLED
    return RTI_TRUE;
#else
    return RTI_FALSE;
#endif
}


/*i \brief Sleep for a number of microseconds
 *
 * \param us[in] The number of microseconds to sleep
 * \return RTI_TRUE if the sleep was successful, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SharedMemorySegment_sleep_us(RTI_UINT32 us)
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

/*i \brief Check a currently attached segment has been unlinked
 *
 * \details A segment continues to exist until all handles have been detached,
 *          even if it has been unlinked. It is undesirable to call unlink on
 *          a segment twice because the second time may succeed at destroying
 *          a different segment.
 *
 * \param handle[in]    The handle to the segment
 * \param unlinked[out] RTI_TRUE if it has been unlinked, RTI_FALSE otherwise
 * \return RTI_TRUE if the check was successful, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SharedMemorySegment_is_unlinked(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        RTI_BOOL *unlinked)
{
    RTI_BOOL success = RTI_FALSE;
    int retval;
    struct OSAPI_SharedMemorySegmentHandleImpl *h_impl =
            (struct OSAPI_SharedMemorySegmentHandleImpl *)handle;
    struct OSAPI_SharedMemorySegmentHeaderImpl *header =
            (struct OSAPI_SharedMemorySegmentHeaderImpl *)handle->ptr_header;
    int fd = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    int rc;
    struct stat statbuf, new_statbuf;
    char slash_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    *unlinked = RTI_FALSE;

    /* Prepend a leading slash to the segment name and check the length of the
     * resulting string.
     */
    rc = snprintf(slash_name,
        OSAPI_SHMEM_MAX_NAME_LENGTH + 1,
        "/%s",
        header->name);
    if ((rc < 0) || (rc >= OSAPI_SHMEM_MAX_NAME_LENGTH + 1))
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, header->name);
        goto done;
    }
    /* Reattach to the segment by name */
    fd = shm_open(slash_name, O_RDWR, 0);
    if (fd == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        if (errno == ENOENT)
        {
            /* Did not exist, it has already been unlinked */
            *unlinked = RTI_TRUE;
            success = RTI_TRUE;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errno);
        }
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    /* Check if the opened segment is the same segment that handle is attached to */
    if (fstat(h_impl->native_handle, &statbuf) == -1) {
        goto done;
    }
    if (fstat(fd, &new_statbuf) == -1) {
        goto done;
    }
    /* POSIX spec says that a file is uniquely identified by (st_dev, st_ino) */
    if ((statbuf.st_dev != new_statbuf.st_dev) ||
        (statbuf.st_ino != new_statbuf.st_ino))
    {
        /* This is a different segment, must have been unlinked */
        *unlinked = RTI_TRUE;
    }

    success = RTI_TRUE;
done:
    if (fd != OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        /* Return value is ignored here because our presence in this block
         * already indicates a fatal error.
         */
        retval = close(fd);
        IGNORE_RETVAL(retval);
    }

    return success;
}

RTI_UINT64
OSAPI_SharedMemorySegment_get_max_size()
{
    FILE *f = NULL;
    RTI_UINT64 max_size = OSAPI_SharedMemorySegment_fv_DefaultMaxSize;
    int rc;
    RTI_UINT64 header_size =
        (RTI_UINT64)OSAPI_SharedMemorySegmentHeader_get_size(OSAPI_SHMEM_MODE_READ);

    f = fopen("/proc/sys/kernel/shmmax", "r");
    if (f != NULL)
    {
        /* Justification: max_size is considered tainted because it comes from a
         * kernel parameter file. This reads the current system-wide maximum
         * size for a single shared memory segment as configured in the Linux kernel.
         * This is a trusted value because it is set by the system.
         */
        /* coverity[tainted_data] */
        if (fscanf(f, "%llu", &max_size) == 0)
        {
            /* Parsing failed. Fallback already assigned. */
            max_size = OSAPI_SharedMemorySegment_fv_DefaultMaxSize;
        }

        /* coverity[cert_err33_c_violation] */
        /* coverity[cert_pos54_c_violation] */
        rc = fclose(f);
        IGNORE_RETVAL(rc);
    }


    if (max_size < header_size)
    {
        return 0;
    }

    /* Justification: Coverity warns that this can undeflow even with the above
     * guard. If max_size is clamped to an upper bound then the warning is fixed.
     * The upper bound is UINT64 max. If a clamp was added it would result in
     * deadcode.
     */
    /* coverity[overflow : FALSE] */
    /* coverity[return_overflow : FALSE] */
    return max_size - header_size;
}

RTI_BOOL
OSAPI_SharedMemorySegment_is_owner_alive(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        RTI_BOOL *alive)
{
    struct OSAPI_SharedMemorySegmentHeaderImpl *header =
            (struct OSAPI_SharedMemorySegmentHeaderImpl *)handle->ptr_header;

    *alive = OSAPI_Process_is_alive(header->owner_pid);
    return RTI_TRUE;
}

RTI_SIZE_T
OSAPI_SharedMemorySegmentHandle_get_size(OSAPI_SHMEM_MODE mode)
{
    UNUSED_ARG(mode);

    return sizeof(struct OSAPI_SharedMemorySegmentHandleImpl);
}

RTI_SIZE_T
OSAPI_SharedMemorySegmentHeader_get_size(OSAPI_SHMEM_MODE mode)
{
    RTI_SIZE_T size;

    if (mode == OSAPI_SHMEM_MODE_WRITE)
    {
        size = sizeof(struct OSAPI_SharedMemorySegmentHeaderImpl);
    }
    else /* OSAPI_SHMEM_MODE_LOCKABLE || OSAPI_SHMEM_MODE_ROBUST */
    {
        size = sizeof(struct OSAPI_SharedMemorySegmentHeaderImplLockable);
    }

    /* Align size to platform alignment */
    size = OSAPI_SHARED_MEMORY_ALIGN(size, OSAPI_SHARED_MEMORY_ALIGNMENT);

    return size;
}

RTI_BOOL
OSAPI_SharedMemorySegment_exists(const char *name)
{
    int fd;
    int retval;
    int rc;
    char slash_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    /* Prepend a leading slash to the segment name and check the length of the
     * resulting string.
     */
    rc = snprintf(slash_name, OSAPI_SHMEM_MAX_NAME_LENGTH + 1, "/%s", name);
    if ((rc < 0) || (rc >= OSAPI_SHMEM_MAX_NAME_LENGTH + 1))
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, name);
        return RTI_FALSE;
    }
    fd = shm_open(slash_name, O_RDWR, 0);
    if (fd == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        return RTI_FALSE;
    }
    /* Return value intentionally ignored as no action could be taken on an
     * error code from close() at this point in the function.
     */
    retval = close(fd);
    IGNORE_RETVAL(retval);
    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_create_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        RTI_UINT64 size,
        OSAPI_SHMEM_MODE mode,
        RTI_BOOL *exists)
{
    RTI_BOOL success = RTI_FALSE;
    int errnum;
    int native_handle = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    struct OSAPI_SharedMemorySegmentHandleImpl *h_impl =
            (struct OSAPI_SharedMemorySegmentHandleImpl *)handle;
    struct OSAPI_SharedMemorySegmentHeaderImpl *header = NULL;
    struct OSAPI_SharedMemorySegmentHeaderImplLockable *lockable_header = NULL;
    RTI_BOOL is_lockable = (mode >= OSAPI_SHMEM_MODE_LOCKABLE);
    void *sharedMemory = MAP_FAILED;
    RTI_SIZE_T header_size;
    RTI_UINT64 total_size;
    pthread_mutexattr_t mutexattr;
    int rc;
    RTI_BOOL lock_attr_initialized = RTI_FALSE;
    RTI_BOOL lock_initialized = RTI_FALSE;
    RTI_BOOL user_lock_initialized = RTI_FALSE;
    char slash_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    *exists = RTI_FALSE;

#if !OSAPI_ROBUST_LOCKING_ENABLED
    if (mode == OSAPI_SHMEM_MODE_ROBUST)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_MODE(OSAPI_LOGKIND_ERROR, name, mode);
        goto done;
    }
#endif /* !OSAPI_ROBUST_LOCKING_ENABLED */

    /* Prepend a leading slash to the segment name and check the length of the
     * resulting string.
     */
    rc = snprintf(slash_name, OSAPI_SHMEM_MAX_NAME_LENGTH + 1, "/%s", name);
    if ((rc < 0) || (rc >= OSAPI_SHMEM_MAX_NAME_LENGTH + 1))
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, name);
        goto done;
    }
    /* Set the permission bits of the shared memory segment to 00666 to allow
     * any user to read or write it. Micro has no requirement for allowing only
     * certain users to access shared memory segments.
     */
    native_handle = shm_open(
            slash_name,
            O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (native_handle == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        /* shm_open failed, check why... */
        errnum = errno;
        if (errnum != EEXIST)
        {
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum);
        }
        else
        {
            *exists = RTI_TRUE;
            OSAPI_LOG_SHMEM_SEGMENT_NAME_ALREADY_EXISTS(OSAPI_LOGKIND_INFO, name);
        }
        goto done;
    }

    /* Calculate the effective size of memory to allocate. */
    header_size = OSAPI_SharedMemorySegmentHeader_get_size(mode);
    total_size = size + header_size;

    /* Defines the size of the created shmem */
    if (ftruncate(native_handle, (off_t)total_size))
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_FTRUNCATE(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    sharedMemory =
            mmap(0,          /* Addr: not supported */
                 total_size, /* Size of memory to map: all */
                 MMAP_PROT_ARG_RW,
                 MAP_SHARED,
                 native_handle,
                 0);

    if (sharedMemory == MAP_FAILED)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum)
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }
    header = (struct OSAPI_SharedMemorySegmentHeaderImpl *)sharedMemory;
    header->parent_.size = size;
    header->parent_.mode = mode;
    header->owner_pid = OSAPI_Process_getpid();
    OSAPI_Memory_copy(&header->name, name, OSAPI_String_length(name));

    h_impl->parent_.ptr_header = (struct OSAPI_SharedMemorySegmentHeader *)sharedMemory;
    h_impl->parent_.ptr_user_data = (char *)sharedMemory + header_size;
    h_impl->native_handle = native_handle;

    /* Initialize the mutex for inter-process usage */
    rc = pthread_mutexattr_init(&mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    lock_attr_initialized = RTI_TRUE;
    rc = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
    if (rc != 0) {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETTYPE(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
#if OSAPI_ROBUST_LOCKING_ENABLED
    if (mode == OSAPI_SHMEM_MODE_ROBUST)
    {
        rc = pthread_mutexattr_setrobust(&mutexattr, PTHREAD_MUTEX_ROBUST);
        if (rc != 0)
        {
            OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST(OSAPI_LOGKIND_ERROR, rc)
            goto done;
        }
    }
#endif /* OSAPI_ROBUST_LOCKING_ENABLED */

    rc = pthread_mutex_init(&header->lock, &mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    lock_initialized = RTI_TRUE;

    if (is_lockable)
    {
        lockable_header = (struct OSAPI_SharedMemorySegmentHeaderImplLockable *)header;
        rc = pthread_mutex_init(&lockable_header->user_lock, &mutexattr);
        if (rc != 0)
        {
            OSAPI_LOG_PTHREAD_MUTEX_INIT(OSAPI_LOGKIND_ERROR, rc)
            goto done;
        }
        user_lock_initialized = RTI_TRUE;
        /* In order to allow the caller to complete initialization before
         * allowing any other process/thread to access this segment,
         * take the user lock here and return with the user lock held. It is
         * a part of the contract that the caller will call unlock() after
         * initialization is complete. Therefore, it is safe to take the
         * lock here.
         */
        /* coverity[misra_c_2012_rule_22_16_violation] */
        rc = pthread_mutex_lock(&lockable_header->user_lock);
        if (rc != 0)
        {
            goto done;
        }
    }

    /* Mark as initialized and allow other processes to attach */
    OSAPI_Atomic_store(&header->initialized,
                       OSAPI_SHMEM_SEGMENT_INITIALIZED_MAGIC_COOKIE,
                       OSAPI_ATOMIC_MEMORY_ORDER_RELEASE);

    success = RTI_TRUE;
done:
    if (lock_attr_initialized)
    {
        /* The return value of pthread_mutexattr_destroy is intentionally
         * ignored here because the overall function has already succeeded or
         * failed. We are just trying to cleanup, and the result of destroying
         * the mutex attribute does not change the result of the overall function.
         */
        /* coverity[cert_pos54_c_violation] */
        rc = pthread_mutexattr_destroy(&mutexattr);
#if OSAPI_ENABLE_LOG
        if (rc != 0)
        {
            OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        }
#else
        IGNORE_RETVAL(rc);
#endif
    }
    if (!success)
    {
        /* Return values are ignored in the if() blocks below because our presence
         * in this block already indicates a fatal error.
         */
        if (user_lock_initialized)
        {
            rc = pthread_mutex_destroy(&lockable_header->user_lock);
            IGNORE_RETVAL(rc);
        }
        if (lock_initialized)
        {
            rc = pthread_mutex_destroy(&header->lock);
            IGNORE_RETVAL(rc);
        }
        if (sharedMemory != MAP_FAILED)
        {
            rc = munmap(sharedMemory, total_size);
            IGNORE_RETVAL(rc);
        }
        if (native_handle != OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
        {
            rc = close(native_handle);
            IGNORE_RETVAL(rc);
        }
        h_impl->parent_.ptr_header = NULL;
        h_impl->parent_.ptr_user_data = NULL;
        h_impl->native_handle = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    }
    return success;
}

RTI_BOOL
OSAPI_SharedMemorySegment_attach_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        const char *name,
        OSAPI_SHMEM_MODE mode,
        OSAPI_SHMEM_ATTACH_STATUS *status_out)
{
    RTI_BOOL success = RTI_FALSE;
    int retval;
    void *sharedMemory = MAP_FAILED;
    RTI_UINT64 total_size; /* Total size of memory allocated */
    int native_handle = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    RTI_UINT32 retry_number = 0;
    struct stat statbuf;
    int prot_arg;
    struct OSAPI_SharedMemorySegmentHandleImpl *h_impl =
            (struct OSAPI_SharedMemorySegmentHandleImpl *)handle;
    RTI_UINT64 mapped_size;
    OSAPI_SHMEM_ATTACH_STATUS shmem_status = OSAPI_SHMEM_ATTACH_STATUS_OK;
    char slash_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
#if OSAPI_ENABLE_LOG
    int errnum;
#endif /* OSAPI_ENABLE_LOG */

    if (mode >= OSAPI_SHMEM_MODE_WRITE)
    {
        prot_arg = MMAP_PROT_ARG_RW;
    }
    else if (mode == OSAPI_SHMEM_MODE_READ)
    {
        prot_arg = MMAP_PROT_ARG_RO;
    }
    else
    {
        goto done;
    }

    retval = snprintf(slash_name, OSAPI_SHMEM_MAX_NAME_LENGTH + 1, "/%s", name);
    if (retval < 0 || retval > OSAPI_SHMEM_MAX_NAME_LENGTH + 1)
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, name);
        goto done;
    }
    native_handle = shm_open(slash_name, O_RDWR, 0);

    if (native_handle == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        if (errnum == ENOENT)
        {
            /* Segment does not exist */
            shmem_status = OSAPI_SHMEM_ATTACH_STATUS_SEGMENT_NOT_FOUND;
            /* The attaching did not succeed, but this function still returns TRUE
             *   The caller should check for the status to see what happened
             *   exactly. */
            success = RTI_TRUE;
        }
        else
        {
            OSAPI_LOG_SHMEM_SHM_OPEN(OSAPI_LOGKIND_ERROR, errnum);
        }
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    /* Don't progress until the segment has been truncated */
    while (RTI_TRUE)
    {
        if (fstat(native_handle, &statbuf) == -1) {
            goto done;
        }

        if (statbuf.st_size > 0) {
            /* Segment has been truncated, continue */
            break;
        } else if (retry_number < OSAPI_SHMEM_WAIT_FOR_INIT_MAX_TRIES) {
            /* Segment has not been truncated, wait and retry */
            ++retry_number;
            if (!OSAPI_SharedMemorySegment_sleep_us(OSAPI_SHMEM_WAIT_FOR_INIT_SLEEP_US))
            {
                goto done;
            }
        } else {
            /* Segment has not been truncated, but we've waited too long */
            OSAPI_SHMEM_SEGMENT_NOT_INITIALIZED(OSAPI_LOGKIND_ERROR, name);
            goto done;
        }
    }

    /* Get size of platform independent header */
    mapped_size = (RTI_UINT64)sizeof(struct OSAPI_SharedMemorySegmentHeader);

    /* First map only the header, so total_size can be retrieved */
    sharedMemory =
            mmap(0,           /* Addr: not supported */
                 mapped_size, /* Size of memory to map: header */
                 prot_arg,
                 MAP_SHARED,
                 native_handle,
                 0);

    if (sharedMemory == MAP_FAILED)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    /* Don't progress until segment has been initialized */
    while (RTI_TRUE)
    {
        struct OSAPI_SharedMemorySegmentHeaderImpl *header =
                (struct OSAPI_SharedMemorySegmentHeaderImpl *)sharedMemory;

        if (OSAPI_Atomic_load(&header->initialized,
                              OSAPI_ATOMIC_MEMORY_ORDER_ACQUIRE)
                == OSAPI_SHMEM_SEGMENT_INITIALIZED_MAGIC_COOKIE)
        {
            /* Segment has been initialized, continue */
            break;
        } else if (retry_number < OSAPI_SHMEM_WAIT_FOR_INIT_MAX_TRIES) {
            /* Segment has not been initialized, wait and retry */
            ++retry_number;
            if (!OSAPI_SharedMemorySegment_sleep_us(OSAPI_SHMEM_WAIT_FOR_INIT_SLEEP_US)) {
                goto done;
            }
        } else {
            /* Segment has not been initialized, but we've waited too long */
            OSAPI_SHMEM_SEGMENT_NOT_INITIALIZED(OSAPI_LOGKIND_ERROR, name);
            goto done;
        }
    }

    /* Excellent, memory mapped. At the top of the segment I can find the
     * header, containing the total size of the created segment.
     */
    total_size = ((struct OSAPI_SharedMemorySegmentHeader *)sharedMemory)->size;

    /* Un-map the mapped header */
    if (munmap(sharedMemory, mapped_size) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    /* Re-Map the area, this time the correct size */
    mapped_size = total_size;
    sharedMemory =
            mmap(0,           /* Addr: not supported */
                 mapped_size, /* Size of memory to map: all */
                 prot_arg,
                 MAP_SHARED,
                 native_handle,
                 0);

    if (sharedMemory == MAP_FAILED)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MMAP(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    h_impl->parent_.ptr_header = (struct OSAPI_SharedMemorySegmentHeader *)sharedMemory;
    h_impl->parent_.ptr_user_data = (char *)sharedMemory +
            OSAPI_SharedMemorySegmentHeader_get_size(h_impl->parent_.ptr_header->mode);
    h_impl->native_handle = native_handle;

    success = RTI_TRUE;
done:
    if (!success)
    {
        /* Return values are ignored in the if() blocks below because our presence
         * in this block already indicates a fatal error.
         */
        if (sharedMemory != MAP_FAILED)
        {
            retval = munmap(sharedMemory, mapped_size);
            IGNORE_RETVAL(retval);
        }
        if (native_handle != OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
        {
            retval = close(native_handle);
            IGNORE_RETVAL(retval);
        }
        h_impl->parent_.ptr_header = NULL;
        h_impl->parent_.ptr_user_data = NULL;
        h_impl->native_handle = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    }
    else
    {
        /* If requested, send the status back to the caller. */
        if (status_out != NULL)
        {
            *status_out = shmem_status;
        }
    }
    return success;
}

RTI_BOOL
OSAPI_SharedMemorySegment_detach_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
#if OSAPI_ENABLE_LOG
    int errnum;
#endif /* OSAPI_ENABLE_LOG */
    struct OSAPI_SharedMemorySegmentHandleImpl *h_impl =
            (struct OSAPI_SharedMemorySegmentHandleImpl *)handle;

    if (h_impl->native_handle == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        /* Already detached/deleted */
        return RTI_FALSE;
    }

    if (munmap(handle->ptr_header, handle->ptr_header->size) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_MUNMAP(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        return RTI_FALSE;
    }
    if (close(((struct OSAPI_SharedMemorySegmentHandleImpl *)handle)->native_handle) == -1)
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_CLOSE(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        return RTI_FALSE;
    }
    /* Invalidate the native handle */
    h_impl->native_handle = OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT;
    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_delete_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
#if OSAPI_ENABLE_LOG
    int errnum;
#endif /* OSAPI_ENABLE_LOG */
    RTI_BOOL success = RTI_FALSE;
    struct OSAPI_SharedMemorySegmentHandleImpl *h_impl =
            (struct OSAPI_SharedMemorySegmentHandleImpl *)handle;
    struct OSAPI_SharedMemorySegmentHeaderImpl *header =
            (struct OSAPI_SharedMemorySegmentHeaderImpl *)handle->ptr_header;
    char slash_name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
    int rc;
    RTI_BOOL unlinked;

    if (h_impl->native_handle == OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT)
    {
        /* Already detached/deleted */
        return RTI_FALSE;
    }

    rc = pthread_mutex_lock(&header->lock);
    if (rc == EOWNERDEAD)
    {
        /* Always mark consistent because the header isn't
         * modified after initialization.
         */
        rc = pthread_mutex_consistent(&header->lock);
        if (rc != 0)
        {
            OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT(OSAPI_LOGKIND_ERROR, rc);
            return RTI_FALSE;
        }
    }
    else if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_LOCK(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    /* Check if the segment has been unlinked */
    if (!OSAPI_SharedMemorySegment_is_unlinked(handle, &unlinked))
    {
        OSAPI_LOG_SHMEM_CHECK_UNLINKED(OSAPI_LOGKIND_ERROR, header->name);
        goto done;
    }
    if (unlinked)
    {
        /* Already unlinked by another process */
        success = RTI_TRUE;
        goto done;
    }

    /* Prepend a leading slash to the segment name and check the length of the
     * resulting string.
     */
    rc = snprintf(slash_name,
             OSAPI_SHMEM_MAX_NAME_LENGTH + 1,
             "/%s",
             header->name);
    if ((rc < 0) || (rc >= OSAPI_SHMEM_MAX_NAME_LENGTH + 1))
    {
        OSAPI_LOG_SHMEM_INVALID_SEGMENT_NAME(OSAPI_LOGKIND_ERROR, header->name);
        goto done;
    }
    /* Mark the segment as destroyed with shm_unlink().
     * The segment is only destroyed by the OS when the last handle
     * detaches from it.
     */
    if (shm_unlink(slash_name))
    {
#if OSAPI_ENABLE_LOG
        errnum = errno;
        OSAPI_LOG_SHMEM_UNLINK(OSAPI_LOGKIND_ERROR, errnum);
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    /* While deleting, it would be desirable to destroy the mutexes contained
     * in the header. However, it unfeasible to do so. The destruction needs
     * to be synchronized such that it is only done once, but the mutex being
     * destroyed cannot be used for synchronization.
     * On most POSIX platforms, a mutex doesn't allocate any resources, so
     * we do not try to destroy it.
     */

    success = RTI_TRUE;
done:
    /* The return values below are ignored because the segment is being deleted
     * and no action could be taken on a returned error at this point in the
     * function.
     */
    rc = pthread_mutex_unlock(&header->lock);
    IGNORE_RETVAL(rc);
    rc = OSAPI_SharedMemorySegment_detach_impl(handle);
    IGNORE_RETVAL(rc);
    return success;
}

RTI_BOOL
OSAPI_SharedMemorySegment_lock_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle,
        OSAPI_SHMEM_STATUS *status_out)
{
    int rc;
    struct OSAPI_SharedMemorySegmentHeaderImplLockable *lockable_header =
            (struct OSAPI_SharedMemorySegmentHeaderImplLockable *)handle->ptr_header;

    /* Justification: This function can only be called on a user_lock which is
     * is unlocked by the calling thread. The expected behavior is to
     * take the lock, so it is safe to call pthread_mutex_lock here.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    rc = pthread_mutex_lock(&lockable_header->user_lock);

    if (rc == EOWNERDEAD)
    {
        *status_out = OSAPI_SHMEM_STATUS_OWNER_DEAD;
        return RTI_TRUE;
    }
    else if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_LOCK(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    *status_out = OSAPI_SHMEM_STATUS_OK;
    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_unlock_impl(struct OSAPI_SharedMemorySegmentHandle *handle)
{
    int rc;

    struct OSAPI_SharedMemorySegmentHeaderImplLockable *lockable_header =
            (struct OSAPI_SharedMemorySegmentHeaderImplLockable *)handle->ptr_header;

    /* Justification: This function can only be called on a user_lock which is
     * is locked by the calling thread. The expected behavior is to
     * release the lock, so it is safe to call pthread_mutex_unlock here.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    rc = pthread_mutex_unlock(&lockable_header->user_lock);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_UNLOCK(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemorySegment_mark_consistent_impl(
        struct OSAPI_SharedMemorySegmentHandle *handle)
{
#if OSAPI_ROBUST_LOCKING_ENABLED
    int rc;

    struct OSAPI_SharedMemorySegmentHeaderImplLockable *lockable_header =
            (struct OSAPI_SharedMemorySegmentHeaderImplLockable *)handle->ptr_header;

    rc = pthread_mutex_consistent(&lockable_header->user_lock);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
#else
    UNUSED_ARG(handle);
    return RTI_FALSE;
#endif /* OSAPI_ROBUST_LOCKING_ENABLED */
}

