/*
 * FILE: freertosSystem.c - System functionality
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
 * 21oct2014,eh MICRO-953: update FreeRTOS port
 * 04mar2012,tk Rewritten
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI system routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "../common/System.h"

#define OSAPISYSTEM_MAX_TIMERS          8
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
    struct OSAPI_SystemTimerHandler timer_handler[OSAPISYSTEM_MAX_TIMERS];
    RTI_BOOL thread_exit;
    RTI_BOOL thread_exited;
    xTaskHandle timer_tid;
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

RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemFreeRTOS);

struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;

/* lock for timer initialization */
RTI_PRIVATE int OSAPI_System_fv_initLock = 0;

#endif /* OSAPI_PLATFORM ==  OSAPI_PLATFORM_FREERTOS */

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_FREERTOS

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_System_g.mutex);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_System_g.mutex);
}

RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_get_time(OSAPI_NtpTime * now)
{
    /* Determine time from number of ticks since system started */
    portTickType ticks_since_start, ms_since_start;

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("now", now, RTI_TRUE);)

    ticks_since_start = xTaskGetTickCountFromISR();
    ms_since_start = ticks_since_start * portTICK_RATE_MS;

    OSAPI_NtpTime_from_millisec(now,
                                ms_since_start / 1000, ms_since_start % 1000);
    return RTI_TRUE;
}

RTI_PRIVATE void
OSAPI_SystemFreeRTOS_on_timer_callback(xTimerHandle timerHandle)
{
    RTI_UINT32 last_tick_ns;
    RTI_BOOL bretval;
    RTI_INT32 i, j;
    struct OSAPI_SystemFreeRTOS *self =
            (struct OSAPI_SystemFreeRTOS *)OSAPI_System_gv_System;

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
    self->tick_nanosec += (OSAPISYSTEM_TIMER_RESOLUTION * 1000000);
    self->tick_nanosec %= 1000000000;

    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
}

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

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->thread_exited = RTI_FALSE;
    self->thread_exit = RTI_FALSE;
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
         return RTI_FALSE;
    }

    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
        return RTI_FALSE;
    }

    self->system_timer =
        xTimerCreate("systimer", self->timer_res, pdTRUE,
                     (void *)OSAPISYSTEM_MAX_TIMERS,
                     OSAPI_SystemFreeRTOS_on_timer_callback);
    if (self->system_timer == NULL)
    {
        return RTI_FALSE;
    }

    if (pdTRUE != xTimerStart(self->system_timer, portMAX_DELAY))
    {
        return RTI_FALSE;
    }

    self->is_initialized = RTI_TRUE;
    OSAPI_System_fv_initLock = 0;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_initialize(void)
{
    return OSAPI_SystemFreeRTOS_initializeI(&OSAPI_System_g);
}


RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_finalizeI(struct OSAPI_SystemFreeRTOS *self)
{
    RTI_INT32 i;

    if (!OSAPI_System_lock())
    {
        return RTI_FALSE;
    }

    for (i = 0; self->timer_count && (i < OSAPISYSTEM_MAX_TIMERS); ++i)
    {
        if (self->timer_handler[i].handler != NULL)
        {
            /* stop, delete timer */
        }
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
        --self->timer_count;
    }

    if (self->system_timer != NULL)
    {
        if (pdTRUE != xTimerStop(self->system_timer, portMAX_DELAY))
        {
            return RTI_FALSE;
        }
        if (pdTRUE != xTimerDelete(self->system_timer, portMAX_DELAY))
        {
            return RTI_FALSE;
        }
        self->system_timer = NULL;
    }

#ifndef RTI_CERT
    if ((self->tick_mutex != NULL) && !OSAPI_Mutex_delete(self->tick_mutex))
    {
        return RTI_FALSE;
    }
#endif /* !RTI_CERT */

    if (!OSAPI_System_unlock())
    {
        return RTI_FALSE;
    }

#ifndef RTI_CERT
    if ((self->mutex != NULL) && !OSAPI_Mutex_delete(self->mutex))
    {
        return RTI_FALSE;
    }
#endif /* !RTI_CERT */

    self->is_initialized = RTI_FALSE;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_finalize(void)
{
    return OSAPI_SystemFreeRTOS_finalizeI(&OSAPI_System_g);
}

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

    OSAPI_SystemFreeRTOS_initialize();

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

RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (!OSAPI_System_gv_System->is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
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
    ok = RTI_TRUE;

    if (!OSAPI_System_unlock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return ok;
}

RTI_PRIVATE RTI_INT32
OSAPI_SystemFreeRTOS_get_timer_resolution(void)
{
    return (OSAPISYSTEM_TIMER_RESOLUTION * 1000000);
}


RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_NtpTime now;

    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out", uuid_out, RTI_TRUE);)

    if (!OSAPI_SystemFreeRTOS_get_time(&now))
    {
        return RTI_FALSE;
    }

    uuid_out->value[0] = (RTI_UINT32)now.sec;
    uuid_out->value[1] = (RTI_UINT32)now.frac;
    uuid_out->value[2] = (RTI_UINT32)&OSAPI_System_g;
    uuid_out->value[3] = (RTI_UINT32)now.frac;

    return RTI_TRUE;
}

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

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemFreeRTOS_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{
    struct OSAPI_SystemFreeRTOS *self =
            (struct OSAPI_SystemFreeRTOS *)OSAPI_System_gv_System;

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
void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{
    intf->start_timer = OSAPI_SystemFreeRTOS_start_timer;
    intf->stop_timer = OSAPI_SystemFreeRTOS_stop_timer;
    intf->get_timer_resolution = OSAPI_SystemFreeRTOS_get_timer_resolution;
    intf->get_time = OSAPI_SystemFreeRTOS_get_time;
    intf->initialize = OSAPI_SystemFreeRTOS_initialize;
    intf->finalize = OSAPI_SystemFreeRTOS_finalize;
    intf->generate_uuid = OSAPI_SystemFreeRTOS_generate_uuid;
    intf->get_hostname = OSAPI_SystemFreeRTOS_get_hostname;
    intf->get_ticktime = OSAPI_SystemFreeRTOS_get_ticktime;
}

#endif
