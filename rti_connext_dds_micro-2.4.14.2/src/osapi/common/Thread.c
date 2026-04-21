/*
 * FILE: Thread.c - Platform independent Thread functionality
 *
 * (c) Copyright 2008-2015 Real-Time Innovations
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
 * 15aug2022, ad MICRO-3813/PR.30698
 * - Exclude create_sem for cert deos
 * - Added comments to preprocessor commands for ease of readability
 * 11mar2022, am MICRO-3515
 * - Removed unused functions for DEOS when compiling for RTI_CERT because UDP is excluded.
 *   OSAPI_Thread_wakeup
 *   OSAPI_Thread_destroy 
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include thread semaphore functionality when
 *   when OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 *  - Removed empty blocks in OSAPI_Thread_exec
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_Thread_new
 *   OSAPI_Thread_destroy
 *   OSAPI_Thread_start
 *   OSAPI_Thread_wakeup
 * 13dec2020,tk MICRO-2740/PR.28040
 *              - Renamed is_premptive to is_preemptive
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
#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "System.h"
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

    if (me->create_sem)
    {
        OSAPI_Semaphore_delete(me->create_sem);
    }

    if (me->start_sem)
    {
        OSAPI_Semaphore_delete(me->start_sem);
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    if (me->thread_options & OSAPI_THREAD_SUSPEND_ENABLE)
    {
        OSAPI_System_delete_thread_semaphore();
    }
#endif

    OSAPI_TRACE_THREAD("deleted thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    OSAPI_Heap_free_struct(me);
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
struct OSAPI_Thread*
OSAPI_Thread_new(const struct OSAPI_ThreadProperty *properties)
{
    struct OSAPI_Thread *me;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Thread);

    if (me == NULL)
    {
        return NULL;
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    if ((properties->options & OSAPI_THREAD_SUSPEND_ENABLE) &&
        !OSAPI_System_add_thread_semaphore())
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(me);
#endif
        return NULL;
    }
#endif

    me->start_sem = OSAPI_Semaphore_new();
    me->name[0] = 0;
    me->on_before_delete = NULL;
    me->on_before_start = NULL;
    me->thread_options = properties->options;

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
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
#endif /*defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN*/


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

    me->user_routine = NULL;
    me->wakeup_routine = NULL;

    me->thread_info.stop_thread = RTI_FALSE;
    me->thread_info.user_data = NULL;
    me->thread_info.is_preemptive = RTI_TRUE;

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
void*
OSAPI_Thread_exec(void *param)
{
    struct OSAPI_Thread *me = (struct OSAPI_Thread *)param;
#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
    RTI_BOOL brc;
#endif
    
    me->tid = OSAPI_Thread_self();

    /* When ARINC653 is enabled, there is no way to check if this function is
     * running and give this semaphore back to the create_thread call. The
     * thread_create call for ARINC also never waits on this semaphore. This
     * creates issues when destroying the thread, as we use the same semaphore
     * to guarantee that the thread has finished its operation. By removing the
     * give_semaphore here we keep the semaphore in a "taken" state so that
     * thread cleanup can work correctly.
     */
    
    /*DEOS is only defined if ENABLE_ARINC_653 is true */
#ifndef ENABLE_ARINC_653 
    /* give semaphore to signal that we're spawned */
    if (!OSAPI_Semaphore_give(me->create_sem))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_EXEC_CREATE(OSAPI_LOGKIND_ERROR)
        }
#endif /*OSAPI_ENABLE_LOG*/
        return NULL;
    }
#endif /*ENABLE_ARINC_653*/

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
        return NULL;
    }

    OSAPI_TRACE_THREAD("started thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    if (me->on_before_start)
    {
        me->on_before_start(me);
    }

    /* here we actually run the user's function */
    me->user_routine(&me->thread_info);

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
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
#endif /* !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN) */


#if defined(RTI_FREERTOS)
    /* Delete this task */
    vTaskDelete(NULL);
#endif

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
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    OSAPI_TRACE_THREAD("destroy thread",RTI_FALSE)
    OSAPI_TRACE_STRING("name",me->name,RTI_TRUE)

    /* set the stop flag to true */
    me->thread_info.stop_thread = RTI_TRUE;

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

    /* wake up the thread, who should notice the stop flag state and exit */
    if (!OSAPI_Thread_wakeup(me))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(OSAPI_LOGKIND_ERROR,me)
        }
#endif
        return RTI_FALSE;
    }

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
    /* pend on semaphore to guarantee thread exit */
    if (!OSAPI_Semaphore_take(me->create_sem,
                             OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
    {
#if OSAPI_ENABLE_LOG
        if (OSAPI_Log_is_initialized())
        {
            OSAPI_LOG_THREAD_DESTROY_NO_WAKEUP(OSAPI_LOGKIND_ERROR,me->create_sem)
        }
#endif
        return RTI_FALSE;
    }
#endif

    if (me->on_before_delete)
    {
        me->on_before_delete(me);
    }

#ifndef RTI_CERT
    OSAPI_Thread_delete(me);
#endif

    return RTI_TRUE;
}
#endif /*!(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)*/

#endif /*!OSAPI_NO_THREADS*/
