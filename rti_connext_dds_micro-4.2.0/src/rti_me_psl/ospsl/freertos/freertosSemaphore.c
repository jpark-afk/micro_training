/*
 * FILE: freertosSemaphore.c - Semaphore functionality
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
 * \brief FreeRTOS implementation of OSAPI semaphore routines
 */
#include "rti_me_psl.h"

#include "FreeRTOS.h"
#include "semphr.h"


/*** SOURCE_BEGIN ***/


/* PRIVATE API */

struct OSAPI_Semaphore
{
    xSemaphoreHandle semaphore;
};

/* PUBLIC API */

#ifndef RTI_CERT
/*ci
 * \brief Delete a semaphore
 *
 * \param[in] self Pointer to the semaphore to be deleted.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Semaphore_delete(struct OSAPI_Semaphore *self)
{
    if (self != NULL)
    {
        if (self->semaphore != NULL)
        {
            vSemaphoreDelete(self->semaphore);
        }
        OSAPI_Heap_free_buffer(self);
    }
    return RTI_TRUE;
}
#endif

/*ci
 * \brief Create a new semaphore
 *
 * \param[in] None.
 *
 * \return Pointer to the newly created semaphore on success, NULL on failure.
 */
struct OSAPI_Semaphore *
OSAPI_Semaphore_new(void)
{
    struct OSAPI_Semaphore *semaphore = NULL;

    OSAPI_Heap_allocate_buffer((char**)&semaphore,
                               sizeof(struct OSAPI_Semaphore),
                               OSAPI_ALIGNMENT_DEFAULT);

    if (semaphore == NULL)
    {
        return NULL;
    }

    vSemaphoreCreateBinary(semaphore->semaphore);
    if (semaphore->semaphore == NULL)
    {
    #ifndef RTI_CERT
        OSAPI_Heap_free_buffer(semaphore);
    #endif
        return NULL;
    }

    if (pdFALSE == xSemaphoreTake(semaphore->semaphore, 0))
    {
    #ifndef RTI_CERT
        OSAPI_Semaphore_delete(semaphore);
        OSAPI_Heap_free_buffer(semaphore);
    #endif
        return NULL;
    }

    return semaphore;
}

/* ci
 *
 * \brief Take a semaphore
 *
 * \details Attempt to take semaphore with a timeout (can be infinite).
 * if RTI_TRUE is returned, fail_reason will contain the error condition
 * (ie, errno, etc). fail_reason is OS-specific (ie, not wrapped around by
 * OSAPI-defined reasons)
 *
 * \param[in] self Pointer to the semaphore to be taken.
 * \param[in] timeoutMs Timeout in milliseconds.
 * \param[out] fail_reason Pointer to store the failure reason.
 * 
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Semaphore_take(struct OSAPI_Semaphore *self, int timeoutMs,
                     int *fail_reason)
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

/*ci
 * \brief Give a semaphore
 *
 * \param[in] self Pointer to the semaphore to be given.
 *
 * \return RTI_TRUE on success, or if the semaphore has never been taken
 *         RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Semaphore_give(struct OSAPI_Semaphore *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (pdTRUE != xSemaphoreGive(self->semaphore))
    {
        return RTI_TRUE;
    }
    return RTI_TRUE;
}