/*
 * FILE: freertosMutex.c - Mutex functionality
 *
 * (c) Copyright, Real-Time Innovations 2024-2024
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
 * \brief FreeRTOS implementation of OSAPI mutex routines
 */

#include "rti_me_psl.h"

#include "FreeRTOS.h"
#include "semphr.h"

struct OSAPI_Mutex
{
    xSemaphoreHandle mutex;
    RTI_UINT32 depth;
};

/*** SOURCE_BEGIN ***/


#ifndef RTI_CERT
/*ci
 * \brief Delete a mutex
 *
 * \param[in] mutex Pointer to the mutex to be deleted.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Mutex_delete(OSAPI_Mutex_T *mutex)
{
    OSAPI_PRECONDITION_ALWAYS(mutex == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)
        
    if (mutex->mutex != NULL)
    {
        vSemaphoreDelete(mutex->mutex);
    }
    OSAPI_Heap_free_buffer(mutex);


    return RTI_TRUE;
}
#endif

/*ci
 * \brief Create a new mutex
 *
 * \param[in] None.
 *
 * \return Pointer to the newly created mutex on success, NULL on failure.
 */
OSAPI_Mutex_T*
OSAPI_Mutex_new(void)
{
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_buffer((char**)&mutex, 
                                   sizeof(struct OSAPI_Mutex), 
                                   OSAPI_ALIGNMENT_DEFAULT);
    if (mutex == NULL)
    {
        return NULL;
    }

    OSAPI_Memory_zero(mutex, sizeof(struct OSAPI_Mutex));

    mutex->mutex = xSemaphoreCreateRecursiveMutex();
    if (mutex->mutex == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(mutex);
#endif
        return NULL;
    }

    return mutex;
}

/*ci
 * \brief Take a mutex
 *
 * \param[in] mutex Pointer to the mutex to be taken.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Mutex_take(struct OSAPI_Mutex * mutex)
{
    OSAPI_PRECONDITION_ALWAYS(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    if (pdTRUE != xSemaphoreTakeRecursive(mutex->mutex, portMAX_DELAY))
    {
        return RTI_FALSE;
    }
    ++mutex->depth;
    return RTI_TRUE;
}

/*ci
 * \brief Give a mutex
 *
 * \param[in] mutex Pointer to the mutex to be given.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
OSAPI_Mutex_give(struct OSAPI_Mutex * mutex)
{
    OSAPI_PRECONDITION_ALWAYS(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)
                        
    if (pdTRUE != xSemaphoreGiveRecursive(mutex->mutex))
    {
        return RTI_FALSE;
    }
    --mutex->depth;
    return RTI_TRUE;
}

/*ci
 * \brief Get the depth of a mutex
 *
 * \param[in] self Pointer to the mutex.
 *
 * \return Depth of the mutex.
 */
RTI_UINT32
OSAPI_Mutex_get_depth(OSAPI_Mutex_T *self)
{
    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    return self->depth;
}