/*
 * FILE: posixSempahore.c - POSIX semaphore functionality
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 09nov2021,tk MICRO-3330/PR.29900
 * - Updated OSAPI_SystemPosix_semaphore_timeout to not take mutex and update
 *   local state if the timeout is OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL.
 * - Updated OSAPI_Semaphore_take to check if the timeout is
 *   OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL and if so do not update
 *   local state variables.
 * - Return when the condition is signaled and the timeout is
 *   OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL.
 * - Assign ticks OSAPI_SEMAPHORE_TIMEOUT_INFINITE instead of -1 in
 *   OSAPI_Semaphore_take when timeout is OSAPI_SEMAPHORE_TIMEOUT_INFINITE.
 * - Replaced if (failReason) with if (failReason != NULL) in
 *   OSAPI_Semaphore_take.
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in OSAPI_SystemPosix_semaphore_timeout()
 * 05jan2021,tk
 *   MICRO-2775/PR#28502
 *     - Removed redundant assignment to fail_reason in OSAPI_Semaphore_take().
 *     - Explicitly ignore unused return code from OSAPI_SystemPosix_unlock()
 *       in OSAPI_Semaphore_take().
 *     - Return RTI_FALSE in OSAPI_Semaphore_take() if the global lock
 *       cannot be taken.
 *     - Removed redundant assignment of RTI_TRUE to result in
 *       OSAPI_Semaphore_take().
 *     - Set result to RTI_FALSE if OSAPI_SystemPosix_unlock() fails in
 *       OSAPI_Semaphore_take().
 * 21dec2020,fmt MICRO-2774/PR#28501 
 *   - OSAPI_Semaphore_new() should return error if pthread_mutex_init() fails.
 * 22apr2014,tk MICRO-772: OSAPI_Semaphore_take() can handle spurious wakeups
 * 12aug2012,tk Updated
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI semaphore routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_POSIX

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif

#include "posixSystem.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_POSIX

/* PRIVATE API */

/*ci \brief Wakeup a semaphore
 *
 * \param[in] self The semaphore to wakeup
 * \param[in] timed_out TRUE if the semaphore timed out or was removed
 *
 * \return TRUE  if the call suceeded, FALSE otherwise
 */
RTI_BOOL
OSAPI_SystemPosix_semaphore_timeout(OSAPI_Semaphore_T *self,RTI_BOOL timed_out)
{
    RTI_BOOL result = RTI_FALSE;
    int rc;
    RTI_BOOL mutex_taken = RTI_FALSE;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!timed_out && (self->ticks != OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL))
    {
        /* For a semaphore with external timeout do not take the mutex as this
         * function may be called from a signal handler.
         */
        mutex_taken = RTI_TRUE;

        /* start by locking the control mutex, then verify that we
         * can/need to give the semaphore.
         */
        rc = pthread_mutex_lock(&self->sem_mutex);
        if (rc != 0)
        {
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR,rc)
            return RTI_FALSE;
        }
    }

    /* Binary semaphore: don't re-increment if full (but return success) */
    if (self->sem_count > 0)
    {
        result = RTI_TRUE;
    }
    else
    {
        /* OK, give the semaphore and signal via the condition variable.
         * If self->ticks == OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL it is a special
         * case with an external timeout and in this case do not update the
         * semaphore because the semaphore variables are not protected.
         */
        if (self->ticks != OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL)
        {
            self->sem_count++;
            self->timed_out = timed_out;
            self->remove = RTI_TRUE;
        }
        rc = pthread_cond_signal(&self->sem_given);
        if (rc == 0)
        {
            result = RTI_TRUE;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR,rc)
        }
#endif
    }

    if (mutex_taken)
    {
        rc = pthread_mutex_unlock(&self->sem_mutex);
        if (rc != 0)
        {
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR,rc)
            return RTI_FALSE;
        }
    }

    return result;
}

/* PUBLIC API */

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *self)
{
    int rc;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    OSAPI_SystemPosix_remove_timed_sem(self);

    rc = pthread_mutex_destroy(&self->sem_mutex);
    if (rc != 0)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR,rc)
        return RTI_FALSE;
    }

    rc = pthread_cond_destroy(&self->sem_given);
    if (rc != 0)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR,rc)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(self);

    return RTI_TRUE;
}
#endif

