/*
 * FILE: posixShmMonitor.c - POSIX shared memory monitor implementation
 *
 * Copyright 2022-2022 Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rti_me_psl.h"

#include <stdio.h>
#include "posixShmMonitor.h"
#include "netio_zcopy/netio_zcopy_default_notif_mech.h"
#include <pthread.h>

#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "osapi/osapi_log.h"



/*** SOURCE_BEGIN ***/



RTI_SIZE_T
OSAPI_SharedMemoryMonitor_get_size(void)
{
    return sizeof(struct OSAPI_SharedMemoryMonitor);
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_initialize(
        RTI_SIZE_T mem_len,
        void *mem,
        struct OSAPI_SharedMemoryMonitor **self_out)
{
    RTI_BOOL result = RTI_FALSE;
    struct OSAPI_SharedMemoryMonitor *self;
    int rc;

    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    /* Check preconditions */
    OSAPI_PRECONDITION(self_out == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self_out", self_out, RTI_TRUE);)

    *self_out = NULL;

    /* Align memory and verify that the aligned memory is large enough */
    self = (struct OSAPI_SharedMemoryMonitor *)OSAPI_SHARED_MEMORY_ALIGN(
            (RTI_UINT64)mem,
            (RTI_UINT64)OSAPI_SHARED_MEMORY_ALIGNMENT);
    if ((char *)self + OSAPI_SharedMemoryMonitor_get_size() > (char *)mem + mem_len)
    {
        goto done;
    }

    /* Initialize the mutex for inter-process usage */
    rc = pthread_mutexattr_init(&mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETPSHARED(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
#if OSAPI_ROBUST_LOCKING_ENABLED
    rc = pthread_mutexattr_setrobust(&mutexattr, PTHREAD_MUTEX_ROBUST);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_SETROBUST(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
#endif /* OSAPI_ROBUST_LOCKING_ENABLED */
    rc = pthread_mutex_init(&self->mutex, &mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_mutexattr_destroy(&mutexattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEXATTR_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    /* Initialize the cond for inter-process usage */
    rc = pthread_condattr_init(&condattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_CONDATTR_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_CONDATTR_SETPSHARED(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_cond_init(&self->cond, &condattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_COND_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }
    rc = pthread_condattr_destroy(&condattr);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_CONDATTR_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        goto done;
    }

    *self_out = self;
    result = RTI_TRUE;

done:
    return result;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_SharedMemoryMonitor_finalize(struct OSAPI_SharedMemoryMonitor *self)
{
    RTI_BOOL result = RTI_TRUE;
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* Destroy everything */
    rc = pthread_mutex_destroy(&self->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        result = RTI_FALSE;
    }
    rc = pthread_cond_destroy(&self->cond);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_COND_DESTROY(OSAPI_LOGKIND_ERROR, rc)
        result = RTI_FALSE;
    }

    return result;
}
#endif /* !RTI_CERT */

RTI_BOOL
OSAPI_SharedMemoryMonitor_acquire(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out)
{
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION((self == NULL) || (status_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status_out", status_out, RTI_TRUE);)

    /* Justification: This function can only be called on a monitor which is
     * also unlocked by the calling thread. The expected behavior is to
     * release the lock in OSAPI_SharedMemoryMonitor_release.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    rc = pthread_mutex_lock(&self->mutex);
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
OSAPI_SharedMemoryMonitor_release(struct OSAPI_SharedMemoryMonitor *self)
{
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* Justification: This function can only be called on a monitor which is
     * currently locked by the calling thread. The expected behavior is to
     * release the lock, so it is safe to call pthread_mutex_unlock here. The
     * lock is acquired by the calling thread by using
     * OSAPI_SharedMemoryMonitor_acquire
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    rc = pthread_mutex_unlock(&self->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_UNLOCK(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_wait(
        struct OSAPI_SharedMemoryMonitor *self,
        OSAPI_SHMEM_STATUS *status_out)
{
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION((self == NULL) || (status_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("status_out", status_out, RTI_TRUE);)

    rc = pthread_cond_wait(&self->cond, &self->mutex);
    if (rc == EOWNERDEAD)
    {
        *status_out = OSAPI_SHMEM_STATUS_OWNER_DEAD;
        return RTI_TRUE;
    }
    else if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_COND_WAIT(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    *status_out = OSAPI_SHMEM_STATUS_OK;
    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_signal(struct OSAPI_SharedMemoryMonitor *self)
{
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    rc = pthread_cond_signal(&self->cond);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_COND_SIGNAL(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_SharedMemoryMonitor_mark_consistent(struct OSAPI_SharedMemoryMonitor *self)
{
#if OSAPI_ROBUST_LOCKING_ENABLED
    int rc;

    /* Check preconditions */
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    rc = pthread_mutex_consistent(&self->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_PTHREAD_MUTEX_CONSISTENT(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
#else
    UNUSED_ARG(self);
    return RTI_FALSE;
#endif /* OSAPI_ROBUST_LOCKING_ENABLED */
}


