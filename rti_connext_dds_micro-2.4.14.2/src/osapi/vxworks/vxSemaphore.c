/*
 * FILE: vxSempahore.c - VxWorks semaphore functionality
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
 * \brief VxWorks implementation of OSAPI semaphore routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_VXWORKS

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sysLib.h>
#include <semLib.h>
#include <tickLib.h>
#include <errno.h>

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
    SEM_ID sem_id;
};
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(struct OSAPI_Semaphore *self)
{
     /* VxWorks Cert subset API does not include semDelete */
    STATUS status;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    status = semDelete(self->sem_id);
    if (status != OK)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR,status)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(self);
    return RTI_TRUE;
}
#endif /* !RTI_CERT */

struct OSAPI_Semaphore*
OSAPI_Semaphore_new(void)
{
    struct OSAPI_Semaphore *me;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Semaphore);

    if (me == NULL)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    me->sem_id = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
    if (me->sem_id)
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
OSAPI_Semaphore_take(struct OSAPI_Semaphore *self,
                    RTI_INT32 timeoutMs,
                    RTI_INT32 *failReason)
{
    STATUS result;
    RTI_UINT32 ticks, remainder;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (timeoutMs == OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
    {
        ticks = WAIT_FOREVER;
    }
    else
    {
        /* Try to make this safe by keeping remainder. Then multiply remainder w/
         * ticksPerSec before dividing by 1000 */
#define MILLI_TO_1 1000
        ticks = (timeoutMs / MILLI_TO_1) * sysClkRateGet();
        remainder = timeoutMs % MILLI_TO_1;
        ticks += (remainder * sysClkRateGet()) / MILLI_TO_1;
#undef MILLI_TO_1
    }

    result = semTake(self->sem_id, ticks);

    if (result == OK)
    {
        if (failReason)
        {
            *failReason = OSAPI_SEMAPHORE_RESULT_OK;
        }
        return RTI_TRUE;
    }

    if ((errno == S_objLib_OBJ_TIMEOUT) || 
        (errno == S_objLib_OBJ_UNAVAILABLE))
    {
        if (failReason)
        {
            *failReason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
        }
        return RTI_TRUE;
    }

    OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,result)
    if (failReason)
    {
        *failReason = OSAPI_SEMAPHORE_RESULT_ERROR;
    }

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Semaphore_give(struct OSAPI_Semaphore *self)
{
    STATUS status;

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    status = semGive(self->sem_id);
    if (status != OK)
    {
        OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR,status)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#endif
