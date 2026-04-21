/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif
#ifndef TimeManager_h
#include "TimeManager.h"
#endif

void
TimeManager_delete(TimeManager * self)
{
    return;
}

void
TimeManager_create(TimeManager * self)
{
    self->_clock = NULL;
}

DDS_Boolean
TimeManager_initialize(TimeManager * self, int test_duration_sec)
{
    self->_test_duration.sec = test_duration_sec;
    self->_test_duration.frac = 0;

    return DDS_BOOLEAN_TRUE;
}


DDS_Boolean
TimeManager_get_start_time(TimeManager * self)
{
    OSAPI_NtpTime_setZero(&self->_start_time);
    OSAPI_NtpTime_setZero(&self->_finish_time);
    if (!OSAPI_System_get_time(&self->_start_time))
    {
        return DDS_BOOLEAN_FALSE;
    }
    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
TimeManager_get_stop_time(TimeManager * self)
{
    if (!OSAPI_System_get_time(&self->_finish_time))
    {
        return DDS_BOOLEAN_FALSE;
    }
    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
TimeManager_calculate_clock_overhead(TimeManager * self)
{
    int i = 0;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_NtpTime begin_time = OSAPI_NTP_TIME_ZERO,
        clock_traversal_time = OSAPI_NTP_TIME_ZERO;

    if (!OSAPI_System_get_time(&begin_time))
    {
        return DDS_BOOLEAN_FALSE;
    }

#define TIME_MANAGER_CALCULATION_LOOP_COUNT_MAX 100
    for (i = 0; i < TIME_MANAGER_CALCULATION_LOOP_COUNT_MAX; ++i)
    {
        if (!OSAPI_System_get_time(&clock_traversal_time))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    OSAPI_NtpTime_decrement(&clock_traversal_time, &begin_time);

    self->_clock_overhead = OSAPI_NtpTime_toDouble(&clock_traversal_time) /
        (double)TIME_MANAGER_CALCULATION_LOOP_COUNT_MAX;
    ok = RTI_TRUE;

    finally:

    if (ok == RTI_TRUE)
    {
        return DDS_BOOLEAN_TRUE;
    }
    else
    {
        return DDS_BOOLEAN_FALSE;
    }
}


double
TimeManager_get_delta_time(TimeManager * self)
{
    OSAPI_NtpTime delta_time;
    double return_value;

    OSAPI_NtpTime_subtract(&delta_time, &self->_finish_time, &self->_start_time);
    return_value = OSAPI_NtpTime_toDouble(&delta_time);
    return_value -= self->_clock_overhead;
    return return_value;
}

DDS_Boolean
TimeManager_is_test_complete(TimeManager * self)
{
    OSAPI_NtpTime delta_time, time_now;
    double delta_time_double, duration_double;

    if (!OSAPI_System_get_time(&time_now))
    {
        return DDS_BOOLEAN_FALSE;
    }
    OSAPI_NtpTime_subtract(&delta_time, &time_now, &self->_start_time);

    delta_time_double = OSAPI_NtpTime_toDouble(&delta_time);
    delta_time_double -= self->_clock_overhead;

    duration_double = OSAPI_NtpTime_toDouble(&self->_test_duration);
    if (delta_time_double >= duration_double)
    {
        return DDS_BOOLEAN_TRUE;
    }
    else
    {
        return DDS_BOOLEAN_FALSE;
    }
}

void
TimeManager_sleep(TimeManager * self, struct DDS_Duration_t *time_to_sleep)
{
    OSAPI_Thread_sleep(time_to_sleep->sec * 1000 + time_to_sleep->nanosec / 1000000);
}
