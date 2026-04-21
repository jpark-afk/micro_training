/*
 * FILE: System.c - Platform independent System functionality
 *
 * (c) Copyright 2012-2018, Real-Time Innovations
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
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include thread semaphore functionality when
 *   when OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 * - Added cert_str31_c_violation suppression in OSAPI_System_return_thread_semaphore
 * - Added cert_str31_c_violation suppression in OSAPI_System_add_thread_semaphore
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_System_set_listener
 *   OSAPI_System_get_listener
 *   OSAPI_System_get_interface
 * 13jan2021,tk MICRO-2807/PR27549 Removed unused functions for CERT.
 * 10jan2021,tk MICRO-2807/PR.27549  Removed unused functions for CERT
 * 05jan2021,tk
 *   MICRO-2781/PR#28519
 *     - Removed code only used when logging is enabled in
 *       OSAPI_System_return_thread_semaphore(),
 *       OSAPI_System_get_thread_semaphore(),
 *       OSAPI_System_add_thread_semaphore(),
 *       OSAPI_System_delete_thread_semaphore()
 *   MICRO-2780/PR#28511
 *     - Removed redundant assignment of NULL to sys->thread_mutex in
 *       OSAPI_System_initialize().
 *     - Check return value from OSAPI_Mutex_new() in OSAPI_System_initialize().
 * 19oct2020,tk MICRO-2582/PR#28143
 *     - Use OSAPI_PRECONDITION_ALWAYS for:
 *       - OSAPI_System_get_property
 *       - OSAPI_System_set_property
 *       - OSAPI_System_get_listener
 *       - OSAPI_System_get_interface
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
#include "System.h"

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

    if (sys->thread_sem_count < OSAPI_SYSTEM_MAX_THREAD_SEMAPHORE)
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
          (intf->start_timer == NULL) ||
          (intf->stop_timer == NULL))) ||
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

    OSAPI_System_gv_System->property = *property;

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_System_initialize(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;

    if (sys->is_initialized)
    {
        return RTI_TRUE;
    }

    OSAPI_System_get_native_interface(&sys->n_intf);

    sys->r_intf = sys->u_intf;

    if (sys->r_intf.initialize == NULL)
    {
        sys->r_intf.initialize = sys->n_intf.initialize;
    }

    if (sys->r_intf.finalize == NULL)
    {
        sys->r_intf.finalize = sys->n_intf.finalize;
    }

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

    if (sys->r_intf.stop_timer == NULL)
    {
        sys->r_intf.stop_timer = sys->n_intf.stop_timer;
    }

    if (sys->r_intf.get_hostname == NULL)
    {
        sys->r_intf.get_hostname = sys->n_intf.get_hostname;
    }

    if (sys->r_intf.get_ticktime == NULL)
    {
        sys->r_intf.get_ticktime = sys->n_intf.get_ticktime;
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
    {
        RTI_INT32 count;

        sys->thread_mutex = OSAPI_Mutex_new();
        if (sys->thread_mutex == NULL)
        {
            return RTI_FALSE;
        }

        for (count = 0; count < OSAPI_SYSTEM_MAX_THREAD_SEMAPHORE; ++count)
        {
            sys->thread_semaphore[count] = NULL;
        }

        sys->thread_sem_last = -1;
        sys->thread_sem_count = 0;
    }
#endif

    sys->is_initialized = RTI_TRUE;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_System_finalize(void)
{
    struct OSAPI_System *sys = OSAPI_System_gv_System;
    struct OSAPI_SystemProperty def_property = OSAPI_SystemProperty_INITIALIZER;

    if (!sys->is_initialized)
    {
        return RTI_FALSE;
    }

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
OSAPI_System_get_time(OSAPI_NtpTime *now)
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
