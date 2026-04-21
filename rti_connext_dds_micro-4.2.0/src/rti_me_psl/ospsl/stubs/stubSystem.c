/*
 * FILE: stubSystem.c - stub system functionality
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI system routines
 */
#include "rti_me_psl.h"

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

#define OSAPISYSTEM_MAX_TIMERS          8
#define OSAPISYSTEM_TIMER_RESOLUTION    100

struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction handler;
    void *param;
};

struct OSAPI_SystemWin
{
    struct OSAPI_System _parent;
};

RTI_PRIVATE struct OSAPI_SystemWin OSAPI_System_g = { OSAPI_System_INITIALIZER };
RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemWin);
struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;
struct OSAPI_LogEntryI *OSAPI_Log_gv_LogIntf = NULL;

/*** SOURCE_BEGIN ***/

/******************************************************************************
 *  Public API
 ******************************************************************************/

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_get_hostname(char *const hostname)
{
    UNUSED_ARG(hostname);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_get_time(OSAPI_SystemTime *now)
{
    UNUSED_ARG(now);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_start_timer(OSAPI_Timer_T self,
                        OSAPI_TimerTickHandlerFunction tick_handler)
{
    UNUSED_ARG(self);
    UNUSED_ARG(tick_handler);

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_stop_timer(OSAPI_Timer_T self)
{
    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    UNUSED_ARG(self);

    return RTI_TRUE;
}
#endif

RTI_PRIVATE RTI_INT32
OSAPI_WinSystem_get_timer_resolution(void)
{
    return 0;
}

RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_initialize(void)
{
    return RTI_FALSE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_finalize(void)
{
    return RTI_FALSE;
}
#endif

RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

    UNUSED_ARG(uuid_out);

    return RTI_FALSE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_WinSystem_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{
    UNUSED_ARG(sec);
    UNUSED_ARG(nanosec);

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
