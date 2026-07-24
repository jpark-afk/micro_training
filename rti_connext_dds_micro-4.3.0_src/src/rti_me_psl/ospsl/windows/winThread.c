/*
 * FILE: winThread.c - Win thread functionality
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
 * 26mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI thread routines
 */
#include "rti_me_psl.h"

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
#include "osapi/osapi_string.h"

#include "winThread.h"

/*** SOURCE_BEGIN ***/

struct OSAPI_WinNativeThread
{
    char name[64];

    HANDLE native_thread_handle;

    OSAPI_ThreadRoutine thread_routine;

    struct OSAPI_ThreadInfo thread_info;

    OSAPI_ThreadRoutine thread_wakeup;

    OSAPI_ThreadHandle thread_handle;

    volatile RTI_BOOL running;
};

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

void
OSAPI_Thread_start_native(void *thread)
{
    struct OSAPI_WinNativeThread *native_thread =
                                    (struct OSAPI_WinNativeThread*)thread;
    native_thread->running = RTI_TRUE;
    native_thread->thread_routine(&native_thread->thread_info);
    native_thread->running = RTI_FALSE;
}

/*
    allocates needed memory and spawns the thread. the spawned thread pends
    until start() is called before actually running user's routine. thread
    spawning and pending states are guaranteed when RTI_TRUE is returned
 */
struct OSAPI_WinNativeThread*
OSAPI_Thread_create_win_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup)
{    RTI_INT32 len;
    HANDLE th_handle = NULL;
    struct OSAPI_WinNativeThread *native_thread;

    OSAPI_PRECONDITION((property == NULL) || (name == NULL) ||
                       (thread_entry == NULL),
                return NULL,
            OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("thread_entry",
                           thread_entry == NULL ? NULL : (void*)1,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    OSAPI_Heap_allocate_buffer((char**)&native_thread,
                               sizeof(struct OSAPI_WinNativeThread),
                               OSAPI_ALIGNMENT_DEFAULT);
    if (native_thread == NULL)
    {
        return NULL;
    }

    native_thread->thread_routine = thread_entry;
    native_thread->thread_wakeup = thread_wakeup;

    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= 64)
    {
        len = 64-1;
    }
    OSAPI_Memory_copy(native_thread->name,name,len);
    native_thread->name[len] = 0;
    native_thread->thread_info.stop_thread = RTI_FALSE;
    native_thread->thread_info.user_data = thread_data;
    native_thread->thread_info.is_preemptive = RTI_TRUE;

    native_thread->native_thread_handle = (HANDLE)
        _beginthread(OSAPI_Thread_start_native,property->stack_size,
                     (void *)native_thread);
    if (native_thread->native_thread_handle == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    return native_thread;

fail:

    return NULL;
}

OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup)
{
    struct OSAPI_WinNativeThread *native_thread;

    native_thread = OSAPI_Thread_create_win_native(name,property,thread_entry,thread_data,thread_wakeup);

    return (OSAPI_ThreadHandle)native_thread;
}

RTI_BOOL
OSAPI_Thread_destroy_win_native(struct OSAPI_WinNativeThread *native_thread)
{
    native_thread->thread_info.stop_thread = RTI_TRUE;

    if (native_thread->thread_wakeup != NULL)
    {
        native_thread->thread_wakeup(&native_thread->thread_info);
    }

    while (native_thread->running)
    {
        Sleep(1);
    }

#ifndef RTI_CERT
    OSAPI_Heap_free_buffer(native_thread);
#endif

    return RTI_TRUE;
}


MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle)
{
    struct OSAPI_WinNativeThread *native_thread =
                        (struct OSAPI_WinNativeThread*)handle;

    return OSAPI_Thread_destroy_win_native(native_thread);
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{
    OSAPI_ThreadId tid;

    tid.handle.handle64 = (RTI_UINT64)GetCurrentThreadId();

    return tid;
}

void
OSAPI_Thread_nanosleep(RTI_UINT32 ns)
{
    Sleep((DWORD)1);
}

void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    Sleep((DWORD)ms);
}

#if OSAPI_ENABLE_LOG
RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return errno;
}

void
OSAPI_Log_set_last_error_code(RTI_INT32 err)
{
    errno = err;
}
#endif
