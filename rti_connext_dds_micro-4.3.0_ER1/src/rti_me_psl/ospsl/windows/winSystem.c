/*
 * FILE: winSystem.c - Win system functionality
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
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

#if OSAPI_HIGH_PRECISION_TIMER_ENABLED
#include <realtimeapiset.h>
#endif

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
#include "osapi/osapi_time.h"

#include "winThread.h"

#define OSAPISYSTEM_MAX_TIMERS   \
    OSAPI_System_fv_SystemWin->_parent.property.max_timers

/*ci \brief Timer resolution in NS
 */
#define OSAPISYSTEM_TIMER_RESOLUTION_NS (10 * OSAPI_TIME_NSEC_PER_MSEC)

struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction handler;
    void *param;
};

struct OSAPI_SystemWin
{
    /*ci \brief base-class
     */
    struct OSAPI_System _parent;

    /*ci \brief Array of timer objects
     */
    struct OSAPI_SystemTimerHandler *timer_handler;

    /*ci \brief Number of counters
     */
    RTI_INT32 timer_count;

    struct OSAPI_Mutex *mutex;

    /*ci \brief Actual timer resolution returned from Windows in milliseconds
     */
    UINT timer_res_ms;

    /*ci The number of seconds passed since the Micro system was started
     */
    RTI_INT32 tick_sec;

    /*ci The number of nanoseconds passed since the Micro system was started
     */
    RTI_UINT32 tick_nanosec;

    /*ci The tick resolution in nanoseconds, calculated from the actual
     * timer resolution in milliseconds in timer_res_ms.
     */
    RTI_UINT32 tick_resolution_ns;

    /*ci Mutex to protect updates to the Micro tick timer
     */
    struct OSAPI_Mutex *tick_mutex;

#if OSAPI_TIMER_THREAD_ENABLED
    /*ci \brief Timer thread when enabled
     */
    struct OSAPI_WinNativeThread *timer_thread;

    /*ci \brief Timer semaphore to pend on
     */
    HANDLE timer_sem;
#else
    /*ci \brief Set to TRUE when main timer is deleted
     */
    RTI_BOOL is_deleted;

    /*ci \brief Set to TRUE when main timer is running
     */
    RTI_BOOL timer_is_busy;

    /*ci \brief queue timer when using queued timers
     */
    HANDLE system_timer;
#endif

    /*ci \brief Accumulated timer ticks
     */
    ULONGLONG tick_acc;

    /*ci \brief The number of ticks since last time the timers were updated
     */
    ULONGLONG tick_last;
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

/*ci \brief Return the current system time.
 *
 * \details
 * Return current system system time. The system time is monotonically
 * increasing.
 *
 * \return system time as unsigned 64-bit integer.
 */
RTI_PRIVATE ULONGLONG
OSAPI_WinSystem_get_system_time(void)
{
    ULONGLONG curr_tick;

#if OSAPI_HIGH_PRECISION_TIMER_ENABLED
    QueryInterruptTimePrecise(&curr_tick);
#else
    curr_tick = GetTickCount64();
#endif

    return curr_tick;
}

RTI_PRIVATE void
OSAPI_WinSystem_run_timers_one_tick(struct OSAPI_SystemWin *self)
{
    RTI_INT32 i, j;
    RTI_UINT32 last_tick_ns;
    RTI_BOOL bretval;

    for (i = 0, j = self->timer_count;(i < OSAPISYSTEM_MAX_TIMERS) && j; ++i)
    {
        if (self->timer_handler[i].handler)
        {
            j--;
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
    }

    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        return;
    }

    last_tick_ns = self->tick_nanosec;
    self->tick_nanosec += self->tick_resolution_ns;
    self->tick_nanosec %= OSAPI_TIME_NSEC_PER_SEC;

    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);
}

RTI_PRIVATE void
OSAPI_WinSystem_run_timers(struct OSAPI_SystemWin *self)
{
    RTI_BOOL bretval;
    ULONGLONG curr_tick;

    curr_tick = OSAPI_WinSystem_get_system_time();

    (void)OSAPI_System_lock();

    self->tick_acc += (curr_tick - self->tick_last);
    self->tick_last = curr_tick;

    while (self->tick_acc >= (self->tick_resolution_ns / NS_PER_OS_TICK))
    {
        OSAPI_WinSystem_run_timers_one_tick(self);
        self->tick_acc -= self->tick_resolution_ns / NS_PER_OS_TICK;
    }

    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);

}

