/*
 * FILE: winSystem.c - Win system functionality
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 20aug2012,tk CR-75, cleanup
 * 04mar2012,tk Rewritten
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI system routines
 */
#include "rti_me_psl.h"

#include <stdlib.h>
#include <Windows.h>
#include <Mmsystem.h>
#include <Timeapi.h>

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
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#define OSAPISYSTEM_MAX_TIMERS   OSAPI_System_fv_SystemWin->_parent.property.max_timers
#define OSAPISYSTEM_TIMER_RESOLUTION    100

struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction handler;
    void *param;
};

struct OSAPI_SystemWin
{
    struct OSAPI_System _parent;
    struct OSAPI_SystemTimerHandler *timer_handler;
    RTI_INT32 timer_count;
    struct OSAPI_Mutex *mutex;
    struct OSAPI_Thread *timer_thread;
    UINT timer_res;
    HANDLE system_timer;
    RTI_BOOL is_deleted;
    RTI_BOOL timer_is_busy;
    RTI_INT32 tick_sec;
    RTI_UINT32 tick_nanosec;
    RTI_UINT32 tick_resolution_ns;
    struct OSAPI_Mutex *tick_mutex;
};

RTI_PRIVATE struct OSAPI_SystemWin OSAPI_System_g = { OSAPI_System_INITIALIZER };
RTI_PRIVATE struct OSAPI_SystemWin *OSAPI_System_fv_SystemWin = &OSAPI_System_g;
RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemWin);
struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;
struct OSAPI_LogEntryI *OSAPI_Log_gv_LogIntf = NULL;

/*** SOURCE_BEGIN ***/

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_System_fv_SystemWin->mutex);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_System_fv_SystemWin->mutex);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_timer_thread(struct OSAPI_ThreadInfo *thread_param)
{
    struct OSAPI_SystemWin *self = (struct OSAPI_SystemWin *)thread_param->user_data;
    RTI_INT32 i, j;

    while (!thread_param->stop_thread)
    {
        (void)OSAPI_System_lock();
        for (i = 0, j = self->timer_count; (i < OSAPISYSTEM_MAX_TIMERS) && j;
             ++i)
        {
            if (self->timer_handler[i].handler)
            {
                j--;
                self->timer_handler[i].handler(self->timer_handler[i].param);
            }
        }
        (void)OSAPI_System_unlock();
        Sleep(OSAPISYSTEM_TIMER_RESOLUTION);
    }

    return RTI_TRUE;
}

