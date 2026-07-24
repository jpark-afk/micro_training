/*
 * FILE: System.c - Platform independent System functionality
 *
 * Copyright (c) 2012-2026, Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 09jul2015,tk MICRO-1405/PR#15285 Added comments for the constants used for
 *                                  OSAPI_System_fv_ObjectId
 * 20feb2014,eh MICRO-813/PR#9172 Fix Lint warnings
 * 04feb2015,tk MICRO-1034/PR#13591 Corrected copying of hostname in get_hostname
 * 17oct2013,tk Added system listener and interface functionality
 * 20aug2012,tk Written
 */
/*ce
 * \file
 * \brief Implementation of the System API
 *
 * \details
 * This file implements the public system API. Most of the functions are facade
 * meothods to a concrete implementation of an API for a particular platforms,
 * and some are platform independent. This file shall not contain any platform
 * dependent code.
 */
#include "osapi/osapi_system.h"

/*** SOURCE_BEGIN ***/

#if OSAPI_THREAD_SEMAPHORE_ENABLED
void
OSAPI_System_return_thread_semaphore(OSAPI_Semaphore_T *sem)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    RTI_BOOL retval;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    /* The returned semaphore is never accessed and must never
     * be accessed after it has been returned with OSAPI_System_return_thread_semaphore().
     */
    ++sys->thread_sem_last;

    /* coverity[cert_str31_c_violation] */
    sys->thread_semaphore[sys->thread_sem_last] = sem;

    retval = OSAPI_Mutex_give(sys->thread_mutex);

#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        /* The return value is ignored because a semaphore has already been
         * added to the array. If the state of thread_mutex causes a failure
         * on return it is not be safe to remove the semaphore from the
         * array again.
         */
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(retval);
#endif
}

void
OSAPI_System_return_user_thread_semaphore(OSAPI_Semaphore_T *sem)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    RTI_BOOL retval;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    if (sys->thread_sem_last >= -1)
    {
        ++sys->thread_sem_last;
        --sys->user_blocked_threads;
        sys->thread_semaphore[sys->thread_sem_last] = sem;
    }
#if OSAPI_ENABLE_LOG
    else
    {
        OSAPI_LOG_THREAD_SEMPOOL_INVALID_SEM(OSAPI_LOGKIND_ERROR)
    }
#endif /* OSAPI_ENABLE_LOG */

    retval = OSAPI_Mutex_give(sys->thread_mutex);

#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        /* The return value is ignored because a semaphore has already been
         * added to the array. If the state of thread_mutex causes a failure
         * on return it is not be safe to remove the semaphore from the
         * array again.
         */
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(retval);
#endif
}

OSAPI_Semaphore_T*
OSAPI_System_get_thread_semaphore(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    OSAPI_Semaphore_T *rval = NULL;
    RTI_BOOL retval;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if ((sys->thread_sem_last >= 0) &&
        (sys->thread_sem_last < sys->thread_sem_count))
    {
        rval = sys->thread_semaphore[0];
        sys->thread_semaphore[0] = sys->thread_semaphore[sys->thread_sem_last];
        --sys->thread_sem_last;
    }

    retval = OSAPI_Mutex_give(sys->thread_mutex);
#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    /* The return value is ignored because a semaphore has already been
     * allocated and removed from the array. If the state of thread_mutex
     * causes a failure on return, it would not be safe to return the
     * semaphore to the array.
     */
    IGNORE_RETVAL(retval);
#endif

    return rval;
}
OSAPI_Semaphore_T*
OSAPI_System_get_user_thread_semaphore(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    OSAPI_Semaphore_T *rval = NULL;
    RTI_BOOL retval;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if ((sys->thread_sem_last >= 0) &&
        (sys->thread_sem_last < sys->thread_sem_count) &&
        (sys->user_blocked_threads < sys->property.max_user_blocking_threads))
    {
        rval = sys->thread_semaphore[0];
        sys->thread_semaphore[0] = sys->thread_semaphore[sys->thread_sem_last];
        --sys->thread_sem_last;
        ++sys->user_blocked_threads;
    }

    retval = OSAPI_Mutex_give(sys->thread_mutex);
#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    /* The return value is ignored because a semaphore has already been
     * allocated and removed from the array. If the state of thread_mutex
     * causes a failure on return, it would not be safe to return the
     * semaphore to the array.
     */
    IGNORE_RETVAL(retval);
#endif

    return rval;
}

