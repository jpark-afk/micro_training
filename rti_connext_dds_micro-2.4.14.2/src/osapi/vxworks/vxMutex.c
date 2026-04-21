/*
 * FILE: vxMutex.c - VxWorks mutex functionality
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
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief VxWorks implementation of OSAPI mutex routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_VXWORKS

#include <stdio.h>
#include <stdlib.h>
#include <vxWorks.h>
#include <semLib.h>

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "../common/Mutex.h"

struct OSAPI_Mutex
{
    struct OSAPI_MutexBase _base;
    SEM_ID mutex_id;
};
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(struct OSAPI_Mutex *mutex)
{
    STATUS status;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    OSAPI_Mutex_finalize(mutex);

    if (mutex->mutex_id != NULL)
    {
        status = semDelete(mutex->mutex_id);
        if (status != OK)
        {
            OSAPI_LOG_MUTEX_DELETE(OSAPI_LOGKIND_ERROR,status)
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free_struct(mutex);
    return RTI_TRUE;
}
#endif /* !RTI_CERT */

struct OSAPI_Mutex*
OSAPI_Mutex_new(void)
{
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_struct(&mutex, struct OSAPI_Mutex);

    if (mutex == NULL)
    {
        return NULL;
    }

    OSAPI_Mutex_initialize(mutex);

    mutex->mutex_id = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
    if (mutex->mutex_id == NULL)
    {
        return NULL;
    }

    return mutex;
}


RTI_BOOL
OSAPI_Mutex_take_os(struct OSAPI_Mutex *mutex)
{
    STATUS status;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    status = semTake(mutex->mutex_id, WAIT_FOREVER);
    if (status == OK)
    {
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR,status)

    return RTI_FALSE;
}


RTI_BOOL
OSAPI_Mutex_give_os(struct OSAPI_Mutex *mutex)
{
    STATUS status;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    status = semGive(mutex->mutex_id);
    if (status == OK)
    {
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,status)

    return RTI_FALSE;
}

#endif
