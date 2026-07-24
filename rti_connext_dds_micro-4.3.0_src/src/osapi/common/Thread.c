/*
 * FILE: Thread.c - Platform independent Thread functionality
 *
 * (c) Copyright 2008-2024 Real-Time Innovations
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
 * 14oct2014,tk MICRO-946/PR# 12303 - Initialize name to empty string.
 *                                  - Added details to Thread_new() about
 *                                    uninitialized variables.
 * 20aug2008,tk Written
 */
/*ce
 * \file
 * \brief Implementation of the Thread API.
 *
 * \details
 * This file implements the platform independent thread APIs. Platform dependent
 * thread functionality is find in the platform specific code.
 */
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_system.h"
#include "Thread.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*e \ingroup OSAPI_ThreadClass
 *
 *  \brief Delete a thread data-structure
 *
 *  \details
 *  Delete a thread structure previsously created with OSAPI_Thread_new
 *
 *  \param [in] me - Thread structure to delete
 *
 *  \sa OSAPI_Thread_new
 */
void
OSAPI_Thread_delete(struct OSAPI_Thread *me)
{
    OSAPI_PRECONDITION(me == NULL,return,
            OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    if (!OSAPI_Thread_delete_native(me->thread_handle))
    {
        return;
    }

    if (me->create_sem != NULL)
    {
        OSAPI_Semaphore_delete(me->create_sem);
    }

    if (me->start_sem != NULL)
    {
        OSAPI_Semaphore_delete(me->start_sem);
    }

    OSAPI_Heap_free_buffer(me);

    OSAPI_TRACE_THREAD("deleted thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)
}
#endif /* !RTI_CERT */

#if !OSAPI_NO_THREADS
/*e \ingroup OSAPI_ThreadClass
 *
 *  \brief Create a thread.
 *
 *  \details
 *
 *  Create a new structure to hold thread information. This function is
 *  is called by the OS specific OSAPI_Thread_create function. Since the
 *  returned structure has platform specific fields it cannot be fully
 *  initialized. Specifically, the following fields are _not_ initialized
 *  by this function:
 *  \li thread_handle Initialized in OSAPI_Thread_create() function.
 *  \li tid           Initialized in OSAPI_Thread_exec() function after the
 *                    thread has been created.
 *
 *  \param[in] property The property the thread is created with
 *
 *  \return On success a new, initialized thread object is returned. Otherwise
 *          NULL is returned.
 *
 *  \sa \ref OSAPI_Thread_delete
 */
RTI_PRIVATE struct OSAPI_Thread*
OSAPI_Thread_new(const struct OSAPI_ThreadProperty *property)
{
    struct OSAPI_Thread *me;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Thread);

    if (me == NULL)
    {
        return NULL;
    }

    OSAPI_Memory_zero(me,sizeof(struct OSAPI_Thread));

    me->name[0] = 0;
    me->thread_options = property->options;

    me->create_sem = OSAPI_Semaphore_new();

    if (me->create_sem == NULL)
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_SEM(OSAPI_LOGKIND_ERROR,me->create_sem)
        }
#endif /*OSAPI_ENABLE_LOG*/
#if !defined(RTI_CERT)
        OSAPI_Thread_delete(me);
#endif /*!defined(RTI_CERT)*/
        return NULL;
    }

    me->start_sem = OSAPI_Semaphore_new();
    if (me->start_sem == NULL)
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_SEM(OSAPI_LOGKIND_ERROR,me->start_sem)
        }
#endif /*OSAPI_ENABLE_LOG*/

#ifndef RTI_CERT
        OSAPI_Thread_delete(me);
#endif /*RTI_CERT*/
        return NULL;
    }

    me->thread_info.stop_thread = RTI_FALSE;

    return me;
}
#endif /*!OSAPI_NO_THREADS*/

#if !OSAPI_NO_THREADS


/*e \ingroup OSAPI_ThreadClass
 *
 *  \brief Entry point for OS specific thread function
 *
 *  \details
 *
 *  This function must always be passed as the thread entry point for
 *  OS specific thread creation function. This ensure that all threads
 *  behaves the same at the OSAPI level.
 *  This function executes a thread object up to the point where the user
 *  entry point is called. However, it does not call the user-defined entry
 *  point. Refer to \ref OSAPI_Thread_start for starting a thread.
 *
 *  \param [in] param - Parameter to OSAPI_Thread, called by OS thread
 *
 *  \return Handle to stopped thread on success, NULL on failure.
 *
 *  \sa \ref OSAPI_Thread_start
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Thread_exec(struct OSAPI_ThreadInfo *thread_info)
{
    struct OSAPI_Thread *me = (struct OSAPI_Thread *)thread_info->user_data;
#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
    RTI_BOOL brc;
#endif

    me->tid = OSAPI_Thread_self();

    /* give semaphore to signal that we're spawned */
    if (!OSAPI_Semaphore_give(me->create_sem))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_EXEC_CREATE(OSAPI_LOGKIND_ERROR)
        }