RTI_BOOL
OSAPI_System_add_thread_semaphore(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    OSAPI_Semaphore_T *sem = NULL;
    RTI_BOOL rval = RTI_FALSE;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (sys->thread_sem_count < OSAPI_System_gv_System->property.max_user_blocking_threads)
    {
        sem = OSAPI_Semaphore_new();
        if (sem != NULL)
        {
            /* It is assumed that add_thread_semaphore and
             * return_thread_semaphore are called in the correct order since
             * these are internal APIs and if not called in the correct order
             * then the system is no longer stable.
             */
            ++sys->thread_sem_last;
            ++sys->thread_sem_count;
            /* coverity[cert_str31_c_violation] */
            sys->thread_semaphore[sys->thread_sem_last] = sem;
            rval = RTI_TRUE;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }
#endif
    }
#if OSAPI_ENABLE_LOG
    else
    {
        OSAPI_LOG_THREAD_SEMPOOL_EXCEEDED(OSAPI_LOGKIND_ERROR)
    }
#endif

    {
        RTI_BOOL bval;

        bval = OSAPI_Mutex_give(sys->thread_mutex);
#if OSAPI_ENABLE_LOG
        if (!bval)
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        }
#endif

        /* The return value is ignored because the semaphore has already
         * been added successfully to the array. However, if returning the
         * thread_mutex fails it is not safe to remove the added semaphore
         * from the array and thus the return value is ignored.
         */
        IGNORE_RETVAL(bval);
    }

    return rval;
}

#ifndef RTI_CERT
void
OSAPI_System_delete_thread_semaphore(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    OSAPI_Semaphore_T *rval = NULL;
    RTI_BOOL retval;

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    if ((sys->thread_sem_last >= 0) &&
        (sys->thread_sem_last < sys->thread_sem_count))
    {
        rval = sys->thread_semaphore[sys->thread_sem_last];
        OSAPI_Semaphore_delete(rval);
        --sys->thread_sem_last;
        --sys->thread_sem_count;
    }
#if OSAPI_ENABLE_LOG
    else
    {
        OSAPI_LOG_THREAD_SEMPOOL_INVALID_SEM(OSAPI_LOGKIND_ERROR)
    }
#endif

    retval = OSAPI_Mutex_give(sys->thread_mutex);
#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    /* This function does not return an error, this ignore the
     * return value.
     */
    IGNORE_RETVAL(retval);
#endif
}
#endif

#endif /* OSAPI_THREAD_SEMAPHORE_ENABLED */


