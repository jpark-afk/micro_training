/*
 * FILE: genericSystem.c - Generic OS independent  system functionality
 *
 * Copyright 2022-2025 Real-Time Innovations, Inc.
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
 * 28jul2023,ad MICRO-5431/PR.32080
 * - Removed use of a union in OSAPI_SystemGeneric_generate_uuid. Different
 *   type sizes, depending on system architecture, were used for the PID which
 *   could have resulted in a uuid with a PID of 0x0.
 * - Changed the algorithm to use the entire 64 bytes of the system PID on 8
 *   byte systems and to only use the lower 4 bytes on system 4 byte system.
 * 25may2023,ad MICRO-5018
 * - Changed the implementation name to the correct name in the
 *   comments for OSAPI_SystemGeneric_get_hostname
 * 24may2023,ad MICRO-5086/PR.31846
 * - Changed stop_timer and start_timer to check
 *   is_initialized using OSAPI_SystemGeneric_fv_System._parent.is_initialized
 *   rather than the global OSAPI_SystemGeneric_gv_System in order to handle
 *   the scenario where they are not pointing to the same memory.
 * 20may2023,tk MICRO-5014 / PR.31823
 * - Added RTI_PRIVATE to OSAPI_SystemGeneric_fv_System (local variable)
 * 25apr2023,ad MICRO-4814/PR.31479
 * - The clock_tick operation was changed to check ownership first so that the
 *   operation would match how clock_tick_from_time performed its checks.
 * 02nov2022,tk Written
 */
/*ce
 * \file
 * \brief Implementation of a generic OS System API
 */
#include "osapi/osapi_config.h"

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif

/*ci \brief Maximum number of timer that can be created. Documented as a
 *   hard resource-limit.
 */
#define OSAPISYSTEM_MAX_TIMERS   OSAPI_SystemGeneric_fv_System->_parent.property.max_timers

/*ci \brief Timer callback information
 */
struct OSAPI_SystemTimerHandler
{
    /*ci \brief The callback function to call on each tick
     */
    OSAPI_TimerTickHandlerFunction handler;

    /*ci \brief User defined parameter passed to the callback
     */
    void *param;

    /*ci \brief Flag indicating this timer is enabled or not
     */
    volatile RTI_BOOL enabled;

#ifndef RTI_CERT
    /*ci \brief Flag indicating if this timer is being deleted
     */
    volatile RTI_BOOL deleted;
#endif
};

/*ci \brief Generic System variables
 */
struct OSAPI_SystemGeneric
{
    /*ci \brief base-class
     */
    struct OSAPI_System _parent;

    /*ci \brief Array of timer objects
     */
    struct OSAPI_SystemTimerHandler *timer_handler;

#ifndef RTI_CERT
    /*ci \brief Flag to indicated a timer has been deleted
     */
    RTI_BOOL timer_deleted;

    /*ci \brief Flag to indicate a timer is busy, used for busy waiting
     */
    RTI_BOOL timer_is_busy;
#endif

    /*ci
     * \brief Current tick in seconds
     */
    RTI_INT32 tick_sec;

    /*ci
     * \brief Current tick in nanosecs
     */
    RTI_INT32 tick_nanosec;

    /*ci
     * \brief Tick resolution in nanoseconds
     */
    RTI_INT32 tick_resolution_ns;

    /*ci
     * \brief Tick resolution as system time
     */
    struct OSAPI_SystemTime tick_resolution;

    /*ci \brief Lock to ensure atomic updates of ticks
     */
    OSAPI_Mutex_T *tick_mutex;

    /*ci \brief TRUE if the Generic system owns ,start,stop,get_ticktime
     *   functions.
     */
    RTI_BOOL is_timer_owner;

    /*ci \brief
     */
    struct OSAPI_SystemTime last_timestamp;

    /*ci \brief Remaining time from last tick
     */
    struct OSAPI_SystemTime timestamp_rem;

    /*ci \brief True is time has started
     */
     RTI_BOOL is_time_started;

    /*ci TRUE is the function OSAPI_System_clock_tick
     *   has been called.
     */
     RTI_BOOL is_clock_tick_called;

