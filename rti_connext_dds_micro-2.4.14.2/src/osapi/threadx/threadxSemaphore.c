/*
 * FILE: threadxSempahore.c - ThreadX semaphore functionality
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
 * \brief ThreadX implementation of OSAPI semaphore routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

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
    TX_SEMAPHORE handle;              /* native OS representation */
};

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Semaphore_delete(OSAPI_Semaphore_T *me)
{
    UINT rc;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    rc = tx_semaphore_delete(&me->handle);
    if (rc != TX_SUCCESS)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR, rc)
        return RTI_FALSE;

    }

    OSAPI_Heap_free_struct(me);

    return RTI_TRUE;
}
#endif

OSAPI_Semaphore_T*
OSAPI_Semaphore_new(void)
{
    UINT                    rc;
    struct OSAPI_Semaphore *me;

    OSAPI_Heap_allocate_struct(&me, struct OSAPI_Semaphore);

    if (me == NULL)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    rc = tx_semaphore_create(&me->handle,
                             "micro-semaphore",
                             0  /* initial count */);
    if (rc != TX_SUCCESS)
    {
        OSAPI_Heap_free_struct(me);

        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)

        return NULL;
    }

    return me;
}

RTI_BOOL
OSAPI_Semaphore_take(OSAPI_Semaphore_T *me,
                     RTI_INT32 timeout,
                     RTI_INT32 *fail_reason)
{
    UINT     status;
    RTI_BOOL result;
    ULONG wait_option;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    if (timeout == OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
    {
        wait_option = TX_WAIT_FOREVER;
    }
    else
    {
        wait_option = timeout / OSAPI_MS_TIMER_TICK;
    }
    status = tx_semaphore_get(&me->handle, wait_option);

    switch (status)
    {
        /* TX_DELETED means the semaphore was deleted while thread was 
         * suspended. This should not happen
         */
        case TX_DELETED:
            OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_WARNING,status)
            result = RTI_TRUE;
            break;
        case TX_NO_INSTANCE:
        case TX_SUCCESS:
            result = RTI_TRUE;
            break;
        default:
            result = RTI_FALSE;
            break;
    }

    if (fail_reason)
    {
        switch (status)
        {
            case TX_SUCCESS:
                *fail_reason = OSAPI_SEMAPHORE_RESULT_OK;
                break;
            case TX_NO_INSTANCE:
                *fail_reason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
                break;
            default:
                *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
                break;
        }
    }

    return result;
}

RTI_BOOL
OSAPI_Semaphore_give(OSAPI_Semaphore_T *me)
{
    UINT rc;

    OSAPI_PRECONDITION(me == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    /* maximum count is 1 */
    rc = tx_semaphore_ceiling_put(&me->handle, 1);
    switch (rc)
    {
        case TX_CEILING_EXCEEDED:
        case TX_SUCCESS:
            break;

        default:
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR, rc)
            return RTI_FALSE;
    }

    return RTI_TRUE;
}

#endif
