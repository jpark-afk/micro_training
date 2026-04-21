/*
 * FILE: threadxMutex.c - ThreadX mutex functionality
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
 * 13dec2016,francisco  File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI mutex routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

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
    TX_MUTEX hmutex;
};

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(struct OSAPI_Mutex *mutex)
{
    UINT rc;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    rc = tx_mutex_delete(&mutex->hmutex);
    if (rc != TX_SUCCESS)
    {
        OSAPI_LOG_MUTEX_DELETE(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(mutex);

    return RTI_TRUE;
}
#endif

struct OSAPI_Mutex*
OSAPI_Mutex_new(void)
{
    UINT                rc;
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_struct(&mutex, struct OSAPI_Mutex);

    if (mutex == NULL)
    {
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    OSAPI_Mutex_initialize(mutex);

    rc = tx_mutex_create(&mutex->hmutex, "micro-mutex", TX_INHERIT);
    if (rc != TX_SUCCESS)
    {
        OSAPI_Heap_free_struct(mutex);
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    return mutex;
}

RTI_BOOL
OSAPI_Mutex_take_os(struct OSAPI_Mutex *mutex)
{
    UINT rc;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    rc =  tx_mutex_get(&mutex->hmutex, TX_WAIT_FOREVER);
    if (rc == TX_SUCCESS)
    {
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR, rc)

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Mutex_give_os(struct OSAPI_Mutex *mutex)
{
    UINT rc;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    rc = tx_mutex_put(&mutex->hmutex);
    if (rc == TX_SUCCESS)
    {
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR, rc)

    return RTI_FALSE;
}

#endif
