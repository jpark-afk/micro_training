/*
 * FILE: freertosThread.c - Thread functionality
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
 * 21oct2014,eh MICRO-953: update FreeRTOS port
 * 06mar2012,tk Written
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI thread routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS

#include "FreeRTOS.h"
#include "task.h"

#include "osapi/osapi_string.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "../common/Thread.h"

#endif /* OSAPI_INCLUDE_FREERTOS */

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_FREERTOS

/*
 *\brief
 * Wrap OSAPI_Thread_exec function so it can be called as
 * TaskFunction_t on xTaskCreate without a warning.
 *
 * \param[in] param The param that will be passed to OSAPI_Thread_exec.
 *
 * \sa OSAPI_Thread_exec
*/
RTI_PRIVATE void
OSAPI_Thread_freertos_exec(void *param)
{
    void *ignore = OSAPI_Thread_exec(param);
    IGNORE_RETVAL(ignore);
}

RTI_PRIVATE RTI_BOOL
OSAPI_Thread_map_to_native_priority(RTI_INT32 *nativePriority,
                                    RTI_INT32 priorityLevel)
{

    *nativePriority = 0;

    if (priorityLevel >= 0)
    {
        *nativePriority = priorityLevel;
        return RTI_TRUE;
    }

    switch (priorityLevel)
    {
        case OSAPI_THREAD_PRIORITY_LOW:
            *nativePriority = tskIDLE_PRIORITY;
            break;
        case OSAPI_THREAD_PRIORITY_BELOW_NORMAL:
            *nativePriority = tskIDLE_PRIORITY + 1;
            break;
        case OSAPI_THREAD_PRIORITY_NORMAL:
            *nativePriority = tskIDLE_PRIORITY + 2;
            break;
        case OSAPI_THREAD_PRIORITY_ABOVE_NORMAL:
            *nativePriority = tskIDLE_PRIORITY + 3;
            break;
        case OSAPI_THREAD_PRIORITY_HIGH:
            *nativePriority = tskIDLE_PRIORITY + 4;
            break;
        default:
            OSAPI_LOG_THREAD_PRIORITY_MAP(OSAPI_LOGKIND_ERROR,*nativePriority,
                                          priorityLevel)
            return RTI_FALSE;
    }

    if (*nativePriority > (configMAX_PRIORITIES - 1))
    {
        *nativePriority = configMAX_PRIORITIES - 1;
    }

    return RTI_TRUE;
}


struct OSAPI_Thread *
OSAPI_Thread_create(const char *name,
                   const struct OSAPI_ThreadProperty *properties,
                   OSAPI_ThreadRoutine user_routine,
                   void *user_data,
                   OSAPI_ThreadRoutine wakeup_routine)
{
    int stack = 0;
    int priority = tskIDLE_PRIORITY;
    portBASE_TYPE retcode;
    struct OSAPI_Thread *me = NULL;
    RTI_INT32 len;

    /* create the thread handle */
    me = OSAPI_Thread_new(properties);
    if (me == NULL)
    {
        return NULL;
    }

    /* assign user's values */
    me->user_routine = user_routine;
    me->wakeup_routine = wakeup_routine;
    me->thread_info.user_data = user_data;
    me->thread_info.is_preemptive = RTI_TRUE;

    /* Ignore a too long name since it has not function impact */
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

    if (properties->stack_size > OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack = properties->stack_size / sizeof(portSTACK_TYPE);
    }
    else
    {
        stack = OSAPI_PLATFORM_FREERTOS_STACK_SIZE_DEFAULT;
    }

    retcode =
        xTaskCreate(OSAPI_Thread_freertos_exec, name, stack, me, priority,
                    &me->thread_handle);
    if (pdTRUE != retcode)
    {
        goto fail;
    }

    /* spawned thread gives semaphore immediately when started */
    if (!OSAPI_Semaphore_take(me->create_sem,
                              OSAPI_SEMAPHORE_TIMEOUT_INFINITE, NULL))
    {
        goto fail;
    }

    return me;

fail:
#ifndef RTI_CERT
    OSAPI_Thread_delete(me);
#endif /* RTI_CERT */
    return NULL;
}

OSAPI_ThreadId
OSAPI_Thread_self()
{
    return xTaskGetCurrentTaskHandle();
}

#ifndef RTI_CERT
void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    vTaskDelay(ms / portTICK_RATE_MS);
}
#endif /* RTI_CERT */

#endif /* OSAPI_INCLUDE_FREERTOS */
