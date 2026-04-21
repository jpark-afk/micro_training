/*
 * FILE: freertosSemaphore.c - Semaphore functionality
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
 * \brief FreeRTOS implementation of OSAPI semaphore routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS

#include "FreeRTOS.h"
#include "semphr.h"

#include "osapi/osapi_heap.h"
#include "osapi/osapi_mutex.h"
#include "osapi/osapi_semaphore.h"

struct OSAPI_Semaphore
{
    xSemaphoreHandle semaphore;
};
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_FREERTOS

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(struct OSAPI_Semaphore *self)
{
    if (self != NULL)
    {
        if (self->semaphore != NULL)
        {
            vSemaphoreDelete(self->semaphore);
        }
        OSAPI_Heap_free_struct(self);
    }
    return RTI_TRUE;
}
#endif

struct OSAPI_Semaphore *
OSAPI_Semaphore_new(void)
{
    struct OSAPI_Semaphore *semaphore = NULL;

    OSAPI_Heap_allocate_struct(&semaphore, struct OSAPI_Semaphore);

    if (semaphore == NULL)
    {
        return NULL;
    }

    vSemaphoreCreateBinary(semaphore->semaphore);
    if (semaphore->semaphore == NULL)
    {
        OSAPI_Heap_free_struct(semaphore);
        return NULL;
    }

    if (pdFALSE == xSemaphoreTake(semaphore->semaphore, 0))
    {
#ifndef RTI_CERT
        OSAPI_Semaphore_delete(semaphore);
#endif /* RTI_CERT */
        return NULL;
    }

    return semaphore;
}

RTI_BOOL
OSAPI_Semaphore_take(struct OSAPI_Semaphore *self, int timeoutMs, int *fail_reason)
{
    if (self == NULL)
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        return RTI_FALSE;
    }

    if (timeoutMs != OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
    {
        timeoutMs /= portTICK_RATE_MS;
        ++timeoutMs;
    }
    else
    {
        timeoutMs = portMAX_DELAY;
    }

    if (pdTRUE == xSemaphoreTake(self->semaphore, timeoutMs))
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_OK;
        }
    }
    else
    {
        /* If reached here, semaphore timed-out */
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_Semaphore_give(struct OSAPI_Semaphore *self)
{
    if (self == NULL)
    {
        return RTI_FALSE;
    }
    if (pdTRUE != xSemaphoreGive(self->semaphore))
    {
        return RTI_TRUE;
    }
    return RTI_TRUE;
}

#endif
