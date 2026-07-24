/*
 * FILE: Semaphore.c - Platform independent semaphore APIs
 *
 * Copyright (c) 2025-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_features.h"

#include "osapi/osapi_cc.h"

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif

RTI_BOOL
OSAPI_Semaphore_take_sec_nanosec(OSAPI_Semaphore_T *self,
                                 RTI_INT32 sec,
                                 RTI_UINT32 nanosec,
                                 RTI_INT32 *reason)
{
    RTI_INT32 sec_left = sec;
    RTI_INT32 ms_left;
    RTI_INT32 local_reason = OSAPI_SEMAPHORE_RESULT_OK;

    if ((sec < OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC)
        || ((nanosec >= OSAPI_TIME_NSEC_PER_SEC)
            && (nanosec != OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC)))
    {
        if (reason != NULL)
        {
            *reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        return RTI_FALSE;
    }

    if (((sec == OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC)
          && (nanosec != OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC))
        || ((sec != OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC)
             && (nanosec == OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC)))
    {
        if (reason != NULL)
        {
            *reason = OSAPI_SEMAPHORE_RESULT_ERROR;
        }
        return RTI_FALSE;
    }

    if (sec == OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC)
    {
        return OSAPI_Semaphore_take(self,
                                    OSAPI_SEMAPHORE_TIMEOUT_INFINITE,
                                    reason);
    }

    /* nanosec must be < OSAPI_TIME_NSEC_PER_SEC, thus there can be no sign
     * conversion.
     */
    ms_left = (RTI_INT32)nanosec / OSAPI_TIME_NSEC_PER_MSEC;
    if (nanosec % OSAPI_TIME_NSEC_PER_MSEC)
    {
        ms_left++;
    }

    while ((sec_left > 0) || (ms_left > 0))
    {
        RTI_INT32 max_wait_in_ms;
        RTI_INT32 leftover_ms;
        RTI_BOOL brc;

        if (sec_left >= (INT_MAX / OSAPI_TIME_MSEC_PER_SEC))
        {
            max_wait_in_ms = (INT_MAX / OSAPI_TIME_MSEC_PER_SEC) * OSAPI_TIME_MSEC_PER_SEC;
        }
        else
        {
            max_wait_in_ms = sec_left * OSAPI_TIME_MSEC_PER_SEC;
        }

        leftover_ms = INT_MAX - max_wait_in_ms;

        if (ms_left >= leftover_ms)
        {
            max_wait_in_ms += leftover_ms;
            ms_left -= leftover_ms;
        }
        else
        {
            max_wait_in_ms += ms_left;
            ms_left = 0;
        }

        brc = OSAPI_Semaphore_take(self,max_wait_in_ms,&local_reason);
        if (reason != NULL)
        {
            *reason = local_reason;
        }

        if (!brc)
        {
            return brc;
        }

        if (local_reason == OSAPI_SEMAPHORE_RESULT_OK)
        {
            return RTI_TRUE;
        }
        sec_left -= max_wait_in_ms / OSAPI_TIME_MSEC_PER_SEC;
    }

    /* last resort, timeout is 0  */
    return OSAPI_Semaphore_take(self,0,reason);
 }
