/*
 * FILE: winMutex.c - Win mutex functionality
 *
 * Copyright 2012-2024 Real-Time Innovations, Inc. All rights reserved.
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
 * \brief Win implementation of OSAPI mutex routines
 */
#include "rti_me_psl.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <malloc.h>

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

struct OSAPI_Mutex
{
    HANDLE hmutex;
    RTI_UINT32 depth;
};

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(struct OSAPI_Mutex *mutex)
{
    DWORD ec;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    if (!CloseHandle(mutex->hmutex))
    {
        ec = GetLastError();
        OSAPI_LOG_MUTEX_DELETE(OSAPI_LOGKIND_ERROR,ec)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_buffer(mutex);

    return RTI_TRUE;
}
#endif

struct OSAPI_Mutex *
OSAPI_Mutex_new(void)
{
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_buffer((char**)&mutex, 
                               sizeof(struct OSAPI_Mutex),
                               OSAPI_ALIGNMENT_DEFAULT);

    if (mutex == NULL)
    {
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    mutex->hmutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex->hmutex == NULL)
    {
        return NULL;
    }

    return mutex;
}

RTI_BOOL
OSAPI_Mutex_take(struct OSAPI_Mutex *mutex)
{
    DWORD rc;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    rc = WaitForSingleObject(mutex->hmutex, INFINITE);
    if (rc == WAIT_OBJECT_0)
    {
        ++mutex->depth;
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR,rc)

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Mutex_give(struct OSAPI_Mutex *mutex)
{
    DWORD ec;

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    if (ReleaseMutex(mutex->hmutex))
    {
        --mutex->depth;
        return RTI_TRUE;
    }

    ec = GetLastError();

    OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,ec)

    return RTI_FALSE;
}

RTI_UINT32
OSAPI_Mutex_get_depth(OSAPI_Mutex_T *mutex)
{
    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    return mutex->depth;
}