#if OSAPI_TIMER_THREAD_ENABLED

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_timer_thread(struct OSAPI_ThreadInfo *thread_param)
{
    struct OSAPI_SystemWin *self = (struct OSAPI_SystemWin *)thread_param->user_data;
    DWORD rc;

    self->tick_last = OSAPI_WinSystem_get_system_time();

    while (!thread_param->stop_thread)
    {
        rc = WaitForSingleObject(self->timer_sem, self->timer_res_ms);

#if OSAPI_ENABLE_LOG
        if (rc != WAIT_TIMEOUT)
        {
            if (rc == WAIT_FAILED)
            {
                rc = GetLastError();
            }
            OSAPI_LOG_WIN_WAIT_FAILED(OSAPI_LOGKIND_ERROR,rc);
        }
#endif

        OSAPI_WinSystem_run_timers(self);
    }

    return RTI_TRUE;
}
#else
VOID CALLBACK
OSAPI_SystemWin_timer_callback(PVOID param,BOOL TimerOrWaitFired)
{
    struct OSAPI_SystemWin *self = (struct OSAPI_SystemWin *)param;

    if (self->is_deleted)
    {
        self->timer_is_busy = RTI_FALSE;
    }
    else
    {
        OSAPI_WinSystem_run_timers(self);
    }
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_initialize(struct OSAPI_SystemWin *self)
{
    LONG prev_value;
#if !OSAPI_TIMER_THREAD_ENABLED
    TIMECAPS tc;
    MMRESULT mrc;
#endif

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


    self->tick_sec = 0;
    self->tick_nanosec = 0;
    self->timer_res_ms = OSAPISYSTEM_TIMER_RESOLUTION_NS / OSAPI_TIME_NSEC_PER_MSEC;

#if OSAPI_TIMER_THREAD_ENABLED
    self->timer_sem = CreateSemaphore(NULL,  /* security attributes */
                                 0,       /* empty */
                                 1,       /* maximum count */
                                 NULL /* unamed */ );

    if (self->timer_sem == NULL)
    {
        return RTI_FALSE;
    }

    self->timer_thread = OSAPI_Thread_create_win_native("osapi_timer",
                    &self->_parent.property.timer_property.thread,
                    OSAPI_SystemWin_timer_thread,(void *)self,NULL);

    if (self->timer_thread == NULL)
    {
        return RTI_FALSE;
    }
#else
    self->timer_is_busy = RTI_TRUE;
    self->is_deleted = RTI_FALSE;

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        return RTI_FALSE;
    }
    self->timer_res_ms = min(max(tc.wPeriodMin, self->timer_res_ms), tc.wPeriodMax);
    mrc = timeBeginPeriod(self->timer_res_ms);
    if (mrc != TIMERR_NOERROR)
    {
        return RTI_FALSE;
    }

    if (!CreateTimerQueueTimer(&self->system_timer,(HANDLE)NULL,
                          (WAITORTIMERCALLBACK)OSAPI_SystemWin_timer_callback,
                          self,0,self->timer_res_ms,WT_EXECUTEDEFAULT))
    {
        return RTI_FALSE;
    }
    self->tick_last = OSAPI_WinSystem_get_system_time();
#endif

    self->tick_resolution_ns =  self->timer_res_ms * OSAPI_TIME_NSEC_PER_MSEC;

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci \brief Finalize the windows system
 *
 * \details
 *
 * This function is not thread-safe.
 *
 * \param[in] self System to finalize
 *
 * \return TRUE on success, FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemWin_finalize(struct OSAPI_SystemWin *self)
{
    if (!self->_parent.is_initialized)
    {
        return RTI_FALSE;
    }

#if OSAPI_TIMER_THREAD_ENABLED
    if (!OSAPI_Thread_destroy_win_native(self->timer_thread))
    {
        return RTI_FALSE;
    }
    self->timer_thread = NULL;
    if (!CloseHandle(self->timer_sem))
    {
        return RTI_FALSE;
    }
    self->timer_sem = NULL;
#else
    /* This only works if the system was successfully initialized */
    self->is_deleted = RTI_TRUE;
    while (self->timer_is_busy)
    {
        Sleep(10);
    }

    if (!DeleteTimerQueueTimer(NULL,self->system_timer,INVALID_HANDLE_VALUE))
    {
        return RTI_FALSE;
    }
    timeEndPeriod(self->timer_res_ms);
#endif

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

    return OSAPI_System_fv_SystemWin->tick_resolution_ns;
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