OSAPI_Semaphore_T*
OSAPI_Semaphore_new(void)
{
    struct OSAPI_Semaphore *me;
    int rc;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Semaphore);

    if (me == NULL)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    rc = pthread_cond_init(&me->sem_given, NULL);
    if (rc != 0)
    {
        OSAPI_LOG_SEMAPHORE_NEW_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto failure;
    }

    rc = pthread_mutex_init(&me->sem_mutex, NULL);
    if (rc != 0)
    {
        rc = pthread_cond_destroy(&me->sem_given);
        /* The function is already returning an error so
         * we can ignore this return value
         */
        IGNORE_RETVAL(rc);

        OSAPI_LOG_SEMAPHORE_NEW_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto failure;
    }

    me->sem_count = 0;
    me->next = NULL;
    me->prev = NULL;
    me->remove = RTI_FALSE;

    return me;

failure:

#ifndef RTI_CERT
    OSAPI_Heap_free_struct(me);
#endif /* RTI_CERT */

    return NULL;
}

/* Attempt to take semaphore with the specified timeout (can be inifnite).
 * if RTI_TRUE is returned, fail_reason will contain the error condition
 * (ie, errno, etc). fail_reason is OS-specific (ie, not wrapped around by
 * OSAPI-defined reasons)
 */
RTI_BOOL
OSAPI_Semaphore_take(OSAPI_Semaphore_T *self,
                     RTI_INT32 timeout_ms,
                     RTI_INT32 *fail_reason)
{
    int status;
    RTI_BOOL result = RTI_TRUE;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (fail_reason != NULL)
    {
        *fail_reason = OSAPI_SEMAPHORE_RESULT_OK;
    }

    if (!OSAPI_SystemPosix_lock())
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        return RTI_FALSE;
    }

    status = pthread_mutex_lock(&self->sem_mutex);
    if (status != 0)
    {
        RTI_BOOL bretval;

        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)

        bretval = OSAPI_SystemPosix_unlock();

        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }

        /* Ignore the return value since this function has already failed
         * and FALSE is returned.
         */
        IGNORE_RETVAL(bretval);
        return RTI_FALSE;
    }

    self->timed_out = RTI_FALSE;

    if (self->sem_count > 0)
    {
        /* semaphore already available */
        self->sem_count--;
        if (!OSAPI_SystemPosix_unlock())
        {
            if (fail_reason != NULL)
            {
                *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
            }

            result = RTI_FALSE;
        }
        goto done;
    }

    /* A timeout of 0 returns immediately. Set the reason to timeout here
     * since if the semaphore was already available it would have returned
     * earlier.
     */
    if (timeout_ms == 0)
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
        }
    }
    else if (timeout_ms == OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL)
    {
        self->ticks = OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL;
    }
    else if (timeout_ms != OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
    {
        /* timeout_ms > 0 and finite. Always round up */
        RTI_INT32 ms_res = OSAPI_System_get_timer_resolution() / OSAPI_NTP_TIME_USEC_PER_SEC;

        self->ticks = timeout_ms / ms_res;
        if (timeout_ms % ms_res)
        {
            ++self->ticks;
        }

        /* A timed semaphore is very likely scheduled between two ticks. Thus
         * add 1 tick to ensure that timeout is never shorter than requested.
         * At the worst it will be just under two ticks longer (1 for rounded
         * up, 1 for compensate for scheduling in the middle of a tick).
         */
        ++self->ticks;

        OSAPI_SystemPosix_add_timed_sem(self);
        self->remove = RTI_FALSE;
    }
    else
    {
        self->ticks = OSAPI_SEMAPHORE_TIMEOUT_INFINITE;
    }

    if (!OSAPI_SystemPosix_unlock())
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        result = RTI_FALSE;
        goto done;
    }

    if (timeout_ms == 0)
    {
        /* Finish here to avoid blocking */
        goto done;
    }

    /* Wait forever, timed semaphores are handled by the system timer. This
     * ensure that timeouts are not subject to changes in the system
     * clock and that precision of timed semaphores can be overridden
     */
wait_cond_again:
    status = pthread_cond_wait(&self->sem_given, &self->sem_mutex);

    if (self->ticks == OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL)
    {
        /* for a semaphore with external timeout, always return
         * immediately.
         */
        if (status != 0)
        {
            result = RTI_FALSE;
            OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)
            if (fail_reason != NULL)
            {
                *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
            }
        }
        goto done;
    }

    if (self->sem_count == 0)
    {
        goto wait_cond_again;
    }

    if ((status == 0) && !self->timed_out)
    {
        self->sem_count--;
    }
    else if (self->timed_out)
    {
        /* Substract one because the timeout triggered via the
         * system timer incremented the count by 1.
         */
        self->sem_count--;
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
        }
    }
    else
    {
        result = RTI_FALSE;
        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
    }

done:
    status = pthread_mutex_unlock(&self->sem_mutex);

    if (status != 0)
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)
        return RTI_FALSE;
    }

    return result;
}

RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *self)
{
    return OSAPI_SystemPosix_semaphore_timeout(self,RTI_FALSE);
}

#endif
