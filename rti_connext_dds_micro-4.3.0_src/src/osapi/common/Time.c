/*
 * FILE: Time.c - Implementation of Time API
 *
 * Copyright 2008-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 29may2015,tk MICRO-1329/PR#15063 Moved implementation comments from
 *                                  osapi_time.h here
 * 25feb2015,tk MICRO-1084/PR#14056 Improved conversion between NTP and nanosec
 *                                  The implementation is taken from the Core
 *                                  libraries.
 * 24feb2015,tk Refactored from osapi_time.h
 */
/*ce
 * \file
 * \brief Platform independent time functionality
 * \ingroup OSAPIModule
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_time.h"

/*** SOURCE_BEGIN ***/

/*  \ingroup OSAPITimeClass
 *
 */

RTI_BOOL
OSAPI_SystemTime_is_infinite(const OSAPI_SystemTime *time)
{
    return ((time->sec == OSAPI_SYSTEM_TIME_SEC_MAX) ? RTI_TRUE : RTI_FALSE);
}

RTI_BOOL
OSAPI_SystemTime_is_invalid(const OSAPI_SystemTime *time)
{
    return ((time->sec == OSAPI_SYSTEM_TIME_SEC_INVALID) &&
            ((time->nanosec == OSAPI_SYSTEM_TIME_NANO_INVALID)) ?
                                                        RTI_TRUE : RTI_FALSE);
}

void
OSAPI_SystemTime_subtract(struct OSAPI_SystemTime *const answer,
                          const struct OSAPI_SystemTime *const t1,
                          const struct OSAPI_SystemTime *const t2)
{
    if (OSAPI_SystemTime_is_infinite(t1))
    {
        *answer = *t1;
    }
    else if (OSAPI_SystemTime_is_infinite(t2))
    {
        answer->sec = 0;
        answer->nanosec = 0;
    }
    else
    {
        answer->sec = t1->sec - t2->sec;

        if (t1->nanosec < t2->nanosec)
        {
            answer->sec--;
            answer->nanosec = t1->nanosec - t2->nanosec + OSAPI_TIME_NSEC_PER_SEC;
        }
        else
        {
            answer->nanosec = t1->nanosec - t2->nanosec;
        }
    }
}

RTI_INT32
OSAPI_SystemTime_compare(const struct OSAPI_SystemTime *time1,
                         const struct OSAPI_SystemTime *time2)
{
    return ((((time1)->sec) > ((time2)->sec)) ? 1 :
        ((((time1)->sec) < ((time2)->sec)) ? -1 :
        ((((time1)->nanosec) > ((time2)->nanosec)) ? 1 :
        ((((time1)->nanosec) < ((time2)->nanosec)) ? -1 : 0))));
}

void
OSAPI_SystemTime_to_ntp(RTI_UINT32 *const out_sec,
                               RTI_UINT32 *const out_frac,
                               const OSAPI_SystemTime *const time)
{
    *out_sec  = (RTI_UINT32)time->sec;
    *out_frac = (RTI_UINT32)(((RTI_UINT64)(time->nanosec) * 2305843009U) >> 29);
}

void
OSAPI_Time_from_ntp(RTI_UINT32 *const to_sec,
                    RTI_UINT32 *const to_nsec,
                    RTI_UINT32 from_sec,
                    RTI_UINT32 from_frac)
{
    *to_sec = from_sec;

    *to_nsec = (RTI_UINT32)((((RTI_UINT64)from_frac << 29)/2305843009U) + 1);

    if ((from_frac == 0) || (from_frac == 2305843009U))
    {
        (*to_nsec)--;
    }

     if ((*to_nsec >= 1000000000) && (*to_sec != 0x7FFFFFFF))
     {
        (*to_nsec) -= 1000000000;
        (*to_sec)++;
     }
}
