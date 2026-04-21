/*
 * FILE: vxThread.c - VxWorks thread functionality
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
 * 25aug2016,eh Fixed MICRO-1559: VxWorks 64-Bit compatibility
 * 14oct2014,tk MICRO-946 Set thread name
 * 26mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief VxWorks implementation of OSAPI thread routines
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_string.h"


#if OSAPI_INCLUDE_VXWORKS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <vxWorks.h>
#include <taskLib.h>
#ifdef RTI_RTP
  #include <taskLibCommon.h>
  #include <tlsLib.h>
  #include <unistd.h>
#else
  #include <taskVarLib.h>
#endif
#include <sysLib.h>
#include <tickLib.h>

#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "../common/Thread.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS

#define OSAPI_VXTHREAD_DEFAULT_STACKSIZE (128*1024)

/*ci
 * \brief Convert a priority level to the native VxWorks priority level
 *
 * \details
 * This function takes a priority level specified in the OSAPI_ThreadProperty
 * property and converts it to a VxWorks priority. If the priority level is
 * >= 0 then the level is used as is. If it is negative it is converted
 * to a positive number based on a predefined set of logical level.
 *
 * \param[inout]  native_priority The native VxWorks priority
 * \param[in]     priority_level  The priority level to convert
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_map_to_native_thread_priority(RTI_INT32 *native_priority,
                                    RTI_INT32 priority_level)
{
    *native_priority = 0;

    if (priority_level >= 0)
    {
        *native_priority = priority_level;
        return RTI_TRUE;
    }

    /* we center our priority around the typical 100 */
    switch (priority_level)
    {
        case OSAPI_THREAD_PRIORITY_LOW:
            *native_priority = 120;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_BELOW_NORMAL:
            *native_priority = 110;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_NORMAL:
            *native_priority = 100;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_ABOVE_NORMAL:
            *native_priority = 71;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_HIGH:
            *native_priority = 68;
            return RTI_TRUE;
        default:
            OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,*native_priority,
                                          priority_level)
            return RTI_FALSE;
    }

    return RTI_TRUE;
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
                   void *user_data,
                   OSAPI_ThreadRoutine wakeup_routine)
{
    int os_options = 0;
    int priority;
    struct OSAPI_Thread *me;
    int stack_size;
    RTI_INT32 len;

    OSAPI_PRECONDITION((properties == NULL) || (name == NULL),
                       return NULL,
                       OSAPI_Log_entry_add_pointer("properties",properties,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

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

    /* Calculate native priority */
    if (!OSAPI_map_to_native_thread_priority(&priority, properties->priority))
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,properties->priority)
        goto fail;
    }

    /* user indicates they will be doing fp, setup vxworks thread state correctly
     */
    if (properties->options & OSAPI_THREAD_FLOATING_POINT)
    {
        os_options |= VX_FP_TASK;
    }

#if !defined(RTI_RTP)
#if defined (RTI_LOG_ERROR)
    /* RTI_LOG_ERROR is the lowest form of debugging, indicating there's always a
     * chance that we'll be printing in the thread - we automatically enabled
     * VX_STDIO regardless of their thread option */
    os_options |= VX_STDIO;
#else
    /* user indicates they will be doing stdio, setup vxworks thread state
     * correctly */
    if (properties->options & OSAPI_THREAD_STDIO)
    {
        os_options |= VX_STDIO;
    }
#endif /* RTI_LOG_ERROR */
#endif /* !RTI_RTP */

    if (properties->stack_size == OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack_size = OSAPI_VXTHREAD_DEFAULT_STACKSIZE;
    }
    else
    {
        stack_size = properties->stack_size;
    }
    me->thread_handle = taskSpawn((char *)name,
                                 priority,
                                 os_options,
                                 stack_size,
                                 (FUNCPTR) OSAPI_Thread_exec,
                                 (OSAPI_UserArg)me, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    if (me->thread_handle == (OSAPI_ThreadHandle)ERROR)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    /* spawned thread gives semaphore immediately when started */
    if (!OSAPI_Semaphore_take(me->create_sem,
                              OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
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
    return taskIdSelf();
}

void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    taskDelay(((sysClkRateGet() * ms) + 500) / 1000);
}

#endif

