#ifndef RTI_CERT
RTI_BOOL
OSAPI_System_set_listener(struct OSAPI_SystemListener *listener)
{
    struct OSAPI_SystemListener null = OSAPI_SystemListener_INITIALIZER;

    if (OSAPI_System_gv_System->is_initialized)
    {
        return RTI_FALSE;
    }

    if (listener == NULL)
    {
        OSAPI_System_gv_System->listener = null;
    }
    else
    {
        OSAPI_System_gv_System->listener = *listener;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_System_get_listener(struct OSAPI_SystemListener *listener)
{
    OSAPI_PRECONDITION_ALWAYS(listener == NULL,return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("listener",listener,RTI_TRUE);)

    *listener = OSAPI_System_gv_System->listener;

    return RTI_TRUE;
}
#endif

RTI_BOOL
OSAPI_System_set_interface(struct OSAPI_SystemI *intf)
{
    struct OSAPI_SystemI null_intf = OSAPI_SystemI_INITIALIZER;

    if (((intf != NULL) &&
         ((intf->get_timer_resolution == NULL) ||
 #if RTI_INCLUDE_SYSTEM_FINALIZER
          (intf->stop_timer == NULL) ||
  #endif
          (intf->start_timer == NULL))) ||
        OSAPI_System_gv_System->is_initialized)
    {
        return RTI_FALSE;
    }

    if (intf == NULL)
    {
        OSAPI_System_gv_System->u_intf = null_intf;
    }
    else
    {
        OSAPI_System_gv_System->u_intf = *intf;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_System_get_interface(struct OSAPI_SystemI *intf)
{
    OSAPI_PRECONDITION_ALWAYS(intf == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    *intf = OSAPI_System_gv_System->u_intf;

    return RTI_TRUE;
}
#endif

RTI_BOOL
OSAPI_System_get_property(struct OSAPI_SystemProperty *property)
{
    OSAPI_PRECONDITION_ALWAYS(property == NULL,return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    *property = OSAPI_System_gv_System->property;

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_System_set_property(struct OSAPI_SystemProperty *property)
{
    OSAPI_PRECONDITION_ALWAYS(property == NULL,return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    if (OSAPI_System_gv_System->is_initialized)
    {
        OSAPI_LOG_SYSTEM_SET_PROPERTY(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (property->max_timers < 1)
    {
        return RTI_FALSE;
    }

    if (property->max_user_blocking_threads < 0)
    {
        return RTI_FALSE;
    }

    OSAPI_System_gv_System->property = *property;

    return RTI_TRUE;
}

#if RTI_PIL || RTI_PSL
/*ci \brief Global variable set by genericSystem to be able to
 *          switch to the generic system.
 */
extern struct OSAPI_System *OSAPI_SystemGeneric_gv_System;
#endif

RTI_BOOL
OSAPI_System_initialize(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    struct OSAPI_SystemI g_intf = OSAPI_SystemI_INITIALIZER;
#if OSAPI_THREAD_SEMAPHORE_ENABLED
    RTI_INT32 count;
    RTI_BOOL result = RTI_FALSE;
#endif

    if (sys->is_initialized)
    {
        return RTI_TRUE;
    }

#if OSAPI_ENABLE_LOG
    OSAPI_Log_gv_LogIntf = &OSAPI_Log_fv_LogIntf;
#endif

    sys->n_intf = g_intf;
    OSAPI_System_get_native_interface(&sys->n_intf);

#if RTI_PIL
    sys->is_timer_owner = RTI_FALSE;

    if (sys->n_intf.get_timer_resolution == NULL)
    {
        return RTI_FALSE;
    }

    if (sys->n_intf.get_time == NULL)
    {
        return RTI_FALSE;
    }

    OSAPI_SystemGeneric_get_native_interface(&g_intf);

    if ((sys->n_intf.initialize == NULL) &&
#if RTI_INCLUDE_SYSTEM_FINALIZER
        (sys->n_intf.finalize == NULL) &&
        (sys->n_intf.stop_timer == NULL) &&
#endif
        (sys->n_intf.start_timer == NULL) &&
        (sys->n_intf.generate_uuid == NULL) &&
        (sys->n_intf.get_ticktime == NULL))
    {
        struct OSAPI_SystemI t;
        struct OSAPI_SystemI u_intf = OSAPI_SystemI_INITIALIZER;

        /* Save the native system interface */
        t = sys->n_intf;

        /* Save the user system interface */
        u_intf = OSAPI_System_gv_System->u_intf;

        /* Switch to the generic system */
        sys = OSAPI_SystemGeneric_gv_System;
        sys->property = OSAPI_System_gv_System->property;
        sys->listener = OSAPI_System_gv_System->listener;

        OSAPI_System_gv_System = OSAPI_SystemGeneric_gv_System;
        OSAPI_System_gv_System->u_intf = u_intf;

        /* Create a the native system using the user supplied
         * get_time and get_timer_resolution functions.
         */
        sys->n_intf = g_intf;
        sys->n_intf.get_timer_resolution = t.get_timer_resolution;
        sys->n_intf.get_time = t.get_time;
        sys->is_timer_owner = RTI_TRUE;
    }
    else if ((sys->n_intf.initialize == NULL) ||
#if RTI_INCLUDE_SYSTEM_FINALIZER
             (sys->n_intf.finalize == NULL) ||
             (sys->n_intf.stop_timer == NULL) ||
#endif
             (sys->n_intf.start_timer == NULL) ||
             (sys->n_intf.generate_uuid == NULL) ||
             (sys->n_intf.get_ticktime == NULL))
    {
        return RTI_FALSE;
    }

#endif /* RTI_PIL */

    sys->r_intf = sys->u_intf;

    if (sys->n_intf.get_hostname == NULL)
    {
        sys->n_intf.get_hostname = g_intf.get_hostname;
    }

    if (sys->r_intf.initialize == NULL)
    {
        sys->r_intf.initialize = sys->n_intf.initialize;
    }

#if RTI_INCLUDE_SYSTEM_FINALIZER
    if (sys->r_intf.finalize == NULL)
    {
        sys->r_intf.finalize = sys->n_intf.finalize;
    }
#endif

    if (sys->r_intf.generate_uuid == NULL)
    {
        sys->r_intf.generate_uuid = sys->n_intf.generate_uuid;
    }

    if (sys->r_intf.get_time == NULL)
    {
        sys->r_intf.get_time = sys->n_intf.get_time;
    }

    if (sys->r_intf.get_timer_resolution == NULL)
    {
        sys->r_intf.get_timer_resolution = sys->n_intf.get_timer_resolution;
    }

    if (sys->r_intf.start_timer == NULL)
    {
        sys->r_intf.start_timer = sys->n_intf.start_timer;
    }

#if RTI_INCLUDE_SYSTEM_FINALIZER
    if (sys->r_intf.stop_timer == NULL)
    {
        sys->r_intf.stop_timer = sys->n_intf.stop_timer;
    }
#endif

    if (sys->r_intf.get_hostname == NULL)
    {
        sys->r_intf.get_hostname = sys->n_intf.get_hostname;
    }

    if (sys->r_intf.get_ticktime == NULL)
    {
        sys->r_intf.get_ticktime = sys->n_intf.get_ticktime;
    }

    if (sys->r_intf.task_scheduler_start == NULL)
    {
        sys->r_intf.task_scheduler_start = sys->n_intf.task_scheduler_start;
    }

    if (sys->r_intf.task_scheduler_stop == NULL)
    {
        sys->r_intf.task_scheduler_stop = sys->n_intf.task_scheduler_stop;
    }

    if ((sys->r_intf.start_timer == sys->n_intf.start_timer) &&
#if RTI_INCLUDE_SYSTEM_FINALIZER
        (sys->r_intf.stop_timer == sys->n_intf.stop_timer) &&
#endif
        (sys->r_intf.get_ticktime == sys->n_intf.get_ticktime))
    {
        sys->is_timer_owner = RTI_TRUE;
    }
    else
    {
        sys->is_timer_owner = RTI_FALSE;
    }

    if (sys->listener.on_system_initialize)
    {
        if (!sys->listener.on_system_initialize(sys->listener.listener_data,sys))
        {
            return RTI_FALSE;
        }
    }

    if (!sys->r_intf.initialize())
    {
        return RTI_FALSE;
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    sys->thread_mutex = OSAPI_Mutex_new();
    if (sys->thread_mutex == NULL)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Mutex_take(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (sys->property.max_user_blocking_threads > 0)
    {
        OSAPI_Heap_allocate_array(
                        &sys->thread_semaphore,
                        (RTI_SIZE_T)sys->property.max_user_blocking_threads,
                        OSAPI_Semaphore_T*);

        if (sys->thread_semaphore == NULL)
        {
            result = OSAPI_Mutex_give(sys->thread_mutex);
            UNUSED_ARG(result);
            return RTI_FALSE;
        }

        sys->thread_sem_count = 0;
        sys->thread_sem_last = -1;

        for (count = 0; count < sys->property.max_user_blocking_threads; ++count)
        {
            if (!OSAPI_System_add_thread_semaphore())
            {
                OSAPI_Heap_free_array(sys->thread_semaphore);
                result = OSAPI_Mutex_give(sys->thread_mutex);
                UNUSED_ARG(result);
                return RTI_FALSE;
            }
        }
    }
    else
    {
        sys->thread_semaphore = NULL;
    }
#endif

    /* coverity[thread1_overwrites_value_in_field : FALSE] */
    sys->is_initialized = RTI_TRUE;

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    if (!OSAPI_Mutex_give(sys->thread_mutex))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }
#endif

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_System_finalize(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    struct OSAPI_SystemProperty def_property = OSAPI_SystemProperty_INITIALIZER;
#if OSAPI_THREAD_SEMAPHORE_ENABLED
    RTI_INT32 count;
#endif

    if (!sys->is_initialized)
    {
        return RTI_FALSE;
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    if (sys->thread_semaphore != NULL)
    {
        for (count = 0; count < sys->property.max_user_blocking_threads; ++count)
        {
            OSAPI_System_delete_thread_semaphore();
        }
        OSAPI_Heap_free_array(sys->thread_semaphore);
        sys->thread_semaphore = NULL;
    }
#endif

    if (sys->listener.on_system_finalize)
    {
        sys->listener.on_system_finalize(sys->listener.listener_data,sys);
    }

    if (!OSAPI_System_gv_System->r_intf.finalize())
    {
        return RTI_FALSE;
    }

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    if (sys->thread_mutex != NULL)
    {
        if (!OSAPI_Mutex_delete(sys->thread_mutex))
        {
            /* Nothing to do at this point */
        }
    }
#endif

    OSAPI_System_gv_System->property = def_property;

    OSAPI_System_gv_System->is_initialized = RTI_FALSE;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
OSAPI_System_get_time(OSAPI_SystemTime *now)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;

    return sys->r_intf.get_time(now);
}

RTI_BOOL
OSAPI_System_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    return OSAPI_System_gv_System->r_intf.generate_uuid(uuid_out);
}

RTI_INT32
OSAPI_System_get_timer_resolution(void)
{
    return OSAPI_System_gv_System->r_intf.get_timer_resolution();
}

RTI_BOOL
OSAPI_System_start_timer(OSAPI_Timer_T self,
                         OSAPI_TimerTickHandlerFunction tick_handler)
{
    return OSAPI_System_gv_System->r_intf.start_timer(self,tick_handler);
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_System_stop_timer(OSAPI_Timer_T self)
{
    return OSAPI_System_gv_System->r_intf.stop_timer(self);
}
#endif

RTI_BOOL
OSAPI_System_get_hostname(char *const hostname)
{
    RTI_SIZE_T len;

    /* If the hostname is not specified in the property, e.g. in the case of
     * manual configuration, call the native method.
     */
    if (OSAPI_System_gv_System->property.hostname[0] == 0)
    {
        return OSAPI_System_gv_System->r_intf.get_hostname(hostname);
    }

    /* If the hostname is not retrieved by the native interface, copy out the
     * hostname from the property. Note that no check is made on the length
     * because property.hostname holds a maximum of OSAPI_SYSTEM_MAX_HOSTNAME
     * bytes and this function requires hostname to be able to hold at least
     * OSAPI_SYSTEM_MAX_HOSTNAME of bytes. Thus, if both these requirements are
     * met then this function cannot fail.
     */
    len = OSAPI_String_length(OSAPI_System_gv_System->property.hostname);
    OSAPI_Memory_copy(hostname,OSAPI_System_gv_System->property.hostname,len);
    hostname[len] = 0;

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_System_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{

    return OSAPI_System_gv_System->r_intf.get_ticktime(sec,nanosec);
}

RTI_BOOL
OSAPI_System_is_timer_owner(void)
{
    return OSAPI_System_gv_System->is_timer_owner;
}

