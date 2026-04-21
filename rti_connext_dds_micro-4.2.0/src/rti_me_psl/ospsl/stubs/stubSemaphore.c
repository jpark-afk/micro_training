/*
 * FILE: stubSempahore.c - stub semaphore functionality
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI semaphore routines
 */
#include "rti_me_psl.h"

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
    RTI_UINT32 value;
};

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *me)
{
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    UNUSED_ARG(me);

    return RTI_TRUE;
}
#endif

OSAPI_Semaphore_T*
OSAPI_Semaphore_new(void)
{
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
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)
    UNUSED_ARG(me);
    UNUSED_ARG(timeoutMs);
    UNUSED_ARG(failReason);

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *me)
{
    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)
   UNUSED_ARG(me);

    return RTI_TRUE;
}
