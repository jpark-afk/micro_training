/*
 * FILE: freertosMutex.c - Mutex functionality
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
 * 21oct2014,eh MICRO-953: update FreeRTOS port
 * 06mar2012,tk Written
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI mutex routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"

#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_mutex.h"

#include "../common/Mutex.h"

struct OSAPI_Mutex
{
    struct OSAPI_MutexBase _base;
    xSemaphoreHandle mutex;
};
#endif

/*** SOURCE_BEGIN ***/
#if OSAPI_INCLUDE_FREERTOS

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(struct OSAPI_Mutex *mutex)
{
    if (mutex != NULL)
    {
        if (mutex->mutex != NULL)
        {
            vSemaphoreDelete(mutex->mutex);
        }
        OSAPI_Heap_free_struct(mutex);
    }

    return RTI_TRUE;
}
#endif

struct OSAPI_Mutex *
OSAPI_Mutex_new(void)
{
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_struct(&mutex, struct OSAPI_Mutex);
    if (mutex == NULL)
    {
        return NULL;
    }

    OSAPI_Mutex_initialize(mutex);

    mutex->mutex = xSemaphoreCreateRecursiveMutex();
    if (mutex->mutex == NULL)
    {
        OSAPI_Heap_free_struct(mutex);
        return NULL;
    }

    return mutex;
}

RTI_BOOL
OSAPI_Mutex_take_os(struct OSAPI_Mutex * mutex)
{
    if (mutex == NULL)
    {
        return RTI_FALSE;
    }
    if (pdTRUE != xSemaphoreTakeRecursive(mutex->mutex, portMAX_DELAY))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}


RTI_BOOL
OSAPI_Mutex_give_os(struct OSAPI_Mutex * mutex)
{
    if (mutex == NULL)
    {
        return RTI_FALSE;
    }
    if (pdTRUE != xSemaphoreGiveRecursive(mutex->mutex))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}
#endif
