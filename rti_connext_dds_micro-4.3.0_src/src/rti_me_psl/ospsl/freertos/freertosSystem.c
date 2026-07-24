/*
 * FILE: freertosSystem.c - System functionality
 *
 * (c) Copyright, Real-Time Innovations 2024-2025
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI system routines
 */

#include "rti_me_psl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define OSAPISYSTEM_MAX_TIMERS   OSAPI_SystemFreeRTOS_fv_System->_parent.property.max_timers
#define OSAPISYSTEM_TIMER_RESOLUTION    20

struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction handler;
    void *param;
};

struct OSAPI_SystemFreeRTOS
{
    struct OSAPI_System _parent;
    RTI_BOOL is_initialized;
    struct OSAPI_SystemTimerHandler *timer_handler;
    RTI_INT32 timer_count;
    portTickType timer_res;
    xTimerHandle system_timer;
    RTI_INT32 tick_sec;
    RTI_UINT32 tick_nanosec;
    struct OSAPI_Mutex *tick_mutex;
    struct OSAPI_Mutex *mutex;
};


RTI_PRIVATE struct OSAPI_SystemFreeRTOS OSAPI_System_g =
{
    ._parent = OSAPI_System_INITIALIZER
};

RTI_PRIVATE struct OSAPI_SystemFreeRTOS *OSAPI_SystemFreeRTOS_fv_System = &OSAPI_System_g;

RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemFreeRTOS);

struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;

struct OSAPI_LogEntryI *OSAPI_Log_gv_LogIntf = NULL;

/* lock for timer initialization */
RTI_PRIVATE int OSAPI_System_fv_initLock = 0;

/*ci \brief Global counter to create UUID, incremented in each call to generate
 *   a UUID.
 */
RTI_PRIVATE RTI_UINT32 OSAPI_System_fv_UuidCounter = 0xdeadc0de;

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Take the system lock
 *
 * \details
 * Function to protect system resources
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_System_g.mutex);
}

/*ci
 * \brief Release the system lock
 *
 * \details
 * Release lock taken with \ref OSAPI_System_lock
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_System_g.mutex);
}

/*ci
 * \brief Get the current system time
 *
 * \details
 * Implementation of OSAPI_System_get_time
 *
 * @param[out] now Time
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * @pre Initialized system
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_get_time(OSAPI_SystemTime * now)
{
    /* Determine time from number of ticks since system started */
    portTickType ticks_since_start, ms_since_start;

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("now", now, RTI_TRUE);)

    ticks_since_start = xTaskGetTickCountFromISR();
    ms_since_start = ticks_since_start * portTICK_RATE_MS;

    now->sec = ms_since_start / OSAPI_TIME_MSEC_PER_SEC;
    now->nanosec = (ms_since_start % 1000);
    return RTI_TRUE;
}

/*ci
 * \brief Timer callback function
 *
 * \details
 * This function is called when the system timer expires
 *
 * @param[in] timerHandle Timer handle
 */