VOID CALLBACK
OSAPI_SystemWin_timer_callback(PVOID param,BOOL TimerOrWaitFired)
{
    struct OSAPI_SystemWin *self = (struct OSAPI_SystemWin *)param;
    RTI_INT32 i, j;
    RTI_UINT32 last_tick_ns;
    RTI_BOOL bretval;

    if (self->is_deleted)
    {
        self->timer_is_busy = RTI_FALSE;
        return;
    }

    (void)OSAPI_System_lock();

    for (i = 0, j = self->timer_count; (i < OSAPISYSTEM_MAX_TIMERS) && j;
            ++i)
    {
        if (self->timer_handler[i].handler)
        {
            j--;
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
    }

    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return;
    }

    last_tick_ns = self->tick_nanosec;
    self->tick_nanosec += self->tick_resolution_ns;
    self->tick_nanosec %= 1000000000;

    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);

    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_initialize(struct OSAPI_SystemWin *self)
{
    LONG prev_value;
    #define TARGET_RESOLUTION 10
    TIMECAPS tc;

    if (self->_parent.is_initialized)
    {
        return RTI_TRUE;
    }

    prev_value = InterlockedExchange(
                        &OSAPI_System_fv_SystemWin->_parent.is_initialized,1);
    if (prev_value != 0)
    {
        while (!OSAPI_System_fv_SystemWin->_parent.is_initialized);
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
    self->timer_count = 0;

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

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        return RTI_TRUE;
    }

    self->timer_res = min(max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
    timeBeginPeriod(self->timer_res);

#if 0
    self->timer_thread = OSAPI_Thread_create("osapi_timer",
                    &self->_parent.property.timer_property.thread,
                    OSAPI_SystemWin_timer_thread,(void *)self,NULL);

    if (self->timer_thread == NULL)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Thread_start(self->timer_thread))
    {
        return RTI_FALSE;
    }
#endif

    self->timer_is_busy = RTI_TRUE;
    self->is_deleted = RTI_FALSE;
    self->tick_sec = 0;
    self->tick_nanosec = 0;

    if (!CreateTimerQueueTimer(&self->system_timer,(HANDLE)NULL,
                          (WAITORTIMERCALLBACK)OSAPI_SystemWin_timer_callback,
                          self,0,self->timer_res,
                          WT_EXECUTEDEFAULT))
    {
        return RTI_FALSE;
    }

    self->_parent.is_initialized = RTI_TRUE;

    self->tick_resolution_ns =  self->timer_res * 1000000;

    return RTI_TRUE;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_finalize(struct OSAPI_SystemWin *self)
{
    if (!self->_parent.is_initialized)
    {
        return RTI_FALSE;
    }

    /* This only works if the system was successfully initialized */
    self->is_deleted = RTI_TRUE;
    while (self->timer_is_busy)
    {
        Sleep(10);
    }

    if (!OSAPI_System_lock())
    {
        return RTI_FALSE;
    }

#if 0
    if (!OSAPI_Thread_destroy(self->timer_thread))
    {
        (void)OSAPI_System_unlock();
        return RTI_FALSE;
    }
#endif

    if (!DeleteTimerQueueTimer(NULL,self->system_timer,INVALID_HANDLE_VALUE))
    {
        (void)OSAPI_System_unlock();
        return RTI_FALSE;
    }

    if (self->timer_handler != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(self->timer_handler);
#endif
        self->timer_handler = NULL;
    }

    self->timer_count = 0;

    if ((self->tick_mutex != NULL) && !OSAPI_Mutex_delete(self->tick_mutex))
    {
        return RTI_FALSE;
    }

    if (!OSAPI_System_unlock())
    {
        return RTI_FALSE;
    }

    if ((self->mutex != NULL) && !OSAPI_Mutex_delete(self->mutex))
    {
        return RTI_FALSE;
    }

    self->_parent.is_initialized = RTI_FALSE;

    return RTI_TRUE;
}
#endif

/******************************************************************************
 *  Public API
 ******************************************************************************/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_get_hostname(char *const hostname)
{
    DWORD length = OSAPI_SYSTEM_MAX_HOSTNAME;

    if (!GetComputerName(hostname,&length))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_WinSystem_get_time(OSAPI_SystemTime *now)
{
#define RTIDRT_EPOCHFILETIME (116444736000000000i64)
    FILETIME ft;
    ULARGE_INTEGER li;
    __int64 t;
    RTI_INT64 sec;
    RTI_UINT32 usec;

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("now",now,RTI_TRUE);)

    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    t = li.QuadPart;
    t -= RTIDRT_EPOCHFILETIME;
    t /= 10;
    sec = (RTI_INT64)(t / 1000000);
    usec = (RTI_UINT32)(t % 1000000);

    now->sec = sec;
    now->nanosec = usec * 1000;

#undef RTIDRT_EPOCHFILETIME

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_WinSystem_start_timer(OSAPI_Timer_T self,
                        OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("tick_handler",tick_handler,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemWin->_parent.is_initialized)
    {
        if (!OSAPI_SystemWin_initialize(OSAPI_System_fv_SystemWin))
        {
            OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (OSAPI_System_fv_SystemWin->timer_count == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        (void)OSAPI_System_unlock();
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_fv_SystemWin->timer_handler[i].handler == NULL)
        {
            break;
        }
    }

    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        (void)OSAPI_System_unlock();
        return RTI_FALSE;
    }

    OSAPI_System_fv_SystemWin->timer_handler[i].handler = tick_handler;
    OSAPI_System_fv_SystemWin->timer_handler[i].param = self;
    ++OSAPI_System_fv_SystemWin->timer_count;

    return OSAPI_System_unlock();
}

RTI_BOOL
OSAPI_WinSystem_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemWin->_parent.is_initialized)
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
        if (OSAPI_System_fv_SystemWin->timer_handler[i].param == self)
        {
            OSAPI_System_fv_SystemWin->timer_handler[i].handler = NULL;
            OSAPI_System_fv_SystemWin->timer_handler[i].param = NULL;
            --OSAPI_System_fv_SystemWin->timer_count;
        }
    }

    if (!OSAPI_System_unlock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_INT32
OSAPI_WinSystem_get_timer_resolution(void)
{
    if (!OSAPI_System_fv_SystemWin->_parent.is_initialized)
    {
        if (!OSAPI_SystemWin_initialize(OSAPI_System_fv_SystemWin))
        {
            OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
            return 0;
        }
    }

    return OSAPI_System_g.timer_res * 1000000;
    /* return OSAPISYSTEM_TIMER_RESOLUTION; */
}

RTI_BOOL
OSAPI_WinSystem_initialize(void)
{
    return OSAPI_SystemWin_initialize(OSAPI_System_fv_SystemWin);
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_WinSystem_finalize(void)
{
    return OSAPI_SystemWin_finalize(OSAPI_System_fv_SystemWin);
}
#endif

RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_SystemTime now;
    static RTI_UINT32 uuid_counter = 0xdeadc0de;
    union
    {
        OSAPI_ProcessId self_id;
        RTI_INT32 self_int;
        RTI_UINT32 self_uint;
    } self;

    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

    if (!OSAPI_WinSystem_get_time(&now))
    {
        return RTI_FALSE;
    }

#if RTI_ENDIAN_LITTLE
    self.self_uint = uuid_counter++;
    uuid_out->value[0] = ((self.self_uint>>24)&0x000000ffU) |
                         ((self.self_uint>>8)&0x0000ff00U)  |
                         ((self.self_uint<<24)&0xff000000U) |
                         ((self.self_uint<<8)&0x00ff0000U);
#else
    uuid_out->value[0] = uuid_counter++;
#endif
    self.self_id = OSAPI_Process_getpid();
    uuid_out->value[1] = self.self_uint;
    uuid_out->value[2] = (RTI_UINT32)now.nanosec;
    uuid_out->value[3] = (RTI_UINT32)now.sec;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{

    if (!OSAPI_Mutex_take(OSAPI_System_fv_SystemWin->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = OSAPI_System_fv_SystemWin->tick_sec;
    *nanosec = OSAPI_System_fv_SystemWin->tick_nanosec;

    if (!OSAPI_Mutex_give(OSAPI_System_fv_SystemWin->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/

struct OSAPI_SystemI OSAPI_WinSystem_gv_SysIntf =
{
    OSAPI_WinSystem_start_timer,
#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    OSAPI_WinSystem_stop_timer,
#else
    NULL,
#endif
#endif
    OSAPI_WinSystem_get_timer_resolution,
    OSAPI_WinSystem_get_time,
    OSAPI_WinSystem_initialize,
#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    OSAPI_WinSystem_finalize,
#else
    NULL,
#endif
#endif
    OSAPI_WinSystem_generate_uuid,
    OSAPI_WinSystem_get_hostname,
    OSAPI_WinSystem_get_ticktime,
    NULL,
    NULL
};

struct OSAPI_SystemI *OSAPI_System_gv_SysIntf = &OSAPI_WinSystem_gv_SysIntf;

void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{
    OSAPI_PRECONDITION_ALWAYS(intf == NULL,
                          return,
                          OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    *intf = OSAPI_WinSystem_gv_SysIntf;
}
