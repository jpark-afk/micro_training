/*
 * FILE: threadxThread.c - ThreadX thread functionality
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
 * 13dec2016,francisco  File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI thread routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

#include "osapi/osapi_string.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "../common/Thread.h"

/* pool for thread stack is created in system init */
LINK_SECTION_BSS_SDRAM
extern
TX_BYTE_POOL thread_pool;

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_Thread_map_to_native_priority(RTI_INT32 *native_priority,
                                    RTI_INT32  priority_level)
{
    if (priority_level >= 0)
    {
        *native_priority = priority_level;
        return RTI_TRUE;
    }

    switch (priority_level)
    {
        case OSAPI_THREAD_PRIORITY_LOW:
            *native_priority = TX_TIMER_THREAD_PRIORITY + 4;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_BELOW_NORMAL:
            *native_priority = TX_TIMER_THREAD_PRIORITY + 3;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_NORMAL:
            *native_priority = TX_TIMER_THREAD_PRIORITY + 2;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_ABOVE_NORMAL:
            *native_priority = TX_TIMER_THREAD_PRIORITY + 1;
            return RTI_TRUE;
        case OSAPI_THREAD_PRIORITY_HIGH:
            *native_priority = TX_TIMER_THREAD_PRIORITY;
            return RTI_TRUE;
        default:
            OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,
                                          *native_priority,
                                          priority_level)
            return RTI_FALSE;
    }

    if (*native_priority > (TX_MAX_PRIORITIES - 1))
    {
        *native_priority = TX_MAX_PRIORITIES - 1;
    }    

    return RTI_TRUE;
}

RTI_PRIVATE void
OSAPI_Thread_on_before_delete(struct OSAPI_Thread *me)
{
    UINT rc;

    /* destroy thread */
    rc = tx_thread_delete(&me->thread_handle.thread);
    IGNORE_RETVAL(rc);

    /* release thread stack before the thread is deleted */
    if (me->thread_handle.stack != NULL)
    {
        rc = tx_byte_release(me->thread_handle.stack);
        IGNORE_RETVAL(rc);
        me->thread_handle.stack = NULL;
    }
}

static void OSAPI_Thread_exec_(ULONG param)
{
    void* ret;
    ret = OSAPI_Thread_exec((void*)param);
    (void)ret;
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
    int                  stack_size;
    int                  priority = TX_TIMER_THREAD_PRIORITY;
    struct OSAPI_Thread *me = NULL;
    RTI_INT32            len;
    UINT                 rc;
    void                *stack_start = NULL;

    OSAPI_PRECONDITION((properties == NULL) || (name == NULL),
                        return NULL,
                        OSAPI_Log_entry_add_pointer("properties",properties,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    /* create the thread handle */
    me = OSAPI_Thread_new(properties);
    if (me == NULL)
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    if (properties->stack_size > OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack_size = properties->stack_size;
    }
    else
    {
        stack_size = OSAPI_PLATFORM_THREADX_STACK_SIZE_DEFAULT;
    }

    /* allocate the thread stack */
    rc = tx_byte_allocate(&thread_pool, &stack_start, stack_size, TX_NO_WAIT);
    if ((rc != TX_SUCCESS) || (stack_start == NULL))
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto fail;
    }

    /* asign user's values */
    me->user_routine = user_routine;
    me->wakeup_routine = wakeup_routine;
    me->thread_info.user_data = user_data;
    me->thread_info.is_preemptive = RTI_TRUE;
    me->on_before_delete = OSAPI_Thread_on_before_delete;

    /* Ignore a too long name since it has not function impact
     */
    len = OSAPI_String_length(name);
    if (len >= OSAPI_THREAD_MAX_NAME)
    {
        len = OSAPI_THREAD_MAX_NAME-1;
    }
    OSAPI_Memory_copy(me->name,name,len);
    me->name[len] = 0;

    if (!OSAPI_Thread_map_to_native_priority(&priority, properties->priority))
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR,properties->priority)
        goto fail;
    }
    
    /* ensure that thread structure is zero-ed before creating thread. ThreadX
     * can be "confused" if a thread was already created and destroyed using
     * the same TX_THREAD structure
     */
    OSAPI_Memory_zero((void*)&me->thread_handle.thread,
                      sizeof(me->thread_handle.thread));

    rc = tx_thread_create(&me->thread_handle.thread, me->name, 
                          OSAPI_Thread_exec_, 
                          (ULONG)me, stack_start, stack_size,
                          priority, priority,
                          TX_NO_TIME_SLICE, 
                          TX_AUTO_START);
    if (rc != TX_SUCCESS)
    {
        OSAPI_LOG_THREAD_INIT(OSAPI_LOGKIND_ERROR, rc)
        goto fail;
    }
    me->thread_handle.stack = stack_start;

    /* spawned thread gives semaphore immediately when started */
    if (!OSAPI_Semaphore_take(me->create_sem, 
                              OSAPI_SEMAPHORE_TIMEOUT_INFINITE, 
                              NULL))
    {
        OSAPI_LOG_THREAD_NEW(OSAPI_LOGKIND_ERROR)
        goto fail;
    }

    return me;

fail:

#ifndef RTI_CERT
    if (stack_start != NULL)
    {
        rc = tx_byte_release(stack_start);
        IGNORE_RETVAL(rc);
    }
    if (me != NULL)
    {
        OSAPI_Thread_delete(me);
    }
#endif

    return NULL;
}

OSAPI_ThreadId
OSAPI_Thread_self(void)
{
    return (OSAPI_ThreadId)tx_thread_identify();
}

void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    UINT rc;

    rc = tx_thread_sleep((ULONG)ms / OSAPI_MS_TIMER_TICK);
    IGNORE_RETVAL(rc);
}
#endif
