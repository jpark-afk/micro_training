/*
 * FILE: freertosThread.c - Thread functionality
 *
 * (c) Copyright, Real-Time Innovations 2024-2025
 *  
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI thread routines
 */
#include "rti_me_psl.h"
#include "freertosThread.h"
#define MILLI_TO_NANO 1000000

/*** SOURCE_BEGIN ***/

struct OSAPI_FreeRTOSNativeThread
{
    char name[64];

    TaskHandle_t native_thread_handle;

    OSAPI_ThreadRoutine thread_routine;

    struct OSAPI_ThreadInfo thread_info;

    OSAPI_ThreadRoutine thread_wakeup;

    RTI_BOOL running;
};

/*ci
 * \brief Map OSAPI thread priority to native FreeRTOS priority
 *
 * \param[out] nativePriority Pointer to store the native priority.
 * \param[in] priorityLevel OSAPI thread priority level.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
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
        case OSAPI_THREAD_PRIORITY_INHERIT:
            *nativePriority = tskIDLE_PRIORITY + 1;
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

/*ci
 * \brief Start a native FreeRTOS thread
 *
 * \param[in] param Pointer to the native thread structure.
 */
RTI_PRIVATE void
OSAPI_Thread_start_native(void *param)
{
    struct OSAPI_FreeRTOSNativeThread *native_thread = 
        (struct OSAPI_FreeRTOSNativeThread *)param;

    native_thread->running = RTI_TRUE;
    native_thread->thread_routine(&native_thread->thread_info);

    native_thread->running = RTI_FALSE;

    while(1)
    {
        OSAPI_Thread_nanosleep(100*MILLI_TO_NANO);
    }
}


/*ci
 * \brief Destroy a native FreeRTOS thread
 *
 * \param[in] thread Pointer to the native FreeRTOS thread.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Thread_destroy_freertos_native(struct OSAPI_FreeRTOSNativeThread *thread)
{
    thread->thread_info.stop_thread = RTI_TRUE;
    
    if (thread->thread_wakeup != NULL)
    {
        thread->thread_wakeup(&thread->thread_info);
    }

    while (thread->running);

    vTaskDelete(thread->native_thread_handle);

#ifndef RTI_CERT
    OSAPI_Heap_free_buffer(thread);
#endif

    return RTI_TRUE;
}

/*ci
 * \brief Create a native FreeRTOS thread
 *
 * \param[in] name Name of the thread.
 * \param[in] property Pointer to the thread properties.
 * \param[in] thread_entry Pointer to the thread entry function.
 * \param[in] thread_data Pointer to the thread data.
 * \param[in] thread_wakeup Pointer to the thread wakeup function.
 *
 * \return Pointer to the created native FreeRTOS thread on success, NULL on failure.
 */
struct OSAPI_FreeRTOSNativeThread*
OSAPI_Thread_create_freertos_native(const char *name,
                                    const struct OSAPI_ThreadProperty *property,
                                    OSAPI_ThreadRoutine thread_entry,
                                    void *thread_data,
                                    OSAPI_ThreadRoutine thread_wakeup)
{
    struct OSAPI_FreeRTOSNativeThread *native_thread;
    int priority = tskIDLE_PRIORITY;
    int stack_size = configMINIMAL_STACK_SIZE;

    OSAPI_PRECONDITION((property == NULL) || (name == NULL) ||
                       (thread_entry == NULL), return NULL,
                       OSAPI_Log_entry_add_pointer("property",property,
                                                   RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("name",name,
                                                   RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("thread_entry",
                                                   thread_entry,RTI_TRUE);)

    OSAPI_Heap_allocate_buffer((char**)&native_thread,
                               sizeof(struct OSAPI_FreeRTOSNativeThread), 
                               OSAPI_ALIGNMENT_DEFAULT);
    if (native_thread == NULL)
    {
        return NULL;
    }

    native_thread->thread_routine = thread_entry;
    native_thread->thread_wakeup = thread_wakeup;

    size_t len = OSAPI_String_length(name);
    if (len >= sizeof(native_thread->name))
    {
        len = sizeof(native_thread->name) - 1;
    }
    OSAPI_Memory_copy(native_thread->name, name, len);
    native_thread->name[len] = '\0';

    native_thread->thread_info.stop_thread = RTI_FALSE;
    native_thread->thread_info.user_data = thread_data;
    native_thread->thread_info.is_preemptive = RTI_TRUE;

    if (property->stack_size > OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE)
    {
        stack_size = property->stack_size;
    }

    if (!OSAPI_Thread_map_to_native_priority(&priority, property->priority))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(native_thread);
#endif
        return NULL;
    }

    BaseType_t retcode = xTaskCreate((TaskFunction_t)OSAPI_Thread_start_native,
                                     native_thread->name, stack_size,
                                     native_thread,
                                     priority,
                                     &native_thread->native_thread_handle);
    if (retcode != pdPASS)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(native_thread);
#endif
        return NULL;
    }

    return native_thread;
}

/*ci
 * \brief Create a native thread
 *
 * \param[in] name Name of the thread.
 * \param[in] property Pointer to the thread properties.
 * \param[in] thread_entry Pointer to the thread entry function.
 * \param[in] thread_data Pointer to the thread data.
 * \param[in] thread_wakeup Pointer to the thread wakeup function.
 *
 * \return Handle to the created native thread on success, NULL on failure.
 */
OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup)
{
    struct OSAPI_FreeRTOSNativeThread *native_thread;

    native_thread = OSAPI_Thread_create_freertos_native(name,property,
                                                        thread_entry,
                                                        thread_data,
                                                        thread_wakeup);

    return (OSAPI_ThreadHandle)native_thread;
}

/*ci
 * \brief Delete a native thread
 *
 * \param[in] handle Handle to the native thread.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle)
{
    struct OSAPI_FreeRTOSNativeThread *native_thread =
        (struct OSAPI_FreeRTOSNativeThread*)handle;

    return OSAPI_Thread_destroy_freertos_native(native_thread);
}

/*ci
 * \brief Get the current thread ID
 *
 * \param[in] None.
 *
 * \return The current thread ID.
 */
OSAPI_ThreadId
OSAPI_Thread_self()
{
    OSAPI_ThreadId result;
    result.handle.data = (void*) xTaskGetCurrentTaskHandle();
    return result;
}

#if OSAPI_ENABLE_LOG
/*ci
 * \brief Get the last error code
 *
 * \param[in] None.
 *
 * \return The last error code.
 */
RTI_INT32
OSAPI_Log_get_last_error_code(void)
{
    return errno;
}

/*ci
 * \brief Set the last error code
 *
 * \param[in] err The error code to set.
 */
void
OSAPI_Log_set_last_error_code(RTI_INT32 err)
{
    errno = err;
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Sleep for a specified number of milliseconds
 *
 * \param[in] ms Number of milliseconds to sleep.
 */
void
OSAPI_Thread_sleep(RTI_UINT32 ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
#endif

/*ci
 * \brief Sleep for a specified number of nanoseconds
 *
 * \param[in] ns Number of nanoseconds to sleep.
 */
void
OSAPI_Thread_nanosleep(RTI_UINT32 ns)
{
    vTaskDelay(pdMS_TO_TICKS(ns / 1000000));
}