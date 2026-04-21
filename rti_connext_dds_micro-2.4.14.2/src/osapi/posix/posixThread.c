/*
 * FILE: posixThread.c - POSIX thread functionality
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
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 * - Added suppression of cert_pos54_c_violation in OSAPI_Thread_create
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in OSAPI_Thread_get_policy()
 * 23feb2021,tk MICRO-2914/PR#28853
 *    - Set the priority unless OSAPI_THREAD_PRIORITY_INHERIT is set.
 * 13jan2021,tk MICRO-2807/PR#27549 Removed unused functions for CERT.
 * 10jan2021,tk MICRO-2807/PR.27549  Removed unused functions for CERT
 * 13dec2020,tk
 *    - MICRO-2681 / PR.28253 Added user_routine to OSAPI_PRECONDITION check in
 *      OSAPI_Thread_create()
 * 13dec2020,tk
 *    - MICRO-2617 / PR.28218 Removed OSAPI_Thread_get_policy() for QNX and
 *      only use SCHED_FIFO to determine min and max priorities.
 * 13dec2020,tk
 *    - MICRO-2621 / PR.28228 Made OSAPI_Thread_map_to_native_priority() more robust.
 * 13dec2020,tk
 *    - MICRO-2682/PR.28258
 *      - Fail if one of pthread_attr_setscope(),pthread_attr_setschedpolicy(),
 *        or pthread_attr_setinheritsched() returns an error when RTI_CERT is
 *        enabled.
 *      - Clarified why the return value from pthread_attr_destroy() is
 *        ignored for CERT.
 * 13dec2020,tk MICRO-2740/PR.28040
 *              - Renamed is_premptive to is_preemptive
 * 01oct2015,tk MICRO-638  Set thread-name when supported
 * 01oct2015,tk MICRO-1502 Explicitly set PTHREAD_EXPLICIT_SCHED
 * 26mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI thread routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_POSIX

#include "osapi/osapi_types.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"

#include "../common/Thread.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_POSIX

#ifndef RTI_CERT
/*ci \brief Callback installed to update the thread before it is started
 *
 * \details
 *
 * This function sets the thread name before it is started.
 *
 * \param[in] self The thread to update
 */
RTI_PRIVATE void
OSAPI_Thread_on_before_start(struct OSAPI_Thread *self)
{
#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) && !OSAPI_ENABLE_STRICT_POSIX
    /* NOTE: The thread's name is set in the context of the thread,
     * _not_ when it is created. For some reason that does not work on Linux
     * and on Darwin it is not possible to specify which thread to set the name
     * on
     */
#if defined(__linux__) && (__GLIBC__ > 2) || (__GLIBC__ == 2 && (__GLIBC_MINOR__ >= 12))
    int rc = 0;
    rc = pthread_setname_np(self->thread_handle,self->name);
    if (rc != 0)
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_ERROR,rc)
        }
#endif
    }
#define OSAPI_HAVE_SETNAME 1
#elif __APPLE__
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_6
    int rc = 0;
    rc = pthread_setname_np(self->name);
    if (rc != 0)
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_ERROR,rc)
        }
#endif
    }
#define OSAPI_HAVE_SETNAME 1
#endif /* __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_6*/
#endif /* __MAC_OS_X_VERSION_MAX_ALLOWED */
#endif /* __APPLE__ */
#endif

#if !OSAPI_HAVE_SETNAME
    UNUSED_ARG(self);
#if OSAPI_ENABLE_LOG
    if (OSAPI_Log_is_initialized())
    {
        OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_WARNING,EOPNOTSUPP)
    }
#endif
#endif
}
#endif

#ifndef RTI_QNX
/*ci \brief Get the thread scheduling policy for the thread
 *
 *  \return The scheduling policy of the thread
 */
MUST_CHECK_RETURN RTI_PRIVATE int
OSAPI_Thread_get_policy(void)
{
    int sched_policy = SCHED_FIFO;
    int current_policy;
    struct sched_param dummy;
    int rc;

    /* If we're being spawned from a real-time thread, use the parent's policy
     * for the child instead of forcing SCHED_FIFO. */
    rc = pthread_getschedparam(pthread_self(), &current_policy, &dummy);

    if (rc == 0)
    {
        if ((current_policy == SCHED_RR) || (current_policy == SCHED_FIFO))
        {
            sched_policy = current_policy;
        }
    }
#if OSAPI_ENABLE_LOG
    else
    {
        OSAPI_LOG_THREAD_GET_POLICY(OSAPI_LOGKIND_ERROR,rc)
    }
#endif
    return sched_policy;
}
#endif

