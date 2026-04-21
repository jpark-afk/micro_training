/*
 * FILE: stubThread.c - stub thread functionality
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
 * \brief Win implementation of OSAPI thread routines
 */
#include "rti_me_psl.h"

#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"

/*** SOURCE_BEGIN ***/

struct OSAPI_WinNativeThread
{
    RTI_BOOL running;
};

/*
    allocates needed memory and spawns the thread. the spawned thread pends
    until start() is called before actually running user's routine. thread
    spawning and pending states are guaranteed when RTI_TRUE is returned
 */
OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data, 
                           OSAPI_ThreadRoutine thread_wakeup)
{    
    UNUSED_ARG(name);
    UNUSED_ARG(property);
    UNUSED_ARG(thread_entry);
    UNUSED_ARG(thread_data);
    UNUSED_ARG(thread_wakeup);

    return NULL;
}

RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle)
{
    UNUSED_ARG(handle);

    return RTI_TRUE;
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{    
    OSAPI_ThreadId tid;

    tid.handle.data = NULL;

    return tid;
}

void
OSAPI_Thread_nanosleep(RTI_UINT32 ns)
{
    UNUSED_ARG(ns);
}

#ifndef RTI_CERT
void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    UNUSED_ARG(ms);
}
#endif

#if OSAPI_ENABLE_LOG
RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return 0;
}

void
OSAPI_Log_set_last_error_code(RTI_INT32 err)
{
    UNUSED_ARG(err);
}
#endif