RTI_PRIVATE void
OSAPI_SystemFreeRTOS_on_timer_callback(xTimerHandle timerHandle)
{
    RTI_UINT32 last_tick_ns;
    RTI_BOOL bretval;
    RTI_INT32 i, j;
    struct OSAPI_SystemFreeRTOS *self = OSAPI_SystemFreeRTOS_fv_System;

    UNUSED_ARG(timerHandle);

    if (!self->_parent.is_initialized)
    {
        return;
    }

    if (!OSAPI_System_lock())
    {
        return;
    }

    for (i = 0, j = self->timer_count; (i < OSAPISYSTEM_MAX_TIMERS) && j; ++i)
    {
        if (self->timer_handler[i].handler)
        {
            j--;
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
    }

    if (!OSAPI_Mutex_take(self->tick_mutex)) {
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return;
    }

    last_tick_ns = self->tick_nanosec;
    self->tick_nanosec += (self->timer_res * 1000000);
    self->tick_nanosec %= 1000000000;

    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
}

/*ci
 * \brief Internal FreeRTOS system initialization routine
 *
 * \details
 * This function initializes the OSAPI FreeRTOS system abstraction and
 * sets up the timer handling
 *
 * @param[in] self FreeRTOS system structure
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_initializeI(struct OSAPI_SystemFreeRTOS *self)
{
    RTI_INT32 i;

    while (__sync_lock_test_and_set(&OSAPI_System_fv_initLock, 1));

    if (self->_parent.is_initialized)
    {
        OSAPI_System_fv_initLock = 0;
        return RTI_TRUE;
    }

    self->timer_handler = NULL;

    if (OSAPISYSTEM_MAX_TIMERS > 1)
    {
        OSAPI_Heap_allocate_buffer((char**)&self->timer_handler,
                  (RTI_SIZE_T)OSAPISYSTEM_MAX_TIMERS *
                  (RTI_SIZE_T)sizeof(struct OSAPI_SystemTimerHandler),
                  OSAPI_ALIGNMENT_DEFAULT);

        if (self->timer_handler == NULL)
        {
            return RTI_FALSE;
        }
    }
    else
    {
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->timer_count = 0;
    self->timer_res = OSAPISYSTEM_TIMER_RESOLUTION / portTICK_RATE_MS;
    if (self->timer_res < 1)
    {
        self->timer_res = 1;
    }
    self->tick_sec = 0;
    self->tick_nanosec = 0;

    self->mutex = OSAPI_Mutex_new();
    if (self->mutex == NULL)
    {
        goto fail;
    }

    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
        goto fail;
    }

    self->system_timer =
        xTimerCreate("systimer", self->timer_res, pdTRUE,
                     (void *)OSAPISYSTEM_MAX_TIMERS,
                     OSAPI_SystemFreeRTOS_on_timer_callback);
    if (self->system_timer == NULL)
    {
        goto fail;
    }

    if (pdTRUE != xTimerStart(self->system_timer, portMAX_DELAY))
    {
        goto fail;
    }

    self->is_initialized = RTI_TRUE;
    OSAPI_System_fv_initLock = 0;

    return RTI_TRUE;

fail:
#ifndef RTI_CERT
    OSAPI_Heap_free_buffer(self->timer_handler);
#endif
    OSAPI_System_fv_initLock = 0;
    return RTI_FALSE;
}

/*ci
 * \brief Initialize the system
 *
 * \details
 * Implementation of OSAPI_System_initialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_initialize(void)
{
    return OSAPI_SystemFreeRTOS_initializeI(&OSAPI_System_g);
}

#ifndef RTI_CERT
/*ci
 * \brief Internal FreeRTOS system finalization routine
 *
 * \details
 * This function finalizes the OSAPI FreeRTOS system abstraction and
 * cleans up the timer handling
 *
 * @param[in] self FreeRTOS system structure
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_finalizeI(struct OSAPI_SystemFreeRTOS *self)
{
    RTI_BOOL bretval;

    if (!OSAPI_System_lock())
    {
        return RTI_FALSE;
    }

    if (self->timer_handler != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(self->timer_handler);
#endif
        self->timer_handler = NULL;
    }

    if (self->system_timer != NULL)
    {
        if (pdTRUE != xTimerStop(self->system_timer, portMAX_DELAY))
        {
            goto fail;
        }
        if (pdTRUE != xTimerDelete(self->system_timer, portMAX_DELAY))
        {
            goto fail;
        }
        self->system_timer = NULL;
    }

    if ((self->tick_mutex != NULL) && !OSAPI_Mutex_delete(self->tick_mutex))
    {
        goto fail;
    }

    if (!OSAPI_System_unlock())
    {
        return RTI_FALSE;
    }

    if ((self->mutex != NULL) && !OSAPI_Mutex_delete(self->mutex))
    {
        return RTI_FALSE;
    }

    self->is_initialized = RTI_FALSE;

    return RTI_TRUE;

fail:
    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
    return RTI_FALSE;
}

/*ci
 * \brief Finalize the system
 *
 * \details
 * Implementation of OSAPI_System_finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_finalize(void)
{
    return OSAPI_SystemFreeRTOS_finalizeI(&OSAPI_System_g);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Start a timer
 *
 * \details
 * Implementation of OSAPI_System_start_timer
 *
 * @param[in] self Timer object
 * @param[in] tick_handler Timer handle
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_start_timer(OSAPI_Timer_T self,
                                 OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("tick_handler", tick_handler, RTI_TRUE);)

    if (!OSAPI_SystemFreeRTOS_initialize())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_g.timer_handler[i].handler == NULL)
        {
            break;
        }
    }
    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    OSAPI_System_g.timer_handler[i].handler = tick_handler;
    OSAPI_System_g.timer_handler[i].param = self;
    ++OSAPI_System_g.timer_count;

    ok = RTI_TRUE;

done:
    if (!OSAPI_System_unlock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return ok;
}

#ifndef RTI_CERT

/*ci
 * \brief Stop the timer
 *
 * \details
 * Implementation of OSAPI_System_stop_timer
 *
 * @param[in] self Timer
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * @pre Initialized system
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!OSAPI_SystemFreeRTOS_fv_System->_parent.is_initialized)
    {
        goto fail;
    }

    if (!OSAPI_System_lock())
    {
        goto fail;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_g.timer_handler[i].param == self)
        {
            OSAPI_System_g.timer_handler[i].handler = NULL;
            OSAPI_System_g.timer_handler[i].param = NULL;
            --OSAPI_System_g.timer_count;
        }
    }


    if (!OSAPI_System_unlock())
    {
        goto fail;
    }

    ok = RTI_TRUE;

    return ok;

fail:
    OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
    return ok;

}
#endif /* !RTI_CERT */

/*ci
 * \brief Get the resolution of the clock driving the timer in nanoseconds
 *
 * \details
 * Implementation of OSAPI_System_get_timer_resolution
 *
 * This function returns the frequency of the system timer used to implement
 * OSAPI_SystemI::start_timer and OSAPI_SystemI::stop_timer API
 *
 * @return Timer resolution in nanoseconds or 0 if the system has not been initialized
 */
RTI_PRIVATE RTI_INT32
OSAPI_SystemFreeRTOS_get_timer_resolution(void)
{
    return (OSAPISYSTEM_TIMER_RESOLUTION * 1000000);
}

/*ci
 * \brief Generate a unique universal identifier (UUID)
 *
 * \details
 * Implementation of OSAPI_System_generate_uuid
 *
 * @param[inout] uuid_out The generated UUID
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_SystemTime now;
    RTI_UINT64 pid;

    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out", uuid_out, RTI_TRUE);)

    if (!OSAPI_SystemFreeRTOS_get_time(&now))
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

    uuid_out->value[2] = (RTI_UINT32)now.nanosec;
    uuid_out->value[3] = (RTI_UINT32)now.sec;

    return RTI_TRUE;
}

/*ci
 * \brief Get the hostname
 *
 * \details
 * Implementation of OSAPI_System_get_hostname
 *
 * @param[in] hostname Hostname to fill in
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_get_hostname(char *const hostname)
{
    RTI_SIZE_T len;

    len = OSAPI_String_length(OSAPI_PLATFORM_FREERTOS_HOSTNAME);
    if (len >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        len = OSAPI_SYSTEM_MAX_HOSTNAME - 1;
    }
    OSAPI_Memory_copy(hostname,OSAPI_PLATFORM_FREERTOS_HOSTNAME, len);
    hostname[len] = 0;

    return RTI_TRUE;
}

/*ci
 * \brief Get the current tick time
 *
 * \details
 * Implementation of OSAPI_System_get_ticktime interface
 *
 * @param[inout] sec The current tick-time seconds part
 * @param[inout] nanosec The current tick-time nano-seconds part
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{
    struct OSAPI_SystemFreeRTOS *self = OSAPI_SystemFreeRTOS_fv_System;

    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = self->tick_sec;
    *nanosec = self->tick_nanosec;

    if (!OSAPI_Mutex_give(self->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/
/*ci
 * \brief Get the native interface
 *
 * \details
 * Implementation of OSAPI_System_get_native_interface
 *
 * @param[in] intf Pointer to the system interface structure
 */
void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{
    intf->start_timer = OSAPI_SystemFreeRTOS_start_timer;
    intf->get_timer_resolution = OSAPI_SystemFreeRTOS_get_timer_resolution;
    intf->get_time = OSAPI_SystemFreeRTOS_get_time;
    intf->initialize = OSAPI_SystemFreeRTOS_initialize;
    intf->generate_uuid = OSAPI_SystemFreeRTOS_generate_uuid;
    intf->get_hostname = OSAPI_SystemFreeRTOS_get_hostname;
    intf->get_ticktime = OSAPI_SystemFreeRTOS_get_ticktime;
#ifndef RTI_CERT
    intf->stop_timer = OSAPI_SystemFreeRTOS_stop_timer;
    intf->finalize = OSAPI_SystemFreeRTOS_finalize;
#endif /* !RTI_CERT */
}