/*ci \brief Map the user defined priority into native priorty
 *
 * \details
 *
 * Map the user defined priority to native priority. Zero and positive
 * numbers are used as is, negative numbers are converted into a range as
 * a percentage of min and max supported. If priority_level >= 0 is outside
 * the min/max supported by the system an error is returned and if a
 * priority level outside < 0 is outside
 * [OSAPI_THREAD_PRIORITY_LOW,OSAPI_THREAD_PRIORITY_INHERIT] an error is returned.
 *
 * \param[out] native_priority The native priority after a successful mapping.
 *                             For logical priorities between
 *                             [OSAPI_THREAD_PRIORITY_LOW,
 *                             OSAPI_THREAD_PRIORITY_HIGH] the result is
 *                             rounded down. If priority_level ==
 *                             OSAPI_THREAD_PRIORITY_INHERIT, then the
 *                             native_priority is _not_ assigned a value.
 * \param[in] priority_level The user-defined priority to be mapped.
 *
 * \return RTI_TRUE for a successful mapping, RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_Thread_map_to_native_priority(int *native_priority, int priority_level)
{
    int min, max;
#ifndef RTI_QNX
    int sched_policy;

    sched_policy = OSAPI_Thread_get_policy();
    min = sched_get_priority_min(sched_policy);
    max = sched_get_priority_max(sched_policy);
#else
    min = sched_get_priority_min(SCHED_FIFO);
    max = sched_get_priority_max(SCHED_FIFO);
#endif

    *native_priority  = 0;

    if ((min == -1) || (max == -1))
    {
        OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,min,max)
        return RTI_FALSE;
    }

    if (priority_level >= 0)
    {
        if ((priority_level < min) || (priority_level > max))
        {
            OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,min,max)
            return RTI_FALSE;
        }

        *native_priority = priority_level;
        return RTI_TRUE;
    }

    if (priority_level < OSAPI_THREAD_PRIORITY_INHERIT)
    {
        OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,
                                      OSAPI_THREAD_PRIORITY_LOW,
                                      OSAPI_THREAD_PRIORITY_INHERIT)
        return RTI_FALSE;
    }

    if (priority_level == OSAPI_THREAD_PRIORITY_INHERIT)
    {
        return RTI_TRUE;
    }

    /* posix thread gives us min and max, so we interpolate for the desired
     * value and round down.
     */
    *native_priority = min + (((max - min) * priority_level)/OSAPI_THREAD_PRIORITY_HIGH);

    return RTI_TRUE;
}

/*ci \brief Create a new thread
 *
 * \details
 * Allocates needed memory and spawns the thread. the spawned thread pends
 * until start() is called before actually running user's routine. thread
 * spawning and pending states are guaranteed when RTI_TRUE is returned
 *
 * \param[in] name The name of the thread. Must not be NULL.
 * \param[in] properties The thread properties. Must not be NULL.
 * \param[in] user_routine The user defined function to called from the
 *                         created thread. Must not be NULL.
 * \param[in] wakeup_routine Optional function to unblock a thread so it can
 *                           be deleted.
 *
 * \return NULL on failure, pointer to created thread on success.
 */
struct OSAPI_Thread*
OSAPI_Thread_create(const char *name,
                    const struct OSAPI_ThreadProperty *properties,
                    OSAPI_ThreadRoutine user_routine,
                    void *user_param, OSAPI_ThreadRoutine wakeup_routine)
{
    pthread_attr_t attr;
    RTI_SIZE_T stack = 0;
    struct sched_param sch_param;
    int rc;
    /* Initialize to 0 to avoid a compiler warning. priority is only
     * used if priority is not inherited, in which case it is assigned
     * a native value.
     */
    int priority = 0;
    struct OSAPI_Thread *me;
    RTI_SIZE_T len;

    OSAPI_PRECONDITION((properties == NULL) || (name == NULL) ||
                       (user_routine == NULL),
                       return NULL,
               OSAPI_Log_entry_add_pointer("properties",properties,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("user_routine",
                       user_routine == NULL ? NULL : (void*)1,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    /* create the thread handle */
    me = OSAPI_Thread_new(properties);
    if (me == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    /* asign user's values */
    me->user_routine = user_routine;
    me->wakeup_routine = wakeup_routine;
    me->thread_info.user_data = user_param;
    me->thread_info.is_preemptive = RTI_TRUE;
#ifndef RTI_CERT
    me->on_before_start = OSAPI_Thread_on_before_start;
#else
    me->on_before_start = NULL;
#endif
    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= OSAPI_THREAD_MAX_NAME)
    {
        len = OSAPI_THREAD_MAX_NAME-1;
    }
    OSAPI_Memory_copy(me->name,name,len);
    me->name[len] = 0;

    rc = pthread_attr_init(&attr);
    if (rc != 0)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto fail;
    }

#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
     (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)

    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (rc != 0)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto fail;
    }
#endif

    /* Set the scheduling policy. This can fail on certain operating systems, due 
     * to the lack of superuser privileges of the application. If this happens,
     * the default scheduling policy is used. This occurs i.e. on Linux: changing 
     * the policy from the default SCHED_OTHER to either SCHED_FIFO or SCHED_RR
     * require superuser privileges.
     */
    if (properties->options & OSAPI_THREAD_REALTIME_PRIORITY)
    {
        rc = pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
        if (rc != 0)
        {
#ifdef RTI_CERT
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
            goto fail;
#else
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_WARNING,rc)
#endif
        }

        rc = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        if (rc != 0)
        {
#ifdef RTI_CERT
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
            goto fail;
#else
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_WARNING,rc)
#endif
        }
    }

