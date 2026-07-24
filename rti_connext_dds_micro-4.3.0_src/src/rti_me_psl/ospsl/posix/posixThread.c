/*
 * FILE: posixThread.c - POSIX thread functionality
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI thread routines
 */
#include "rti_me_psl.h"
#include "posixThread.h"

/*** SOURCE_BEGIN ***/

struct OSAPI_PosixNativeThread
{
    char name[64];

    pthread_t native_thread_handle;

    OSAPI_ThreadRoutine thread_routine;

    struct OSAPI_ThreadInfo thread_info;

    OSAPI_ThreadRoutine thread_wakeup;

    OSAPI_ThreadHandle thread_handle;

#if (ENABLE_FACE_COMPLIANCE != FACE_COMPLIANCE_LEVEL_NONE) && \
     (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFETY_BASE_EXTENDED)
    volatile RTI_BOOL running;
#endif
};

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
OSAPI_Thread_set_name(struct OSAPI_PosixNativeThread *native_thread)
{
#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) && !OSAPI_ENABLE_STRICT_POSIX
    /* NOTE: The thread's name is set in the context of the thread,
     * _not_ when it is created. For some reason that does not work on Linux
     * and on Darwin it is not possible to specify which thread to set the name
     * on
     */
#if defined(__QNXNTO__) || \
    (defined(__linux__) && ((__GLIBC__ > 2) || \
     ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 12))))
    int rc = 0;
    rc = pthread_setname_np(native_thread->native_thread_handle,
                            native_thread->name);
    if (rc != 0)
    {
#if OSAPI_ENABLE_LOG
        OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_ERROR,rc)
#endif
    }
#define OSAPI_HAVE_SETNAME 1
#elif __APPLE__
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_6
    int rc = 0;
    rc = pthread_setname_np(native_thread->name);
    if (rc != 0)
    {
#if OSAPI_ENABLE_LOG
        OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_ERROR,rc)
#endif
    }
#define OSAPI_HAVE_SETNAME 1
#endif /* __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_6*/
#endif /* __MAC_OS_X_VERSION_MAX_ALLOWED */
#endif /* __APPLE__ */
#endif

#if !OSAPI_HAVE_SETNAME
    UNUSED_ARG(native_thread);
#if OSAPI_ENABLE_LOG
    OSAPI_LOG_SET_THREAD_NAME(OSAPI_LOGKIND_WARNING,EOPNOTSUPP)
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

RTI_PRIVATE void*
OSAPI_Thread_start_native(void *param)
{
    struct OSAPI_PosixNativeThread *native_thread = 
                                (struct OSAPI_PosixNativeThread*)param;

#ifndef RTI_CERT
    OSAPI_Thread_set_name(native_thread);
#endif
#if (ENABLE_FACE_COMPLIANCE != FACE_COMPLIANCE_LEVEL_NONE) && \
     (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFETY_BASE_EXTENDED)
    native_thread->running = RTI_TRUE;
#endif
    native_thread->thread_routine(&native_thread->thread_info);

#if (ENABLE_FACE_COMPLIANCE != FACE_COMPLIANCE_LEVEL_NONE) && \
     (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFETY_BASE_EXTENDED)
    native_thread->running = RTI_FALSE;
#endif
    return NULL;
}