     /*ci TRUE is the function OSAPI_System_clock_tick_from_time
      *   has been called.
      */
     RTI_BOOL is_clock_tick_from_time_called;
};

/*ci \brief Private variable with for the Generic system implementation
 */
RTI_PRIVATE struct OSAPI_SystemGeneric OSAPI_System_g =
{
 ._parent = OSAPI_System_INITIALIZER
};

/*ci \brief Private pointer to the Generic system implementation
 */
RTI_PRIVATE struct OSAPI_SystemGeneric *OSAPI_SystemGeneric_fv_System = &OSAPI_System_g;

/*ci \brief Variable used by System.c to switch to the generic system
 */
struct OSAPI_System *OSAPI_SystemGeneric_gv_System = &OSAPI_System_g._parent;

/*ci \brief Global counter to create UUID, incremented in each call to generate
 *   a UUID.
 */
RTI_PRIVATE RTI_UINT32 OSAPI_System_fv_UuidCounter = 0xdeadc0de;

/*** SOURCE_BEGIN ***/

RTI_PRIVATE MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemGeneric_initialize_impl(struct OSAPI_SystemGeneric *self);

/******************************************************************************
 *  Internal API
 ******************************************************************************/
/*ci
 * \brief Take the system lock
 *
 * \details
 *
 * Function to protect system resources
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_SystemGeneric_fv_System->tick_mutex);
}

/*ci
 * \brief Release the system lock
 *
 * \details
 *
 * Release lock taken with \ref OSAPI_System_lock
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_SystemGeneric_fv_System->tick_mutex);
}

/*ci
 * \brief Start a timer
 *
 * \details
 *
 * Implementation of OSAPI_System_start_timer.
 *
 * @param [in]  self         Timer object.
 * @param [in]  tick_handler Timer handler to call on each clock tick.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_start_timer(OSAPI_Timer_T self,
                                OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;
    RTI_BOOL brc;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("tick_handler",
                                    tick_handler == NULL ? NULL : (void*)0x1,
                                    RTI_TRUE);)

    if (!OSAPI_SystemGeneric_fv_System->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_SystemGeneric_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (!OSAPI_SystemGeneric_fv_System->timer_handler[i].enabled)
        {
            break;
        }
    }

    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        brc = OSAPI_SystemGeneric_unlock();
        /* ignore, already returning error */
        IGNORE_RETVAL(brc);
        return RTI_FALSE;
    }

    OSAPI_SystemGeneric_fv_System->timer_handler[i].handler = tick_handler;
    OSAPI_SystemGeneric_fv_System->timer_handler[i].param = self;

    /* At this point the timer is enabled and may fire before this function
     * returns.
     */
    OSAPI_SystemGeneric_fv_System->timer_handler[i].enabled = RTI_TRUE;

    return OSAPI_SystemGeneric_unlock();
}

#if RTI_INCLUDE_SYSTEM_FINALIZER

/*ci \brief Flag to indicate whether or not to wait for a timer acknowledging
            being deleted or not.
 */
RTI_PRIVATE RTI_BOOL OSAPI_SystemGeneric_gv_SetWaitTimerDelete = RTI_FALSE;

void
OSAPI_SystemGeneric_set_wait_timer_delete(RTI_BOOL value)
{
    OSAPI_SystemGeneric_gv_SetWaitTimerDelete = value;
}

/*ci
 * \brief Stop the timer.
 *
 * \details
 * Implementation of OSAPI_System_stop_timer to stop a timer started
 * with start timer.
 *
 * @param [in] self Timer
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL),
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_SystemGeneric_fv_System->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_SystemGeneric_fv_System->timer_handler[i].param == self)
        {
            OSAPI_SystemGeneric_fv_System->timer_handler[i].deleted = RTI_TRUE;

            /* Wait for the tick routine to acknowledge the deletion */
            if (OSAPI_SystemGeneric_gv_SetWaitTimerDelete)
            {
                while (OSAPI_SystemGeneric_fv_System->timer_handler[i].deleted);
            }

            /* The timer routine has acknowledged the deletion, caller can
             * safely free resources on return. A context switch is safe.
             */
        }
    }

    return RTI_TRUE;
}
#endif

