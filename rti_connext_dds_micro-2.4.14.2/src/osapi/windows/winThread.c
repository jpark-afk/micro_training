/*
 * FILE: winThread.c - Win thread functionality
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
 * 13dec2020,tk MICRO-2740/PR.28040
 *              - Renamed is_premptive to is_preemptive
 * 26mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI thread routines
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_string.h"

#if OSAPI_INCLUDE_WINDOWS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <process.h>

#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "../common/Thread.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_WINDOWS

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_Thread_map_to_native_priority(RTI_INT32 *nativePriority,
                                    RTI_INT32 priorityLevel)
{
    if (priorityLevel >= 0)
    {
        *nativePriority = priorityLevel;
        return RTI_TRUE;
    }

    /* we center our priority around the typical 100 */
    switch (priorityLevel)
    {
        case OSAPI_THREAD_PRIORITY_LOW:
            *nativePriority = THREAD_PRIORITY_LOWEST;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_BELOW_NORMAL:
            *nativePriority = THREAD_PRIORITY_BELOW_NORMAL;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_NORMAL:
            *nativePriority = THREAD_PRIORITY_NORMAL;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_ABOVE_NORMAL:
            *nativePriority = THREAD_PRIORITY_ABOVE_NORMAL;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_HIGH:
            *nativePriority = THREAD_PRIORITY_HIGHEST;
            return RTI_TRUE;
        default:
            OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,*nativePriority,
                                          priorityLevel)
            return RTI_FALSE;
    }
}

/*
    allocates needed memory and spawns the thread. the spawned thread pends
    until start() is called before actually running user's routine. thread
    spawning and pending states are guaranteed when RTI_TRUE is returned
 */
struct OSAPI_Thread*
OSAPI_Thread_create(const char *name,
                   const struct OSAPI_ThreadProperty *properties,
                   OSAPI_ThreadRoutine user_routine,
                   void *user_data, OSAPI_ThreadRoutine wakeup_routine)
{
    struct OSAPI_Thread *me;
    RTI_INT32 len;

    OSAPI_PRECONDITION((properties == NULL) || (name == NULL),
                        return NULL,
                        OSAPI_Log_entry_add_pointer("properties",properties,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    /* create the thread handle */
    me = OSAPI_Thread_new(properties);
    if (me == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    /* asign user's values */
    me->user_routine = user_routine;
    me->wakeup_routine = wakeup_routine;
    me->thread_info.user_data = user_data;
    me->thread_info.is_preemptive = RTI_TRUE;

    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= OSAPI_THREAD_MAX_NAME)
    {
        len = OSAPI_THREAD_MAX_NAME-1;
    }
    OSAPI_Memory_copy(me->name,name,len);
    me->name[len] = 0;

    me->thread_handle = (HANDLE)
        _beginthread((void (__cdecl *) (void *))OSAPI_Thread_exec,
                     properties->stack_size, (void *)me);
    if (me->thread_handle == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    /* spawned thread gives semaphore immediately when started */
    if (!OSAPI_Semaphore_take(me->create_sem, OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    return me;

fail:

    return NULL;
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{
    return GetCurrentThreadId();
}

void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    Sleep((DWORD)ms);
}
#endif