RTI_BOOL
OSAPI_Thread_destroy_posix_native(struct OSAPI_PosixNativeThread *thread)
{
    thread->thread_info.stop_thread = RTI_TRUE;
    if (thread->thread_wakeup != NULL)
    {
        thread->thread_wakeup(&thread->thread_info);
    }

#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
     (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)
    {
        int rc;
        rc = pthread_join(thread->native_thread_handle,NULL);

        if (rc != 0)
        {
            OSAPI_LOG_THREAD_JOIN(OSAPI_LOGKIND_ERROR,rc);
            return RTI_FALSE;
        }
    }
#else
    while (thread->running);
#endif

#ifndef RTI_CERT
    OSAPI_Heap_free_buffer(thread);
#endif

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
struct OSAPI_PosixNativeThread*
OSAPI_Thread_create_posix_native(const char *name,
                                 const struct OSAPI_ThreadProperty *property,
                                 OSAPI_ThreadRoutine thread_entry,
                                 void *thread_data,
                                 OSAPI_ThreadRoutine thread_wakeup)
{
    pthread_attr_t attr;
    RTI_SIZE_T stack = 0;
    struct sched_param sch_param;
    int rc;
    RTI_SIZE_T len;
    struct OSAPI_PosixNativeThread *native_thread;

    /* Initialize to 0 to avoid a compiler warning. priority is only
     * used if priority is not inherited, in which case it is assigned
     * a native value.
     */
    int priority = 0;
    
    OSAPI_PRECONDITION((property == NULL) || (name == NULL) ||
                       (thread_entry == NULL),
                return NULL,
                OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("thread_entry",
                       thread_entry == NULL ? NULL : (void*)1,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    OSAPI_Heap_allocate_buffer((char**)&native_thread,
                               sizeof(struct OSAPI_PosixNativeThread),
                               OSAPI_ALIGNMENT_DEFAULT);
    if (native_thread == NULL)
    {
        return NULL;
    }

    native_thread->thread_routine = thread_entry;
    native_thread->thread_wakeup = thread_wakeup;

    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= 64)
    {
        len = 64-1;
    }
    OSAPI_Memory_copy(native_thread->name,name,len);
    native_thread->name[len] = 0;
    native_thread->thread_info.stop_thread = RTI_FALSE;
    native_thread->thread_info.user_data = thread_data;
    native_thread->thread_info.is_preemptive = RTI_TRUE;

    rc = pthread_attr_init(&attr);
    if (rc != 0)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
        goto fail;
    }

#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
     (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)
    /* see MICRO-6807 for details on why threads are joinable */
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
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
    if (property->options & OSAPI_THREAD_REALTIME_PRIORITY)
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
    if (!OSAPI_Thread_map_to_native_priority(&priority, property->priority))
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,property->priority)
        goto fail;
    }

    /* temporarily disabled since MAG doesn't allow changing it.
     */
    if ((property->priority != OSAPI_THREAD_PRIORITY_INHERIT) && 
        (property->priority != OSAPI_THREAD_PRIORITY_NORMAL))
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
            OSAPI_LOG_THREAD_SCHEDPARAM(OSAPI_LOGKIND_ERROR,rc,property->priority)
            goto fail;
        }
    }

    if (property->stack_size != OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack = property->stack_size;
        rc = pthread_attr_setstacksize(&attr, stack);
        if (rc != 0)
        {
            OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,rc)
            goto fail;
        }
    }

    rc = pthread_create(&(native_thread->native_thread_handle),&attr,
                        OSAPI_Thread_start_native,(void *)native_thread);

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

    return native_thread;

fail:
#ifndef RTI_CERT
    if (native_thread != NULL)
    {
        OSAPI_Heap_free_buffer(native_thread);
    }
#endif
    return NULL;
}

OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup)
{
    struct OSAPI_PosixNativeThread *native_thread;

    native_thread = OSAPI_Thread_create_posix_native(name,property,thread_entry,thread_data,thread_wakeup);

    return (OSAPI_ThreadHandle)native_thread;
}

MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle)
{
    struct OSAPI_PosixNativeThread *native_thread = 
                        (struct OSAPI_PosixNativeThread*)handle;

    return OSAPI_Thread_destroy_posix_native(native_thread);
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{
    OSAPI_ThreadId tid;
    
    tid.handle.handle64 = (RTI_UINT64)pthread_self();
    
    return tid;
}

#if OSAPI_ENABLE_LOG
RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return errno;
}

void
OSAPI_Log_set_last_error_code(RTI_INT32 err)
{
    errno = err;
}
#endif

void
OSAPI_Thread_nanosleep(RTI_UINT32 ns)
{
    struct timespec remain,next;
    int rval;

    next.tv_sec = ns / 1000000000;
    next.tv_nsec = ns % 1000000000;

#if OSAPI_HAVE_NANOSLEEP
    do
    {
        rval = nanosleep(&next,&remain);
        if ((rval == -1) && (errno == EINTR))
        {
            next = remain;
        }
    } while ((rval == -1) && (errno == EINTR));

#elif OSAPI_HAVE_CLOCK_NANOSLEEP
    do
    {
        rval = clock_nanosleep(CLOCK_REALTIME,0,&next,&remain);
        if (rval == EINTR)
        {
            next = remain;
        }
    } while (rval == EINTR);
#else
#error "No clock function available to implement sleep"
#endif
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