/*ci
 * \brief Run all the system timers
 *
 * \details
 *
 * Update periodic system timers created with \ref OSAPI_System_start_timer.
 *
 * \param[in] self Pointer to Generic system instance
 */
RTI_PRIVATE void
OSAPI_SystemGeneric_run_timers(struct OSAPI_SystemGeneric *self)
{
    RTI_INT32 i;
    RTI_INT32 last_tick_ns;
    RTI_BOOL bretval;

    for (i = 0; (i < OSAPISYSTEM_MAX_TIMERS); ++i)
    {
        /* Any timer with enabled set to true must be valid, it takes
         * at most one tick to delete a timer.
         */
        if (self->timer_handler[i].enabled)
        {
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
#ifndef RTI_CERT
        if (self->timer_handler[i].deleted)
        {
            self->timer_handler[i].deleted = RTI_FALSE;

            self->timer_handler[i].enabled = RTI_FALSE;

            /* At this point the timer may get re-allocated and set the
             * enabled flag to true.
             */
        }
#endif
    }


    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        return;
    }

    last_tick_ns = self->tick_nanosec;

    /* Add seconds in the resolution to make sure the remainder is less
     * than 1 sec
     */
    self->tick_sec = self->tick_sec +
           (self->tick_resolution_ns / OSAPI_TIME_NSEC_PER_SEC);

    /* Add the remaining nanoseconds in the resolution to the
     * current nanosec counter, but do not go past 1 sec.
     */
    self->tick_nanosec = (self->tick_nanosec +
         (self->tick_resolution_ns % OSAPI_TIME_NSEC_PER_SEC)) %
                     OSAPI_TIME_NSEC_PER_SEC;


    /* If self->tick_nanosec wrapped around, add 1 more second */
    if (self->tick_nanosec < last_tick_ns)
    {
         self->tick_sec++;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);
}

/*ci
 *
 * \brief Internal Generic system initialization routine
 *
 * \details
 *
 * This function initializes the Generic OSAPI System abstraction and
 * sets up the timer handling.
 *
 * \param[in]    self - Generic system structure

 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemGeneric_initialize_impl(struct OSAPI_SystemGeneric *self)
{
    RTI_INT32 i;

    if (OSAPISYSTEM_MAX_TIMERS > 1)
    {
        OSAPI_Heap_allocate_array(&self->timer_handler,
                                  (RTI_SIZE_T) OSAPISYSTEM_MAX_TIMERS,
                                  struct OSAPI_SystemTimerHandler);

        if (self->timer_handler == NULL)
        {
            return RTI_FALSE;
        }
    }
    else
    {
        return RTI_FALSE;
    }

    /* Let the user do their own initialization first */
    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
        self->timer_handler[i].enabled = RTI_FALSE;
#ifndef RTI_CERT
        self->timer_handler[i].deleted = RTI_FALSE;
#endif
    }

    /* OSAPI_System_initialize() is by design not thread-safe */
    /* coverity[missing_lock] */
    self->tick_sec = 0;
    self->tick_nanosec = 0;
    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_array(self->timer_handler);
