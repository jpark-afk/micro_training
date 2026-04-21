/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/09 14:17:42 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef TimeManager_h
#define TimeManager_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif
#include "osapi/osapi_system.h"

#define THROUGHPUT_TEST_MAX_SLEEP_SEC 1 /* 1 Seconds */

#ifdef WIN32
  #define THROUGHPUT_TEST_DELAY_DECREMENT_NS 10000000 /* 10 Milliseconds */
#else /* For all other OS make resolution 1 ms */
  #define THROUGHPUT_TEST_DELAY_DECREMENT_NS 100000000 /* 1 Millisecond */
#endif

typedef struct TimeManager {
    struct OSAPISystem *_clock; /* A pointer to a high resolution clock */
    double _clock_overhead;
    
    OSAPI_NtpTime _start_time;
    OSAPI_NtpTime _finish_time;
    OSAPI_NtpTime _test_duration;
} TimeManager;
      
void TimeManager_delete(TimeManager *self);
void TimeManager_create(TimeManager *self);
DDS_Boolean TimeManager_initialize(TimeManager *self, int test_duration_sec);
DDS_Boolean TimeManager_get_start_time(TimeManager *self);
DDS_Boolean TimeManager_get_stop_time(TimeManager *self); 
DDS_Boolean TimeManager_calculate_clock_overhead(TimeManager *self);
double TimeManager_get_delta_time(TimeManager *self);
DDS_Boolean TimeManager_is_test_complete(TimeManager *self);
void TimeManager_sleep(TimeManager *self, struct DDS_Duration_t *time_to_sleep);

#endif