#if !defined(__ANDROID__)
    /* Make sure that the attributes here are used, the
     * default behavior is to inherit from the creating thread
     * PTHREAD_INHERIT_SCHED.
     *
     * NOTE 1: It may or may not be possible to change the priority,
     *         depending on the scheduling policy (only SCHED_FIFO supported).
     *
     * NOTE 2: Even if OSAPI_THREAD_REALTIME_PRIORITY is not set,
     *         this call succeeds but the priority cannot be set
     *         (even as root). However, don't be too restrictive
     *         here as some system may allow it.
     */
    rc = pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    if (rc != 0)
    {
#ifdef RTI_CERT
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto fail;
#else
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_WARNING,rc)
#endif
    }
#endif

    /* Calculate and set the priority. This should be done after the scheduling
     * policy has been adjusted. Setting the priority is scheduling policy
     * dependent. I.e. On Linux, changing the priority for the default
     * SCHED_OTHER policy will fail.changing the priority is only applicable for
     * SCHED_FIFO and SCHED_RR scheduling policies.
     */
    if (!OSAPI_Thread_map_to_native_priority(&priority, properties->priority))
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,properties->priority)
        goto fail;
    }

    if (properties->priority != OSAPI_THREAD_PRIORITY_INHERIT)
    {
        OSAPI_Memory_zero(&sch_param,sizeof(sch_param));
        /* only alter priority if it is not explicitly set to
         * OSAPI_THREAD_PRIORITY_INHERIT, which uses the priority inherited
         * from the parent process.
         */
        sch_param.sched_priority = priority;

        rc = pthread_attr_setschedparam(&attr, &sch_param);
        if (rc != 0)
        {
            OSAPI_LOG_THREAD_SCHEDPARAM(OSAPI_LOGKIND_ERROR,rc,properties->priority)
            goto fail;
        }
    }

    if (properties->stack_size != OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack = properties->stack_size;
        rc = pthread_attr_setstacksize(&attr, stack);
        if (rc != 0)
        {
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
            goto fail;
        }
    }

    rc = pthread_create(&(me->thread_handle),&attr,
                        OSAPI_Thread_exec,(void *)me);

    if (rc != 0)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto fail;
    }

    /* rc is only used for logging purposes
     */
    /* coverity[cert_pos54_c_violation] */
    rc = pthread_attr_destroy(&attr);
#if OSAPI_ENABLE_LOG
    if (rc != 0)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_WARNING,rc)
    }
#else
    /* The attr variable is not accessed after the last successful call
     * using it and the call to pthread_attr_destroy(). Thus, the OS
     * must determine if a failure to destroy attr is a reason to enter a
     * fail-safe state or not.
     */
    IGNORE_RETVAL(rc);
#endif

    /* spawned thread gives semaphore immediately when started */
    if (!OSAPI_Semaphore_take(me->create_sem,
                             OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
    {
        OSAPI_LOG_THREAD_CREATE(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    return me;

fail:

    return NULL;
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{
    return pthread_self();
}

#ifndef RTI_CERT
void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    RTI_UINT32 is;
    struct timespec remain,next;
    int rval;

    is = ms / 1000;

    next.tv_sec = is;
    next.tv_nsec = (ms - (is * 1000U)) * 1000000U;

#if defined(__APPLE__)

    do
    {
        rval = nanosleep(&next,&remain);
        if ((rval == -1) && (errno == EINTR))
        {
            next = remain;
        }
    } while ((rval == -1) && (errno == EINTR));

#elif !defined(USE_TIMER_THREAD_SLEEP) && \
      (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE)

    do
    {
        rval = clock_nanosleep(CLOCK_REALTIME,0,&next,&remain);
        if (rval == EINTR)
        {
            next = remain;
        }
    } while (rval == EINTR);

#else

    do
    {
        rval = nanosleep(&next,&remain);
        if ((rval == -1) && (errno == EINTR))
        {
            next = remain;
        }
    } while ((rval == -1) && (errno == EINTR));

#endif
}
#endif

#endif