#endif
        return RTI_FALSE;
    }

    self->tick_resolution_ns = OSAPI_System_get_timer_resolution();

    self->tick_resolution.sec = self->tick_resolution_ns /
           OSAPI_TIME_NSEC_PER_SEC;
    self->tick_resolution.nanosec = (RTI_UINT32)self->tick_resolution_ns %
           OSAPI_TIME_NSEC_PER_SEC;

    self->is_timer_owner = OSAPI_System_is_timer_owner();
    self->is_time_started = RTI_FALSE;
    self->is_clock_tick_called = RTI_FALSE;
    self->is_clock_tick_from_time_called = RTI_FALSE;

    self->last_timestamp.sec = 0;
    self->last_timestamp.nanosec = 0;
    self->timestamp_rem.sec = 0;
    self->timestamp_rem.nanosec = 0;

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Internal Generic system finalization routine
 *
 * \details
 *
 * This function finalizes the OSAPI Generic system abstraction and
 * deletes the timer handling.
 *
 * \param[in]    self - Generic System structure
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_finalize_impl(struct OSAPI_SystemGeneric *self)
{
    RTI_BOOL retval = RTI_FALSE;

    if (self->timer_handler != NULL)
    {
        OSAPI_Heap_free_array(self->timer_handler);
        self->timer_handler = NULL;
    }

    if (self->tick_mutex != NULL)
    {
        if (!OSAPI_Mutex_delete(self->tick_mutex))
        {
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    return retval;
}
#endif

/*ci
 * \brief Initialize the system.
 *
 * \details
 * Implementation of OSAPI_System_initialize
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_initialize(void)
{
    return OSAPI_SystemGeneric_initialize_impl(OSAPI_SystemGeneric_fv_System);
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the system.
 *
 * \details
 * Implementation of OSAPI_System_finalize
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_finalize(void)
{
    return OSAPI_SystemGeneric_finalize_impl(OSAPI_SystemGeneric_fv_System);
}
#endif

/*ci
 * \brief Generate a unique universal identifier (UUID)
 *
 * \details
 * Implementation of OSAPI_System_generate_uuid
 *
 * \param[inout] uuid_out The generated UUID
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    RTI_UINT64 pid = 0;

    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

    if (!OSAPI_System_get_time(&now))
    {
        return RTI_FALSE;
    }

#if RTI_ENDIAN_LITTLE
    uuid_out->value[0] = ((OSAPI_System_fv_UuidCounter & 0xff000000U)>>24) |
                         ((OSAPI_System_fv_UuidCounter & 0x00ff0000U)>>8)  |
                         ((OSAPI_System_fv_UuidCounter & 0x000000ffU)<<24) |
                         ((OSAPI_System_fv_UuidCounter & 0x0000ff00U)<<8);
    OSAPI_System_fv_UuidCounter++;
#else
    uuid_out->value[0] = OSAPI_System_fv_UuidCounter++;
#endif

    pid = OSAPI_Process_getpid();
    uuid_out->value[1] = (RTI_UINT32)(pid >> 32) ^ (RTI_UINT32)pid;

    uuid_out->value[2] = (RTI_UINT32)(now.nanosec);
    uuid_out->value[3] = (RTI_UINT32)(now.sec);

    return RTI_TRUE;
}

/*ci
 * \brief Get the hostname
 *
 * \details
 * Implementation of OSAPI_SystemI.get_hostname
 *
 * \param[in] hostname Hostname to fill in
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_get_hostname(char *const hostname)
{
#ifdef RTI_CERT
    const char *host_name = "rti_me_cert";
#else
    const char *host_name = "rti_me";
#endif
    RTI_SIZE_T len;

   /*
    * Although there is no way to change the cert_host_name used in
    * OSAPI_SystemGeneric_get_hostname() for RCC, truncate the name if
    * the length exceeds OSAPI_SYSTEM_MAX_HOSTNAME in case it is changed
    * to something too long.
    */
    len = OSAPI_String_length(host_name);
    if (len >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        len = OSAPI_SYSTEM_MAX_HOSTNAME-1;
    }
    OSAPI_Memory_copy(hostname,host_name,len);
    hostname[len] = 0;

    return RTI_TRUE;
}

/*ci
 * \brief Get the get_ticktime
 *
 * \details
 * Implementation of OSAPI_System_get_ticktime interface
 *
 * \param[inout] sec     The current tick-time seconds part
 * \param[inout] nanosec The current tick-time nano-seconds part
 *
 * @return This function returns RTI_TRUE on success, RTI_FALSE
 *         on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemGeneric_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{

    if (!OSAPI_Mutex_take(OSAPI_SystemGeneric_fv_System->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = OSAPI_SystemGeneric_fv_System->tick_sec;
    *nanosec = (RTI_UINT32)OSAPI_SystemGeneric_fv_System->tick_nanosec;

    if (!OSAPI_Mutex_give(OSAPI_SystemGeneric_fv_System->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/
void
OSAPI_SystemGeneric_get_native_interface(struct OSAPI_SystemI *intf)
{
    OSAPI_PRECONDITION_ALWAYS(intf == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    intf->initialize = OSAPI_SystemGeneric_initialize;

#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    intf->finalize = OSAPI_SystemGeneric_finalize;
#else
    intf->finalize = NULL;
#endif
#endif

    intf->start_timer = OSAPI_SystemGeneric_start_timer;

#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    intf->stop_timer = OSAPI_SystemGeneric_stop_timer;
#else
    intf->stop_timer = NULL;
#endif
#endif

    intf->get_timer_resolution = NULL;
    intf->get_time = NULL;
    intf->generate_uuid = OSAPI_SystemGeneric_generate_uuid;
    intf->get_hostname = OSAPI_SystemGeneric_get_hostname;
    intf->get_ticktime = OSAPI_SystemGeneric_get_ticktime;
}

RTI_BOOL
OSAPI_System_clock_tick(RTI_UINT32 ticks)
{
    RTI_UINT32 i = 0;

    /* Do not run timers if we are not the owner of the timer functions
     * since we do not know how time is handled.
     */
    if (!OSAPI_SystemGeneric_fv_System->is_timer_owner)
    {
        return RTI_FALSE;
    }

    if (OSAPI_SystemGeneric_fv_System->is_clock_tick_from_time_called)
    {
        return RTI_FALSE;
    }

    OSAPI_SystemGeneric_fv_System->is_clock_tick_called = RTI_TRUE;

    for (i = 0; i < ticks; i++)
    {
        OSAPI_SystemGeneric_run_timers(OSAPI_SystemGeneric_fv_System);
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_System_clock_tick_from_time(void)
{
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    struct OSAPI_SystemTime diff = OSAPI_TIME_ZERO;
    struct OSAPI_SystemTime rem = OSAPI_TIME_ZERO;
    struct OSAPI_SystemGeneric *self = OSAPI_SystemGeneric_fv_System;

    /* Do not run timers if we are not the owner of the timer functions
     * since we do not know how time is handled.
     */
    if (!OSAPI_SystemGeneric_fv_System->is_timer_owner)
    {
        return RTI_FALSE;
    }

    if (self->is_clock_tick_called)
    {
        return RTI_FALSE;
    }
    self->is_clock_tick_from_time_called = RTI_TRUE;

    /* Time starts from the first call */
    if (!self->is_time_started)
    {
        if (!OSAPI_System_get_time(&self->last_timestamp))
        {
            return RTI_FALSE;
        }

        self->is_time_started = RTI_TRUE;
        return RTI_TRUE;
    }

    if (!OSAPI_System_get_time(&now))
    {
        return RTI_FALSE;
    }

    /* Do not accept time moving backwards */
    if (OSAPI_SystemTime_compare(&now,&self->last_timestamp) < 0)
    {
        return RTI_FALSE;
    }

    /* Start further back for any leftover from last time */
    OSAPI_SystemTime_subtract(&diff,&self->last_timestamp,&self->timestamp_rem);
    self->last_timestamp = diff;
    OSAPI_SystemTime_subtract(&diff,&now,&self->last_timestamp);

    /* Avoid function call */
    if (diff.sec < 0)
    {
        return RTI_FALSE;
    }

    while (OSAPI_SystemTime_compare(&diff,&self->tick_resolution) >= 0)
    {
        OSAPI_SystemGeneric_run_timers(OSAPI_SystemGeneric_fv_System);

        /* Do not call OSAPI_SystemTime_subtract to avoid unnecessary error
         * checking.
         */
        rem.sec = diff.sec - self->tick_resolution.sec;

        if (diff.nanosec < self->tick_resolution.nanosec)
        {
            rem.sec--;
            rem.nanosec = diff.nanosec - self->tick_resolution.nanosec + OSAPI_TIME_NSEC_PER_SEC;
        }
        else
        {
            rem.nanosec = diff.nanosec - self->tick_resolution.nanosec;
        }
        diff = rem;
    }

    self->last_timestamp = now;
    self->timestamp_rem = diff;

    return RTI_TRUE;
}
