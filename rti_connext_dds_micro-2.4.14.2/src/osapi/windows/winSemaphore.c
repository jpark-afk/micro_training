/*
 * FILE: winSempahore.c - Win semaphore functionality
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
 * 12aug2012,tk Updated
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI semaphore routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_WINDOWS

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <windows.h>

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif

struct OSAPI_Semaphore
{
    HANDLE handle;              /* native OS representation */
};

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_WINDOWS

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *me)
{
    DWORD ec;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    if (!CloseHandle(me->handle))
    {
        ec = GetLastError();
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR,ec)
        return RTI_FALSE;

    }

    OSAPI_Heap_free_struct(me);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

OSAPI_Semaphore_T*
OSAPI_Semaphore_new(void)
{
    struct OSAPI_Semaphore *me;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Semaphore);

    if (me == NULL)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    me->handle = CreateSemaphore(NULL,  /* security attributes */
                                 0,       /* empty */
                                 1,       /* maximum count */
                                 NULL /* unamed */ );
    if (me->handle != NULL)
    {
        return me;
    }

    OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)

    return NULL;
}

/*
    attempt to take semaphore with the specified timeout (can be inifnite).
    if RTI_TRUE is returned, failReason will contain the error condition (ie, errno, etc).
    failReason is OS-specific (ie, not wrapped around by OSAPI-defined reasons)
*/
RTI_BOOL
OSAPI_Semaphore_take(OSAPI_Semaphore_T *me,
                    RTI_INT32 timeoutMs,
                    RTI_INT32 *failReason)
{
    DWORD status;
    RTI_BOOL result;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    status = WaitForSingleObject(me->handle, timeoutMs);

    switch (status)
    {
        /* WAIT_ABANDONED means the thread owning the mutex exited
         * This should not happen
         */
        case WAIT_ABANDONED:
            OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_WARNING,status)
        case WAIT_OBJECT_0:
        case WAIT_TIMEOUT:
            result = RTI_TRUE;
            break;
        default:
            result = RTI_FALSE;
    }

    if (failReason)
    {
        switch (status)
        {
            case WAIT_OBJECT_0:
                *failReason = OSAPI_SEMAPHORE_RESULT_OK;
                break;
            case WAIT_TIMEOUT:
                *failReason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
                break;
            default:
                *failReason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
    }

    return result;
}

RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *me)
{
    DWORD e;
    LONG pc=0;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    /* Too many posts are ok for notification NT returns ERROR_TOO_MANY_POSTS */
    if (!ReleaseSemaphore(me->handle, 1, &pc))
    {
        e = GetLastError();
        if (e != ERROR_TOO_MANY_POSTS)
        {
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR,e)
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

#endif