#endif /* OSAPI_ENABLE_LOG */
        return RTI_FALSE;
    }

    if (me->thread_info.thread_state.on_before_start != NULL)
    {
        me->thread_info.thread_state.on_before_start(me->thread_info.thread_state.native_data);
    }

    /* take semaphore before we can begin */
    if (!OSAPI_Semaphore_take(me->start_sem,
                              OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
    {
        OSAPI_LOG_THREAD_EXEC_START(OSAPI_LOGKIND_ERROR,me->start_sem)
        }
#endif
        return RTI_FALSE;
    }

    OSAPI_TRACE_THREAD("started thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    /* here we actually run the user's function */
    me->user_routine(&me->thread_info);

    /* give again as signal that we're really leaving. Note that the
     * OSAPI_Thread_destroy routing is blocked on this sem.
     */
    brc = OSAPI_Semaphore_give(me->create_sem);
#if OSAPI_ENABLE_LOG
    if (!brc)
    {
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_EXEC_CREATE(OSAPI_LOGKIND_ERROR)
        }
    }
#else
    IGNORE_RETVAL(brc);
#endif /* OSAPI_ENABLE_LOG */

    return RTI_TRUE;
}

MUST_CHECK_RETURN OSAPIDllExport struct OSAPI_Thread*
OSAPI_Thread_create(const char *name,
                    const struct OSAPI_ThreadProperty *property,
                    OSAPI_ThreadRoutine thread_entry,
                    void *thread_data,
                    OSAPI_ThreadRoutine thread_wakeup)
{
    struct OSAPI_Thread *me;
    RTI_SIZE_T len;

    OSAPI_PRECONDITION((property == NULL) || (name == NULL) ||
                       (thread_entry == NULL),
                        return NULL,
        OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("thread_entry",
                                    thread_entry == NULL ?
                                        NULL : (void*)1,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    /* create the thread handle */
    me = OSAPI_Thread_new(property);
    if (me == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    /* assign user's values */
    me->user_routine = thread_entry;
    me->wakeup_routine = thread_wakeup;
    me->thread_info.user_data = thread_data;

    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= OSAPI_THREAD_MAX_NAME)
    {
        len = OSAPI_THREAD_MAX_NAME-1;
    }
    OSAPI_Memory_copy(me->name,name,len);
    me->name[len] = 0;

    /* create native thread. Note that even if the native thread starts running
     * OSAPI_Thread_exec is not doing anything until started.
     */
    me->thread_handle = OSAPI_Thread_create_native(name,property,
                                    OSAPI_Thread_exec,me,NULL);

    if (me->thread_handle == NULL)
    {
        OSAPI_LOG_THREAD_CREATE(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

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

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
RTI_BOOL
OSAPI_Thread_wakeup(struct OSAPI_Thread *me)
{
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    OSAPI_TRACE_THREAD("wakeup thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    if (me->wakeup_routine)
    {
        return me->wakeup_routine(&me->thread_info);
    }

    return RTI_TRUE;
}
#endif /*!(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)*/

RTI_BOOL
OSAPI_Thread_start(struct OSAPI_Thread *me)
{
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    OSAPI_TRACE_THREAD("start thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    if (!OSAPI_Semaphore_give(me->start_sem))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_START(OSAPI_LOGKIND_ERROR,me->start_sem)
        }
#endif
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
RTI_BOOL
OSAPI_Thread_destroy(struct OSAPI_Thread *me)
{
    RTI_INT32 reason = 0;
    RTI_INT32 timeout = OSAPI_SEMAPHORE_TIMEOUT_INFINITE;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    OSAPI_TRACE_THREAD("destroy thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    /* set the stop flag to true */
    me->thread_info.stop_thread = RTI_TRUE;

    /* Do not use a timeout for the timer thread since it causes a race
     * condition where a timer may never expire because it is tied to the
     * system clock in some ports. 10ms is sufficient to not enter a very
     * fast loop, and the need to retry is rare.
     */
    if (OSAPI_String_cmp("osapi_timer",me->name))
    {
        timeout = 10;
    }

    /* start the thread if it was never started, otherwise it's a noop */
    if (!OSAPI_Thread_start(me))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_DESTROY_NO_START(OSAPI_LOGKIND_ERROR,me)
        }
#endif
        return RTI_FALSE;
    }

    /* Since there is no way to specify how long to try to destroy a thread,
     * try forever.
     */
    while (1)
    {
        /* Wake up the thread, who should notice the stop flag state and exit */
        if (!OSAPI_Thread_wakeup(me))
        {
            OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(OSAPI_LOGKIND_ERROR,me)
            return RTI_FALSE;
        }

        /* Pend on semaphore to guarantee thread exit.
         */
        if (!OSAPI_Semaphore_take(me->create_sem,timeout,&reason))
        {
            OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(OSAPI_LOGKIND_ERROR,me->create_sem)
            return RTI_FALSE;
        }
        else if (reason == OSAPI_SEMAPHORE_RESULT_TIMEOUT)
        {
            /* No action to take other than try again */
        }
        else if (reason != OSAPI_SEMAPHORE_RESULT_OK)
        {
            OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(OSAPI_LOGKIND_ERROR,me->create_sem)
            return RTI_FALSE;
        }
        else
        {
            /* The semaphore was successfully given, finish loop */
            break;
        }
    }

    if (me->thread_info.thread_state.on_before_delete != NULL)
    {
        me->thread_info.thread_state.on_before_delete(me->thread_info.thread_state.native_data);
    }

#ifndef RTI_CERT
    OSAPI_Thread_delete(me);
#endif /* !RTI_CERT */

    return RTI_TRUE;
}
#endif /*!(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)*/

RTI_BOOL
OSAPI_Thread_is_self(OSAPI_ThreadId *pid)
{
    OSAPI_ThreadId self = OSAPI_Thread_self();

    return self.handle.handle64 == pid->handle.handle64;
}
#endif /*!OSAPI_NO_THREADS*/
